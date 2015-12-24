#include "tcop.h"

static char sql[SQLBUFF_MAX] = {0};

int OPTranHandle(xmlDoc *doc)
{
    int ret = 0;
    char *p = NULL;
    char bankId[16] = {0};
    char autoOper[16] = {0};
    char autoOrg[16] = {0};
    char tmp[24] = {0};

    if (OP_OPTCODE == OPT_TRAN_QUERY) {
        return QueryTableByIDToXML(doc, 
                strcmp(GetArchivedate(), XMLGetNodeVal(doc, "//opWorkdate")) < 0 ? "trnjour" : "htrnjour", 
                OP_OPTCODE, GetTrnjourWhere(doc));
    }

    if (!atoi(XMLGetNodeVal(doc, "//opInoutflag")))
        XMLSetNodeVal(doc, "//opInoutflag", OP_OUTTRAN_FLAG);

    switch (OP_OPTCODE)
    {
        case OPT_TRAN_OUTINPUT:
            XMLSetNodeVal(doc, "//opWorkround", GetRound());
            XMLSetNodeVal(doc, "//opClearstate", vstrcat("%c", CLRSTAT_UNSETTLED));
            ret = InsertTrnjour(doc);
            break;

        case OPT_TRAN_OUT:
            XMLSetNodeVal(doc, "//opWorkround", GetRound());
            XMLSetNodeVal(doc, "//opClearstate", vstrcat("%c", CLRSTAT_UNKNOW));
            ret = InsertTrnjour(doc);
            break;

        case OPT_TRAN_IN:
            XMLSetNodeVal(doc, "//opInoutflag", OP_INTRAN_FLAG);
            XMLSetNodeVal(doc, "//opClearstate", vstrcat("%c", CLRSTAT_SETTLED));
            if ((ret = CheckInTrans(doc)) != 0)
                break;
            //来账的行内柜员使用默认柜员
            sprintf(sql, "SELECT bankid, autooper, autoorg FROM bankinfo WHERE exchno='%s'", 
                    XMLGetNodeVal(doc, "//opAcceptor"));
            if ((ret = db_query_strs(sql, bankId, autoOper, autoOrg)) == 0) {
                XMLSetNodeVal(doc, "//opInnerorganid", bankId);
                XMLSetNodeVal(doc, "//opOperid", autoOper);
                XMLSetNodeVal(doc, "//opInnerBank", autoOrg);
            }
            ret = InsertTrnjour(doc);
            break;

        case OPT_TRAN_OUTCHECK:
            ret = db_exec("UPDATE trnjour SET workdate='%s', workround='%s', "
                    "clearstate='%c', auditor='%s', trncode='%d' WHERE %s", 
                    XMLGetNodeVal(doc, "//opWorkdate"), 
                    GetRound(),
                    CLRSTAT_UNKNOW, 
                    XMLGetNodeVal(doc, "//opOperid"),
                    OP_TCTCODE,
                    GetSigleTrnjourWhere(doc));
            break;

        case OPT_TRAN_OUTMODIFY:
            ret = UpdateTableByID(doc, "trnjour", 0, GetSigleTrnjourWhere(doc));
            break;

        case OPT_TRAN_QRYSIGLE:
            p = XMLGetNodeVal(doc, "//opWorkdate");
            strcpy(tmp, *p == 0 ? GetWorkdate() : p);

            if (strcmp(tmp, GetArchivedate()) <= 0)
                ret = QueryTable(doc, "htrnjour", GetSigleTrnjourWhere(doc));
            else
                ret = QueryTable(doc, "trnjour", GetSigleTrnjourWhere(doc));
            break;
    }

    return ret;
}

int OPTranAfterCommToPH(xmlDoc *doc, int tcRet)
{
    char setbuf[512] = {0};

    switch (tcRet)
    {
        case 0:
            sprintf(setbuf, "truncflag='%s',fee=%.2lf,feepayer='%s',"
                    "cleardate='%s',clearround='%s',clearstate='%c',exchgdate='%s',exchground='%s'", 
                    XMLGetNodeVal(doc, "//opTruncflag"), 
                    atof(XMLGetNodeVal(doc, "//opFee")), 
                    XMLGetNodeVal(doc, "//opFeePayer"), 
                    XMLGetNodeVal(doc, "//opCleardate"), 
                    XMLGetNodeVal(doc, "//opClearround"), 
                    CLRSTAT_SETTLED,
                    XMLGetNodeVal(doc, "//opExchgdate"), 
                    XMLGetNodeVal(doc, "//opExchground")
                    );
            break;

        case E_SYS_COMM_PH:
            sprintf(setbuf, "truncflag='%s',result=%d,clearstate='%c'", 
                    XMLGetNodeVal(doc, "//opTruncflag"), 
                    E_SYS_COMM_PH, CLRSTAT_UNKNOW);
            break;

        default:
            sprintf(setbuf, "truncflag='%s',cleardate='%s',clearround='%s',"
                    "result=%d,clearstate='%c',exchgdate='%s',exchground='%s'", 
                    XMLGetNodeVal(doc, "//opTruncflag"), 
                    XMLGetNodeVal(doc, "//opCleardate"),
                    XMLGetNodeVal(doc, "//opClearround"), tcRet, CLRSTAT_FAILED,
                    XMLGetNodeVal(doc, "//opExchgdate"), 
                    XMLGetNodeVal(doc, "//opExchground"));
            break;
    }
    sprintf(sql, "UPDATE trnjour SET %s WHERE %s", setbuf, GetSigleTrnjourWhere(doc));

    return db_exec(sql);
}

char *GetSigleTrnjourWhere(xmlDoc *doc)
{
    char refid[32] = {0};
    char workdate[9] = {0};
    char *p = NULL;

    strcpy(refid, XMLGetNodeVal(doc, "//opRefid"));
    sdpStringTrimHeadChar(refid, '0');

    p = XMLGetNodeVal(doc, "//opWorkdate");
    strcpy(workdate, p[0] == 0 ? GetWorkdate() : p);

    return vstrcat("nodeid=%s AND workdate='%s' AND convert(decimal, refid)=%s AND originator='%s' AND inoutflag='%s'", 
            XMLGetNodeVal(doc, "//opNodeid"), 
            workdate, refid,
            XMLGetNodeVal(doc, "//opOriginator"),
            XMLGetNodeVal(doc, "//opInoutflag"));
}

char *GetTrnjourWhere(xmlDoc *doc)
{
    int len = 0;
    char *p = NULL;

    len += sprintf(sql+len, "nodeid=%s ", XmlGetStringDup(doc, "//opNodeid"));

    if (*(p = XmlGetStringDup(doc, "//opWorkdate")) != 0)
        len += sprintf(sql+len, "AND workdate='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opInoutflag")) != 0)
        len += sprintf(sql+len, "AND inoutflag='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opOriginator")) != 0)
        len += sprintf(sql+len, "AND originator='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opAcceptor")) != 0)
        len += sprintf(sql+len, "AND acceptor='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opTruncflag")) != 0)
        len += sprintf(sql+len, "AND truncflag='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opSettlamt")) != 0)
        len += sprintf(sql+len, "AND settlamt=%s ", p);

    if (*(p = XmlGetStringDup(doc, "//opClearstate")) != 0)
        len += sprintf(sql+len, "AND clearstate='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opNotetype")) != 0)
        len += sprintf(sql+len, "AND notetype='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opNoteno")) != 0)
        len += sprintf(sql+len, "AND noteno='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opAcctoper")) != 0)
        len += sprintf(sql+len, "AND acctoper='%s' ", p);

    if (*(p = XmlGetStringDup(doc, "//opReserved")) != 0)
        len += sprintf(sql+len, "AND %s", p);

    return sql;
}

int CheckInTrans(xmlDoc *doc)
{
    char clearstate[2] = {0};
    char result[8] = {0};
    char dcflag[4] = {0};
    char where[512] = {0};
    int ret = 0;

    INFO("提入交易重复性检查...");
    strcpy(where, GetSigleTrnjourWhere(doc));

    if ((ret = db_query_str(result, sizeof(result), "SELECT count(1) FROM trnjour WHERE %s", where)) != 0)
        return ret;

    if (atoi(result) != 1)
        return 0;

    sprintf(sql, "SELECT str(result), clearstate, dcflag FROM trnjour WHERE %s", where);
    if ((ret = db_query_strs(sql, result, clearstate, dcflag)) != 0)
        return ret;

    if (clearstate[0] == CLRSTAT_SETTLED || clearstate[0] == CLRSTAT_CHECKED)
        return E_TRAN_EXISTSUCC;

    //贷记直接返回成功
    if (dcflag[0] == OP_CREDITTRAN)
    {
        if (clearstate[0] != CLRSTAT_SETTLED)
        {
            //提入贷记清算不成功的交易直接更新成已清算
            sprintf(sql, "UPDATE trnjour SET result=0,clearstate='%c' WHERE %s", CLRSTAT_SETTLED, where);
            if (db_exec(sql) != 0)
                return E_DB;
        }
        return E_TRAN_EXISTSUCC;
    }

    return E_TRAN_EXISTUNKNOW;
} 
