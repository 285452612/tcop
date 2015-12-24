#include "interface.h"

int CommProtocol(void *in)
{
    BankCommData *pcd = (BankCommData *)in;
    BankCommData rsp;
    int ret = 0;

    memset(&rsp, 0, sizeof(rsp));

    rsp.headlen = pcd->headlen = 5;

    //±¨Í·
    pcd->head[0] = 0x02;
    pcd->head[1] = 0x00;
    pcd->head[2] = 0x00;
    pcd->head[3] = (unsigned char)(pcd->bodylen / 256);
    pcd->head[4] = (unsigned char)(pcd->bodylen - (int)pcd->head[3] * 256);

    ret = CommToBank(pcd, &rsp, NULL);

    if (ret != 0)
        return -1;

    *pcd = rsp;

    return CommResponseProtocol(pcd);
}

int CommResponseProtocol(BankCommData *rsp)
{
    char buf[12] = {0};

    memset(buf, 0, sizeof(buf));
    memcpy(buf, rsp->body, 5);

    if (atoi(buf) == 0)
        return 0;

    return 1;
}
