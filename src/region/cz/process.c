#include "interface.h"
#include "comm.h"

static int ret = 0;

//����Ԥ����
int OP_DoInit(char *req, int *plen)
{
    return 0;
}

//�г�֪ͨ
int PF_1602(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    return ret;
}
