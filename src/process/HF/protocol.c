#include "interface.h"

static int ret = 0;

int GetRecvBodyLength(char *rsphead)
{
    char buf[11] = {0};

    memcpy(buf, rsphead, 10);
    buf[10] = 0;

    return atoi(buf);
}

int CommProtocol(void *in)
{
    char buf[5] = {0};
    BankCommData *req = (BankCommData *)in;
    BankCommData rsp;

    memset(&rsp, 0, sizeof(rsp));

    rsp.headlen = req->headlen = 10;

    //��ͷ����10�ֽڴ�ű����峤����0
    sprintf(req->head, "%010d", req->bodylen);

    rsp.taillen = req->taillen = 1;
    req->tail[0] = 0x03;

    if (CommToBank(req, &rsp, GetRecvBodyLength) != 0) 
        return -1;

    if (rsp.tail[0] != 0x03)
    {
        INFO("������ͨѶ���ս������Ʒ�����ȷ![%0x]", rsp.tail[0]);
        return -1;
    }

    *req = rsp;

    if (memcmp(rsp.body+68, "0000", 4) == 0)
        return 0;
    else
        return 1;
}
