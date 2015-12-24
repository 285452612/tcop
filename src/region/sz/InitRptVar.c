#include "comm.h"
#include "interface.h"

char gs_originator[13] = {0x00};
char gs_bankname[81] = {0x00};
char gs_sysname[61] = {0x00};
char gs_oper[9] = {0x00};

int InitRptVar(xmlDocPtr xmlReq)
{
    xmlGetVal(xmlReq, "//MsgHdrRq/Originator", gs_originator);
    xmlGetVal(xmlReq, "//MsgHdrRq/AcctOper", gs_oper);
    org_name(gs_originator, gs_bankname);

    memset(gs_sysname, 0, sizeof(gs_sysname));
    strcpy(gs_sysname, GetSysPara("SYSNAME"));
    if (*gs_sysname == 0x00)
        strcpy(gs_sysname, "同城票据清算系统");

    return 0;
}
