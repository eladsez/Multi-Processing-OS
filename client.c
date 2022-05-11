#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 1024 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, bytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    size_t line_len = 0;
    char *stack_command;

    while (1) {
//        printf("stackCommand~$ ");

        getline(&stack_command, &line_len, stdin);
        if (strncmp(stack_command, "PUSH", 4) && strncmp(stack_command, "POP", 3) && strncmp(stack_command, "TOP", 3)
            && strncmp(stack_command, "EXIT", 4)){
            printf("ERROR: Invalid stack command.\n");
            continue;
        }
        if (send(sockfd, stack_command, line_len + 1, 0) == -1) {
            perror("socket send at send command");
            break;
        }
        if (!strncmp(stack_command, "EXIT", 4)){
            break;
        }
        if (!strncmp(stack_command, "TOP", 3)){
            if ((bytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) <= 0) {
                perror("recv");
                break;
            }
            buf[bytes] = '\0';
            if (strncmp(buf, "ERROR", 5))
                printf("OUTPUT: %s", buf);
            else
                printf("%s", buf);
        }
    }
    close(sockfd);
    return 0;
}