The stack implementation is in stack.cpp file.
The malloc, calloc and free implementation is in memAlloc.cpp file.
Our tests are in the test.cpp file.
This code is process-safe, by using flock and fcntl.
Also, there is a makefile with make all and make clean targets as requested.
    
                          *-HOW TO RUN-*
First open a terminal in the Multi-Processing-OS directory and type <make all>.
Now after everything compiled you can run the server and the client.
To run the server type <./server>.
To run the client type <./client 127.0.0.1> (you can change 127.0.0.1 with the ip of the machine the server was bind to)
We added test.c file to test our code and performance
to run it first compiles it with <make test> rule (not needed if you already used <make all>), then run it with <./test>.


                           *-EXTRA-*
When you run the client (after running the server of course) the next commands are possibles:
1. <PUSH "str"> - Will push the str into the stack
2. <TOP> - Will print the top string stored in the stack
3. <POP> - Will delete the top string stored in the stack
4. <EXIT> - Will Exit from the client connection (The server will still keep alive)
There is also some outputs when error occurred, like invalid stack command etc.


The malloc implementation is inspired from some GitHub repos and stackoverflow.
