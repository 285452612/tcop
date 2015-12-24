#include "tcop.h"

static char sql[SQLBUFF_MAX] = {0};

//信息类交易处理
int OPInfoHandle(xmlDoc *doc)
{
    int ret = 0;
    char tmp[48] = {0};
    char *p = NULL;

    if (!atoi(XMLGetNodeVal(doc, "//opInoutflag"))) 
        XMLSetNodeVal(doc, "//opInoutflag", isOutTran() ? OP_OUTTRAN_FLAG : OP_INTRAN_FLAG);

    switch (OP_OPTCODE)
    {
        case OPT_INFO_REQSEND:
            ret = InsertTable(doc, "queryinfo");
            break;

        case OPT_INFO_RSPSEND:
            XMLSetNodeVal(doc, "//opInoutflag", OP_INTRAN_FLAG); //提出查复针对原提入查询交易
            p = GetSigleQueryWhere(doc);
            if (ret = db_query_str(tmp, sizeof(tmp), "SELECT COUNT(1) FROM queryinfo WHERE %s", p))
                return ret;
            if (atoi(tmp) == 0)
                return E_DB_NORECORD;
            if (atoi(tmp) != 1)
                return E_DB;
            ret = UpdateTable(doc, "queryinfo", p);
            break;

        case OPT_INFO_MSGSEND:
            ret = InsertTable(doc, "freemsg");
            break;

        case OPT_INFO_QRYSINGLE:
            if ((ret = QueryTable(doc, "queryinfo", GetSigleQueryWhere(doc))) == 0)
            {
                p = XMLGetNodeVal(doc, "//opQueryState");
                if (p != NULL && p[0] == QUERY_STATE_REPLIED)
                    ret = E_TRAN_QUERYREPLIED;
            }
            break;

        case OPT_INFO_INREQBOOK:
            ret = InsertTable(doc, "queryinfo");
            break;

        case OPT_INFO_INRSPBOOK:
            XMLSetNodeVal(doc, "//opInoutflag", OP_OUTTRAN_FLAG); //提入查复针对原提出查询交易
            p = GetSigleQueryWhere(doc);
            if (ret = db_query_str(tmp, sizeof(tmp), "SELECT COUNT(1) FROM queryinfo WHERE %s", p))
                return ret;
            if (atoi(tmp) == 0)
                return E_DB_NORECORD;
            if (atoi(tmp) != 1)
                return E_DB;
            ret = UpdateTable(doc, "queryinfo", p);
            break;

        case OPT_INFO_INMSG:
            ret = InsertTable(doc, "freemsg");
            break;
    }

    return ret;
}

int OPInfoAfterCommToPH(xmlDoc *doc, int tcRet)
{
    char setbuf[512] = {0};

    if (OP_OPTCODE == OPT_INFO_MSGSEND)
    {
        sprintf(sql, "UPDATE freemsg SET result=%d WHERE %s", tcRet == 0 ? 0 : tcRet, GetSigleFreemsgWhere(doc));
        return db_exec(sql);
    }

    sprintf(sql, "UPDATE queryinfo SET result=%d WHERE %s", tcRet == 0 ? 0 : tcRet, GetSigleQueryWhere(doc));

    return db_exec(sql);
}

char *GetSigleQueryWhere(xmlDoc *doc)
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

char *GetSigleFreemsgWhere(xmlDoc *doc)
{
    char refid[32] = {0};

    strcpy(refid, XMLGetNodeVal(doc, "//opRefid"));
    sdpStringTrimHeadChar(refid, '0');

    return vstrcat("nodeid=%s AND workdate='%s' AND convert(decimal, refid)=%s AND originator='%s' AND inoutflag='%s'", 
            XMLGetNodeVal(doc, "//opNodeid"), 
            XMLGetNodeVal(doc, "//opWorkdate"), 
            refid,
            XMLGetNodeVal(doc, "//opOriginator"),
            XMLGetNodeVal(doc, "//opInoutflag"));
}
