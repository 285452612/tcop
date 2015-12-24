/**
  * ���ݵ������뽻�׺�̨��ʱDaemonת������
  */

#include <signal.h>
#include <time.h>
#include <SDKipc.h>
#include "cz_const.h"

static char OP_ADDR[48] = {0};
static int OP_PORT = 0;

/* 
 * ����Ƿ���ͬ���������� 
 * �ɹ� 0 ʧ�� -1
 */
int isDuplicate(const char *process_name)
{
    struct Process *proc;
    int i;

    if ((proc = GetProcess((char *)getenv("LOGNAME"), (char *)process_name, &i))
            != (struct Process *)NULL && i > 1)
    {
        fprintf(stderr, "Other process %s is running!", process_name);
        return 1;
    }
    return 0;
}

/* 
 * ʹ�������ػ���ʽ����
 * �ɹ� 0 ʧ�� -1
 */
int daemonInit(const char *processName)
{
    pid_t pid = 0;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    if ((pid = fork()) < 0) {
        INFO("%s START FAILED!!![%s]", processName, strerror(errno));
        return -1;
    } else if (pid > 0)
        exit(0);

    if (setpgrp() < 0) {
        INFO("%s CHANGE GROUP ERROR!!![%s]", processName, strerror(errno));
        return -1;
    }
    signal(SIGHUP, SIG_IGN);

    return 0;
}

int sendTran(TransInfo *ptranInfo, char *buf, int len)
{
    TCPHEAD head;
    char rspbuf[2048] = {0};
    int i = 0;

    memset(&head, 0, sizeof(head));
    sprintf(head.TrType, "%04d", ptranInfo->commcode);
    head.Sleng = len;
    head.PackInfo |= htonl(PI_DCOMPRESS);

    INFO("����[%d]��ڵ�[%d]��ʼͨѶ...", ptranInfo->commcode, OP_NODEID);

    DBUG("begin comm to node [%d]:addr=[%s]port=[%d]trcode=[%d]Sleng=[%d]", 
            OP_NODEID, OP_ADDR, OP_PORT, ptranInfo->commcode, head.Sleng);
    DBUG("���ͱ���:\n%s]", buf);

    if (cli_sndrcv(OP_ADDR, OP_PORT, &head, buf, NULL, rspbuf, NULL, 0) < 0)
    {
        INFO("����[%d]��ڵ�[%d(%s:%d)]ͨѶʧ��,������=[%s]", 
                ptranInfo->commcode, OP_NODEID, OP_ADDR, OP_PORT, head.RetCode);
        return -1;
    }
    len = head.Sleng;

    DBUG("end comm to node, rsplen [%d]", len); 
    DBUG("���ձ���:\n%s]", rspbuf);

    INFO("����[%d]��ڵ�[%d]ͨѶ�ɹ�", ptranInfo->commcode, OP_NODEID);

    return 0;
}

int transHandle(int tctcode, char *buf)
{
    char tmpNoteno[11] ={0};;
    char tmpAmt[13] ={0};
    char tmpNotetype[3]={0};

    switch (tctcode) {
        case 12:
            buf[190] = (buf[190] == '0' ? '1' : '2'); //�����־ת��
            break;
    }

    return 0;
}

void SaveData(char *tctcode, char *data, int len)
{
    FILE *fp = NULL;
    char filename[128] = {0};
    char buf[2048] = {0};
    int wlen = 0;
    long serial = 0;
    int i = 0;

    serial = GenSerial("innote", 1, 999999999, 1);
    sprintf(filename, "%s/data/%s_%s.dat", OP_HOME, getDate(0), tctcode);
    if ((fp = fopen(filename, "a+")) == NULL)
        return;
    data[len] = '\n';
    wlen = sprintf(buf, "%ld:%s|", serial, getTime(0));
    memcpy(buf+wlen, data, len+1);
    fwrite(buf, wlen+len+1, 1, fp);
    fclose(fp);
    INFO("�������ݳɹ�,��ˮ:%ld �ļ�:%s", serial, filename);
}

void runServer(ExchgInfo *pTransInfo)
{
    ExchgInfo *psvrInfo = NULL;
    int ret = 0;
    char buf[2048] = {0};
    char errmsg[256] = {0};
    int len = 0;
    int i = 0, j = 0, k = 0;
    long begTime = 0, endTime = 0;;

    while (1) {
        begTime = time(NULL);
        for (j = 0; j < sizeof(G_ExchgInfos)/sizeof(ExchgInfo); j++) {
            if (G_ExchgInfos[j].exchgNo[0] == 0)
                break;
            psvrInfo = &G_ExchgInfos[j];
            for (i = 0; i < sizeof(G_TransInfos)/sizeof(TransInfo); i++) {
                if (G_TransInfos[i].commcode == 0)
                    break;
                len = 0;
                memset(buf, 0, sizeof(buf));
                memset(errmsg, 0, sizeof(errmsg));
                ret = SwitchRecord(psvrInfo->prehostAddr, psvrInfo->prehostPort, G_TransInfos[i].tctcode, buf, len, &len, errmsg);
                if (ret != 10) {
                    INFO("REQ:[%s-%s]----------------------------------", psvrInfo->exchgNo, G_TransInfos[i].tctcode);
                    INFO("��ȡ[%s]����,ret=[%d]msg=[%s]buflen[%d]", G_TransInfos[i].tctcode, ret, errmsg, len);
                }
                if (ret)
                    continue;
                for (k = 0; k < len; k++) 
                    if (buf[k] == 0) buf[k] = ' ';
                SaveData(G_TransInfos[i].tctcode, buf, len);
                ret = transHandle(atoi(G_TransInfos[i].tctcode), buf);
                ret = sendTran(&G_TransInfos[i], buf, len);
                INFO("RSP:[%s-%s]----------------------------------[%d]", psvrInfo->exchgNo, G_TransInfos[i].tctcode, ret);
                INFOLINE();
                i--; //����������ȡͬһ�����뽻��
            }
        }
        endTime = time(NULL);
        if (endTime - begTime < 60) 
            sleep(60 - (endTime - begTime));
    }
}

int main(int argc, char *argv[])
{
    pid_t pid = 0;
    char tmp[20] = {0};
    int ret = 0;
    int i = 0;

    OP_NODEID = 999;
    strcpy(OP_HOME, getenv("HOME"));
    sprintf(tmp, "%ld", OP_NODEID);

    if (tapi_getaddr(tmp, OP_ADDR, &OP_PORT) < 0) {
        fprintf(stderr, "ȡ�ڵ�[%s]ͨ�Ų���ʧ��!", tmp);
        return 0;
    }

    if ((isDuplicate(argv[0]) != 0) || (daemonInit(argv[0]) != 0)) {
        fprintf(stderr, "daemonInit failed!\n");
        return 0;
    }

    if ((ret = initSysConfig(NULL, 2)) != 0) {
        fprintf(stderr, "��ʼ������ϵͳ���ó���,%d\n", ret);
        return 0;
    }

    /*
       for (i = 0; i < sizeof(G_ExchgInfos)/sizeof(ExchgInfo); i++) {
       if (G_ExchgInfos[i].exchgNo[0] == 0)
       break;

       if ((pid = fork()) == -1) {
       fprintf(stderr, "FORK FAILED, Parent Continue! %s", strerror(errno));
       return -1;
       } else if (pid) {
       INFO("ǰ�û���[%s]�����ӽ���:pid=[%d]", G_ExchgInfos[i].exchgNo, pid);
       continue;
       }
       runServer(&G_ExchgInfos[i]);
       }
     */
    INFO("���봦����������ɹ�...");

    runServer(&G_ExchgInfos[0]);

    return 0;
}
