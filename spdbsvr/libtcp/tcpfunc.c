/********************************************
**  PROGRAM: tcpfunc.c                     **
**  LAST MODIFY DATE: 08/30/2010           **
**  COMMENT: TCP通信函数程序               **
**                                         **
*********************************************/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "tcpt.h"
#include <sys/errno.h>
#include  <sys/types.h>
#include  <time.h>


extern int errno;

extern int _nDebug;
static char dirty_buf[MMAX+1]; /* Internal buffer for whatever purpose */

/*
 * readn() -- Function to read n bytes from network connection
 *   para   -- nbyte: read nbyte byte
 *   return --
 */
static int readn(int sd, char *ptr, int nbyte)
{
    int	nleft, nread;

    nleft = nbyte;

    while (nleft > 0) 
    {
        if ((nread = read(sd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }

    /* Return number of bytes read == nbyte - nleft */
    return (nbyte - nleft);
}

/*
 * writen() -- Function to write n bytes from network connection
 *   para   --
 *   return --
 */
int writen(int sd, char *ptr, int nbyte)
{
    int	nleft, nwritten;

    nleft = nbyte;
    while (nleft > 0) 
    {
        if ((nwritten = write(sd, ptr, nleft)) <= 0)
        {
            if (errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    /* Return number of bytes written == nbyte - nleft */
    return (nbyte - nleft);
}

/*
 * GetStdMessage() -- Function to get one application level message from socket 
 *   para   --
 *   return -- -1: 0: -99: tcp read -1
 */
int GetStdMessage(int sd, char *ptr, int max)
{
    int	  nread;
    unsigned char  cs_nob[USHORT_LEN + 1];	/* No. of bytes in character */
    unsigned char LRC=0;
    int s_nob;			/* No. of bytes in short */
    int	  nbyte;			/* No. of bytes to read from stream */
    int	  ret = -1;			/* Final return value */
    int i;

    /* First get the number of bytes in application message *
       nread = readn(sd, cs_nob, USHORT_LEN);
       if (nread != USHORT_LEN)
       return -1;

     * Convert to number *
     memcpy(&s_nob, cs_nob, USHORT_LEN);
     s_nob = ntohs(s_nob);
     ****/

    memset(cs_nob, 0, sizeof(cs_nob));
    if ((nread=readn(sd, (char *)cs_nob, 1)) != 1)
    {
        logmsg("GetStdMessage() readn 1 error! nread=[%d]", nread); 
        return -99;
    }

    if (cs_nob[0] != STX)
    {
        logmsg("GetStdMessage() cs_nob=[%x] error!", cs_nob[0]); 
        return -1;
    }
    if ((nread = readn(sd, (char *)cs_nob, 2)) != 2)
    {
        logmsg("GetStdMessage() readn 2 error! nread=[%d]", nread); 
        return -2;
    }
    s_nob = (int)(cs_nob[0]>>4)*1000 + (int)(cs_nob[0]&0x0f)*100;
    s_nob += (int)(cs_nob[1]>>4)*10 + (int)(cs_nob[1]&0x0f);

    if (s_nob > max)
        nbyte = max;
    else
        nbyte = s_nob;

    /* Read remaining bytes */
    nread = readn(sd, (char *)ptr, nbyte);
    ret = nread;

    if (nread != nbyte)
    {
        logmsg("GetStdMessage() readn error! nread=%d, nbyte=%d", nread, nbyte);
        return -3;
    }

    if (readn(sd, (char *)dirty_buf, 2) != 2)
    {
        logmsg("GetStdMessage() readn 2* error!"); 
        return -4;
    }
    if (dirty_buf[0] != ETX)
    {
        logmsg("Read ETX error! [%x]", dirty_buf[0]);
        return -5;
    }

    LRC = 0;
    LRC ^= cs_nob[0];
    LRC ^= cs_nob[1];
    for (i=0; i<nbyte; i++)
        LRC ^= *(ptr+i);
    LRC ^= ETX;
    if (LRC != (unsigned char) dirty_buf[1])
    {
        logmsg("Read LRC error: [%02x %02x]", LRC, dirty_buf[1]);
        ret = -6;
    }

    /* Check if there is remaining bytes to clear 
       Return value independent on result of this section */ 
    nread = s_nob - max;	 
    while (nread > 0) 
    {
        if (nread >= MMAX) 
            nbyte = MMAX;
        else
            nbyte = nread; 
        nbyte = readn(sd, dirty_buf, nbyte);
        if (nbyte > 0)
            nread -= nbyte;
        else
            break;
    }

    /* Return number of application bytes got */
    return (ret);
}


/*
 * PutStdMessage() -- Function to put one application level message from socket 
 *   para   --
 *   return --
 */
PutStdMessage(sd, ptr, nbytes)
    int sd, nbytes;
    char *ptr;
{
    /* Similar variables as getMessage */
    int	nwritten;
    unsigned char	cs_nob[USHORT_LEN + 1];
    short	s_nob;
    unsigned char LRC;
    int i;

    cs_nob[0] = STX;
    if ((nwritten=writen(sd, (char *)cs_nob, 1)) != 1)
    {
        logmsg("PutStdMessage() writen 1 error![%d]", nwritten); 
        return -1;
    }
    cs_nob[0] = ((nbytes/1000)<<4) + ((nbytes/100)%10);
    cs_nob[1] = ((nbytes/10)%10<<4) + (nbytes%10);
    if ((nwritten=writen(sd, (char *)cs_nob, 2)) != 2)
    {
        logmsg("PutStdMessage() writen 2 error![%d]", nwritten); 
        return -1;
    }

    /* Write out application data */
    nwritten = writen(sd, (char *)ptr, nbytes);
    if (nwritten != nbytes)
    {
        logmsg("PutStdMessage() writen nbytes[%d] error, written=%d.", 
                nbytes, nwritten);
        return -1;
    }

    LRC = 0;
    LRC ^= cs_nob[0];
    LRC ^= cs_nob[1];
    for (i=0; i<nbytes; i++)
        LRC ^= *(ptr+i);
    LRC ^= ETX;

    cs_nob[0] = ETX;
    cs_nob[1] = LRC;
    if (writen(sd, (char *)cs_nob, 2) != 2)
    {
        logmsg("PutStdMessage() writen etx2 error");
        return -1;
    }

    return nwritten;
}

#if 0
/*
 * GetPackMessage() -- Function to get a packet from socket 
 *   para   --
 *   return --
 */
int GetPackMessage(int sd, char *ptr, int min, int max)
{
    int	  nread, len;
    int	  nbyte;			/* No. of bytes to read from stream */
    int	  ret = -1;			/* Final return value */
    char temp[30];
    char pStart[20];
    char pEnd[20];

    time_t  start4, end4;


    nread = 0;
    nbyte = max;

    do {
        len = 0;
        len = read(sd, (char *)ptr+nread, nbyte);
        if (len >= 0)
            nread += len;
        nbyte = max - nread;
    } while (nread > 0 && nread < min);

    //ret = recv(sd, ptr, nbyte, 0);

    if (len <= 0)
    {
        logmsg( "[%d] read error! %s\n", getpid(), strerror(errno) );
        ret = len;
    } else
        ret = nread;


    //sprintf(temp, "%s.III", _sPrgName);
    //Save(temp, ptr, ret);

    /* Return number of application bytes got */
    logmsg("[%05d] read=[%s]--[%s][%f][%d]\n", getpid(), pStart, pEnd, difftime(end4, start4), ret);
    return (ret);
}
#endif

int GetPackMessage(int sockfd, char *buf, int min, int max)
{
    int bytes_read;
    int this_read;
    int nleft;

    logmsg("ready read pack length...");
    if ((this_read = read(sockfd, buf, 2)) != 2)
    {
        logmsg("recv length error, %s.", strerror(errno));
        return -1;
    }
    nleft = (unsigned char)buf[0] * 256 + (unsigned char)buf[1];
    logmsg("pack_length=[%d].", nleft);

    // 防止 buf 溢出
    if (nleft > max)
        nleft = max;
    nleft -= 2;
    bytes_read = 2;
    while (nleft > 0)
    {
        if ((this_read = read(sockfd, buf+bytes_read, nleft)) < 0)
        {
            if (errno == EINTR)
                this_read = 0;
            else
            {
                logmsg("read fail, ret=%d, errno=%d.", this_read, errno);
                return -1;
            }
        }
        logmsg("read [%d] bytes...", this_read);
        if (this_read == 0)
        {
            logmsg("read eof.");
            return bytes_read;
        }
        nleft -= this_read;
        bytes_read += this_read;
    }
    logmsg("recv %d bytes.", bytes_read);
    return bytes_read;
}

/*
 * PutPackMessage() -- Function to a packet message to socket 
 *   para   --
 *   return --
 */
int PutPackMessage(int sd, char *ptr, int nbytes)
{
    int	nwritten;

    /* Write out application data */
    nwritten = writen(sd, ptr, nbytes);
    if (nwritten != nbytes)
        return -1;

    return nwritten;
}

/*
 * GetStdFile() -- TCP/IP读文件数据(格式包)
 *  in  para -- sd:
 *              max: 文件数据最大长度
 *  out para -- ptr: 文件数据
 *    return -- -1: 失败, >0: 读到的文件数据长度
 */
int GetStdFile(sd, fname, max)
    int  sd;
    char *fname;
    long max;
{
    int  nread;
    int  nbytes;               /* No. of bytes to read from stream */
    int  ret = -1;             /* Final return value */
    char tmpbuf[1025];
    FILE *fopen( ), *fp;

    if(max <= 0)
        return max;

    fp = fopen(fname, "w");
    if(fp==NULL)
        return -1;

    nread = 0;
    nbytes = max;
    while(nbytes >1000)
    {
        memset(tmpbuf, 0, 1025);
        if(GetStdMessage(sd, tmpbuf, 1000)!=1000)
        {
            fclose(fp);
            return -1;
        }
        fwrite(tmpbuf, 1000, 1, fp);
        nread += 1000;
        nbytes -= 1000;
    }
    /* 不足1000字符 */
    if(nbytes>0)
    {
        memset(tmpbuf, 0, 1025);
        if(GetStdMessage(sd, tmpbuf, nbytes)!=nbytes)
        {
            fclose(fp);
            return -1;
        }
        fwrite(tmpbuf, nbytes, 1, fp);
    }
    nread += nbytes;

    fclose(fp);                           
    return nread;
}


/*
 * PutStdFile() -- TCP/IP写文件数据(格式包)
 *  in  para --  sd:     TCP端口 
 *               fname:  文件数据
 *               nbytes: 文件数据长度
 *  out para -- 无
 *    return -- -1: 失败, >0: 写的文件数据长度
 */
int  PutStdFile(sd, fname, nbytes)
    int  sd;
    char *fname;
    long nbytes;
{
    int  nwritten=0; 
    char tmpbuf[1025];
    FILE *fopen( ), *fp;

    if (nbytes<1)
        return nbytes;

    fp = fopen(fname, "r");
    if(fp==NULL)
        return -1;

    while(nbytes>1000)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        fread(tmpbuf, 1000, 1, fp);
        if(PutStdMessage(sd, tmpbuf, 1000)!=1000)
        {
            fclose(fp);
            return -1;
        }
        nwritten += 1000;
        nbytes -= 1000;
    }
    /* 不足1000字符 */
    if(nbytes>0)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        fread(tmpbuf, nbytes, 1, fp);
        if(PutStdMessage(sd, tmpbuf, nbytes)!=nbytes)
        {
            fclose(fp);
            return -1;
        }
    }
    nwritten += nbytes;

    fclose(fp);                           
    return nwritten;
}

/*
 * GetPackFile() -- TCP/IP读文件数据(任意包)
 *  in  para -- sd:
 *              max: 文件数据最大长度
 *  out para -- ptr: 文件数据
 *    return -- -1: 失败, >0: 读到的文件数据长度
 */
int GetPackFile(sd, fname, max)
    int  sd;
    char *fname;
    long max;
{
    int  nread;
    int  nbytes;               /* No. of bytes to read from stream */
    int  ret = -1;             /* Final return value */
    char tmpbuf[1025];
    FILE *fopen( ), *fp;

    if(max <= 0)
        return max;

    fp = fopen(fname, "w");
    if(fp==NULL)
        return -1;

    nread = 0;
    nbytes = max;
    while(nbytes >1000)
    {
        memset(tmpbuf, 0, 1025);
        if(GetPackMessage(sd, tmpbuf, 1000, 1000)!=1000)
        {
            fclose(fp);
            return -1;
        }
        fwrite(tmpbuf, 1000, 1, fp);
        nread += 1000;
        nbytes -= 1000;
    }
    /* 不足1000字符 */
    if(nbytes>0)
    {
        memset(tmpbuf, 0, 1025);
        if(GetPackMessage(sd, tmpbuf, nbytes, nbytes)!=nbytes)
        {
            fclose(fp);
            return -1;
        }
        fwrite(tmpbuf, nbytes, 1, fp);
    }
    nread += nbytes;

    fclose(fp);                           
    return nread;
}


/*
 * PutPackFile() -- TCP/IP写文件数据(任意包)
 *  in  para --  sd:     TCP端口 
 *               fname:  文件数据
 *               nbytes: 文件数据长度
 *  out para -- 无
 *    return -- -1: 失败, >0: 写的文件数据长度
 */
int  PutPackFile(sd, fname, nbytes)
    int  sd;
    char *fname;
    long nbytes;
{
    int  nwritten=0; 
    char tmpbuf[1025];
    FILE *fopen( ), *fp;

    if (nbytes<1)
        return nbytes;

    fp = fopen(fname, "r");
    if(fp==NULL)
        return -1;

    while(nbytes>1000)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        fread(tmpbuf, 1000, 1, fp);
        if(PutPackMessage(sd, tmpbuf, 1000)!=1000)
        {
            fclose(fp);
            return -1;
        }
        nwritten += 1000;
        nbytes -= 1000;
    }
    /* 不足1000字符 */
    if(nbytes>0)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        fread(tmpbuf, nbytes, 1, fp);
        if(PutPackMessage(sd, tmpbuf, nbytes)!=nbytes)
        {
            fclose(fp);
            return -1;
        }
    }
    nwritten += nbytes;

    fclose(fp);                           
    return nwritten;
}

/*
 * logmsg() --
 *   para   --
 *   return --
 */
void logmsg(char *fmt, ...)
{
    va_list	ap_list;
    FILE	*fp;

    va_start(ap_list, fmt);
    fp = fopen("/tmp/tcp.log", "a+");
    if (fp != NULL) 
    {
        vfprintf(fp, fmt, ap_list);
        fprintf(fp, "\n");
        fclose(fp);
    }
    va_end(ap_list);
}

/*
 * ConnectRemote() -- to initiate a connection and waiting for a connection 
 *   para   --
 *   return --
 */
ConnectRemote(char *hostname, short s_remote_port)
{
    static  char cache_hostname[80+1];  /* Cached hostname */
    static  int first_call = TRUE;      /* Check if it is first called */
    static  struct sockaddr_in	ser_addr;
    struct  sockaddr_in backup;	        /* In case cannot resolve hostname */
    int	    sd;
    int 	i;
    struct  hostent *h;
    struct  in_addr *p;

    if (first_call) 
    {
        cache_hostname[0] = '\0';
        first_call = FALSE;
    }

    if (strcmp(hostname, cache_hostname) != 0) 
    {
        /* Make a backup first */
        backup = ser_addr;

        /* Get IP address of the host */
        memset((char *) &ser_addr, 0, sizeof(ser_addr));
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_addr.s_addr = inet_addr(hostname);
        if ((long) ser_addr.sin_addr.s_addr == INADDR_NONE) 
        {
            h = gethostbyname(hostname);
            if (h != NULL) 
            {
                p = (struct in_addr *) (h->h_addr_list[0]);
                ser_addr.sin_addr.s_addr = p->s_addr;
            }
            else 
            {
                /* Cannot resolve address */
                /* Restore from backup */
                ser_addr = backup;
                logmsg("TCPT : [ %05d ] : Cannot resolve %s", getpid(), hostname);
                return -1;
            }
        }
        /* Cache hostname */
        strcpy(cache_hostname, hostname);
    }

    /* Then setup port for all cases */
    ser_addr.sin_port = htons(s_remote_port);

    /* Open socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        logmsg("TCPT : [ %05d ] : Cannot open socket", getpid());
        return -2;
    }

    if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr)) < 0) 
    {
        close(sd);
        logmsg("connect error %d, %s.", errno, strerror(errno));
        return -3;
    }

    return sd;
}

/*
 * ListenRemote() --
 *   para   --
 *   return --
 */
ListenRemote(short s_own_port)
{
    static 	struct sockaddr_in	ser_addr; /* Cached */
    static	int first_call = TRUE;
    int		sd;
    int		opt;

    if (first_call) 
    {
        memset(&ser_addr, 0, sizeof(ser_addr));
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        first_call = FALSE;
    }

    /* Set port for all cases */
    ser_addr.sin_port = htons(s_own_port);

    /* Get a socket */	
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) 
    {
        logmsg("TCPT : [ %05d ] sd = %d", getpid(), sd);
        return -1;
    }

    /* Set socket reuse address option */
    opt = 1;
    if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,
                (char *) &opt,sizeof(opt))<0) 
    {
        logmsg("TCPT : [ %05d ] : setsockopt error", getpid());
        close(sd);
        return -2;
    }

    if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr)) < 0) 
    {
        logmsg("TCPT : [ %05d ] Cannot bind server at %hd", getpid(), s_own_port);
        close(sd);
        return -3;
    }

    if (listen(sd, 5) < 0)
    {
        close(sd);
        return -4;
    }

    return sd;
}


/*
 * AcceptRemote() --
 *   para   -- sd: 初始socket号
 *   return -- -1: 失败, 0: 成功
 */
AcceptRemote(sd)
    int sd;
{
    struct 	sockaddr_in cli_addr = { AF_INET };
    int		clilen=0;
    int         newsd;

    clilen = sizeof(cli_addr);
    newsd = accept(sd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsd < 0) 
    {
        close(sd);
        return -1;
    }
    else 
    {
        /*  printf("remote ip=[%s]", inet_ntoa(cli_addr.sin_addr)); */
        close(sd);
        return newsd;
    }

}

/*
 * Save() -- 保存包文文件
 *   in  para -- name: 文件名
 *               ptr:  文件内容
 *               len:  文件长度
 *   out para -- 无
 *   return   -- -1: 失败, 0: 成功
 */
Save(name, ptr, len)
    char *name, *ptr;
    int len;
{
    FILE *fp;
    char file[20];

    sprintf(file, "%s", name);
    fp=fopen(file, "wb");
    fwrite(ptr, len, 1, fp);
    fclose(fp);
    return 0;
}
