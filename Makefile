CC = gcc
CFLAGS = -g -Wall

defualt: clean server library client;

server:
	$(CC) $(FLAGS) -lpthread netfileserver.c -o server
library:
	$(CC) $(FLAGS) -c libnetfiles.c
client:
	$(CC) $(FLAGS) -o client client-2.c libnetfiles.o
clean:
	$(RM) client server *.o
