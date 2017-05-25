CC=g++

all: server client
	$(CC) -o bin/server bin/server.o -lsqlite3
	$(CC) -o bin/client bin/client.o
	rm bin/client.o bin/server.o

server:
	mkdir bin
	$(CC) -c -o bin/server.o serverMain.cc

client:
	$(CC) -c -o bin/client.o clientMain.cc

install: all
	./run_once
clean:
	rm -rf bin/
