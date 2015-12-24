#include "interface.h"
#include "comm.h"

static int ret = 0;

//交易预处理
int OP_DoInit(char *req, int *plen)
{
    return 0;
}

//切场通知
int PF_1602(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    return ret;
}
