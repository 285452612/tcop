#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "utils.h"
#include "prefs.h"
#include "tcpt.h"
#include "remote_st.h"

#define WAITTCPTIME  30

extern int tcopProcess(char *nodeid, char *opptrcode, 
        char *reqbuf, int reqlen, char *rspbuf, int *rsplen);
extern void save_pack(char *suffix, char *buf, int len);

static  void ProcTimeout();
static  void StopRun();
static void sig_chld(int signo);

static char PID_FILE[256];

int   _nsd = -1;                        /* SOCKET��*/
int   _nDebug;
char *_sPrgName;

int  giLocalPort = 42001; // ����˿�
int  gimaxProcNums; // ��󲢷�������
char gsRemoteIp[30] = {0};
int  giRemotePort = -1;
char gsNodeId[30] = {0};


static int ReadProfile()
{
    ProfilePtr cfg;
    char path[256];
    char *p = NULL;
    int *i = NULL;

    snprintf(path, sizeof(path), "%s/etc/app.conf", getAppDir());
    if ((cfg = OpenProfile(path)) == NULL)
        ERROR_MSG("�޷��������ļ�: %s.", path);
    if ((i = Profile(cfg, "struct", "MaxProcNums", PREF_INT)) == NULL)
    {
        PRINT("server/MaxProcNums not found, in %s.", path);
        CloseProfile(cfg);
        return -1;
    }
    gimaxProcNums = *i;
    free(i); i = NULL;

    if ((i = Profile(cfg, "struct", "port", PREF_INT)) == NULL)
    {
        PRINT("server/port not found, in %s.", path);
        CloseProfile(cfg);
        return -1;
    }
    giLocalPort = *i;
    free(i); i = NULL;

    if ((p = Profile(cfg, "struct", "nodeid", PREF_STRING))==NULL)
    {
        PRINT("server/nodeid not found, in %s.", path);
        CloseProfile(cfg);
        return -1;
    }
    memset(gsNodeId, 0, sizeof(gsNodeId));
    strcpy(gsNodeId, p);
    free(p); p = NULL;

#if 0
    if ((i = Profile(cfg, "hostinfo", "port", PREF_INT)) == NULL)
    {
        CloseProfile(cfg);
        return -1;
    }
    giRemotePort = *i;
    free(i); i = NULL;

    if ((p = Profile(cfg, "hostinfo", "address", PREF_STRING))==NULL)
    {
        CloseProfile(cfg);
        return -1;
    }
    memset(gsRemoteIp, 0, sizeof(gsRemoteIp));
    strcpy(gsRemoteIp, p);
    free(p); p = NULL;
#endif

    CloseProfile(cfg);
    return 0;
}

/**************
 *    CAUTIONS: tcp_server [-p localport] [-d] 
 **************/
int main (int argc, char **argv)
{
    int	 i, ret, status;

    _sPrgName=argv[0];

    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    if ((ret = fork()) < 0)
        ERROR_MSG("%s: SERVER START FAILED!!!", _sPrgName);
    else if (ret > 0)
        exit(0);

    if (setpgrp() < 0)
        ERROR_MSG("%s: CHANGE GROUP ERROR!!!", _sPrgName);

    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, StopRun);
    signal(SIGCHLD, sig_chld);

    // ����Ƿ����ظ�����
    sprintf(PID_FILE, "%s/.spdbsvr_st.pid", getenv("HOME"));
    pid_check(PID_FILE);

    // ��ȡ�����ļ�����
    if (ReadProfile() != 0)
        ERROR_MSG("%s: Read profile failed!!!", _sPrgName);

    // ��ʼ�����̿����ڴ���
    if (InitProcManage(PID_FILE) != 0)
        ERROR_MSG("%s: InitProcManage Error!!!", _sPrgName);

    printf("%s: interface SERVER START OK!!!\n", _sPrgName);
    fflush(stdout);
    // ��ѭ��
    while (1)
    {
        // �ȴ�ǰ̨����
        if ((_nsd=InstallLink(giLocalPort)) == -1)
        {
            CloseLink();
            continue;
        }

        // FORK�ӽ��̴���
        if ((ret=fork()) == 0)
        {
            /*
            // ��ʼ��sop����
            if (InitConEnv(USE_COP) != 0)
            {
            err_log("InitConEnv() fail, process closed.");
            CloseLink();
            exit(0);
            }
             */

            // ��������
            DoProcess();
            CloseLink();
            // �ͷ�sop����
            //free_sop();
            exit(0);
        }
        else if (ret < 0)
        {
            err_log("%s fork() err, waitting 3s.", _sPrgName);
            sleep(3);
            CloseLink();
        }
        else 
        {
            close(_nsd);
        }
    }
}

/**************
 *        NAME: DoProcess()
 * DESCRIPTION: �������
 *      INPUTS: ��
 *     OUTPUTS: ��
 *     RETURNS: ��
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
int DoProcess()
{
    int	 len, ret;
    int  curProcNums;
    char sBuff[MMAX+1];
    char mBuff[MMAX+1];
    char newBuff[MMAX+1];
    char opptrcode[10];
    char pktfmt[10];
    int errcode;
    char errinfo[83];

    // ���ƽ�������
    curProcNums = UpdateProcNum(1);
    if (curProcNums > gimaxProcNums || curProcNums < 0)
    {
        UpdateProcNum(-1);
        err_log("%s: ����æ.", _sPrgName);
        return -1;
    }

    // ȡ����
    err_log("%s Ready RecvReqData...", _sPrgName);
    memset(sBuff, 0, sizeof(sBuff));
    len = RecvReqData(_nsd, sBuff, MMAX, WAITTCPTIME);

    err_log("%s Receive ReqData len = [%d]", _sPrgName, len);
    if (len <= 0) 
    {
        UpdateProcNum(-1);
        err_log("%s RecvReqData()Err!!!len = [%d]", _sPrgName, len);
        return -1;
    }
    memset( mBuff, 0, sizeof(mBuff));
    memcpy( mBuff, sBuff, sizeof(sBuff) );
    do
    {
        err_log("ready strcut2xml() ...");
        memset(newBuff, 0, sizeof(newBuff));

        memcpy(opptrcode, sBuff, 4);
        opptrcode[4]=0x00;
        err_log("BANK TRNCODE[%s]...", opptrcode);

        len = struct2tc_req(opptrcode, newBuff, sBuff, len);
        //free
        if (len <= 0)
        {
            err_log("structתxml����ʧ��");
            break;
        }

        err_log("TCOP TRNCODE[%s]...", opptrcode);
        err_log("ready tcopProcess() ...");
        memset(sBuff, 0, sizeof(sBuff));
        if (tcopProcess(gsNodeId, opptrcode, newBuff, len, sBuff, &len) < 0)
        {
            err_log("�ӿ�ƽ̨����ʧ��");
            break;
        }

        err_log("ready xml2struct(%s),len=[%d] ...", sBuff, len);
        len = tc2struct_rsp(opptrcode, mBuff, sBuff, len);

        if (len <= 0)
        {
            err_log("xmlתstruct����ʧ��");
            break;
        }
        memset( newBuff, 0, sizeof(newBuff) );
        memcpy( newBuff, mBuff, len);
        err_log("ת�����:[%s].", newBuff);
        newBuff[len]=0x00;
        break;
    } while(0);

    if(len>0)
    {
        /* �ͷ��ذ� */
        save_pack("rsp", newBuff, len);
        if ((ret = SendRespData(_nsd, newBuff, len)))
        {
            err_log("%s SendRespData Err!!!! ret = [%d]", _sPrgName, ret);
        }
    }

    // ���ٽ�����
    UpdateProcNum(-1);
    return 0;
}

/**************
 *        NAME: RecvReqData(P_Sd, P_Buffer, P_MaxLen, P_WaitTime)
 * DESCRIPTION: ����ǰ��TCP/IP��������
 *      INPUTS: int P_Sd;  TCPIP SOCKET 
 *              int P_MaxLen; ����������
 *              int P_WaitTime; �ȴ�ʱ��
 *     OUTPUTS: char P_Buffer; ���ջ�����
 *     RETURNS: -1: ����ʧ�� 
 *              >0: �ɹ������ճ���
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
int RecvReqData( int P_Sd, char *P_Buffer, int P_MaxLen, int P_WaitTime)
{
    int n_Len = -1;

    signal(SIGALRM, ProcTimeout);
    alarm(P_WaitTime);
    n_Len = GetMessage(P_Sd, P_Buffer, P_MaxLen);
    signal(SIGALRM, SIG_IGN);
    alarm(0);

    err_log("%s Get Pack: [len=%d]", _sPrgName, n_Len);
    if (n_Len < 0)
    {
        err_log("%s GetMessage Error, len=[%d]", _sPrgName, n_Len);
        CloseLink();
        return -1;
    }
    save_pack("req", P_Buffer, n_Len);
    /*
    memmove(P_Buffer, P_Buffer+52, n_Len-52);
    //P_Buffer=52;
    n_Len-=52;
    */
    return n_Len;
}


/**************
 *        NAME: SendRespData()
 * DESCRIPTION: ����ǰ��TCP/IP��������
 *      INPUTS: int P_Sd;  TCPIP SOCKET 
 *              char *P_Buffer;�������ݻ�����
 *              int P_Len; ���ݳ���
 *     OUTPUTS: ��
 *     RETURNS: -1: ����ʧ�� 
 *              0: �ɹ�
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
int SendRespData( int P_Sd, char *P_Buffer, int P_Len)
{
    int len;
    unsigned char head_buf[10];

    memset(head_buf, 0, sizeof(head_buf));
    memcpy(head_buf, P_Buffer, 2);
    len = ntohs(*(unsigned short *)&head_buf);

    if (PutPackMessage(P_Sd, P_Buffer, P_Len) != P_Len)
    {
        err_log("%s PutMessage Error", _sPrgName);
        return -1;
    }
    err_log("%s Put Pack[%d] Success Len=[%d]", _sPrgName, len, P_Len);
    return 0;
}


/**************
 *        NAME: InstallLink() 
 * DESCRIPTION: �ȴ�TCP����
 *      INPUTS: port: �˿ں�
 *     OUTPUTS: ��
 *     RETURNS: -1: ʧ��, >0: �ɹ�SOCKET��
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
int InstallLink(int port)
{
    int initsd, newsd;

    err_log("%s Begin ListenRemote...", _sPrgName);
    if ((initsd = ListenRemote(port)) < 0)
    {
        err_log("%s ListenRemote Failure. initsd=[%d]", _sPrgName, initsd);
        return -1;
    } 
    if ((newsd = AcceptRemote(initsd)) < 0)
    {
        err_log("%s AcceptRemote Failure", _sPrgName);
        return -1;
    }
    close(initsd);
    return newsd;
}


/**************
 *        NAME: CloseLink()
 * DESCRIPTION: �ر�TCP����
 *      INPUTS: ��
 *     OUTPUTS: ��
 *     RETURNS: -1: ʧ��, 0: �ɹ�
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
CloseLink()
{
    if (_nsd > 0)
    {
        shutdown(_nsd, 2);
        close(_nsd);
        _nsd = -1;
    }
    return 0;
}

/**************
 *        NAME: ProcTimeout()
 * DESCRIPTION: ��ʱ����
 *      INPUTS: ��
 *     OUTPUTS: ��
 *     RETURNS: ��
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
static void ProcTimeout()
{
    signal(SIGALRM, SIG_IGN);
    alarm(0);
    return;
}

/**************
 *        NAME: StopRun()
 * DESCRIPTION: �ر�ԭ�ӽ���
 *      INPUTS: ��
 *     OUTPUTS: ��
 *     RETURNS: ��
 *       CALLS: ��
 *    CAUTIONS: 
 **************/
static void StopRun()
{
    CloseLink();
    exit(0); 
}

static void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        err_log("child %d terminated.", pid);
    return;
}

static int my_readn(int sd, char *ptr, int nbyte)
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

    return (nbyte - nleft);
}
int GetMessage(int sockfd, char *buf, int max)
{
    const unsigned short head_len = 52;
    unsigned short buf_len;
    unsigned char head_buf[53]={0};
    int bytes_read;
    int readed;

#if 1
    // ����ͷ���� ǰhead_len���ֽ�
    bytes_read = head_len;
    if ((readed = my_readn(sockfd, head_buf, bytes_read)) != bytes_read)
    {
        err_log("recv head error, %s.", strerror(errno));
        return -1;
    }
    //err_log("recv head:[%s]",head_buf);
    /*
    buf_len = ntohs(*(unsigned short *)&head_buf);
    err_log("pack len=[%d]", buf_len);
    memcpy(buf, head_buf, head_len);
    */
    sscanf(head_buf+10,"%d",&buf_len);
    err_log("get body len=[%d]", buf_len);
#endif

    // ������
    bytes_read = buf_len;
    /*
       if (bytes_read > max - readed)
       {
       err_log("recv buffer not enough, size=[%d]!", bytes_read+readed);
       return -1;
       }
     */
    if ((readed = my_readn(sockfd, buf, bytes_read)) != bytes_read)
    {
        err_log("recv pack error, %s.", strerror(errno));
        return -1;
    }
    *(buf+readed) = 0x00;

    return readed;
}
