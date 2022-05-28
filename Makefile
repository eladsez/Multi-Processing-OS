CC = g++
DBG = -Wall -g

all: client server test

client: client.cpp
	$(CC) $(DBG) client.cpp -o client

server: server.o stack.a memAlloc.a
	$(CC) $(DBG) server.o stack.a memAlloc.a -o server

test: test.cpp memAlloc.a stack.a
	$(CC) $(DBG) -w test.cpp memAlloc.a stack.a -o test

server.o: server.cpp stack.h
	$(CC) $(DBG) -c server.cpp

memAlloc.a: memAlloc.o
	ar -rcs memAlloc.a memAlloc.o

memAlloc.o: memAlloc.cpp memAlloc.h
	 $(CC) -c -w memAlloc.cpp

stack.a: stack.o
	ar -rcs stack.a stack.o

stack.o: stack.cpp
	$(CC) $(DBG) -c stack.cpp

clean:
	rm -f *.o *.a server client test
