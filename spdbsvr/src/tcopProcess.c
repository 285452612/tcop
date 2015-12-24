#include "app.h"
#include "tcpapi.h"

int tcopProcess(char *nodeid, char *opptrcode, 
        char *reqbuf, int reqlen, char *rspbuf, int *rsplen)
{
    xmlDoc *opDoc = NULL;
    char *pRspFile = NULL;
    int  len = 0;
    int  ret = 99;
    char tmp[100];

    strcpy(OP_HOME, getHome());

    //���տͻ�������  �����ļ��� $FILES_DIR/G_REQFILE

    DBUG("���յ�������:%s", reqbuf);

    if ((ret = initApp(nodeid, opptrcode, reqbuf)) != 0)
    {
        INFO("Ӧ�ó����ʼ��ʧ��!node=[%s]ret=[%d]", nodeid, ret);
        goto EXIT;
    }

    len = reqlen;
    //Ӧ�ô���
    ret = appsMain(&opDoc, reqbuf, rspbuf, &len);

    INFO("RSP:[%d-%d]----------------------------------[%d][%d]", OP_NODEID, OP_TCTCODE, ret, len);

    if (ret < 0)
        goto EXIT;

    //δ����Ľ���(ת������)����Ҫ�����쳣����
    if (OP_APPTYPE != APPTYPE_UNDEF && opException(opDoc, rspbuf, &len, ret) != 0)
    {
        //*rsplen = len;
        if( len != strlen(rspbuf))
            *rsplen = strlen(rspbuf);
        else
            *rsplen = len;
        goto EXIT;
    }
    //*rsplen = len;
    if( len != strlen(rspbuf))
        *rsplen = strlen(rspbuf);
    else
        *rsplen = len;

    /*
    //��Ӧ�ͻ���
    memset(&head, 0, sizeof(head));
    head.Sleng = len;
    head.PackInfo |= htonl(PI_DCOMPRESS);

    if (strlen(G_RSPFILE) > 0)
    {
    head.Ftype = 'B';
    head.PackInfo |= htonl(PI_FCOMPRESS);
    pRspFile = G_RSPFILE;
    }
    {
    int tmpret = -1;

    if ((tmpret = svr_snd(&head, rspbuf, pRspFile, 0)) != 0)
    ret = E_SYS_COMM_PH;

    INFO("������Ӧ���ݳ���[%d]�ļ�[%s]%s!node=[%d]ret=[%d]", len, 
    pRspFile == NULL ? "" : pRspFile, (tmpret == 0 ? "�ɹ�" : "ʧ��"), OP_NODEID, tmpret);
    }
     */

EXIT:

    OP_DoFinish(opDoc, ret);

    INFOLINE();

    CloseOPDB();

    return ret;
}

