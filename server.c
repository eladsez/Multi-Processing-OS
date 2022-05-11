#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "stack.h"
#include "memAlloc.h"

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold

stack *st;

void quitServer(int signal){ // when ctrl+c is pressed
    freeStack(st);
    printf("\nstack freed! server quit.\n");
    free_all();
    exit(0);
}

void client_handle(int cli_fd) {
    char buff[1024];
    while (strncmp(buff, "EXIT", 4)) {

// In case of using telnet instead of the client.cpp uncomment it
//        if (send(*cli_fd, "stackCommand~$ ", 15, 0) == -1){// sending stack command prompt to client
//            perror("socket send");
//            break;
//        }

        int bytes = recv(cli_fd, buff, sizeof(buff), 0);

        if (bytes < 0) {
            printf("%d ERROR client recv", errno);
            break;
        }

        else if (bytes == 0) {
            printf("seems like one of the clients has disconnect\n");
            break;
        }

        if (!strncmp(buff, "TOP", 3)){ // sending stack top to the client
            if (top(st) != NULL) {
                if (send(cli_fd, top(st), strlen(top(st)), 0) == -1) {
                    perror("socket send at top");
                    break;
                }
                printf("client top: %s", top(st));
            }
            else{
                if (send(cli_fd, "ERROR: The stack is empty\n", 26, 0) == -1) {
                    perror("socket send at top");
                    break;
                }
            }
        }

        else if (!strncmp(buff, "PUSH", 4)){
            char * str = (char*)my_malloc(strlen(buff + 5));
            if (str == NULL){
                perror("str malloc in PUSH");
                continue;
            }
            strcpy(str, buff + 5);
            push(st, str);
            printf("client push: %s", str);
        }

        else if (!strncmp(buff, "POP", 3)){
            if ((*st).size > 0) {
                pop(st);
                printf("client popped\n");
            }
        }

    }
    close(cli_fd);
    printf("client disconnected\n");
}

void sigchld_handler(int s) {
    // waitpid might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get stockade, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main(void) {


    init_mem(&st);
    (*st).size = 0; // init the size of the stack
    signal(SIGINT, quitServer);

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        int pid = fork();
        if (pid == -1){
            perror("fork failed");
            break;
        }
        if (pid == 0){
            client_handle(new_fd);
        }
    }
    free_all();
    return 0;
}