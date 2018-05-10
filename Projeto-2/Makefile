all: client server start

client: client.c
	gcc client.c -o client -lpthread -Wall

server: server.c
	gcc server.c -o server -lpthread -Wall

start: start.c
	gcc start.c -o start -lpthread -Wall

clean:
	rm -f server
	rm -f client
	rm -f start
	rm -f *.o
