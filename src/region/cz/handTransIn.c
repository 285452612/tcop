/**
  * ���ݵ����ֹ��������뽻��
  */

#include <time.h>
#include <SDKipc.h>
#include "cz_const.h"

static char OP_ADDR[48] = {0};
static int OP_PORT = 0;

int transHandle(TransInfo *ptranInfo, char *buf, int len)
{
    TCPHEAD head;
    char tmp[2048] = {0};
    char rspbuf[2048] = {0};
    int i = 0;

    memset(&head, 0, sizeof(head));
    sprintf(head.TrType, "%04d", ptranInfo->commcode);
    head.Sleng = len;
    head.PackInfo |= htonl(PI_DCOMPRESS);

    INFO("�ֹ�����[%d]��ڵ�[%d]��ʼͨѶ...", ptranInfo->commcode, OP_NODEID);

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

int main(int argc, char *argv[])
{
    char tmp[256] = {0};
    char serial[10] = {0};
    int ret = 0;
    FILE *fp = NULL;
    char buf[2048] = {0};
    int len = 0;
    char *p = NULL;
    int i = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s datafile serial\n", argv[0]);
        return 0;
    }

    strcpy(OP_HOME, getenv("HOME"));

    sprintf(tmp, "%s/data/%s", OP_HOME, argv[1]);
    if ((fp = fopen(tmp, "r")) == NULL) {
        fprintf(stderr, "���ļ�[%s]ʧ��!%s\n", tmp, strerror(errno));
        return 0;
    }

    sprintf(serial, "%s:", argv[2]);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if ((p = strstr(buf, serial)) == NULL || p != buf) {
            memset(buf, 0, sizeof(buf));
            continue;
        }
        len = strlen(buf) - 1;
        buf[len] = 0;
        break;
    }
    fclose(fp);

    if (len == 0) {
        fprintf(stderr, "δ�ҵ���Ӧ�ļ�¼:%s|%s\n", argv[1], argv[2]);
        return 0;
    }

    OP_NODEID = 999;
    sprintf(tmp, "%ld", OP_NODEID);

    if (tapi_getaddr(tmp, OP_ADDR, &OP_PORT) < 0) {
        fprintf(stderr, "ȡ�ڵ�[%s]ͨ�Ų���ʧ��!\n", tmp);
        return 0;
    }

    if ((ret = initSysConfig(NULL, 2)) != 0) {
        fprintf(stderr, "��ʼ������ϵͳ���ó���,%d\n", ret);
        return 0;
    }

    memset(tmp, 0, sizeof(tmp));
    p = strchr(argv[1], '_')+1;
    memcpy(tmp, p, strchr(argv[1], '.') - p);

    for (i = 0; i < sizeof(G_TransInfos)/sizeof(TransInfo); i++) {
        if (strcmp(G_TransInfos[i].tctcode, tmp) == 0)
            break;
    }
    if (G_TransInfos[i].tctcode[0] == 0) {
        fprintf(stderr, "δ�ҵ�ͬ�ǽ���[%s]��Ӧ��ͨѶ��\n", tmp);
        return 0;
    }

    transHandle(&G_TransInfos[i], buf+strlen(serial)+7, len-strlen(serial)-7);

    return 0;
}
