/************************************************************************
 *SEND MESSAGE TO SOCKET
 *-----------------------
 *ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, 
 *		const struct sockaddr *dest_addr, socklen_t addrlen);
 *
 *RECIEVE MESSAGE FROM SOCKET
 *---------------------------
 *ssize_t recvfrom(int socket, void *restrict buffer, size_t length, 
 *		int flags, struct sockaddr *restrict address, 
 *		socklen_t *restrict address_len);
 * 
 ************************************************************************/
#ifndef _LIBNETFILES_H_
#define _LIBNETFILES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <fcntl.h>		//for open, ORDLY...
#include <sys/types.h>          //indludes the pthreads info
#include <netinet/in.h>         //for all the port stuff
#include <sys/socket.h>		//for  sockets,bind, listen, accept, socklen_t
#include <arpa/inet.h>		//sockadr_in, inet_ntop
#include <sys/param.h>		//used for definition for MAXHOSTNAMELEN
#include <netdb.h>

#define unrestricted 0
#define exclusive    1
#define transaction  2
#define INVALID_FILE_MODE 9998
	
	int netserverinit(char * hostname, int mode);

	int netopen(const char * pathname , int flags);
/***************************************************************************
	flags must include accessmode 
		O_RDONLY	READ ONLY
		O_WRONLY	WRITE ONLY
		o_RDWR		READ AND WRITE

	return file descriptor if there
		else reutn -1 where erno set appropiately

	ERROR:
	Y	EACCESS		permission is denied 	13
		EINTR		interruped system call 	4
		EISDIR		is a directory		21
	y	ENOENT		no file/directory 	2
	y	EROFS		can only read
	OPTIONAL:
		ENFILE		file table overflow	23
		EWOULDBLOCK	op would block		EAGAIN
		EPERM		op not permitted	1
*****************************************************************************/

	ssize_t netread(int filedest, void * buff, size_t nbyte);
/***************************************************************************
	returns non-negative integer that states number of bytes READ
		else return -1 where erno set appropiately

	ERRORS:
		ETIMEDOUT	connection timeout	110
		EBADF		bad file number		9
		ECONNRESET	connection reset	104
*****************************************************************************/
	
	ssize_t netwrite(int filedest, void * buff, size_t nbyte);
/***************************************************************************
	@for_ext_A
	ssize_t netWRITE(int files, void *buff, size_t nbyte)
	
	returns non-negative integer that states number of bytes WRITTEN
		else return -1 where erno set appropiately

	ERRORS:
		ETIMEDOUT	connection timeout	110
		EBADF		bad file number		9
		ECONNRESET	connection reset	104
*****************************************************************************/

	int netclose(int filedest);
/***************************************************************************
	returns 0 is it is successful
		else return -1 where erno set appropiately

	ERRORS:
		EBADF		bad file number		9
*****************************************************************************/

#endif

