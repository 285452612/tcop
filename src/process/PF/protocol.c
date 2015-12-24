#include "interface.h"

static int ret = 0;

#define PACK_HEAD_LEN 2

int GetRecvBodyLength(char *rsphead)
{
    unsigned short len;

    // ��ͷ����Ҳ��������, �� -2
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

    sprintf(bktcode, "%s", req->head); // ƽ̨��������ڽ�����洢��head��
    BKINFO("��ȡ���ڽ�����[%s]",bktcode);

    ///* sopdbsvr �Ѿ���ʼ��
    if (init_sop_hx() != 0)
    {
        BKINFO("init_sop_hx() fail.");
        return -1;
    }

    BKINFO("��ʼ����ƽ̨������...");
    // ����ƽ̨������
    //BKINFO("��ʼ����ƽ̨������[%d]...", req->bodylen);
    if ((opDoc = xmlParseMemory(req->body, req->bodylen)) == NULL)
    {
        BKINFO("tcop xml parse fail.");
        return -1;
    }
    xmlSaveFile("opdoc_req.xml", opDoc);

    BKINFO("��ʼת��ƽ̨���ĵ����ڱ���...");
    // ƽ̨xml����ת��������sop����
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
    req->headlen = 0; // ����������ʱ��ͷ�����Ѿ���body��

    BKINFO("��ʼ�����ڽ���ͨ��...");
    rsp.headlen= PACK_HEAD_LEN; // ��ȡ2�ֽڱ�ͷ����
    save_pack("hxreq", req->body, req->bodylen);
    if (CommToBank(req, &rsp, GetRecvBodyLength) != 0) 
    {
        BKINFO("������ͨ�Ŵ�");
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
