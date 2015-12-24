#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "utils.h"

extern int tcp_client( char *servername, int serverport, char *sendbuff,
        int sendlen, char *recvbuff, int maxlen,
        int  *recvlen, int iTimeOut);

int CommWithBank(char *sendbuf, int sendlen, char *recvbuf, int *recvlen)
{
    char aczRecvMsg[2048];
    char *p, url[128];
    char ip[30], port[10];
    int timeout, rc;

    memset(url, 0, sizeof(url));
    if ((p = getenv("TEST_ADDR")) == NULL)
        strcpy(url, "127.0.0.1:5091:30");
    else
        strcpy(url, p);

    sscanf(url, "%[^:]:%[^:]:%d", ip, port, &timeout);

    /* 和大前置通讯 */
    //*recvlen = 0;
    memset(aczRecvMsg, 0, sizeof(aczRecvMsg));
    rc = tcp_client(ip, atoi(port), sendbuf, sendlen, 
            aczRecvMsg, sizeof(aczRecvMsg), recvlen, timeout);
    if (rc < 0)
    {
        printf("和%s:%s通讯失败(%d)", ip, port, rc);
        return -1;
    }
    printf("tcp_client ret = [ %d ].\n", rc);
    memcpy(recvbuf, aczRecvMsg, *recvlen);
    return 0;
}


int memfile(char *path, char *buffer)
{
    struct stat buf;
    caddr_t p;
    int fd;

    if ( stat( path, &buf ) == -1 )
    {
        printf("stat(%s) fail.\n", path);
        return -1;
    }
    if ( buf.st_size <= 0 )
    {
        printf("file %s size <= 0.\n", path);
        return -1;
    }

    fd = open( path, O_RDONLY, 0777 );
    if ( fd == -1 )
    {
        printf("open %s fail.\n", path );
        return -1;
    }

    p = mmap( NULL, (int)buf.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    if ( p == (caddr_t)-1 )
    {
        printf("mmap %s fail.\n", path );
        close( fd );
        return -1;
    }
    memcpy(buffer, p, buf.st_size);
    close( fd );
    munmap( p, buf.st_size );

    return buf.st_size;
}

int main(int argc, char *argv[])
{
    char aczBuf[1024];
    unsigned char buf[4096];
    int len, rcvlen;

    if (argc != 2)
    {
        printf("sop file not found.\n");
        exit(-1);
    }

    memset(buf, 0, sizeof(buf));
    if ((len = memfile(argv[1], buf)) <= 0)
    {
        printf("read file fail.\n");
        exit(-1);
    }

    memset(aczBuf,0,sizeof(aczBuf));
    rcvlen = len;
    CommWithBank(buf, len, aczBuf, &rcvlen);

    printf("retinfo=[%d][%s]\n", rcvlen, aczBuf);

    return 0;
}
