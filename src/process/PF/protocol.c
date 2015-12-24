#include "interface.h"

static int ret = 0;

#define PACK_HEAD_LEN 2

int GetRecvBodyLength(char *rsphead)
{
    unsigned short len;

    // 因报头长度也包含在内, 故 -2
    len = ntohs(*(unsigned short *)rsphead) - 2;

    return len;
}

int CommProtocol(void *in)
{
    char bktcode[9] = {0};
    char buf[4096];
    xmlChar *xbuf = NULL;
    BankCommData *req = (BankCommData *)in;
    BankCommData rsp;
    xmlDocPtr opDoc;
    int len, rc;

    memset(&rsp, 0, sizeof(rsp));

    sprintf(bktcode, "%s", req->head); // 平台定义的行内交易码存储在head中
    BKINFO("获取行内交易码[%s]",bktcode);

    ///* sopdbsvr 已经初始化
    if (init_sop_hx() != 0)
    {
        BKINFO("init_sop_hx() fail.");
        return -1;
    }

    BKINFO("开始解析平台请求报文...");
    // 解析平台请求报文
    //BKINFO("开始解析平台请求报文[%d]...", req->bodylen);
    if ((opDoc = xmlParseMemory(req->body, req->bodylen)) == NULL)
    {
        BKINFO("tcop xml parse fail.");
        return -1;
    }
    xmlSaveFile("opdoc_req.xml", opDoc);

    BKINFO("开始转换平台报文到行内报文...");
    // 平台xml报文转换成行内sop报文
    memset(buf, 0, sizeof(buf));
    if ((len = tcop2sop_req(bktcode, opDoc, buf)) <= 0)
    {
        BKINFO("tcop2sop_req() fail.");
        DestoryConEnv();
        return -1;
    }
    req->headlen = 0;
    memset(req->body, 0, sizeof(req->body));
    req->bodylen = len;
    memcpy(req->body, buf, len);
    req->headlen = 0; // 发送至行内时报头长度已经在body中

    BKINFO("开始与行内进行通信...");
    rsp.headlen= PACK_HEAD_LEN; // 先取2字节报头长度
    save_pack("hxreq", req->body, req->bodylen);
    if (CommToBank(req, &rsp, GetRecvBodyLength) != 0) 
    {
        BKINFO("与行内通信错");
        DestoryConEnv();
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memcpy(buf, rsp.head, PACK_HEAD_LEN);
    memcpy(buf+PACK_HEAD_LEN, rsp.body, rsp.bodylen);
   // memcpy(buf, rsp.body, rsp.bodylen);
    save_pack("hxrsp", buf, rsp.bodylen);
    if ((rc = sop2tcop_rsp(bktcode, opDoc, buf, rsp.bodylen)) != 0)
    {
        BKINFO("sop2tcop_rsp() ret=[%d].", rc);
        DestoryConEnv();
        return -1;
    }
    DestoryConEnv();

    xmlSaveFile("opdoc_rsp.xml", opDoc);
    xmlDocDumpMemory(opDoc, &xbuf, &len);
    memset(rsp.body, 0, sizeof(rsp.body));
    rsp.bodylen = len;
    memcpy(rsp.body, xbuf, len);

    *req = rsp;
    return 0;
}
