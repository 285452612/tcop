#include "tcop.h"

static char sql[SQLBUFF_MAX] = {0};
static int ret = 0;

static int OPAdminCommonHandle(xmlDoc *doc);
static int OPAdminAcctHandle(xmlDoc *doc);
static int OPAdminOperHandle(xmlDoc *doc);
static int OPAdminSysHandle(xmlDoc *doc);

//管理类交易处理
int OPAdminHandle(xmlDoc *doc)
{
    if (OP_OPTCODE % OPT_ADM_SYS < 100)
        return OPAdminSysHandle(doc);
    else if (OP_OPTCODE % OPT_ADM_OPER < 100)
        return OPAdminOperHandle(doc);
    else if (OP_OPTCODE % OPT_ADM_ACCT< 100)
        return OPAdminAcctHandle(doc);
    else if (OP_OPTCODE % OPT_ADM_COMMON < 100)
        return OPAdminCommonHandle(doc);

    return E_OTHER;
}

int OPAdminCommonHandle(xmlDoc *doc)
{
    switch (OP_OPTCODE)
    {
        case OPT_ADM_DELTRAN:
            ret = db_exec("UPDATE trnjour SET clearstate='%c', trncode='%d' WHERE %s",
                    CLRSTAT_FAILED, 
                    OP_TCTCODE,
                    GetSigleTrnjourWhere(doc));
            break;
    }

    return ret;
}

int OPAdminOperHandle(xmlDoc *doc)
{
    char name[30] = {0}; 
    char passwd[33] = {0}; 
    char regdate[9] = {0}; 
    char organid[17] = {0}; 
    char status[2] = {0}; 
    char class[2] = {0}; 
    unsigned char sPass[33] = {0};
    unsigned char sNPass[33] = {0};
    const char *p = NULL;
    char where[256] = {0};
    char *pOperid = NULL;
    char *pOrganid = NULL;

    pOperid = XMLGetNodeVal(doc, "//opOperid");
    pOrganid = XMLGetNodeVal(doc, "//opOrganid");

    sprintf(where, " nodeid=%d AND organid='%s' AND operid='%s'", OP_REGIONID, pOrganid, pOperid);

    switch (OP_OPTCODE)
    {
        case OPT_OPER_GETRIGHTS:
            //支持使用行内机构号或同城机构来获取权限
            if (pOrganid != NULL && pOrganid[0] == 0)
                sprintf(where, "nodeid=%d AND bankno='%s' AND operid='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"), pOperid);
            if ((ret = QueryTable(doc, "operinfo", where)) != 0 && ret == E_DB_NORECORD) 
                returnDebug(E_MNG_OPER_NOTEXIST);

            p = XMLGetNodeVal(doc, "//opOperState");
            if (*p == OPER_STATUS_CANCEL)
                returnDebug(E_MNG_OPER_CANCEL);
            db_exec("UPDATE operinfo SET lastlogintime='%08s-%06s' WHERE %s", getDate(0), getTime(0), where);
            break;

        case OPT_OPER_LOGIN:
            p = XMLGetNodeVal(doc, "//opOperPwd");
            if (EncryptData(getDate(0), (unsigned char *)p, strlen(p), sPass, DES_DECRYPT) < 0)
                returnDebug(E_MNG_OPER_PASSWD);

            //支持使用行内机构号或同城机构来登录
            if (pOrganid != NULL && pOrganid[0] == 0)
                sprintf(where, "nodeid=%d AND bankno='%s' AND operid='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"), pOperid);
            if ((ret = QueryTable(doc, "operinfo", where)) != 0 && ret == E_DB_NORECORD) 
                returnDebug(E_MNG_OPER_NOTEXIST);

            if (ChkOperPwd(XMLGetNodeVal(doc, "//opOrganid"), pOperid, XMLGetNodeVal(doc, "//opOperName"), sPass, 
                        XMLGetNodeVal(doc, "//opRegDate"), XMLGetNodeVal(doc, "//opOperPwd")) != 0)
                returnDebug(E_MNG_OPER_PASSWD);

            p = XMLGetNodeVal(doc, "//opOperState");
            if (*p == OPER_STATUS_CANCEL)
                returnDebug(E_MNG_OPER_CANCEL);

            //更新为已签到状态
            ret = db_exec("UPDATE operinfo SET status='%c', lastlogintime='%08s-%06s' WHERE %s", 
                    OPER_STATUS_LOGIN, getDate(0), getTime(0), where);
            break;

        case OPT_OPER_LOGOUT:
             //更改为已签退状态
            ret = db_exec("UPDATE operinfo SET status='%c' WHERE %s", OPER_STATUS_LOGOFF, where);
            break;

        case OPT_OPER_MODPWD:
            sprintf(sql, "SELECT name, passwd, regdate FROM operinfo WHERE %s", where);
            if ((ret = db_query_strs(sql, name, passwd, regdate)) != 0)
                returnDebug(ret);

            //解密旧密码
            p = XMLGetNodeVal(doc, "//opOperPwd");
            if (EncryptData(getDate(0), (unsigned char *)p, strlen(p), sPass, DES_DECRYPT) < 0)
                returnDebug(E_MNG_OPER_PASSWD);

            //检查密码
            if (ChkOperPwd(pOrganid, pOperid, name, sPass, regdate, passwd) != 0)
                returnDebug(E_MNG_OPER_PASSWD);

            //解密新密码
            p = XMLGetNodeVal(doc, "//opOperNewPwd");
            memset(sPass, 0, sizeof(sPass));
            if (EncryptData(getDate(0), (unsigned char *)p, strlen(p), sPass, DES_DECRYPT) < 0)
                returnDebug(E_MNG_OPER_PASSWD);

            GenOperPwd(pOrganid, pOperid, name, sPass, regdate, sNPass);
            //更新密码
            ret = db_exec("UPDATE operinfo SET passwd='%s', pwdchgdate='%s' WHERE %s", sNPass, getDate(0), where);
            break;

        case OPT_OPER_CANCEL:
            ret = db_exec("UPDATE operinfo SET status='%c' WHERE %s", OPER_STATUS_CANCEL, where);
            break;

        case OPT_OPER_CLRPWD:
            sprintf(sql, "SELECT name, passwd, regdate, status FROM operinfo WHERE %s", where);
            if ((ret = db_query_strs(sql, name, passwd, regdate, status)) != 0)
                returnDebug(ret);

            if (status[0] == OPER_STATUS_CANCEL)
                returnDebug(E_MNG_OPER_CANCEL);

            GenOperPwd(pOrganid, pOperid, name, sPass, regdate, sNPass);
            ret = db_exec("UPDATE operinfo SET passwd='%s', pwdchgdate='%s' WHERE %s", sNPass, getDate(0), where);
            break;

        case OPT_OPER_RESET:
            if ((ret = db_query_str(status, sizeof(status), "SELECT status FROM operinfo WHERE %s", where)) != 0)
                returnDebug(ret);

            if (status[0] == OPER_STATUS_CANCEL)
                returnDebug(E_MNG_OPER_CANCEL);

            ret = db_exec("UPDATE operinfo SET status='%c' WHERE %s", OPER_STATUS_LOGOFF, where);
            break;

        case OPT_OPER_AUTHORIZE:
            sprintf(sql, "SELECT name, passwd, regdate, status, class FROM operinfo WHERE %s", where);
            if ((ret = db_query_strs(sql, name, passwd, regdate, status, class)) != 0)
            {
                if (ret == E_DB_NORECORD)
                    return E_MNG_OPER_NOTEXIST;
                else
                    returnDebug(ret);
            }

            p = XMLGetNodeVal(doc, "//opOperPwd");
            if (EncryptData(getDate(0), (unsigned char *)p, strlen(p), sPass, DES_DECRYPT) < 0)
                returnDebug(E_MNG_OPER_PASSWD);

            //检查密码
            if (ChkOperPwd(pOrganid, pOperid, name, sPass, regdate, passwd) != 0)
                returnDebug(E_MNG_OPER_PASSWD);

            if (status[0] == OPER_STATUS_CANCEL)
                returnDebug(E_MNG_OPER_CANCEL);

            if (class[0] != OPER_LEVE_ADMIN)
                returnDebug(E_MNG_OPER_RIGHTS);
            break;
    }

    return ret;
}

int OPAdminAcctHandle(xmlDoc *doc) 
{
    char where[256] = {0};

    sprintf(where, " nodeid=%d AND acctid='%s' AND innerorganid='%s'", 
            OP_REGIONID, XMLGetNodeVal(doc, "//opAcctNo"), XMLGetNodeVal(doc, "//opAcctOrgid"));

    switch (OP_OPTCODE)
    {
        case OPT_ACCT_REGISTER:
            db_exec("DELETE FROM acctinfo WHERE %s", where);
            ret = InsertTable(doc, "acctinfo");
            break;

        case OPT_ACCT_CANCEL:
            ret = db_exec("DELETE FROM acctinfo WHERE %s", where);
            break;

        case OPT_ACCT_MODIFY:
            ret = UpdateTable(doc, "acctinfo", where);
            break;
    }

    return ret;
}

int OPAdminSysHandle(xmlDoc *doc) 
{
    char buf[2048] = {0};
    char tmp[256] = {0};
    char *fields[100] = {NULL};
    char *p = NULL;
    int i = 0, n = 0;

    switch (OP_OPTCODE)
    {
        case OPT_SYS_DOWNFILE:
            if ((p = XMLGetNodeVal(doc, "//opTableName")) == NULL)
                return E_PACK_GETVAL; 

            n = sdpStringSplit(p, fields, 100, '+');
            for (i = 0; i < n; i++)
            {
                if (getFilesdirFile(tmp) == NULL)
                    return E_SYS_CALL;
                QueryTableToFile(tmp, fields[i], "");
                sprintf(buf+strlen(buf), "+%s", basename(tmp));
            }
            XMLSetNodeVal(doc, "//opFilenames", buf+1);
            break;

        case OPT_SYS_QRYARG:
            if ((p = XMLGetNodeVal(doc, "//opArgsName")) == NULL)
                return E_PACK_GETVAL; 
            n = sdpStringSplit(p, fields, 100, '+');
            for (i = 0; i < n; i++) {
                p = GetSysPara(fields[i]);
                sprintf(buf+strlen(buf), "+%s", p == NULL ? "" : p);
            }
            XMLSetNodeVal(doc, "//opArgsValue", buf+1);
            break;

            /*
        case OPT_SYS_XYHREG:
            sprintf(buf, "nodeid=%d AND agreementid='%s'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAgreementid"));
            db_exec("DELETE FROM agreement WHERE %s", buf);
            ret = InsertTable(doc, "agreement");
            break;

        case OPT_SYS_XYHCANCEL:
            sprintf(buf, "nodeid=%d AND agreementid='%s'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAgreementid"));
            ret = db_exec("UPDATE agreement SET state='2' WHERE %s", buf);
            break;
            */
    }

    return ret;
}

int OPAdmAfterCommToPH(xmlDoc *doc, int tcRet)
{
    char *p = NULL;
    char buf[256] = {0};

    switch (OP_OPTCODE)
    {
        case OPT_ACCT_BANKQRY:
            p = XMLGetNodeVal(doc, "//opAcctOrgid");
            if (p != NULL && strlen(p))
                if (db_query_str(buf, sizeof(buf), "SELECT exchno FROM bankinfo WHERE bankid='%s'", p) == 0)
                    XMLSetNodeVal(doc, "//opAcctOrgid", buf);
            break;

        case OPT_SYS_XYHREG:
            sprintf(buf, "nodeid=%d AND agreementid='%s'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAgreementid"));
            db_exec("DELETE FROM agreement WHERE %s", buf);
            ret = InsertTable(doc, "agreement");
            break;

        case OPT_SYS_XYHCANCEL:
            sprintf(buf, "nodeid=%d AND agreementid='%s'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAgreementid"));
            ret = db_exec("UPDATE agreement SET state='2' WHERE %s", buf);
            break;
    }

    return ret;
}
