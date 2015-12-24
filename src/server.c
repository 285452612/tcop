#include "app.h"
#include "tcpapi.h"

int main(int argc, char *argv[]) 
{
    TAPIHEAD head;
    xmlDoc *opDoc = NULL;
    char reqbuf[100*COMMPACK_MAXLEN] = {0};
    char rspbuf[100*COMMPACK_MAXLEN] = {0};
    char *pRspFile = NULL;
    int  len = 0;
    int  ret = 99;

    if (argc > 1 && strcasecmp(argv[1], "-v") == 0)
    {
        fprintf(stderr, "tcop V2.0\n");
        return 0;
    }

    izero(reqbuf);
    izero(rspbuf);
    memset(&head, 0, sizeof(head));

    strcpy(OP_HOME, getHome());

    //接收客户端请求
    if ((ret = svr_rcv(&head, reqbuf, G_REQFILE, 0)) < 0)
    {
        INFO("接收请求数据失败!node=[%s]ret=[%d]", head.NodeId, ret);
        goto EXIT;
    }

    DBUG("接收到请求报文:%d\n%s", head.Sleng, reqbuf);

    INFO("REQ:[%s-%s]----------------------------------", head.NodeId, head.TrType);

    if ((ret = initApp(head.NodeId, head.TrType, reqbuf)) != 0)
    {
        INFO("应用程序初始化失败!node=[%s]ret=[%d]", head.NodeId, ret);
        goto EXIT;
    }

    len = head.Sleng;
    //应用处理
    ret = appsMain(&opDoc, reqbuf, rspbuf, &len);

    INFO("RSP:[%d-%d]----------------------------------[%d][%d]", OP_NODEID, OP_TCTCODE, ret, len);

    if (ret < 0)
        goto EXIT;

    //未定义的交易(转发交易)不需要进行异常处理
    if (OP_APPTYPE != APPTYPE_UNDEF && opException(opDoc, rspbuf, &len, ret) != 0)
        goto EXIT;

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

EXIT:

    OP_DoFinish(opDoc, ret);

    INFOLINE();

    CloseOPDB();

    tapi_svrend();

    return 0;
}
