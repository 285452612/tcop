/***************************************************************************/
/*                                                                         */
/* Name : tcpt.h                                                           */
/* Purpose : Simple TCP/IP socket based send and receive primitives        */
/*                                                                         */
/* Date : Oct 25, 95                                                       */
/*                                                                         */
/* Last Modification :                                                     */
/*                                                                         */
/***************************************************************************/

#ifndef _TCPT_H
#define _TCPT_H

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Macros */
//#define	MMAX		65536//1024*64
#define	MMAX		524288//1024*512
#define	USHORT_LEN	(sizeof(short))

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef STX
#define STX                     0x02
#define ETX                     0x03
#endif

#ifndef INADDR_NONE
#define	INADDR_NONE	((long) 0xffffffff)
#endif

/* Function to read/write n bytes from network connection */
//int	readn(int sd, char *ptr, int nbyte);
int	writen(int sd, char *ptr, int nbyte);

/* Function to initiate a connection and waiting for a connection */
/* Hostname is cached so that if hostname specified is the same as the 
   previous one no gethostbyname etc are called again */
int ConnectRemote(char *hostname, short s_remote_port);
int ListenRemote(short s_own_port);

/* Function to get/put one application level message from socket */
int GetMessage(int sd, char *ptr, int max);
int PutMessage(int sd, char *ptr, int nbytes);

/* Function to log message into log file during daemon mode */
void logmsg(char *fmt, ...);

#endif
