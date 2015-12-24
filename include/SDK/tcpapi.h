/*********************************
	DEFINE FOR TCPAPI
	CREATED BY SUN LAN
	1998.12.16
**********************************/
#include "SDKbool.h"

#ifndef _TCPAPI_H
#define _TCPAPI_H

/* Error code define */
#define E_NET_SUCC          0	/* success */
#define E_NET_NOTRANS	    1	/* no such trans_code */
#define E_NET_APPBUSY	    2	/* application busy */
#define E_NET_DENY          3	/* IP address denied */
#define E_NET_MAC           4   /* Data MAC error */
#define E_NET_FMAC          5   /* File MAC error */
#define E_NET_SYS           6   /* Operation system error */
#define E_NET_SOCK          7   /* Socket error */
#define E_NET_NOBUFS        8   /* No buffers */
#define E_NET_COMPRESS      9   /* Compress error */
#define E_NET_UNCOMPRESS    10  /* uncompress error */
#define E_NET_FTRANSFER     11  /* File transfer error */

/* define for data transport control */
#define SEND_DATA_BEGIN	"SDB"
#define SEND_DATA_END	"SDE"
#define ACCEPT_DATA_RIGHT	"ADR"
#define ACCEPT_DATA_ERROR	"ADE"

/*******************************************
	pre_data when transaction begin
********************************************/
#define PRE_LEN 3

#define TR_LEN  6

#define NODEID_MAX  8       /* max length of nodeid */

/* package information */
#define PI_DATAPACK     0X00000010  /* have data */
#define PI_FILE         0X0000000F  /* have file */
#define PI_DCOMPRESS    0X00000100  /* transfer data with data compress */
#define PI_FCOMPRESS    0X00000200  /* transfer data with file compress */
#define PI_USESECU      0X00000400  /* transfer data with security */
#define PI_PACKDOWN     0X80000000  /* pack send back by remote */

typedef struct Header		/*	Transaction information	*/
{
	unsigned char TrType[TR_LEN+1];	        /*Transaction Type*/
	unsigned int PackInfo;		            /*package information*/
	unsigned int Sleng;		                /*Transaction package length*/
	unsigned char NodeId[NODEID_MAX ];	    /* Node ID.Use for security */
	unsigned char RemoteNode[NODEID_MAX];   /* Remote Node ID */
	unsigned char DataMac[16];
	unsigned char RetCode[2+1];	            /*comm result*/
	unsigned char Ftype;		            /*File Type*/
                        					/*'A'	Asc*/
                        					/*'B'	Binary*/
} TAPIHEAD;

typedef struct Header TCPHEAD;

/* File Transport mode */
#define SENDDIR (char *)getenv("FILES_DIR")
#define RECVDIR (char *)getenv("FILES_DIR")

/* Transport File Name Max Length */
#define MAX_FNAME	40
#define FILEMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP

struct FileInfo		/*	File Information	*/
{
	char Fname[MAX_FNAME];
	unsigned int Fleng;
	unsigned char FMac[16];	/* MAC for file data */
};

int cli_sndrcv( const char *svr_name, int sock_port, TCPHEAD *head_ptr,
	char *reqdata, char *fname_r, char *ansdata, char *fname_a,
	int timeout );

int svr_rcv( TCPHEAD *head_ptr, void *data, char *fname, int timeout );

int svr_snd( TCPHEAD *head_ptr, void *data, char *fname, int timeout );

int tapi_svrend();

int tapi_getaddr( char *svrname, char *ip, int *port );

char *tapi_getnodeid();

bool IsAddress( const char *ipstr );

#endif      /* _TCPAPI_H */
