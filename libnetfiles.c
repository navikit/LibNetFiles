#include "libnetfiles.h"
extern int h_errno;
extern int errno

int host hostfound
static int portNumber = 9999;
char* hostName;
int clientSocketFiledes = -1;
int mode;
char* modes;
int error;

//#define PORT 5108			//Using port number 5108
//#define MAXBUFF 2048			//make a max buffer size of 2048

void setError(char* errorMessage){
	perror(errorMessage);
}

void setErrno(char* message){

	char** tokens;

	tokens = (char**)(malloc(sizeof(char*) * 2));
	tokens[0] = strtok(message, ";");
	tokens[1] = strtok(NULL, ";");

	error = 0;
	error = atoi(tokens[1]);

	if(error == INVALID_FILE_MODE) {printf("%s\n", "File waiting in Queue"); return;}

	printf("Errno is : %d\n", error);
	errno = error;

	return;
}

/********************************************************************
 *Function: int makesocket()
 *-------------------------------------------------------------------
 *CLOSE SOCKET
 ********************************************************************/

int initalizeSocket(){

	struct sockaddr_in serverAdressInfo;
	struct hostent* serverIpAddress = gethostbyname(hostName);

	clientSocketFiledes = socket(AF_INET, SOCK_STREAM, 0);

	if(clientSocketFiledes < 0){
		setError("Error connecting to Socket");
		//set errno
		return -1;
	}

	bzero((char*)&serverAdressInfo, sizeof(serverAdressInfo)); //zeroes out all data pre-init
	serverAdressInfo.sin_family = AF_INET;
	serverAdressInfo.sin_port = htons(portNumber);

	bcopy((char*)serverIpAddress->h_addr, (char*)&serverAdressInfo.sin_addr.s_addr, serverIpAddress->h_length);

	if(connect(clientSocketFiledes, (struct sockaddr*)&serverAdressInfo, sizeof(serverAdressInfo)) < 0){
		setError("Error Connecting to socket");
		return -1;
	}

	return 0;
}

/********************************************************************
 *Function: close_socket (int socket_file_dest)
 *-------------------------------------------------------------------
 *CLOSE SOCKET
 ********************************************************************/
void closeSocket(int clientSocketFiledes){
      if (shutdown(clientSocketFiledes, SHUT_RDWR) < 0) // secondly, terminate the 'reliable' delivery
         if (errno != ENOTCONN && errno != EINVAL) // SGI causes EINVAL
            setError("Sutting Down");
      if (close(clientSocketFiledes) < 0) // finally call close()
         	setError("close");
   
}

/********************************************************************
 *Function: netserverinit (char * host_name)
 *-------------------------------------------------------------------
 *Checks if the hostname exists
 *
 *For more info refer to header file
 ********************************************************************/
int netserverinit(char* hostNamen, int mode){

	struct hostent* serverIpAddress = gethostbyname(hostNamen);

	if(serverIpAddress == NULL){
		setError("Could not find hostname");
		return -1;
	}

	hostFound = 1;
	return 0;

}


/********************************************************************
 *Function: netopen(const char * path_name , int flag)
 *-------------------------------------------------------------------
 *Client uses this to open file over the socket
 *
 *
 *For more info refer to header file
 ********************************************************************/
int netopen(const char* pathname, int flags){
	if(hostFound == 0){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	clientSocketFiledes = -1;
	initalizeSocket();
	int i;
	//will need to tell server which command was entered

	char* buffer = (char*)(calloc(1,strlen(pathname) + 10 + 2)); //*function in int*;pathname;flag/n
	buffer[0] = '1';
	buffer[1] = ';';
	for (i = 0; i < strlen(pathname); ++i)
	{
		buffer[2+i] = pathname[i];
	}
	buffer[i+2] = ';';
	buffer[i+3] = flags + '0';
	buffer[i+4] = ';';
	buffer[i+5] = mode + '0';
	buffer[i+6] = '\0';


	if(send(clientSocketFiledes, buffer, strlen(buffer), 0) < 0){
		setError("Could not send message to server");
		return -1;
	} 

	free(buffer);

	int received_int = 0;
	int return_status = read(clientSocketFiledes, &received_int, sizeof(received_int));

	if (return_status < 0) {setError("Recive failed"); return -1;}

	char newBuffer[512];

	if(received_int < 0) { //set errno here
		int cc = recv(clientSocketFiledes, newBuffer, sizeof(newBuffer), 0);

		if(cc < 0){
			setError("Could not receive Error message from server");
			return -1;
		}
		setErrno(newBuffer);
	}


	while(error == INVALID_FILE_MODE){

		received_int = 0;
		return_status = read(clientSocketFiledes, &received_int, sizeof(received_int));
		if(return_status < 0) {setError("Receive Failed"); return -1;}

		if(received_int > 0){
			close(clientSocketFiledes);
			return ntohl(received_int);
		}

	}

close(clientSocketFiledes);
return ntohl(received_int);
}

/********************************************************************
 *Function: ssize netread(int file_dest, void * buff, size_t nbyte)
 *-------------------------------------------------------------------
 *Client uses this to read file over the socket
 *
 *
 *For more info refer to header file
 *********************************************************************/

ssize_t netread(int fileDescriptor, void* buf, size_t nbyte){

	if(hostFound == 0){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	clientSocketFiledes = -1;
	initalizeSocket();

	int flength = snprintf(NULL, 0 ,"%d" , fileDescriptor);
	int blength = snprintf(NULL, 0 , "%d" , nbyte);

	int totalMallocLength = 2 + flength + blength + 1 + 1;
	
	char* tempBuffer = (char*)(calloc(1,totalMallocLength));
	tempBuffer[0] = '2';
	tempBuffer[1] = ';';

	sprintf(&tempBuffer[2], "%d", fileDescriptor);
	tempBuffer[2 + flength] = ';';

	sprintf(&tempBuffer[3 + flength], "%d", nbyte);

	tempBuffer[totalMallocLength] = '\0';

	if(send(clientSocketFiledes, tempBuffer, strlen(tempBuffer), 0) < 0){
		setError("Could not Send message to Buffer : netRead");
	}


	free(tempBuffer);

	char newBuffer[10000];
	bzero(newBuffer, 10000);
	int total;

	while(1){
		int a = recv(clientSocketFiledes, &newBuffer[0], sizeof(newBuffer), 0);
   		 if ( a == -1 ) setError( "recv failed" );
    	 if ( a == 0 ) break; /* got end-of-stream */
   		 total += a;
	}

		char* first = strtok(newBuffer, ";");
		char* second = strtok(NULL, ";");
		char* third = strtok(NULL, ";");

		int readInt = atoi(third);
		char* readBuffer = (char*)(calloc(1,sizeof(char) * strlen(second)));
		readBuffer = strcpy(&readBuffer[0], second);

char* copyBuffer = buf;
memset(copyBuffer, 0, sizeof(copyBuffer));
copyBuffer = strcpy(&copyBuffer[0], readBuffer);
free(readBuffer);

close(clientSocketFiledes);
return readInt;
}

/********************************************************************
 *Function: ssize netwrite(int file_dest, void * buff, size_t nbyte)
 *-------------------------------------------------------------------
 *Client uses this to read file over the socket
 *
 *
 *For more info refer to header file
 *********************************************************************/
ssize_t netwrite(int fileDescriptor, const void* buf, size_t nbyte){
	if(hostFound == 0){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}


	char* buffer = (char*)buf;
	clientSocketFiledes = -1;
	initalizeSocket();

	int flength = snprintf(NULL, 0 ,"%d" , fileDescriptor);
	int blength = snprintf(NULL, 0 , "%d" , nbyte);
	int buffLenght = strlen(buffer);

	int totalMallocLength = 2 + flength + blength + 1 + 1 + buffLenght + 1;
	
	char* tempBuffer = (char*)(calloc(1,sizeof(char) * totalMallocLength));
	tempBuffer[0] = '3';
	tempBuffer[1] = ';';

	sprintf(&tempBuffer[2], "%d", fileDescriptor);
	tempBuffer[2 + flength] = ';';
	int i;

	for (i = 0; i < strlen(buffer); ++i) tempBuffer[3 + flength + i] = buffer[i];
	
	tempBuffer[3 + flength + i] = ';';
	sprintf(&tempBuffer[4 + flength + i], "%d", nbyte);
	tempBuffer[totalMallocLength] = '\0';

	if(send(clientSocketFiledes, tempBuffer, strlen(tempBuffer), 0) < 0)
		setError("Could not Send message to Buffer : netRead");
	
	free(tempBuffer);

	int received_int = 0;
	int return_status = read(clientSocketFiledes, &received_int, sizeof(received_int));

	if (return_status < 0) {setError("Recive failed"); return -1;} 


	char newBuffer[10000];
	bzero(newBuffer, 10000);

	if(received_int < 0) { //set errno here
		int cc = recv(clientSocketFiledes, newBuffer, sizeof(newBuffer), 0);
		
		if(cc < 0){
			setError("Could not receive Error message from server");
			return -1;
		}
			setErrno(newBuffer);
	}

	
close(clientSocketFiledes);
return ntohl(received_int);

}



/******************************************************************
 *Function: netclose(int fd)
 *-----------------------------------------------------------------
 *Client uses this to close file over the socket
 *
 *
 *For more info refer to header file
 *******************************************************************/
int netclose(int fileDescriptor){
	if(hostFound == 0){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	clientSocketFiledes = -1;
	initalizeSocket();

	int flength = snprintf(NULL, 0 ,"%d" , fileDescriptor);

	char* tempBuffer = (char*)(calloc(1, sizeof(char) * flength + 3));

	tempBuffer[0] = '4';
	tempBuffer[1] = ';';

	sprintf(&tempBuffer[2], "%d", fileDescriptor);
	tempBuffer[2 + flength] = ';';


	if(send(clientSocketFiledes, tempBuffer, strlen(tempBuffer), 0) < 0)
		setError("Could not Send message to Buffer : netRead");
	

	int received_int = 0;
	int return_status = read(clientSocketFiledes, &received_int, sizeof(received_int));

	if(return_status < 0) {setError("Recive failed"); return -1;}

	if(received_int < 0) { //set errno here
		bzero(tempBuffer, flength + 3);
		int cc = recv(clientSocketFiledes, tempBuffer, sizeof(tempBuffer), 0);
		
		if(cc < 0){
			setError("Could not receive Error message from server");
			return -1;
		}
			printf("%s\n", tempBuffer);
			setErrno(tempBuffer);
	}
	
free(tempBuffer);
close(clientSocketFiledes);
return ntohl(received_int);
}

int main(int argc, char* argv[]){

	if(argc != 3){
		setError("Usage ./executableFileName HOSTNAME connectionMode");
	}

	hostName = argv[1];
	modes = argv[2];

	mode = -1;
	if(strcmp(modes, "unrestricted") == 0) mode = unrestricted;
	if(strcmp(modes, "exclusive") == 0) mode = exclusive;
	if(strcmp(modes, "transaction") == 0) mode = transaction;


	if(mode == -1){ printf("%s\n", "Please enter mode correctly"); return -1; }

	if(netserverinit(hostName, mode) == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	//hostname found
	int x;
	char* pathname = "/.autofs/ilab/ilab_users/sss269/A3/bstesting.txt\0";

	char* b = malloc(10000);


	int write_status;
	int read_status;
	int open_status;
	int close_status;


	int fileDesc = netopen(pathname, O_RDWR);

	if(fileDesc < 0) return -1;

	x = fileDesc;

	printf("File Handler : %d\n", x);
	read_status = netread(x,b,10000);

	printf("%s\n", b);
	
	printf("Read : %d\n", read_status);
	write_status = netwrite(x, "some string here nigga", 30);
	printf("Write : %d\n", write_status);
	read_status = netread(x,b,10000);
	printf("Read : %d\n", read_status);
	write_status = netwrite(x, "some string here nigga", 30);
	printf("Write : %d\n", write_status);
	read_status = netread(x,b,10000);
	printf("Read : %d\n", read_status);


	sleep(20);

	close_status = netclose(x);





}




