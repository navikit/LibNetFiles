#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <fcntl.h>			//for open, ORDLY...
#include <sys/types.h>                  //indludes the pthreads info
#include <netinet/in.h>                 //for all the port stuff
#include <sys/socket.h>			//for  sockets,bind, listen, accept, socklen_t

#include <arpa/inet.h>			//sockadr_in, inet_ntop
#include <errno.h>                      //includes the arguements

#include <ctype.h>
#include <pthread.h>

#define unrestricted 0
#define exclusive    1
#define transaction  2
#define INVALID_FILE_MODE 9998

/**
 *This is a struct for the arguements for each of the function
 *@what_to_do   o(open), r(read), w(write), c(close)
 *@path_name    path for the file
 *@flag         flag on how to use file
 *@file_dest    file descriptor
 *@buff         buffer
 *@numbyte      size_t of the byte
 *
 */
typedef struct threadArg{

  int clientStructLength;
  int clientSocketFildes;
  struct sockaddr_in clientAdressInfo;
  char buffer[512];
  int fileIndex;

} arg;


/**
 *This is a struct for the message to relay for each of the function
 *@errors   o(open), r(read), w(write), c(close)
 *@file_dest    path for the file
 *@data         flag on how to use file
 */

typedef struct fileInfo{
	int flag;
	int mode;
	int fileDescriptor;
  	char* fileName;
} filesInfo;



void setError(char* errorMessage){
	perror(errorMessage);
	printf("ErrNo is : %d\n", errno);

	return;
}



void* initalizeSocket(void* data){


	struct sockaddr_in cli_addr;
	int clilen;

	serverSocketFildes = socket(AF_INET, SOCK_STREAM, 0);
	
	if(serverSocketFildes < 0){
		setError("Socket Connection Failed");
		return NULL;
	}

	bzero((char*)&serverAdressInfo, sizeof(serverAdressInfo));

	serverAdressInfo.sin_port = htons(portNumber);
	serverAdressInfo.sin_family = AF_INET;
	serverAdressInfo.sin_addr.s_addr = INADDR_ANY;

	if(bind(serverSocketFildes, (struct sockaddr*) &serverAdressInfo, sizeof(serverAdressInfo)) < 0){
		setError("Error on Binding");
		return NULL;
	}

	listen(serverSocketFildes, 5);
	socketInitalized = 1;

	printf("%s\n", "SOCKET INITALIZED");
	totalThreads++;	

 	int newsockfd;

	 while (1) {

      clilen = sizeof(cli_addr);
      newsockfd = accept(serverSocketFildes, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }

		threadHolder = (pthread_t*)realloc(threadHolder,sizeof(pthread_t*) * (totalThreads+1));
		arg* margs = calloc(1, sizeof(arg));
		margs->clientStructLength = clilen;
		margs->clientSocketFildes = newsockfd;
		bzero(margs->buffer,512);

		int cc = recv(margs->clientSocketFildes, margs->buffer, sizeof(margs->buffer), 0);

		if(cc < 0){
			setError("Receving from server failed");
			free(margs);
			return NULL;
		}

		pthread_create(&threadHolder[totalThreads], 0, &decodeRequest, margs);
		pthread_join(threadHolder[totalThreads], NULL);  
		totalThreads++;  
}

return NULL;
}

char** getTokens(char* b){

	char** tokens;
	char* buffer = strdup(b);

	if(buffer[0] == '2'){
		char* first = strtok(buffer, ";"); //which function to run
		char* second = strtok(NULL, ";"); //pathname
		char* third = strtok(NULL, ";");  //flags
		tokens = (char**)(malloc(sizeof(char*) * 3));
		tokens[0] = first;
		tokens[1] = second;
		tokens[2] = third;

	}else if(buffer[0] == '1' || buffer[0] == '3'){

		char* first = strtok(buffer, ";"); //which function to run
		char* second = strtok(NULL, ";"); //pathname
		char* third = strtok(NULL, ";");  //flags
		char* fourth = strtok(NULL, ";");

		tokens = (char**)(malloc(sizeof(char*) * 4));
		tokens[0] = first;
		tokens[1] = second;
		tokens[2] = third;
		tokens[3] = fourth;

	}else if( buffer[0] == '4'){
		char* first = strtok(buffer, ";");
		char* second = strtok(NULL, ";");

		tokens = (char**)(malloc(sizeof(char*) * 4));
		tokens[0] = first;
		tokens[1] = second;
	}

	return tokens;

}

//checks if a given file is open in transaction mode
int checkTransaction(char* pathName){
int i;

	for (i = 0; i < totalFiles; ++i)
	{

		if(strcmp(fileInfo[i].fileName, pathName) == 0){
			if(fileInfo[i].fileDescriptor != -1){
				if(fileInfo[i].mode != transaction) continue;
				else return 1;
			}else continue;
	}
}
	return -1;
}

int checkFileOpen(char* pathName){
int i;
	for (i = 0; i < totalFiles; ++i)
	{
		if(strcmp(fileInfo[i].fileName, pathName) == 0){
			if(fileInfo[i].fileDescriptor != -1){
				return 1;
			}else continue;
		}else continue;
	}

	return -1;
}

int checkExclusive(char* pathName){
int i;
	for (i = 0; i < totalFiles; ++i)
{
		
		if(strcmp(fileInfo[i].fileName, pathName) == 0) {

			if(fileInfo[i].fileDescriptor != -1){

				if(fileInfo[i].mode == exclusive){

					if(fileInfo[i].flag == O_RDWR || fileInfo[i].flag == O_WRONLY){

						return 1;

				}else continue;

			}else continue;

		}else continue;

	}else continue;

}

	return -1;

}

int checkUnrestrictedWrite(char* pathName){
int i;
	for (i = 0; i < totalFiles; ++i)
	{
		
		if(strcmp(fileInfo[i].fileName, pathName) == 0) {

			if(fileInfo[i].fileDescriptor != -1){

				if(fileInfo[i].mode == unrestricted){

						if(fileInfo[i].flag == O_WRONLY || fileInfo[i].flag == O_RDWR)

							return 1;

				}else continue;

			}else continue;

		}else continue;

	}

	return -1;
}


int checkCanOpen(char* pathName, int flag, int mode){


	switch(mode){

		case transaction : 
							if(checkFileOpen(pathName) == -1)  return 1;
							else return -2;

		case exclusive : 	
							if(checkTransaction(pathName) == -1){
								if(flag == O_RDONLY) return 1;
									if(checkExclusive(pathName) == -1){
										if(checkUnrestrictedWrite(pathName) == -1){
											return 1;
									} return -1;
								}return -1;

							}return -1;
			
		case unrestricted :  
							if(checkTransaction(pathName) == -1){

								if((checkExclusive(pathName) == 1) && flag == O_RDONLY) { return 1; }
							
								if(checkExclusive(pathName) == -1)   return 1;
							}
							
							return 0;

		default : 			printf("%s\n", "Could not find mode");	return -5;				

	}
}
	
/*******************************************************
 *Function: void * open_file (void *arg)
 *------------------------------------------------------
 *This is a method to open the file after recieving args
 *args: @what_to_do   o(open), r(read), w(write), c(close) 
 *	@path_name    path for the file
 *	@flag         flag on how to use file
 *	@file_dest    file descriptor
 *	@buff         buffer
 *	@numbyte      size_t of the byte
 *
 *ONLY NEED what_to_do, path_name AND flag
 *
 ******************************************************/
void * open_file (void * argu){

  	struct arguements *args = argu;
	//set arguements and print to check
	printf("Inputs: path = %s with flag = %d\n", args->path_name, args->flag);
	
	//get the message to send back ready 
	struct Message message;
	message.errors = 0;
	if( (message.file_dest = open( args->path_name, args->flag)) < 0){
		message.errors = errno;
		msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
		perror("File Error");
		exit(EXIT_FAILURE);
	}

	/**
	 *send to client that the message was delivered
	 *prints on server side the amount recieved
	 *print on server side the number of bytes sent
	 *@helps_to_ensure_all_sent
	 */
	//used to make code shorter
	int fd = message.file_dest;
	msg_deli = sendto(conn_socket, &message, (int) sizeof( &message ), 0, (struct sockaddr *) &server_addr, socketsize);
	        printf("returns %d to client of size %d\n", fd, (int) sizeof(&fd) );
		printf("only %d of %d bytes were returned\n", msg_deli, (int) sizeof(&fd) );
	++msg_sent;
	pthread_exit(0);
}

/*******************************************************
 *Function: void * read_file (void *arg)
 *------------------------------------------------------
 *This is a method to read the file after recieving args
 *args: @what_to_do   o(open), r(read), w(write), c(close) 
 *	@path_name    path for the file
 *	@flag         flag on how to use file
 *	@file_dest    file descriptor
 *	@buff         buffer
 *	@numbyte      size_t of the byte
 *
 *ONLY NEED what_to_do, file_dest, buff, numbyte
 *
 ******************************************************/
void * read_file(void * argu){
	struct arguements *args = argu;	
	//get the message ready to send back
	struct Message message;
	message.errors = 0;
	if( (message.data = read( args->file_dest, args->buff, args->numbyte)) < 0){
		message.errors = errno;
		msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
		perror("Error with reading");
		exit(EXIT_FAILURE);
	}

	msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
	printf("returns %d to client of size %d\n", (int) message.data, (int) sizeof(args->file_dest) );
		printf("only %d of %d bytes were returned\n", msg_deli, (int) sizeof(args->file_dest ) );
	++msg_sent;
	pthread_exit(0);
}

/*******************************************************
 *Function: void * write_file (void *arg)
 *------------------------------------------------------
 *This is a method to write the file after recieving args
 *args: @what_to_do   o(open), r(read), w(write), c(close) 
 *	@path_name    path for the file
 *	@flag         flag on how to use file
 *	@file_dest    file descriptor
 *	@buff         buffer
 *	@numbyte      size_t of the byte
 *
 *ONLY NEED what_to_do, file_dest, buff, numbyte
 *
 ******************************************************/
void * write_file(void * argu){
	struct arguements *args = argu;
	//get the message ready to send back
	struct Message message;
	message.errors = 0;
	if( (message.data = write( args->file_dest, args->buff, args->numbyte)) < 0){
		message.errors = errno;
		msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
		perror("Error with writing");
		exit(EXIT_FAILURE);
	}

	msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
	printf("returns %d to client of size %d\n", (int) message.data, (int) sizeof(args->file_dest ) );
		printf("only %d of %d bytes were returned\n", msg_deli, (int) sizeof(args->file_dest ) );
	++msg_sent;
	pthread_exit(0);
}

/*******************************************************
 *Function: void * close_file (void *arg)
 *------------------------------------------------------
 *This is a method to open the file after recieving args
 *args: @what_to_do   o(open), r(read), w(write), c(close) 
 *	@path_name    path for the file
 *	@flag         flag on how to use file
 *	@file_dest    file descriptor
 *	@buff         buffer
 *	@numbyte      size_t of the byte
 *
 *ONLY NEED what_to_do AND file_dest
 *
 ******************************************************/
void * close_file(void * argu){
	struct arguements *args = argu;
	struct Message message;
       
	message.errors = 0;
	//(atoi(args->file_dest))
	if( (message.file_dest = close(args->file_dest)) < 0){
		message.errors = errno;
		msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
		perror("Error with closing");
		exit(EXIT_FAILURE);
	}
	msg_deli = sendto(conn_socket, &message, (int) sizeof(&message), 0, (struct sockaddr *) &server_addr, socketsize);
	printf("successfully closed");
	pthread_exit(0);
	
}


/*******************************************************
 *Function: void * clientHandler (void * clientfd)
 *------------------------------------------------------
 *Generic Implemetation to reads message from client
 *	then passes correct arguements to run
 ******************************************************/
void * clientHandler(void * clientfd){
  printf("Inside the client handler\n");
  	int cliSock = *(int *)clientfd;
	int util;
 //	int send, sent, thisTime;
 // 	char buf[1024];
 //	bzero(buf, 1024);
	
	//make the buffer to the max size
        void * args = (void *)malloc(sizeof(struct arguements));
	bzero((char *)args, sizeof(struct arguements));
	printf("BUFFER ZEROED\n");
	//recieve a message from the socker
	/**
	 *recvfrom(int socket, void *restrict buffer, size_t length,
	 *         int flags, struct sockaddr * restrict address, 
	 *         socklen_t * restrict address_len)
	 *@recieves_a_message_from_socket
	 */
	
	//util = recvfrom(cliSock, args, (size_t) sizeof(struct arguements), 0, (struct sockaddr*) &client_addr, &socketsize);
	util = read(*(int *)clientfd, args, sizeof(struct arguements));
	//prints the ip, request and the length requested

	printf("%d\n", util);
	if(util <= 0){
	  printf("%d\n", errno);
	  pthread_exit(0);
	}
	
	//	printf("Connection from %s and requesting %s of length %d\n", (char *)inet_ntoa(client_addr.sin_addr) , request_buff, (int) strlen(request_buff));

	printf("trying threads\n");	
	pthread_t threads[MAXTHREADS];	//MAKE THREADS READY FOR OPEN,READ,WRITE AND CLOSE
	int counter = 0;                //keeps track of the threads
	int rc;
	int msg_sent;                   //for the response stuff
	int msg_deli = -32;             //the delivery msg
	int msg_len = -1;

	while(conn_socket && counter < 10){
	        msg_sent = -1;
		msg_len = (int) strlen(request_buff);		
		switch(((struct arguements *)args)->what_to_do){
			case 'o':
			  pthread_create(&threads[counter], NULL, &open_file,(void*) &args);
				pthread_join( threads[counter], NULL );
			        counter++;
				break; 
			case 'r':
			  pthread_create(&threads[counter], NULL, &read_file, (void*)&args);
				pthread_join( threads[counter], NULL );
			        counter++;
				break; 
			case 'w':
			  pthread_create(&threads[counter], NULL, &write_file, (void*)&args);
				pthread_join( threads[counter], NULL );
			        counter++;		
				break; 
			case 'c':
			  pthread_create(&threads[counter], NULL, &close_file, (void*) &args);
				pthread_join( threads[counter], NULL );				
			        counter++;
				break; 
			default:
				printf("errors \n");

				break;
		}//end of switch
		
		//meaning no message sent over
		if(msg_sent > -1){
			break;
		}
	}//end while loop

	//close all connection after done
	printf("CLOSING CONECTION\n");
	close(conn_socket);
	close(use_socket);
	return EXIT_SUCCESS;
}

/*******************************************************
 *Function: Main method
 *------------------------------------------------------
 *Creates socket, binds, listens
 *	and waits to get connections
 ******************************************************/
int main ( int argc, char * argv[] ){
	
  //struct sockaddr_in server_addr;		//structs for the server
  //struct sockaddr_in client_addr;		//struct for the client
	//int use_socket;				//port number we are currently using
	//int conn_socket;			//port number we are connected to
	Message message;			//set to make send over message	
		message.errors = 0;

	printf("\n\n\tSTARTING SERVER...\n\n");
	socklen_t socketsize = sizeof(struct sockaddr_in);	//sets the size of the struct

	//set all the stuff for the server and bind the socket as well
	memset( &server_addr, 0, sizeof(server_addr) );			//zeros out entire struct
		server_addr.sin_family = AF_INET;			//helps set connection to TCP
		server_addr.sin_addr . s_addr = htonl(INADDR_ANY);	//set to recieve from any addresses
		server_addr.sin_port  = htons(PORT);			//set the port number

	//creates socket
	if( (use_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		message.errors = errno;		
		perror("Error with creating socket\n");
		exit(EXIT_FAILURE);
	}

	//bind the socket together
	if( bind (use_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr) )  < 0){
		message.errors = errno;		        
		perror("Error with binding\n");
	        exit(EXIT_FAILURE);
	}

	//listen with allowing a queue of 1 connection pending
	printf("We are listening to port number: %d \n", ntohs(server_addr.sin_port) );
	if( listen( use_socket, 1) < 0){
		message.errors = errno;	
	        perror("Error with listening\n");
        	exit(EXIT_FAILURE);
	}
	
	//wait to accept the thread
	printf("Waiting for client to connect\n");
	if( ( conn_socket = accept (use_socket, (struct sockaddr*) &client_addr, &socketsize)) < 0){
		message.errors = errno;	
	 	perror("Error with accepting connection\n");
		exit(EXIT_FAILURE);
        }

	//make threads for the different threads
	pthread_t Client;				//create worker thread
	pthread_t * clientHandle = &Client;		//client handler taking client socket as arg
	pthread_attr_t clientAttr;			//client attribute ( void *clientStat)
	void * clientStat;
	
	pthread_attr_init(&clientAttr);
	pthread_attr_setdetachstate(&clientAttr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&clientAttr, PTHREAD_SCOPE_SYSTEM);

	//Creates a thread to manage all the connections
	if(pthread_create(clientHandle, &clientAttr, clientHandler, (void *)&conn_socket) < 0){
	    printf("COULD NOT CREATE CLIENT-SERVICE THREAD");
	}

	pthread_join(Client, &clientStat);
}



















