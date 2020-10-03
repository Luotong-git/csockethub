all: server.c client.c
	gcc server.c -o server -lpthread
	gcc client.c -o client -lpthread