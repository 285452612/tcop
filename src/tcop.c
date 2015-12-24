#include "tcop.h"

static char sql[SQLBUFF_MAX] = {0};

int OPTranHandle(xmlDoc *doc);
int OPInfoHandle(xmlDoc *doc);
int OPAdminHandle(xmlDoc *doc);
int OPQryPrtHandle(xmlDoc *doc);
int OPFundHandle(xmlDoc *doc);

int OPInitOPTran(xmlDoc *doc)
{
    int ret = 0;

    INFO("开始平台交易[%d]预处理...", OP_OPTCODE);

    if (*XMLGetNodeVal(doc, "//opWorkdate") == 0)
        XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate());

    XMLSetNodeVal(doc, "//opNodeid", vstrcat("%d", OP_REGIONID));
    XMLSetNodeVal(doc, "//opTrcode", vstrcat("%d", OP_OPTCODE));
    XMLSetNodeVal(doc, "//opTctcode", vstrcat("%d", OP_TCTCODE));
    XMLSetNodeVal(doc, "//opTrdate", getDate(0));
    XMLSetNodeVal(doc, "//opTrtime", getTime(0));

    switch (OP_OPTCODE / 100)
    {
        case 1: //交易类
            ret = OPTranHandle(doc); break;
        case 2: //信息类
            ret = OPInfoHandle(doc); break;
        case 3: //管理类
        case 4: case 5: case 6: 
            ret = OPAdminHandle(doc); break;
        case 7: //查询打印类 
            ret = OPQryPrtHandle(doc); break;
        case 9: //资金类
            ret = OPFundHandle(doc); break;
    }

    INFO("平台交易[%d]预处理完成!ret=[%d]", OP_OPTCODE, ret);

    return ret;
}

int OPAfterCommToPH(xmlDoc *doc)
{
    char *p = NULL;
    int ret = 0;

    if (OP_APPTYPE != APPTYPE_OPBANK_AUTOSVR && OP_APPTYPE != APPTYPE_OUTTRANS_SERVER)
        return 0;

    p = XMLGetNodeVal(doc, "//opTCRetcode");

    switch (OP_OPTCODE / 100)
    {
        case 1: ret = OPTranAfterCommToPH(doc, atoi(p)); break;
        case 2: ret = OPInfoAfterCommToPH(doc, atoi(p)); break;
        case 4: case 5: case 6:
                ret = OPAdmAfterCommToPH(doc, atoi(p)); break;
    }

    if (OP_APPTYPE == APPTYPE_OUTTRANS_SERVER)
        INFO("与前置通讯后进行平台预处理完成![%d]", ret);
    else
        INFO("与行内通讯后进行平台预处理完成![%d]", ret);

    return ret;
}

int OP_DoFinish(xmlDoc *doc, int ret)
{
    char setbuf[512] = {0};

    if (doc == NULL)
        return 0;

    if (OP_OPTCODE == OPT_TRAN_OUTINPUT)
    {
        if (!isSuccess(ret))
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
    }

    if (OP_OPTCODE == OPT_TRAN_OUTCHECK || OP_OPTCODE == OPT_TRAN_OUT)
    {
        if (isSuccess(ret))
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_SETTLED);
        else if (ret != E_SYS_COMM_PH && ret != E_SYS_COMM)
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
        else
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_UNKNOW);
    }

    if (OP_OPTCODE == OPT_TRAN_IN && !isSuccess(ret))
    {
        //提入交易应用处理不成功(应答不成功)

        if (ret == E_SYS_COMM_PH) {
            //应答通讯失败
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_UNKNOW);
        } else if (ret < 0) {
            //应用处理错未应答
            sprintf(setbuf, "result=%d,clearstate='%c'", E_OTHER, CLRSTAT_UNKNOW);
        } else {
            //应用处理失败(应答失败)
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
        }
    }

    if (setbuf[0] == 0)
        return 0;

    sprintf(sql, "UPDATE trnjour SET %s WHERE %s", setbuf, GetSigleTrnjourWhere(doc));
    INFO("交易结束更新交易状态 [%s]", setbuf);

    return db_exec(sql);
}

