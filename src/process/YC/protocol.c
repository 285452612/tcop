#include "interface.h"

int CommProtocol(void *in)
{
    BankCommData *req = (BankCommData *)in;
    BankCommData rsp;

    memset(&rsp, 0, sizeof(rsp));

    rsp.headlen = req->headlen = 10;

    //报头定长10字节存放报文体长度左补0
    sprintf(req->head, "%010d", req->bodylen);

    if (CommToBank(req, &rsp, NULL) != 0) 
        return -1;

    *req = rsp;
    return 0;
}
