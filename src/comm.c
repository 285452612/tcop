#include "comm.h"

xmlDoc *CommDocToPH(int *ret, int trncode, xmlDoc *reqdoc, char *pfile)
{
    xmlDoc *rspDoc = NULL;
    unsigned char *req = NULL;
    char rsp[100*COMMPACK_MAXLEN] = {0};
    char rspfile[1024*2] = {0};
    int len = 0;
    int result = 0;
    char xpath[256]={0};
    xmlNodePtr tNode = NULL;

    *ret = 0;
#if 0
    /*********************************************
     *发送人行前去掉BankData的相关报文字段(如果有)
     * add by lt@sunyard.com  2011-7-11
     *********************************************/
    sprintf( xpath, "/UFTP/BankData" );
    if ((tNode = sdpXmlSelectNode(reqdoc, xpath)) == NULL) {
        DBUG("未找到节点[%s]", xpath);
    }
    else
    {
        DBUG("释放节点[%s]", xpath);
        xmlUnlinkNode(tNode);
    }
    /*end add by lt@sunyard.com 2011-7-11*/
#endif

    xmlDocDumpMemory(reqdoc, &req, &len);

    memset(rsp, 0, sizeof(rsp));
    if (pfile == NULL) //不传输文件
        result = CommToPHNoFile(trncode, req, rsp, &len);
    else if (pfile[0] == 0) //可能接收文件
        result = CommToPH(trncode, req, NULL, rsp, rspfile, &len);
    else { //可能发送且接收文件
        INFO("发送文件:[%s]", pfile);
        result = CommToPH(trncode, req, pfile, rsp, rspfile, &len);
    }

    if (result != 0)
        *ret = E_SYS_COMM_PH;
    else 
    {
        if (rspfile[0] != 0)
        {
            INFO("接收文件:[%s]", rspfile);
            strcpy(pfile, rspfile);
        }

        if ((rspDoc = xmlRecoverDoc(rsp)) == NULL)
            *ret = E_PACK_INIT;
    }

    if (*ret)
        INFO("与前置交互异常,ret:[%d]", *ret);

    return rspDoc;
}

int CommToNode(int nodeid, int trncode, char *reqbuf, char *reqfile, char *rspbuf, char *rspfile, int *plen)
{
    TCPHEAD head;
    char addr[24] = {0};
    int port = 0;
    int timeout = 0;
    int i = 0, len=0;
    char tmp[400*COMMPACK_MAXLEN] = {0};
    //add by lt@sunyard.com 2011-7-11
    xmlDoc *reqdoc = NULL;
    char xpath[256]={0};
    xmlNodePtr tNode = NULL;

    sprintf(tmp, "%ld", nodeid);
    memset(addr, 0, sizeof(addr));

    if (tapi_getaddr(tmp, addr, &port) < 0)
    {
        INFO("取节点[%d]通信参数失败!", nodeid);
        return E_SYS_COMM_CFG;
    }

    memset(&head, 0, sizeof(head));

    sprintf(head.TrType, "%06d", trncode);
    head.Sleng = *plen;
    head.PackInfo |= htonl(PI_DCOMPRESS);

    if (reqfile != NULL && reqfile[0] != 0)
    {
        head.PackInfo |= htonl(PI_FCOMPRESS);
        timeout += 300;
    }

    /*********************************************
     *发送人行前去掉BankData的相关报文字段(如果有)
     * add by lt@sunyard.com  2011-7-11
     *********************************************/
    if ((reqdoc = xmlRecoverDoc(reqbuf)) == NULL)
    {
        DBUG("解析xml文件失败");
    }
    sprintf( xpath, "/UFTP/BankData" );
    if ((tNode = sdpXmlSelectNode(reqdoc, xpath)) == NULL) {
        DBUG("未找到节点[%s]", xpath);
    }
    else
    {
        DBUG("释放节点[%s]", xpath);
        xmlUnlinkNode(tNode);
        reqbuf = NULL;
        xmlDocDumpMemory(reqdoc, &reqbuf, &len);
        plen = &len;
        head.Sleng = len;
    }
    /*end add by lt@sunyard.com 2011-7-11*/

    INFO("交易[%d]与节点[%d]开始通讯...", OP_TCTCODE, nodeid);
    DBUG("begin comm to node [%d]:addr=[%s]port=[%d]trcode=[%d]Sleng=[%d]reqfile=[%s]timeout=[%d]", 
            nodeid, addr, port, trncode, head.Sleng, reqfile, timeout);
    {
        memcpy(tmp, reqbuf, *plen);
        for (i = 0; i < *plen - 1; i++)
            if (tmp[i] == 0) tmp[i] = ' ';
        DBUG("发送报文:\n%s]", tmp);
    }

    if (cli_sndrcv(addr, port, &head, reqbuf, reqfile, rspbuf, rspfile, timeout) < 0)
    {
        INFO("交易[%d]与节点[%d(%s:%d)]通讯失败,错误码=[%s]", 
                OP_TCTCODE, nodeid, addr, port, head.RetCode);
        return E_SYS_COMM;
    }
    *plen = head.Sleng;

    DBUG("end comm to node, rsplen [%d]", *plen);
    {
        memcpy(tmp, rspbuf, *plen);
        for (i = 0; i < *plen - 1; i++)
            if (tmp[i] == 0) tmp[i] = ' ';
        tmp[i+1] = 0;
        DBUG("接收报文:\n%s]", tmp);
    }

    INFO("交易[%d]与节点[%d]通讯成功", OP_TCTCODE, nodeid);

    return 0;
}

int CommToBankBase(char *envName, BankCommData *preq, BankCommData *prsp, FPGetRecvBodyLength func)
{
    char *p = NULL;
    char tmp[48] = {0};
    char addr[24] = {0};
    int port, timeout;
    int ret = E_OTHER;

    INFO("TCOP_BANKADDR:%s", getenv(envName));
    if ((p = getenv(envName)) == NULL)
        ret = E_SYS_CALL;
    else {
        strcpy(tmp, p);

        if (sscanf(tmp, "%[^:]:%d:%d", addr, &port, &timeout) < 3)
            ret = E_SYS_COMM_CFG;
        else if ((ret = CommToBankNative(addr, port, preq, prsp, func, timeout)) != 0) 
            ret = E_SYS_COMM_BANK;
    }

    if (ret != 0) 
        INFO("与行内通讯失败!参数:[%s]ret=[%d]", tmp, ret);

    return ret;
}

int CommToBank(BankCommData *preq, BankCommData *prsp, FPGetRecvBodyLength func)
{
    return CommToBankBase("TCOP_BANKADDR", preq, prsp, func);
}

int CommSocket(char *envName, unsigned char *req, int reqlen, unsigned char *rsp, int *rsplen)
{
    BankCommData reqdata = {0}, rspdata = {0};
    int ret = 0;

    memcpy(reqdata.body, req, reqlen);
    reqdata.bodylen = reqlen;
    rspdata.bodylen = *rsplen;

    if ((ret = CommToBankBase(envName, &reqdata, &rspdata, NULL)) == 0) {
        *rsplen = rspdata.bodylen;
        memcpy(rsp, rspdata.body, *rsplen);
    }

    return ret;
}
