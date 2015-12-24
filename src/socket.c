#include <setjmp.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/select.h>

#include "comm.h"

static jmp_buf jmpenv;
static void sig_alarm();
static void resettimer();

static int CONNECT(const char *ipaddr, int port);
static int SEND(int sock, void *buffer, int length);
static int RECV(int sock, void *buffer, int length);
static int RECV_MOST(int sock, void *buffer, int length);

int CommToBankNative(char *addr, int port, BankCommData *preq, 
        BankCommData *prsp, FPGetRecvBodyLength func, int timeout)
{
    int sock = -1;
    char *tmpfile = NULL, *flist = NULL;
    int filenum = 0;

    signal(SIGALRM, sig_alarm);
    alarm(timeout);

    if (setjmp(jmpenv) != 0)
    {
        INFO("与行内通讯超时!TIMEOUT=[%d]", timeout);
        signal(SIGALRM, SIG_DFL);
        return -1;
    }

    if ((sock = CONNECT(addr, port)) < 0)
    {
        INFO("Connect to BANK error!ADDR=[%s:%d]", addr, port);
        resettimer();
        return -1;
    }

    DBUG("reqHeadLen=[%d]reqBodyLen=[%d]reqTailLen=[%d]rspHeadLen=[%d]rspBodyLen=[%d]rspTailLen=[%d]", 
            preq->headlen, preq->bodylen, preq->taillen, prsp->headlen, prsp->bodylen, prsp->taillen);

    if (preq->headlen > 0) 
    {
        if (SEND(sock, preq->head, preq->headlen) < 0)
        {
            INFO("Send head_data error!headlen=[%d]", preq->headlen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    DBUG("Begin to send req_data");

    /* 发送报文数据 */
    if (SEND(sock, preq->body, preq->bodylen) < 0)
    {
        INFO("Send req_data error!bodylen=[%d]", preq->bodylen);
        close(sock);
        resettimer();
        return -1;
    }

    if (preq->taillen > 0) 
    {
        if (SEND(sock, preq->tail, preq->taillen) < 0)
        {
            INFO("Send tail_data error!taillen=[%d]", preq->taillen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    /* 发送文件 */
    if (preq->files != NULL && strlen(preq->files) != 0)
    { 
        tmpfile = strdup(preq->files);
        while ((tmpfile = strtok(tmpfile, "+")) != NULL)
        {
            if (sendfile(sock, tmpfile) < 0)
            {
                INFO("Send file [%s] error!", tmpfile);
                close(sock);
                resettimer();
                return -1;
            }
            filenum++;
            tmpfile = NULL;
        }
        INFO("Send [%d] files [%s] success!", filenum, preq->files);
    }

    /* 接收响应数据头报文 */
    if (prsp->headlen > 0)
    {
        if (RECV(sock, prsp->head, prsp->headlen) < prsp->headlen)
        {
            INFO("接收响应数据头失败!headlen=[%d]", prsp->headlen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    if (func != NULL)
    {
        if ((prsp->bodylen = func(prsp->head)) <= 0)
        {
            INFO("获取响应数据长度调用失败!ret=[%d]", prsp->bodylen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    /* 接收响应数据报文 */
    if (prsp->bodylen == 0) {
        if ((prsp->bodylen = RECV_MOST(sock, prsp->body, BLKSIZE)) <= 0) 
        {
            INFO("接收响应数据失败!");
            close(sock);
            resettimer();
            return -1;
        }
    } else {
        if (RECV(sock, prsp->body, prsp->bodylen) < prsp->bodylen)
        {
            INFO("接收响应数据失败!bodylen=[%d]", prsp->bodylen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    DBUG("accept data success, recved length=[%d]", prsp->bodylen);
    DBUG("res_data=[%s]", prsp->body);

    if (prsp->taillen > 0)
    {
        if (RECV(sock, prsp->tail, prsp->taillen) < prsp->taillen)
        {
            INFO("接收响应数据尾失败!taillen=[%d]", prsp->taillen);
            close(sock);
            resettimer();
            return -1;
        }
    }

    close(sock);

    resettimer();

    if (flist != NULL)
        strcpy(prsp->files, flist);

    return 0;
}

void sig_alarm()
{
    longjmp(jmpenv, 1);
}

void resettimer()
{
    signal(SIGALRM, SIG_DFL);
    alarm(0);
}

int CONNECT(const char *ipaddr, int port)
{
    int sock;
    struct sockaddr_in sin;
    struct hostent *h;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        INFO("socket create error![%s]", strerror(errno));
        return -1;
    }

    sin.sin_family=AF_INET;

    if (!IsAddress(ipaddr))
    {
        if ((h = gethostbyname(ipaddr)) == NULL)
        {
            INFO("gethostbyname error[%d]", strerror(errno));
            return -1;
        }
        memcpy(&sin.sin_addr, h->h_addr, h->h_length);
    } else
        sin.sin_addr.s_addr = inet_addr(ipaddr);

    sin.sin_port = htons(port);
    memset(&(sin.sin_zero), 0, sizeof(sin.sin_zero));

    DBUG("begin to connect:[%s][%d]", ipaddr, port);

    if (connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr)) < 0)
    {
        INFO("connect error!errno[%d]:%s", errno, strerror(errno));
        close(sock);
        return -1;
    }

    DBUG("connected!");

    return sock;
}


int SEND(int sock, void *buffer, int length)
{
    int ret, sended = 0;

    while (sended < length)
    {
        if ((ret = send(sock, (void*)((long)buffer + sended), length-sended, 0)) < 0)
        {
            INFO("send data error[%s]", strerror(errno));
            return -1;
        }
        sended += ret;
    }
    return sended;
}

int RECV(int sock, void *buffer, int length)
{
    int ret, recved = 0;

    while (recved < length)
    {
        if ((ret = recv(sock, (void*)((long)buffer+recved), length-recved, 0)) <= 0)
        {
            INFO("recv data error:[%s]", ret < 0 ? strerror(errno) : "socket closed by peer");
            return -1;
        }
        recved += ret;
        DBUG("need recved length=[%d]recved=[%d]", length, recved);
    }

    return recved;
}

int RECV_MOST(int sock, void *buffer, int length)
{
    ssize_t retval;

    while (retval = read(sock, buffer, length), retval == -1 && errno == EINTR)
        ;
    DBUG("recved length=[%d]", retval);

    return retval;
}
