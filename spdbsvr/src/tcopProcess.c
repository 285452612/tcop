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

    //接收客户端请求  接收文件名 $FILES_DIR/G_REQFILE

    DBUG("接收到请求报文:%s", reqbuf);

    if ((ret = initApp(nodeid, opptrcode, reqbuf)) != 0)
    {
        INFO("应用程序初始化失败!node=[%s]ret=[%d]", nodeid, ret);
        goto EXIT;
    }

    len = reqlen;
    //应用处理
    ret = appsMain(&opDoc, reqbuf, rspbuf, &len);

    INFO("RSP:[%d-%d]----------------------------------[%d][%d]", OP_NODEID, OP_TCTCODE, ret, len);

    if (ret < 0)
        goto EXIT;

    //未定义的交易(转发交易)不需要进行异常处理
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
    //响应客户端
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

    INFO("发送响应数据长度[%d]文件[%s]%s!node=[%d]ret=[%d]", len, 
    pRspFile == NULL ? "" : pRspFile, (tmpret == 0 ? "成功" : "失败"), OP_NODEID, tmpret);
    }
     */

EXIT:

    OP_DoFinish(opDoc, ret);

    INFOLINE();

    CloseOPDB();

    return ret;
}

