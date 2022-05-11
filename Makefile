CC = gcc
DBG = -Wall -g

all: client server

test: test.c memAlloc.a stack.a
	$(CC) $(DBG) -w test.c memAlloc.a stack.a -o test

client: client.c
	$(CC) $(DBG) client.c -o client

server: server.o stack.a memAlloc.a
	$(CC) $(DBG) server.o stack.a memAlloc.a -o server

server.o: server.c stack.h
	$(CC) $(DBG) -c server.c

memAlloc.a: memAlloc.o
	ar -rcs memAlloc.a memAlloc.o

memAlloc.o: memAlloc.c memAlloc.h
	 $(CC) -c -w memAlloc.c

stack.a: stack.o
	ar -rcs stack.a stack.o

stack.o: stack.c
	$(CC) $(DBG) -c stack.c

clean:
	rm -f *.o *.a server client test
