all:
	gcc -Wall -g -O0 server.c llist.c -o server -pthread 
	gcc -Wall -g -O0 client.c -o client `pkg-config --cflags gtk+-2.0 --libs gtk+-2.0`

