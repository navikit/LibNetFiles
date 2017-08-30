#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "libnetfiles.h"

#define MAXRCVLEN 500
#define PORTNUM 5108
/*
void sampleConnection(){
	char buffer[MAXRCVLEN + 1]; // +1 so we can add null terminator 
	int len, mysocket;
	struct sockaddr_in dest;
	
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest)); //zero the struct 
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr("127.0.0.1"); //set destination IP number
dest.sin_port = htons(PORTNUM); //set destination port number
	connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	send(mysocket, "Help me, Obi-Wan. You're my only hope!", 38, 0);
	len = recv(mysocket, buffer, MAXRCVLEN, 0);

// We have to null terminate the received data ourselves 
	buffer[len] = '\0';
	printf("\tReceived %s (%d bytes).\n", buffer, len);
	close(mysocket);
}*/
/*
char* localRead(){
	foo_struct* sample = (foo_struct*) malloc(sizeof(foo_struct));
	sample -> x = 10;
	sample -> y = 35.0;
	sample -> words = "Success, yay!";
	
	int fildes = open("/ilab/users/fuck/sike.c", O_RDONLY);
	printf("Look at client.c's file descriptor: %d\n", fildes);
	void* babyBuff = (void*) sample; //malloc(sizeof(char)*510);
	//int status = read(fildes, babyBuff, 10);
	printf("This is the status of the read operation: %d\nCharacters read: %s\n",201,(char*) babyBuff);
	return (char*) babyBuff;
}*/

int main(int argc, char *argv[])
{
  int choice = -1;int util;
	char* recieval;
	
	printf("Welcome to the client software. Ready to interact with remote files?\n");

	printf("Attempting to initiate local server!! \n Testing netserverinit \n");
	util = netserverinit("127.0.0.1");
	printf("%d\n", util);

	

	void * tester;size_t test;
	while(choice != 0){
		printf("What would you like to do?\n1. open a file.\n2. read a file\n3. write a file\n4. close a file\n0. Quit.\n");
		scanf("%d", &choice);
		puts("");
		switch(choice){
		case 0:
			foolio();
			break;
		case 1:
		  netopen("/ilab/users/ic165/Desktop/shithole/client.txt", O_WRONLY);
			break;
		case 2:
		  netread(0,tester,test);
		  break;
		case 3:
		  netwrite(0,tester,test);
		  break;

		case 4:
		  netclose(0);
		  break;
		default:
			printf("That is not a valid option!\n");
		}
		puts("");
	}
	printf("Thank you for your participation!\n");
	return EXIT_SUCCESS;
}
