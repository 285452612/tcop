#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "utils.h"

#define   CONNECT_TIMEOUT       10
#define   READWRITE_TIMEOUT     30

int    svrsock;
static void r_alarm();

/*------------------------------------------*/
/* ��������tcpxchg()                        */
/* ��  ����                                 */
/*         tcpaddr - ���ӵ�ַ               */
/*         tcpport - ���Ӷ˿�               */
/*         snd - ��������                   */
/*         rcv - ��������,ΪNULLʱ����������*/
/*         len - ���͡����ճ���             */
/* ��  �أ�  0  -  �ɹ�                     */
/*          -1  -  ʧ��                     */
/*------------------------------------------*/
int tcp_call(char *tcpaddr, short tcpport,char *snd, char *rcv, int *len)
{
    struct sockaddr_in addr_in;
    int    len_tmp, n, ret;
    char   dlen[12], rcv_tmp[2048];

    err_log("����ҵ���к�̨ͨ��(IP=%s PORT=%d)...", tcpaddr, tcpport);
    svrsock=socket(AF_INET,SOCK_STREAM,0);
    if ( svrsock<0 ) {
        err_log("socket failed!");
        return(-1);
    }
    memset(&addr_in,0,sizeof(struct sockaddr_in));

    addr_in.sin_family=AF_INET;
    addr_in.sin_port=htons(tcpport);
    if ( inet_aton(tcpaddr,&addr_in.sin_addr) != 1 )  // ��ַ����ȷ 
    {
        err_log("inet_aton failed!");
        return(-1);
    }

    signal(SIGALRM,r_alarm);
    alarm(CONNECT_TIMEOUT);
    // ����
    if ((ret=connect(svrsock,(struct sockaddr *)&addr_in,sizeof(addr_in))) < 0)
    {
        if ( errno == EINTR )
            err_log("socket connect timeout!");
        else
            err_log("connect failed!");
        alarm(0);
        close(svrsock);
        return(-1);
    }

    signal(SIGALRM,r_alarm);
    alarm(READWRITE_TIMEOUT);
    sprintf( dlen, "%010d", *len );
    if( write( svrsock, dlen, 10 ) <= 0 )
    {
        err_log("send data length error" );
        alarm( 0 );
        close( svrsock );
        return( -1 );
    }

    if ( write(svrsock,snd,*len) <= 0 )  // ����
    {
        if ( errno == EINTR )
            err_log("socket write timeout!", 0);
        else
            err_log("write failed! errno=(%d)",errno);

        close(svrsock);
        return(-1);
    }
    err_log(" send %d bytes ... success!", *len);

    *len=0;
    // �������rcvΪNULL,��ʾ����Ҫ�������� ��رն˿�,���˳�
    if (rcv == NULL)
    {
        close(svrsock);
        return 0;
    }

    signal(SIGALRM,r_alarm);
    alarm(READWRITE_TIMEOUT);

    memset( dlen, 0, sizeof(dlen) );
    if( (len_tmp = read(svrsock, dlen, 10 ))<10 )
    {
        err_log("recv length error. %s", strerror(errno) );
        alarm(0);
        close( svrsock );
        return( -1 );
    }
    *len=atol(dlen);
    len_tmp=0;
    while( len_tmp < *len )
    {
        memset(rcv_tmp, 0, sizeof(rcv_tmp));
        if ( (n=recv(svrsock, rcv_tmp, *len-len_tmp, 0)) <= 0 )  // ����
        {
            if ( errno == EINTR )
                err_log("socket read timeout!", 0);
            else
                err_log("1.read failed! %s", strerror(errno) );

            close(svrsock);
            break;
        }

        memcpy( rcv+len_tmp, rcv_tmp, n );
        len_tmp += n;
    }
    err_log("recv %d bytes ... success!", *len);

    close(svrsock);
    return( 0 );
}

static void r_alarm()
{
    close(svrsock);
    return;
}
