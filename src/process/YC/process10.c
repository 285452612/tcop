#include "interface.h"
#include "chinese.h"
char *_sSysCode = "3214001";
static char _RecvSerial[8+1];
//static char _AcctSerial[8+1];
static char _FeeSerial[8+1];
int _iFeeAcctFlag=0;

static char gs_originator[13] = {0x00};
static char gs_bankid[13] = {0x00};
static char gs_bankname[81] = {0x00};
static char gs_sysname[61] = {0x00};
static char gs_oper[30] = {0x00};

static char *ChineseDate(long curr_date)
{
    char buf[20];

    sprintf(buf, "%04ld年%02ld月%02ld日", 
            curr_date/10000, curr_date%10000/100, curr_date%100);

    return strdup(buf);
}

static char *GetTmpFileName( char *pFileName )
{
    char buf[256];

    sprintf( buf, "%s/tmpXXXXXX", getenv("FILES_DIR") );
    mkstemp( buf );
    strcpy( pFileName, buf );

    return pFileName;
}

static void ifree(char *p)
{
    if (p != NULL)
    {
        free(p); p = NULL;
    }
}

extern int Acct_Bj(xmlDoc *doc, char *pDate, char *pBank, char *pSerial, char *pInoutflag, int iFlag);

int InitRptVar(xmlDocPtr xmlReq)
{
    char opername[20];
    strcpy(gs_bankid, XMLGetNodeVal(xmlReq, "//opInnerBank"));
    strcpy(gs_originator, XMLGetNodeVal(xmlReq, "//opOriginator"));
    strcpy(gs_oper, XMLGetNodeVal(xmlReq, "//opOperid"));
    // 操作员姓名
    db_query_str(opername, sizeof(opername), 
            "select name from operinfo where nodeid=%d and operid='%s' "
            "and bankno = '%s'", OP_REGIONID, gs_oper, gs_bankid);
    strcat(gs_oper, " "); strcat(gs_oper, opername);
    // 机构名称
    org_name(gs_originator, gs_bankname);
    // 系统名称->报表头
    /*
    memset(gs_sysname, 0, sizeof(gs_sysname));
    strcpy(gs_sysname, GetSysPara("SYSNAME"));
    if (*gs_sysname == 0x00)
    */
    strcpy(gs_sysname, "中国邮政储蓄银行苏州分行");

    return 0;
}

/*
 * 判断账户性质(对公 存折 卡)
 * 返回:
 *      对公 0
 *      存折 1
 *      卡   2
 */
int IsAcctType(char *pAcctNo)
{
    int ret=0;
    char sAcctNo[32+1]={0};
    char *p = NULL;

    memset(sAcctNo, 0, sizeof(sAcctNo));
    if(pAcctNo[0] == '*')
        strcpy(sAcctNo, pAcctNo+1);
    else
        strcpy(sAcctNo, pAcctNo);
    if (strlen(sAcctNo) == 7)
    {
        // 7位转28位
        if ((p = GetSysPara(sAcctNo)) != NULL)
            strcpy(pAcctNo, p);
        else
            return -1;
    }
    else
        strcpy(pAcctNo, sAcctNo);

    //对公账户长度18位 并且 以10开头
    /*
    //if(memcmp(sAcctNo, "10", 2) == 0 && strlen(sAcctNo) == 18) 
    if(strlen(sAcctNo) == 18) 
	    return 0;
    //对公账户长度28位或者7位,为内部帐户 
    if( strlen(sAcctNo) == 28 )
	    return 3;
    if( strlen(sAcctNo) == 7 )
	    return 4;
    if(strlen(pAcctNo) == 19 || strlen(pAcctNo) == 16)
	    return 2;
    */
    switch(strlen(sAcctNo))
    {
        case 18:
            //if(memcmp(sAcctNo, "10", 2) == 0)
            if(sAcctNo[0] == '1')
                ret = 0;
            else
                ret = 1;
            break;
        case 16:
        case 19:
            ret = 2;
            break;
        case 7:
        case 28:
            ret = 3;
            break;
        default:
            ret = 1;
            break;
    }

    return ret;
}

/*帐户转换
  1-清算账户,2-内部5分户*/
int GetBankAcct( char *sAcctNo, char *sBankNo, int Flag ) 
{
    char sql[1024]={0};
    char sTmp[37]={0};
    int ret;

    switch( Flag )
    {
        case 1: //清算账户
            sprintf(sql, "select clearacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 2: //储蓄往来
            sprintf(sql, "select returnacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 3: //资金清算往来
            sprintf(sql, "select debitacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 4: //会计机构
            sprintf(sql, "select bankid from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        default:
            return -1;
    }
    ret = db_query_str(sTmp, sizeof(sTmp), sql );
    if( ret )
    {
        BKINFO("查询机构信息失败.");
        return ret;
    }
    memset( sAcctNo, 0, sizeof(sAcctNo) );
    strcpy( sAcctNo, sTmp );
    return 0;
}

/*
 * 计算手续费
 * 返回 1 折扣批量收
 *      0 实时收
 */
int Fee_Cast(char *pAcctNo, char *pFeeAmt)
{
    int ret=0;
    char sBaseAmt[15+1]={0}, sFeeRate[15+1]={0}, sType[2+1]={0};
    char sFlag[1+1]={0};
    char sSqlStr[1024]={0};

    BKINFO("计算手续费...");
    sprintf(sSqlStr, "select value from feetype where nodeid = %d and typeid = %s", OP_REGIONID, "1");
    ret = db_query_str(sBaseAmt, sizeof(sBaseAmt), sSqlStr);
    if(ret) {
        BKINFO("查询基本手续费失败");
        return ret;
    }
    sprintf(sSqlStr, "select typeid from feeset where nodeid = %d and acctno = '%s'", OP_REGIONID, pAcctNo);
    ret = db_query_str(sType, sizeof(sType), sSqlStr);
    if(ret && ret != E_DB_NORECORD) {
        BKINFO("查询手续费类型失败");
        return ret;
    }
    else if(ret == E_DB_NORECORD) {
        strcpy(pFeeAmt, sBaseAmt);
        return 0;
    }
    sprintf(sSqlStr, "select flag, value from feetype where nodeid = %d and typeid = %s", OP_REGIONID, sType);
    ret = db_query_strs(sSqlStr, sFlag, sFeeRate);
    if(ret) {
        BKINFO("查询手续费费率失败");
        return ret;
    }
    sprintf(pFeeAmt, "%.2f", atof(sFeeRate) * atof(sBaseAmt));
    BKINFO("批量实时标志:%s 手续费:%s", sFlag, pFeeAmt);

    return atoi(sFlag);
}

/*
 * 根据交易结果判断中心清算状态
 */
int IsAcctCode(char *pRet)
{
    switch(atoi(pRet))
    {
        case 0:
            return 0;
            //交易结果不明确
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
        case 207:
        case 3002:
        case 3003:
        case 8043:
        case 8048:
        case 8202:
        case 8203:
        case 8204:
        case 8205:
            return 7;       //清算状态不确定
        default:
            return 2;
    }
}

/*
 * 根据帐号获取协议号
 */
int Qry_Agreement(char *pKhAcct, char *pXyh)
{
    int ret=0;
    char sXyh[60+1]={0};

    // 内部户没有协议号
    if (strlen(pKhAcct) == 28)
    {
        pXyh[0] = 0x00;
        return 0;
    }

    ret = db_query_str(sXyh, sizeof(sXyh), "select agreement from acctinfo where "
            " nodeid = %d and acctid = '%s'",
            OP_REGIONID, pKhAcct);
    if(ret == E_DB_NORECORD) {
        BKINFO("%s帐号信息不存在", pKhAcct);
        return ret;
    }
    else if(ret) {
        BKINFO("%s查询帐号信息失败", pKhAcct);
        return ret;
    }
    if(strlen(sXyh) == 0) {
        BKINFO("%s帐号手续费协议信息为空", pKhAcct);
        return E_DB_NORECORD;
    }
    strcpy(pXyh, sXyh);
    return 0;
}

/*
 * 更新记帐流水表记录为已冲正
 */
int UpAcctSerial(xmlDocPtr opDoc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "update acctjour set result = '%s' \
            where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' \
            and inoutflag = '%s'",
            //XMLGetNodeVal(opDoc, "//opHostSerial"),
            "2",    //已冲正
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opOriginator"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            pInOutFlag
           );
    ret = db_exec(sSqlStr);
    return ret;
}

/*
 * 密钥申请
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_333(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sMAC[16+1]={0};
    char sResult[6+1]={0};

    BKINFO("开始密钥申请...");
    ret = callInterface(8002, opDoc);
    if(ret)
    {
        BKINFO("调用8002失败");
        return ret;
    }

    strcpy(sMAC, XMLGetNodeVal(opDoc, "//opHreserved1"));
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0) {
        BKINFO("密钥申请失败");
        return atoi(sResult);
    }

    if (db_exec("update syspara set paraval = '%s' where paraname = 'MACKEY' and nodeid = %d", sMAC, OP_REGIONID) != 0)
        return E_DB;

    BKINFO("密钥申请完成");

    return ret;
}

/*
 * 提出录入
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_101(void *doc, char *p)
{
    int ret=0,iAcctFlag;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *pDCFlag=NULL, *pAcctNo=NULL;

    /* 判断是否个人帐号 置对公对私帐号标志 */
    pDCFlag = XMLGetNodeVal(opDoc, "//opDcflag");
    if(atoi(pDCFlag) == 1)
        pAcctNo = XMLGetNodeVal(opDoc, "//opBeneacct");
    else if(atoi(pDCFlag) == 2)
        pAcctNo = XMLGetNodeVal(opDoc, "//opPayacct");

    ret = IsAcctType(pAcctNo);
    if (ret < 0)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "账户类型检查出错");
        return 8999;
    }

    /* resflag1保存的是对私帐号 对公帐号的标志 0对公 1对私 */
    ret = db_exec("update trnjour set resflag1 = '%d' where nodeid = %d and "
            " workdate = '%s' and refid = '%s' and inoutflag = '%s' and originator = '%s'",
            (ret == 1 || ret == 2) ? 1 : 0,
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            XMLGetNodeVal(opDoc, "//opInoutflag"),
            XMLGetNodeVal(opDoc, "//opOriginator"));
    if(ret)
        BKINFO("更新帐号标志失败");

    return ret;
}

void SetOperInfo(xmlDocPtr doc)
{
    char sAcctOper[64], sChkOper[64], sTmp[64];
    int ret;

    db_query_str(sTmp, sizeof(sTmp), 
            "select name from operinfo where nodeid=%d and operid='%s' "
            "and bankno='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opOperid"),
            XMLGetNodeVal(doc, "//opInnerBank"));
    sprintf(sChkOper, "%s %s", XMLGetNodeVal(doc, "//opOperid"), sTmp);
    XMLSetNodeVal(doc, "//opChecker", sChkOper);

    ret = db_query_str(sAcctOper, sizeof(sAcctOper), 
            "select acctoper from trnjour where nodeid=%d and workdate='%s' "
            "and originator='%s' and refid='%s' and inoutflag='1'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), 
            XMLGetNodeVal(doc, "//opOriginator"), 
            XMLGetNodeVal(doc, "//opRefid"));
    if(ret == 0)
    {
        ret = db_query_str(sTmp, sizeof(sTmp), 
                "select name from operinfo where nodeid=%d and operid='%s' "
                "and bankno = '%s'", OP_REGIONID, sAcctOper, 
                XMLGetNodeVal(doc, "//opInnerBank"));
        sprintf(sAcctOper+strlen(sAcctOper), " %s", sTmp);
        XMLSetNodeVal(doc, "//opInputer", sAcctOper);
    }
    BKINFO("inputer=[%s]", sAcctOper);
    BKINFO("checker=[%s]", sChkOper);

    return;
}

/*
 * 复核记帐
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_102(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sCurCode[3+1]={0};
    char sResult[6+1]={0};
    char sTmp[40]={0}, sStr[40]={0};

    BKINFO("与中心通讯前后标志:%s", p);

    // 返回操作员姓名
    SetOperInfo(opDoc);

    /* 退票交易不记帐 */
    if(OP_TCTCODE == 7) {
        BKINFO("退票交易行内不处理");
        return 0;
    }
    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//opCurcode"));
    /* 外币不自动记帐 */
    if(memcmp(sCurCode, "CNY", 3) != 0)
        return 0;

    XMLSetNodeVal(opDoc, "//opInoutflag", "1");
    if(p[0] == COMMTOPH_AFTER[0])           //与中心通讯后
        ret = CommToExbAfter(opDoc);
    else if(p[0] == COMMTOPH_BEFORE[0])     //与中心通讯前
        ret = CommToExbBefore(opDoc);

    return ret;
}

int AcctToBank(xmlDoc *opDoc, char *pAcctNo, char *pTxType, char *pTrnCode)
{
    int ret=0;
    char sOpSerial[20+1]={0}, sNoteType[2+1]={0}, sDCFlag[1+1]={0};
    char sHnType[6+1]={0}, sAmt[15+1]={0}, sStr[89+1]={0};
    char sWorkDate[8+1]={0}, sRefId[16+1]={0}, sOutBank[12+1]={0};
    char sOrgId[12+1]={0}, sAcctOper[6+1]={0}, sResult[6+1]={0};
    char sFeeAmt[15+1]={0}, sInoutflag[1+1]={0}, retstr[1+1]={0};
    char sSqlStr[1024]={0};
    long lTmp = 0L;

    /* pTxType=2是手续费记帐 不进行票据种类转换 */
    if(atoi(pTxType) != 2)
    {
        /* 根据同城票据类型查询行内票据类型 */
        BKINFO("转换凭证类型...");
        strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
        strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
        sprintf(sSqlStr, "select banktype from notetypemap \
                where nodeid = %d and tctype = '%s' and dcflag = '%s'",
                OP_REGIONID, sNoteType, sDCFlag);
        ret = db_query_str(sHnType, sizeof(sHnType), sSqlStr);
        if(ret) {
            BKINFO("查询行内票据类型失败");
            return 0; // modi by chenjie ret->0
        }
        XMLSetNodeVal(opDoc, "//opNotetype", sHnType);
    }

    /* 设置向行内记帐流水 */
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(opDoc, "//opOreserved1", sOpSerial);
    BKINFO("生成记帐流水:%s", sOpSerial);

    /* 计算MAC */
    /*
       strcpy(sAmt, XMLGetNodeVal(opDoc, "//opSettlamt"));
       sprintf(sStr, "%s %s %s %s %s %015.0f %s", 
       _sSysCode, 
       pTrnCode,
       XMLGetNodeVal(opDoc, "//opWorkdate"), 
       sOpSerial, 
       pAcctNo,
       atof(sAmt)*100, pTxType);
     */

    XMLSetNodeVal(opDoc, "//opTermno", pTxType);
    //手续费
    if(atoi(pTxType) == 2) {
        BKINFO("手续费记帐...");
        strcpy(sInoutflag, "3");
    }
    else if (atoi(pTxType) == 1) {
        BKINFO("提出借收妥...");
        strcpy(sInoutflag, "4");
    }else{
        BKINFO("交易记帐...");
        strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    }
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sRefId, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sOrgId, XMLGetNodeVal(opDoc, "//opInnerorganid"));
    strcpy(sAcctOper, XMLGetNodeVal(opDoc, "//opOperid"));
    /* 判断记帐流水是否存在 */
    ret = db_query_str(retstr, sizeof(retstr), "select result from acctjour "
            "where nodeid = %d and workdate = '%s' and originator = '%s' and "
            "refid = '%s' and inoutflag = '%s'", 
            OP_REGIONID, sWorkDate, sOutBank, 
            sRefId, sInoutflag);

    if(ret == E_DB_NORECORD) {
        ret = db_exec("insert into acctjour values(%d, '%s', '%s', '%s',"
                "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
                "'%s', '%s', '%s')",
                OP_REGIONID, sWorkDate, sOutBank, sRefId, sInoutflag, 
                pTrnCode, sOpSerial, _RecvSerial, sOrgId, sAcctOper, 
                "", "0", "0", "", "", "", "");
        if(ret) {
            BKINFO("保存记帐信息失败");
            return E_APP_ACCOUNTFAIL;
        }
    }
    else if(ret)
        return E_APP_ACCOUNTFAIL;
    if(atoi(retstr) == 1)   //已记帐
        return E_APP_ACCOUNTSUCC;
#if 0
    /* 判断手续费收费信息是否存在 */
    if(atoi(pTxType) == 2) {
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist "
                "where nodeid = %d and workdate = '%s' and originator = '%s' and "
                "refid = '%s' and inoutflag = '%s'", 
                OP_REGIONID, sWorkDate, sOutBank, 
                sRefId, sInoutflag);
        if(ret == E_DB_NORECORD) {
            ret = db_exec("insert into feelist values(%d, '%s', '%s', '%s', "
                    "'%s', '%s', '%s', '%s', %.2f, '%s')",
                    OP_REGIONID, sWorkDate, "1", sOutBank, sRefId, 
                    sWorkDate, pAcctNo, "", atof(sAmt)/100, "0");
            if(ret) {
                BKINFO("保存手续费清单失败");
                return E_APP_ACCOUNTFAIL;
            }
        }
        else if(ret)
            return E_APP_ACCOUNTFAIL;
    }
    if(atoi(retstr) == 1)   //已收费
        return E_APP_ACCOUNTSUCC;
#endif

    /* 向行内记帐,清空行内应答信息 */
    XMLSetNodeVal(opDoc, "//opBKRetcode","");
    XMLSetNodeVal(opDoc, "//opBKRetinfo","");
    ret = callInterface(atoi(pTrnCode), opDoc);
    if(ret)
        return E_APP_ACCOUNTFAIL;
    memset(sResult, 0, sizeof(sResult));
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if( strlen(sResult) != 0 && atoi(sResult) != 0)
        return E_APP_ACCOUNTFAIL;
    if( strlen(sResult) == 0 )
    {
        ret = db_exec("update acctjour set acctserial = '%s', revserial = '%s', result = '%s', reserved1 = '%s' "
                "where nodeid = %d and workdate = '%s' and originator = '%s' "
                "and refid = '%s' and inoutflag = '%s' and trncode = '%s'",
                sOpSerial, _RecvSerial, "2", XMLGetNodeVal(opDoc, "//opOreserved1"),
                OP_REGIONID, sWorkDate,
                sOutBank, sRefId, sInoutflag, pTrnCode);
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "交易结果不明确,请查询行内结果.");
        return E_SYS_COMM;
    }

    /* 保存记帐流水 */
    memset(_RecvSerial, 0, sizeof _RecvSerial);
    strcpy(_RecvSerial, XMLGetNodeVal(opDoc, "//opHostSerial"));
    XMLSetNodeVal(opDoc, "//opNotetype", sNoteType);
    BKINFO("返回行内流水:%s", _RecvSerial);

    /* reserved1字段保存行内返回的记帐日期 */
    ret = db_exec("update acctjour set acctserial = '%s', revserial = '%s', result = '%s', reserved1 = '%s' "
            "where nodeid = %d and workdate = '%s' and originator = '%s' "
            "and refid='%s' and inoutflag='%s' and trncode='%s'",
            sOpSerial, _RecvSerial, "1", XMLGetNodeVal(opDoc, "//opOreserved1"),
            OP_REGIONID, sWorkDate, 
            sOutBank, sRefId, sInoutflag, pTrnCode);

    return E_APP_ACCOUNTSUCC;
}

int CommToExbAfter(void *doc)
{
    int ret=0, iAcctFlag=0,iFeeType;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sNoteType[2+1]={0}, sOpSerial[20+1]={0};
    char sResult[6+1]={0}, sTruncFlag[1+1]={0}, sBankSerial[8+1]={0};
    char sFlCode[5+1]={0}, sAcctNo[32+1]={0}, sFee[15+1]={0};
    char sAcctDate[8+1]={0};
    char sSqlStr[1024]={0};
    char sOriginator[12+1]={0}, sPayAcct[36+1]={0}, sClearAcct[36+1]={0};
    int trncode;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));

    /* 实时收取手续费取消 */
#if 0
    /* 计算手续费 */
    ret = Fee_Cast(sAcctNo, sFee);
    if(ret == 0)
        XMLSetNodeVal(opDoc, "//opFee", sFee);
#endif

    strcpy(sResult, XMLGetNodeVal(opDoc, "//opTCRetcode"));

    if(sDCFlag[0] == '2' && atoi(sResult) == 0) {
        BKINFO("提出贷提交中心后中心处理成功，行内不处理账务");
        return 0;
    }
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));

    ret = IsAcctCode(sResult);
    if(ret == 7) {
        BKINFO("中心返回交易结果不明确");
        return E_SYS_COMM;
    }
    if(sDCFlag[0] == '1' && atoi(sResult) != 0) {
        BKINFO("提出借中心处理失败，行内不处理");
        return 0;
    }
    /* 判断账户类型 */
    iAcctFlag = IsAcctType(sAcctNo);
    /* 对私账户提出不记帐 */
    if ((iAcctFlag == 1 || iAcctFlag == 2) && atoi(sResult) == 0)
        return E_APP_ACCOUNTSUCC;
    /*
       else if(iAcctFlag != 0 && atoi(sResult) != 0)
       return E_APP_ACCOUNTANDCZ;
     */
    /* 需取消的交易 */
    if(atoi(sResult) != 0) {
        BKINFO("开始取消交易...");
        ret = AcctCancelQry(XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opOriginator"),
                XMLGetNodeVal(opDoc, "//opRefid"), XMLGetNodeVal(opDoc, "//opInoutflag"), sBankSerial, sAcctDate);
        if(ret)
            return E_APP_ACCOUNTNOCZ;
        /*内部账户则要发起反提交易进行*/
        if( iAcctFlag ==3 || iAcctFlag ==4) //内部账
        {
            BKINFO("内部户交易取消.");
            strcpy( sOriginator, XMLGetNodeVal(opDoc, "//opOriginator") );

            if( iAcctFlag == 4 )//7->28
                GetBankAcct( sAcctNo, sOriginator, 2 );
            BKINFO("内部5分户[%s].",sAcctNo);
            XMLSetNodeVal(opDoc, "//opBeneacct", sAcctNo);

            GetBankAcct( sClearAcct, sOriginator, 1 );
            BKINFO("清算账户[%s].",sClearAcct);
            XMLSetNodeVal(opDoc, "//opPayacct", sClearAcct);

            sprintf(sOpSerial, "%ld", GenSerial("account", 1, 999999999, 1));
            XMLSetNodeVal(opDoc, "//opOreserved1", sOpSerial);
            XMLSetNodeVal(opDoc, "//opTermno", "0000");
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "2");
            /*内部帐号付款,提出贷记走6130同8130*/
            trncode = 6130;
        }
        else
        {
            sprintf(sOpSerial, "%ld", GenSerial("account", 1, 999999999, 1));
            XMLSetNodeVal(opDoc, "//opOreserved2", sOpSerial);
            XMLSetNodeVal(opDoc, "//opOreserved3", sAcctDate);
            XMLSetNodeVal(opDoc, "//opHostSerial", sBankSerial);
            XMLSetNodeVal(opDoc, "//opAccountType", "0");
            trncode = 9121;
        }
        /* 在此只有提出贷业务已记帐 如果中心失败需取消 */
        ret = callInterface(trncode, opDoc);   //9121同8121冲正
        if(ret)
            return E_APP_ACCOUNTNOCZ;
        memset(sResult, 0, sizeof sResult);
        strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
        if(atoi(sResult) != 0)
            return E_APP_ACCOUNTNOCZ;
        ret = UpAcctSerial(opDoc, "1");
        return E_APP_ACCOUNTANDCZ;
    }
    sprintf(sSqlStr, "select truncflag from noteinfo \
            where nodeid = %d and notetype = '%s' and dcflag = '%s'", 
            OP_REGIONID, sNoteType, sDCFlag);
    ret = db_query_str(sTruncFlag, sizeof(sTruncFlag), sSqlStr);
    if(ret) {
        BKINFO("查询票据截留类型失败");
        return ret;
    }

    /* 记帐 */
    if(atoi(sTruncFlag) == 0 && atoi(sNoteType) != 3 && atoi(sNoteType) != 4)
        ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8130");
    else {
        strcpy(sFlCode, "0000");
        ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
    }

    return ret;
}

int CommToExbBefore(void *doc)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sNoteType[2+1]={0};
    char sResult[6+1]={0};
    char sFlCode[5+1]={0}, sAcctNo[32+1]={0};
    char sXyh[60+1]={0};
    char sTrnCode[5];
    int iAcctFlag;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
    if(sDCFlag[0] == '1') {
        BKINFO("提出借提交中心前行内不处理账务");
        return 0;
    }

    strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    BKINFO("检查账户性质...");
    iAcctFlag = IsAcctType(sAcctNo);
    /* 对私账户不记帐 */
    if(iAcctFlag == 1 || iAcctFlag == 2)
        return E_APP_ACCOUNTSUCC;

    /* 如果账号为内部户, 则调6130. acctlen=28 */
    if (iAcctFlag == 3)
    {
        sprintf(sTrnCode, "6130");
        XMLSetNodeVal(opDoc, "//opEXBKTxflag", "0");
    }
    else
    {
        sprintf(sTrnCode, "8120");
        /* 贷记业务非进账单通过协议方式记客户账 */
        if(atoi(sNoteType) != 1)
        {
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "4");
            /* 根据帐号获取协议号 */
            ret = Qry_Agreement(sAcctNo, sXyh);
            if(ret)
                return ret;
            XMLSetNodeVal(opDoc, "//opPaykey", sXyh);
        }
    }

    /* 记帐 */
    strcpy(sFlCode, "0000");
    ret = AcctToBank(opDoc, sAcctNo, sFlCode, sTrnCode);
    if(ret != E_APP_ACCOUNTSUCC)
        return ret;
    //BKINFO("RECVSERIAL:%s", _RecvSerial);
    //strcpy(gpBankSerial, _RecvSerial);
    //BKINFO("ACCTSERIAL:%s", _AcctSerial);

    return ret;
}

/* * 手续费
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_110(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[2+1]={0}, sAcctNo[32+1]={0};

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));

    ret = AcctToBank(opDoc, sAcctNo, "0002", "8120");

    return ret;
}

/*
 * 提入确认
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 111 -> 123
 */
int PF10_123(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *pDCFlag=NULL, *pNoteType=NULL;
    char sAcctNo[32+1]={0}, sXyh[60+1]={0};
    char where[1024] = {0};

    pDCFlag = XMLGetNodeVal(opDoc, "//opDcflag");
    pNoteType = XMLGetNodeVal(opDoc, "//opNotetype");

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='2'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'));

    BKINFO("提入确认:借贷[%s]", pDCFlag);
    XMLSetNodeVal(doc, "//opInoutflag", "2");

    if(atoi(pDCFlag) == 1)
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    else if(atoi(pDCFlag) == 2)
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));

    /* 提入借 转帐支票验印 其他协议方式 */
    if(atoi(pDCFlag) == 1) {
        if(atoi(pNoteType) != 2) {
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "4");
            /* 根据帐号获取协议号 */
            ret = Qry_Agreement(sAcctNo, sXyh);
            if(ret)
                return ret;
            XMLSetNodeVal(opDoc, "//opPaykey", sXyh);
        }
        else
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "1");
    }

    ret = AcctToBank(opDoc, sAcctNo, "0000", atoi(pDCFlag) == 1 ? "8120" : "8110");

    if (ret == E_APP_ACCOUNTSUCC)
        db_exec("UPDATE %s SET chkflag='1' %s", 
                strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);

    return ret;
}

/*
 * 提出修改
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_103(void *doc, char *p)
{
    int ret=0;

    ret = PF10_101(doc, p);

    return ret;
}

/*
 * 提出取消
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_301(void *doc, char *p)
{
    return 0;
}

/* 
   检查协议库, 返回平台错误码
 */
int ChkAgreement(char *id)
{
    result_set rs;
    int rc;

    rc = db_query(&rs, "select state from agreement where nodeid=%d "
            "and agreementid='%s'", OP_REGIONID, id);
    if (rc != 0)
    {
        if (rc == E_DB_NORECORD)
            rc = E_MNG_XY_NOTEXIST;
        return rc;
    }
    if (*db_cell(&rs, 0, 0) != '1')
    {
        rc = E_MNG_XY_CANCEL;
        return rc;
    }
    db_free_result(&rs);

    return 0;
}

/*
 * 提入记帐
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_104(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sResult[6+1]={0};
    char sCurCode[3+1]={0}, sNoteType[2+1]={0};
    char sSqlStr[1024]={0}, sTruncFlag[1+1]={0};
    char sAcctNo[32+1]={0}, sFlCode[4+1]={0};
    char sOrgId[12+1]={0}, sAcctCheck[1+1]={0};
    char sAcctState[1+1]={0}, sBAcctName[60+1]={0}, sTAcctName[60+1]={0};
    int iAcctFlag;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    /* 外币不自动记帐 */
    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//opCurcode"));
    if(memcmp(sCurCode, "CNY", 3) != 0) {
        BKINFO("外币无需自动记帐");
        return 0;
    }
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
    sprintf(sSqlStr, "select truncflag from noteinfo \
            where nodeid = %d and notetype = '%s' and dcflag = '%s'", 
            OP_REGIONID, sNoteType, sDCFlag);
    ret = db_query_str(sTruncFlag, sizeof(sTruncFlag), sSqlStr);
    if(ret) {
        BKINFO("查询票据截留类型失败");
        return ret;
    }

    //非截留票据不实时入帐
    if(atoi(sTruncFlag) == 0)
        return 0;

    if(atoi(sNoteType) == 41)
    {
        // 地税发送过来的缴税交易
        if ((ret = ChkAgreement(XMLGetNodeVal(opDoc, "//opAgreement"))) != 0)
            return ret;
    }

#if 1
    if(atoi(sNoteType) == 31)
    {
        XMLSetNodeVal(opDoc, "//opEXBKTxflag", "3");
        //return E_SYS_COMM;
    }
#endif

    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    else if(sDCFlag[0] == '2')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else {
        BKINFO("借贷标志不正确");
        return E_APP_ACCOUNTFAIL;
    }
    if(sAcctNo[0] == '*' && sDCFlag[0] == '1') {
        sprintf(sAcctNo, "%s", sAcctNo+1);
        XMLSetNodeVal(opDoc, "//opPayacct", sAcctNo);
    }
    else if(sAcctNo[0] == '*' && sDCFlag[0] == '2') {
        sprintf(sAcctNo, "%s", sAcctNo+1);
        XMLSetNodeVal(opDoc, "//opBeneacct", sAcctNo);
    }

    /* 由行内接口进行检查 */
#if 0
    strcpy(sAcctCheck, XMLGetNodeVal(opDoc, "//opAcctCheck"));
    /* 提入贷 判断是否需检查户名 */
    if(atoi(sAcctCheck) == 1 && sDCFlag[0] == '2')
    {
        strcpy(sTAcctName, XMLGetNodeVal(opDoc, "//opBenename"));
        ret = callInterface(9202, opDoc);   //9202同8202
        if(ret) {
            BKINFO("查询账户信息失败");
            return E_APP_ACCOUNTFAIL;
        }
        strcpy(sAcctState, XMLGetNodeVal(opDoc, "//opOreserved5"));
        if(atoi(sAcctState) == 1) {
            BKINFO("账户状态不正常");
            return E_APP_ACCOUNTFAIL;
        }
        strcpy(sBAcctName, XMLGetNodeVal(opDoc, "//opOreserved3"));
        if(strcmp(sTAcctName, sBAcctName) != 0) {
            BKINFO("户名不一致:行内[%s] 接收[%s]", sBAcctName, sTAcctName);
            return E_APP_ACCOUNTFAIL;
        }
    }
#endif
    ret = db_query_str(sOrgId, sizeof(sOrgId), "select bankid from bankinfo "
            "where exchno = '%s'", XMLGetNodeVal(opDoc, "//opAcceptor"));
    if (ret != 0)
    {
        BKINFO("行号[%s]转换成行内机构失败", XMLGetNodeVal(opDoc, "//opAcceptor"));
        return E_DB;
    }

    XMLSetNodeVal(opDoc, "//opInnerBank", sOrgId);
    iAcctFlag = IsAcctType(sAcctNo);
    ret = db_exec("update trnjour set resflag1 = '%d',innerorganid='%s' "
            "where nodeid = %d and workdate = '%s' "
            " and refid = '%s' and inoutflag = '%s' and originator = '%s'",
            iAcctFlag, sOrgId, OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            XMLGetNodeVal(opDoc, "//opInoutflag"),
            XMLGetNodeVal(opDoc, "//opOriginator"));
    if(ret) {
        BKINFO("更新帐号标志失败");
        return ret;
    }
    // 个人账户
    if (iAcctFlag == 1 || iAcctFlag == 2)
    {
        if (atoi(sDCFlag) == 1)
        {
            BKINFO("对私帐号没有提入借业务");
            ret = E_APP_ACCOUNTFAIL;
        }
        else
        {
            // 查询开户机构和比对户名
            XMLSetNodeVal(opDoc, "//opAcctNo", sAcctNo);
            ret = callInterface(8202, opDoc);
            if(ret)
                return ret;
            strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
            if(atoi(sResult) != 0)
                return atoi(sResult);
            strcpy(sOrgId, XMLGetNodeVal(opDoc, "//opAcctOrgid"));
            XMLSetNodeVal(opDoc, "//opInnerBank", sOrgId);

            BKINFO("个人账户入账调8110接口, orgid=[%s]...", sOrgId);
            strcpy(sFlCode, "0000"); //??
            ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
        }
    }
    else
    {
        XMLSetNodeVal(opDoc, "//opInoutflag", "2");
        strcpy(sFlCode, "0000");
        if(sDCFlag[0] == '1')
            ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8120");
        else if(sDCFlag[0] == '2')
        {
            /* 如果收款账号为内部户, 则调5130. acctlen=28 */
            if (strlen(sAcctNo) == 28) {
                XMLSetNodeVal(opDoc, "//opEXBKTxflag", "0");
                ret = AcctToBank(opDoc, sAcctNo, sFlCode, "5130");
            }
            else
                ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
        }
        else
            ret = E_APP_ACCOUNTFAIL;
    }

    return ret;
}

/*
 * 账户查询
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_401(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sResult[6+1]={0};
    char sOpenUnit[12+1]={0}, sOpenBank[12+1]={0};
    char sSqlStr[1024]={0};
    int iAcctFlag;

    ret = callInterface(8202, opDoc);
    if(ret)
        return ret;

    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
        return atoi(sResult);

    strcpy(sOpenUnit, XMLGetNodeVal(opDoc, "//opAcctOrgid"));
    iAcctFlag = IsAcctType(XMLGetNodeVal(doc, "//opAcctNo"));
    if (iAcctFlag < 0)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "账户类型检查出错");
        return 8999;
    }
    if (iAcctFlag != 1 && iAcctFlag != 2)
    {
        ret = db_query_str(sOpenBank, sizeof(sOpenBank), 
                "select exchno from bankinfo where bankid = '%s'", sOpenUnit);
        if (ret != 0)
            return E_DB;
    }
    XMLSetNodeVal(opDoc, "//opBankid", sOpenBank);

    return 0;
}

/*
 * 补记
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_106(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sOutBank[12+1]={0}, sWorkDate[8+1]={0};
    char sInoutflag[1+1]={0}, sRefid[32+1]={0};

    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));

    ret = Acct_Bj(opDoc, sWorkDate, sOutBank, sRefid, sInoutflag, 0);

    // 提出交易返回操作员姓名
    if (sInoutflag[0] == '1')
        SetOperInfo(opDoc);

    return ret;
}

/*
 * 冲正、取消
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_107(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sRefid[16+1]={0}, sInoutflag[1+1]={0}, sOutBank[12+1]={0}, sWorkDate[8+1]={0};
    char sBankSerial[8+1]={0}, sOpSerial[20+1]={0};
    char sSqlStr[1024]={0}, sResult[6+1]={0};
    char sTab[20+1]={0}, sAcctFlag[1+1]={0}, sAcctResult[1+1]={0};
    char sDCFlag[1+1]={0};
    long lTmp = 0L;
    int trncode;

    // 返回操作员姓名
    SetOperInfo(opDoc);

    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));

    if(OP_TCTCODE == 10) {
        strcpy(sTab, "hacctjour");
        strcpy(sAcctFlag, "1");
        strcpy(sAcctResult, "2");
    }
    else if(OP_TCTCODE == 12) {
        strcpy(sTab, "acctjour");
        strcpy(sAcctFlag, "0");
        strcpy(sAcctResult, "3");
    }
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(opDoc, "//opOreserved2", sOpSerial);

    sprintf(sSqlStr, "select acctserial from %s \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and inoutflag = '%s' and refid = '%s'",
            sTab, OP_REGIONID, sWorkDate, sOutBank, sInoutflag, sRefid);
    ret = db_query_str(sBankSerial, sizeof(sBankSerial), sSqlStr);
    if(ret)
        return ret;
    XMLSetNodeVal(opDoc, "//opSeqno", sBankSerial);
    XMLSetNodeVal(opDoc, "//opReserved", sAcctFlag);

    BKINFO("借贷标志:%s 提出入标志:%s", sDCFlag, sInoutflag);
    if((atoi(sDCFlag) == 1 && atoi(sInoutflag) == 1) ||
            (atoi(sDCFlag) == 2 && atoi(sInoutflag) == 2))
        trncode = 8111;
    else if((atoi(sDCFlag) == 2 && atoi(sInoutflag) == 1) ||
            (atoi(sDCFlag) == 1 && atoi(sInoutflag) == 2))
        trncode = 8121;
    ret = callInterface(trncode, opDoc);
    if(ret)
        return ret;
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
        return atoi(sResult);
    ret = db_exec("update %s set result = '%s' \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and inoutflag = '%s' and refid = '%s'",
            sTab, sAcctResult,
            OP_REGIONID, sWorkDate, sOutBank, sInoutflag, sRefid);
    return 0;
}

/*
 * 行内交易查询
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_402(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sResult[6+1]={0}, sRet[1+1]={0};
    char sRefid[16+1]={0}, sOutBank[12+1]={0}, sInoutflag[1+1]={0};
    char sWorkDate[8+1]={0}, sDCFlag[1+1]={0};
    char sSqlStr[1024]={0};

    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
#if 0
    /* 提出借或提入贷 提出行为收款行 */
    if((sDCFlag[0] == '1' && sInoutflag[0] == '1') 
            || (sDCFlag[0] == '2' && sInoutflag[0] == '2'))
        strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opBenebank"));
    /* 提入借或提出贷 提出行为付款行 */
    else if((sDCFlag[0] == '1' && sInoutflag[0] == '2') 
            || (sDCFlag[0] == '2' && sInoutflag[0] == '1'))
        strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opPaybank"));
#endif
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));

    ret = CallInBank8201(opDoc, sWorkDate, sOutBank, sRefid, sInoutflag);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(strlen(sResult) == 0 || atoi(sResult) != 0)
    {
        BKINFO("向行内查询交易状态失败");
        return ret;
    }
    ret = atoi(XMLGetNodeVal(opDoc, "//opTreserved1"));
    switch(ret)
    {
        case 1:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "交易成功" );
            break;
        case 0:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "交易失败" );
            break;
        case 2:
        default:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "交易结果不明确" );
            break;
    }

    db_exec("update acctjour set result = '%d' where nodeid = %d and workdate = '%s' \
            and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            ret, OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);

    return ret;
}

int PF10_702(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    int ret;

    /* 模拟返回
       XMLSetNodeVal(opDoc, "//opOreserved1", "1"); // 记录数
       XMLSetNodeVal(opDoc, "//opOreserved2", "20100622|320501061|401|100500604890010001|陈杰|123|收款户名|290020|1234.00|没有用途|");//内容
       return 0;
     */
    ret = callInterface(8204, doc);

    return ret;
}

int CallInBank8201(xmlDoc *doc, char *pDate, char *pBankNo, char *pSerial, char *pInoutflag)
{
    int ret=0;
    char sAcctSerial[12+1]={0}, sOpSerial[20+1]={0};
    char sSqlStr[1024]={0};
    long lTmp=0L;

    BKINFO("向行内查询记帐状态...");
    sprintf(sSqlStr, "select acctserial from acctjour \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBankNo, pSerial, pInoutflag);

    ret = db_query_str(sAcctSerial, sizeof(sAcctSerial), sSqlStr);
    if (ret) {
        BKINFO("查询记帐流水失败");
        return ret;
    }
    XMLSetNodeVal(doc, "//opTermno", sAcctSerial);
    /*
       else if (ret == E_DB_NORECORD)
       {
    // 未找到行内记账流水号,则返回未记账
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    ret = db_exec("insert into acctjour values(%d, '%08ld', '%s', '%s',"
    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
    "'%s', '%s', '%s')",
    OP_REGIONID, current_date(), XMLGetNodeVal(doc, "//opOriginator"), 
    XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opInoutflag"), 
    "8201", "", "", XMLGetNodeVal(doc, "//opInnerBank"), 
    XMLGetNodeVal(doc, "//opOperid"), "", "0", "0", sOpSerial, "", "", "");
    if (ret != 0)
    return ret;
    XMLSetNodeVal(doc, "//opBKRetcode", "0");
    XMLSetNodeVal(doc, "//opTreserved1", "999");
    XMLSetNodeVal(doc, "//opTreserved2", "无" );
    XMLSetNodeVal(doc, "//opBKRetinfo", "未记账");
    return 0;
    }
     */

    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(doc, "//opOreserved2", sOpSerial);

    ret = callInterface(8201, doc);
    if(ret)
        BKINFO("向行内查询记帐状态失败");

    return ret;
}

/*
 * 业务流水记帐流水比对
 */
int PF10_901(void *doc)
{
    int ret=0,i;
    xmlDoc *opDoc = (xmlDoc *)doc;
    xmlDoc *jzDoc;
    result_set yw_rs, fee_rs, jz_rs;
    char settledate[16] = {0};
    char sOriginator[13], sWorkDate[8+1]={0}, sClassid[1+1]={0};
    char sOutBank[12+1]={0}, sRefid[20+1]={0}, sInoutflag[1+1]={0};
    char sInnerBank[12+1]={0}, sAcctOper[12+1]={0};
    char filename[256];
    char condi[512];
    int clearround = 0;

    XmlGetString(opDoc, "//opCleardate", sWorkDate, sizeof(sWorkDate));
    XmlGetString(opDoc, "//opClassid", sClassid, sizeof(sClassid));
    XmlGetString(opDoc, "//opInnerBank", sInnerBank, sizeof(sInnerBank));
    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opOperid", sAcctOper, sizeof(sAcctOper));
    clearround = XmlGetInteger(doc, "//opClearround");
    strcpy(settledate, GetSettledDateround());

    BKINFO("种类:%d 场次:%d 已取对账日期场次:%s 系统日期:%s",
            sClassid, clearround, settledate, GetWorkdate());
    settledate[8] = 0;

    //当隔天第一场未对账时对账场次是前一天最后场次,当天晚上归档时将对账场次置0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, sWorkDate) != 0)
    {
        BKINFO("[%d]>[%d] ? , settldate[%s]!=currdate[%s] ?",
                clearround, atoi(settledate+9), settledate, sWorkDate);
        XMLSetNodeVal(doc, "//opTCRetinfo", "未对账,不允许打印记账清单");
        return 0;
    }

    /* 如果不为清算行发起, 则生成对账报表 */
    if (strcmp(GetCBankno(), sOriginator))
        goto gen_report;

    /* 比对业务流水 */
    ret = db_query(&yw_rs, "select refid, inoutflag, originator, dcflag, "
            "clearstate from trnjour where nodeid=%d and workdate='%s' and"
            " classid=%s and clearstate='C'", OP_REGIONID, sWorkDate, sClassid);
    for (i = 0; i < db_row_count(&yw_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&yw_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&yw_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&yw_rs, i, "originator"));
        ret = db_query(&jz_rs, "select result, acctserial from acctjour \
                where nodeid = %d and workdate = '%s' and originator = '%s' \
                and refid = '%s' and inoutflag = '%s'",
                OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
        /* reserved3为1已比对 */
        if(db_row_count(&jz_rs) == 0) {
            ret = db_exec("insert into acctjour values(%d, '%s', '%s', '%s',"
                    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
                    "'%s', '%s', '%s', '%s')",
                    OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag, "",
                    "", "", sInnerBank, sAcctOper, "", "0", "0", "", "", "1", "");
        }
        else {
            ret = db_exec("update acctjour set reserved3 = '1' "
                    "where nodeid = %d and workdate = '%s' and originator='%s'"
                    " and refid = '%s' and inoutflag = '%s'",
                    OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
        }
        if(ret)
            BKINFO("更新记录[提出行:%s 提出流水:%s]失败", sOutBank, sRefid);
        db_free_result(&jz_rs);
    }
    db_free_result(&yw_rs);

    /* 对记帐不成功或未记帐的交易进行补记 */
    jzDoc = getOPTemplateDoc(100);
    ret = db_query(&fee_rs, "select * from acctjour where nodeid = %d and "
            "workdate='%s' and (reserved3='1' or inoutflag in('3', '4')) "
            "and result!='1'", OP_REGIONID, sWorkDate);
    for (i = 0; i < db_row_count(&fee_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&fee_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&fee_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&fee_rs, i, "originator"));
        ret = Acct_Bj(jzDoc, sWorkDate, sOutBank, sRefid, sInoutflag, 0);
        /* 补记成功，更新为与行内已比对(reserved3 等于 2) 
           if(ret == E_APP_ACCOUNTSUCC)
           db_exec("update acctjour set reserved3 = '2' where nodeid = %d and "
           "workdate = '%s' and originator = '%s' and refid = '%s' and "
           "inoutflag = '%s'",
           OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
         */
    }
    db_free_result(&fee_rs);
    /* 对记帐成功的交易进行取消 */
    ret = db_query(&fee_rs, "select * from acctjour where nodeid = %d and "
            "workdate = '%s' and reserved3 not in('1', '2') and inoutflag not in('3', '4') "
            " and result = '1'",
            OP_REGIONID, sWorkDate);
    for (i = 0; i < db_row_count(&fee_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&fee_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&fee_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&fee_rs, i, "originator"));
        ret = Acct_Qx(jzDoc, sWorkDate, sOutBank, sRefid, sInoutflag);
    }
    db_free_result(&fee_rs);

gen_report:
    memset(filename, 0, sizeof(filename));
    BKINFO("Ready PrintSettleList()...");
    if ((ret = PrintSettleList(opDoc, filename)) == 0)
        XMLSetNodeVal(opDoc, "//opFilenames", filename);
    else
        BKINFO("PrintSettlList() ret=[%d]", ret);

    return 0;
}

//对账取消
int Acct_Qx(xmlDoc *opDoc, char *pDate, char *pBank, char *pSerial, char *pInoutflag)
{
    int ret=0;
    char sResult[6+1]={0};
    char sBankSerial[8+1]={0}, sAmount[15+1]={0};
    char sSqlStr[1024]={0};

    BKINFO("需取消记录日期[%s] 提出行[%s] 流水号[%s] 出入标志[%s]", 
            pDate, pBank, pSerial, pInoutflag);

    sprintf(sSqlStr, "select acctserial from acctjour "
            "where nodeid = %d and workdate = '%s' and "
            "originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_str(sBankSerial, sizeof(sBankSerial), sSqlStr);
    if(ret) {
        BKINFO("取消记帐查询原记帐流水失败");
        return ret;
    }

    sprintf(sSqlStr, "select settlamt from trnjour "
            "where nodeid = %d and workdate = '%s' and refid = '%s' "
            "and inoutflag = '%s' and originator = '%s'",
            OP_REGIONID, pDate, pSerial, pInoutflag, pBank);
    ret = db_query_str(sAmount, sizeof(sAmount), sSqlStr);
    if(ret) {
        BKINFO("取消记帐查询原交易失败");
        return ret;
    }
    XMLSetNodeVal(opDoc, "//opHreserved2", sBankSerial);
    XMLSetNodeVal(opDoc, "//opHreserved3", sAmount);
    XMLSetNodeVal(opDoc, "//opHreserved4", "0");

    ret = callInterface(7111, opDoc);
    if(ret) {
        BKINFO("取消记帐失败");
        return ret;
    }
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0) {
        BKINFO("取消记帐行内返回失败");
        return atoi(sResult);
    }
    db_exec("update acctjour set result = '%s' "
            "where where nodeid = %d and workdate = '%s' and "
            "originator = '%s' and refid = '%s' and inoutflag = '%s'",
            "3", OP_REGIONID, pDate, pBank, pSerial, pInoutflag);

    return ret;
}

//补记   iFlag:0补记 1当前表收妥 2历史表收妥
int Acct_Bj(xmlDoc *doc, char *pDate, char *pBank, char *pSerial, char *pInoutflag, int iFlag)
{
    int ret=0;
    //xmlDoc *doc = NULL;
    char sInnerBank[13];
    char sNoteType[2+1]={0}, sTruncFlag[1+1]={0}, sDCFlag[1+1]={0};
    char sTab[20+1]={0}, sBKTrnCode[6+1]={0}, sTxType[4+1]={0};
    char sAcctNo[32+1]={0};
    char sStr[1024]={0};

    BKINFO("日期[%s] 行号[%s] 流水[%s] 出入标志[%s] 补记收妥标志[%d]", pDate, pBank, pSerial, pInoutflag, iFlag);
    if(iFlag == 0 || iFlag == 1)
        strcpy(sTab, "trnjour");
    else                            //收妥
        strcpy(sTab, "htrnjour");
    sprintf(sStr, "nodeid = %d and workdate = '%s' and refid = '%s' and originator = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pSerial, pBank, pInoutflag);
    ret = QueryTable(doc, sTab, sStr);
    if (ret)
        return ret;
    XmlGetString(doc, "//opInnerorganid", sInnerBank, sizeof(sInnerBank));
    if (sInnerBank[0] == 0x00)
    {
        BKINFO("取行内记账机构号出错");
        return E_OTHER;
    }
    XMLSetNodeVal(doc, "//opInnerBank", sInnerBank);

    strcpy(sTruncFlag, XMLGetNodeVal(doc, "//opTruncflag"));
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//opDcflag"));
    //提出借 非截留
    if(sDCFlag[0] == '1' && sTruncFlag[0] == '0' && pInoutflag[0] == '1') {
        if(iFlag == 0)
            strcpy(sBKTrnCode, "7130");
        //收妥
        else {
            strcpy(sBKTrnCode, "7110");
            strcpy(sTxType, "0001");
        }
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //提出借 截留
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '1' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //提出贷 非截留
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '0' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //提出贷 截留
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '1' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //提入借 非截留
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '0' && pInoutflag[0] == '2')
        return 0;
    //提入借 截留
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '1' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //提入贷 非截留
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '0' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //提入贷 截留
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '1' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    else {
        BKINFO("借贷标志[%s] 截留标志[%s] 提出入标志[%s] 组合有误", sDCFlag, sTruncFlag, pInoutflag);
        return 0;
    }

    // 对私账户不记账
    if (IsAcctType(sAcctNo) != 0)
        return 0;

    //收妥不需要去行内检查状态
    if(strcmp(sTxType, "0001") == 0)
        goto ACCT;
    /* 查询行内记帐状态 */
    ret = CallInBank8201(doc, pDate, pBank, pSerial, pInoutflag);
    if(ret)
        return ret;

    if(XmlGetInteger(doc, "//opBKRetcode") == 0 
            && XmlGetInteger(doc, "//opTreserved1") == 0) {
        db_exec("update acctjour set result='1', acctserial='%s', trncode='%s'"
                " where nodeid = %d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='%s'",
                XMLGetNodeVal(doc, "//opTreserved2"), sBKTrnCode,
                OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
        return 0;
    }

ACCT:
    ret = AcctToBank(doc, sAcctNo, sTxType, sBKTrnCode);

    return ret;
}

/* 
 * 冲正取消 查询记帐交易流水 
 */
int AcctCancelQry(char *pDate, char *pBank, char *pSerial, char *pInoutflag, char *pBankSerial, char *pAcctDate)
{
    int ret=0;
    char sSqlStr[1024]={0};
    char sResult[1+1]={0}, sBankSerial[8+1]={0}, sAcctDate[8+1]={0};

    sprintf(sSqlStr, "select result from acctjour where nodeid = %d and workdate = '%s'"
            " and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_str(sResult, sizeof(sResult), sSqlStr);
    if(ret)
    {
        BKINFO("查询记帐结果失败");
        return E_DB;
    }
    if(atoi(sResult) != 1)
    {
        BKINFO("此交易无需冲正");
        return 0;
    }
    /* reserved2 保存为行内记帐日期 */
    sprintf(sSqlStr, "select acctserial, reserved2 from acctjour where nodeid = %d and workdate = '%s'"
            " and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_strs(sSqlStr, sBankSerial, sAcctDate);
    if(ret)
    {
        BKINFO("查询行内记帐流水失败");
        return E_DB;
    }
    strcpy(pBankSerial, sBankSerial);
    strcpy(pAcctDate, sAcctDate);

    return 0;
}

/* 提出借收妥 */
int PF10_122(void *doc, char *p)
{
    int ret=0,i,iFlag;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char filename[256];
    result_set yw_rs, tp_rs;
    char sClassid[1+1]={0}, sOutBank[12+1]={0};
    char sSTDate[8+1]={0}, sSTRound[1+1]={0}, sDQRound[1+1]={0};
    char sDQ_JHDate[8+1]={0}, sDQ_JHRound[1+1]={0};
    char sSysDate[8+1]={0}, sSysState[1+1]={0};
    char sGDDate[8+1]={0}, sTab[20+1]={0}, sTab1[20+1]={0};
    char sORefid[20+1]={0}, sOWorkdate[8+1]={0}, sTmp[64]={0};
    char sOOutbank[12+1];

    strcpy(sSTDate, XMLGetNodeVal(opDoc, "//opCleardate"));     //收妥日期
    strcpy(sSTRound, XMLGetNodeVal(opDoc, "//opClearround"));
    //strcpy(sClassid, XMLGetNodeVal(opDoc, "//opClassid"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));

    strcpy(sDQ_JHDate, GetExchgdate());
    strcpy(sDQ_JHRound, GetExchground());
    strcpy(sSysDate, GetWorkdate());
    strcpy(sSysState, GetSysStat());
    strcpy(sGDDate, GetArchivedate());
    strcpy(sDQRound, GetRound());

    /* 收妥日期大于系统日期不能收妥 */
    if(atol(sSTDate) > atol(sSysDate)) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "收妥日期或场次不正确");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* 收妥日期等于当前交换日期 并且收妥场次大于等于交换场次不能收妥 */
    else if(atol(sSTDate) == atol(sDQ_JHDate) 
            && atoi(sSTRound) >= atoi(sDQ_JHRound)) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "收妥日期或场次不正确");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* 收妥日期等于当前交换日期 并且 交换场次大于收妥场次1场 并且 系统为工作状态不能收妥 */
    else if(atol(sSTDate) == atol(sDQ_JHDate) && (atoi(sDQ_JHRound) - atoi(sSTRound) == 1) 
            && atoi(sSysState) == 1) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "收妥日期或场次不正确");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* 收妥日期小于当前交换日期 并且 当前交换场次为1 并且 系统为工作状态不能收妥 */
    /* chenjie 改成收妥日期＝当前交换日期 ??? */
    else if(atol(sSTDate) == atol(sDQ_JHDate) && atoi(sDQ_JHRound) == 1
            && atoi(sSysState) == 1) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "收妥日期或场次不正确");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    if(atol(sSTDate) <= atol(sGDDate)) {
        strcpy(sTab, "htrnjour");
        iFlag = 2;
    }
    else{
        strcpy(sTab, "trnjour");
        iFlag = 1;
    }

    /* 根据当天退票信息置原提出交易退票标志 */
    ret = db_query(&tp_rs, "select acceptor, agreement from trnjour where "
            "nodeid = %d and classid = %s and workdate = '%s' and "
            "inoutflag = '%s' and trncode = '%s'",
            OP_REGIONID, "1", sSTDate, "2", "7");
    if (db_row_count(&tp_rs) == 0)
        goto NEXT;
    for (i = 0; i < db_row_count(&tp_rs); i++)
    {
        memset(sTmp, 0, sizeof sTmp);
        memset(sOOutbank, 0, sizeof sOOutbank);
        memset(sOWorkdate, 0, sizeof sOWorkdate);
        memset(sORefid, 0, sizeof sORefid);
        strcpy(sTmp, db_cell_by_name(&tp_rs, i, "agreement"));
        strcpy(sOOutbank, db_cell_by_name(&tp_rs, i, "acceptor"));
        memcpy(sOWorkdate, sTmp, 8);
        strcpy(sORefid, sTmp+8+1);
        /* tpflag=1退票 */
        db_exec("update %s set tpflag = '%s' where nodeid = %d and classid = %s "
                "and originator = '%s' and refid = '%s' and workdate = '%s' and inoutflag = '%s'",
                sTab, "1", OP_REGIONID, "1", sOOutbank, sORefid, sOWorkdate, "1");
    }
    db_free_result(&tp_rs);

NEXT:
    ret = db_query(&yw_rs, "select * from %s where "
            "nodeid = %d and classid = %s and clearstate = '%s' and truncflag = '%s' "
            "and dcflag = '%s' and inoutflag = '%s' and exchgdate = '%s' and exchground = '%s' "
            "and originator = '%s' and notetype not in('%s', '%s') and tpflag = '0' and stflag='0'", //"and stflag='0'", add by chenjie
            sTab, OP_REGIONID, "1", "C", "0", "1", "1", 
            sSTDate, sSTRound, sOutBank, "03", "04");
    if (db_row_count(&yw_rs) == 0) {
        BKINFO("业务流水没有找到");
        return E_DB_NORECORD;
    }
    for (i = 0; i < db_row_count(&yw_rs); i++)
    {
        ret = Acct_Bj(opDoc, db_cell_by_name(&yw_rs, i, "workdate"), sOutBank, 
                db_cell_by_name(&yw_rs, i, "refid"), "1", iFlag);
        /* stflag=1收妥 */
        if(ret == E_APP_ACCOUNTSUCC)
            db_exec("update %s set stflag = '%s' where nodeid = %d and classid = %s and inoutflag = '%s'"
                    " and originator = '%s' and refid = '%s' and workdate = '%s'",
                    sTab, "1", OP_REGIONID, "1", "1", sOutBank,
                    db_cell_by_name(&yw_rs, i, "refid"), db_cell_by_name(&yw_rs, i, "workdate"));
    }
    db_free_result(&yw_rs);

    memset(filename, 0, sizeof(filename));
    BKINFO("Ready PrintAcceptNoteList()...");
    if ((ret = PrintAcceptNoteList(opDoc, filename)) == 0)
        XMLSetNodeVal(opDoc, "//opFilenames", filename);
    else
        BKINFO("PrintAcceptNoteList() ret=[%d]", ret);
    return ret;
}

/*
 * 批量收取手续费
 */
int PF10_153(void *doc, char *p)
{
    /* iFlag=1收手续费存在失败 iFlag=0收手续费全部成功 */
    int ret=0,i=0,iFlag=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char filename[256];
    char *pJzDate=NULL, *pJgh=NULL, sSxfAcct[32+1]={0};
    char *pKhAcct=NULL, sQsDate[8+1]={0}, *pJdbz=NULL;
    char *pWorkDate=NULL, *pOutBank=NULL, *pRefid=NULL;
    char *pSysDate=NULL, *pInoutflag=NULL, *pJhh=NULL;
    char sSqlStr[1024]={0}, sSxf[15+1]={0}, retstr[1+1]={0};
    char sXyh[60+1]={0}, sRemark[60+1]={0};
    char sTmp[64]={0}, sAcctName[60+1]={0};

    pJzDate = XMLGetNodeVal(opDoc, "//opWorkdate");
    pJgh = XMLGetNodeVal(opDoc, "//opInnerBank");
    pJhh = XMLGetNodeVal(opDoc, "//opOriginator");
    pSysDate = GetWorkdate();

    strcpy(sRemark, "交易成功");
    /* reserved 字段存放此机构上次收取手续费的最后日期 */
    /* reserved2字段存放此机构手续费帐号 */
    sprintf(sSqlStr, "select reserved, reserved2 from bankinfo where bankid = '%s'", pJgh);
    ret = db_query_strs(sSqlStr, sQsDate, sSxfAcct);
    if(ret) {
        strcpy(sRemark, "查询机构手续费帐号失败");
        goto END;
    }
    XMLSetNodeVal(opDoc, "//opBeneacct", sSxfAcct);
    /* feeflag=0没有收取手续费 resflag1=0是对公帐号 */
    sprintf(sSqlStr, "select * from htrnjour where nodeid = %d and workdate > '%s' and workdate <= '%s'"
            " and clearstate = 'C' and inoutflag = '1' and ((notetype in(select notetype from noteinfo "
            " where nodeid = %d and feepayer = '1') and dcflag = '2') or (notetype in(select notetype "
            " from noteinfo where nodeid = %d and feepayer = '2') and dcflag = '1')) and feeflag = '0' "
            " and originator = '%s' and resflag1 = '0'",
            OP_REGIONID, sQsDate, pJzDate, OP_REGIONID, OP_REGIONID, pJhh);
    ret = db_query(&rs, sSqlStr);
    if(ret == E_DB_NORECORD) {
        //strcpy(sRemark, "需收手续费的业务流水信息不存在");
        ret = 0;
        goto END;
    }
    else if(ret){
        strcpy(sRemark, "查询需收手续费的业务流水信息失败");
        goto END;
    }
    for (i = 0; i < db_row_count(&rs); i++)
    {
        /* 根据借贷标志获取需收取手续费的客户帐号 */
        pJdbz = db_cell_by_name(&rs, i, "dcflag");
        if(atoi(pJdbz) == 1)
            pKhAcct = db_cell_by_name(&rs, i, "beneacct");
        else if(atoi(pJdbz) == 2)
            pKhAcct = db_cell_by_name(&rs, i, "payingacct");

        pWorkDate = db_cell_by_name(&rs, i, "workdate");
        pOutBank = db_cell_by_name(&rs, i, "originator");
        pRefid = db_cell_by_name(&rs, i, "refid");
        pInoutflag = db_cell_by_name(&rs, i, "inoutflag");
        /* 计算收取的手续费 */
        ret = Fee_Cast(pKhAcct, sSxf);
        if(ret && ret != 1) {
            BKINFO("%s %s %s计算手续费失败", pWorkDate, pOutBank, pRefid);
            continue;
        }
        /* 检查收费信息是否已存在 */
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist "
                " where nodeid = %d and workdate = '%s' and inoutflag = '%s' "
                " and originator = '%s' and refid = '%s'",
                OP_REGIONID, pWorkDate, pInoutflag, pOutBank, pRefid);
        if(ret == E_DB_NORECORD) {
            /* 保存收费信息到手续费表 */
            sprintf(sSqlStr, "insert into feelist values(%d, '%s', '%s', '%s', '%s',"
                    "'%s', '%s', '%s', %.2f, %d, '%s', '%s', '', '', '', '', '')",
                    OP_REGIONID, pWorkDate, pInoutflag,
                    pOutBank, pRefid, pSysDate, pKhAcct, "", atof(sSxf), 1, "0", "0");
            ret = db_exec(sSqlStr);
            if(ret) {
                BKINFO("%s %s %s保存收费信息失败", pWorkDate, pOutBank, pRefid);
                continue;
            }
        }
        else if(ret) {
            BKINFO("%s %s %s查询收费信息失败", pWorkDate, pOutBank, pRefid);
            continue;
        }
    }
    db_free_result(&rs);
    /* resflag1=0为明细手续费信息 resflag1=1为汇总后手续费信息 */
    ret = db_query(&rs, "select acctno, count(*) as bs, sum(amount) as je from feelist "
            " where nodeid = %d and originator = '%s' and feedate = '%s' and result = '0' "
            " and resflag1 = '0' group by acctno",
            OP_REGIONID, pJhh, pSysDate);
    /* 保存手续费汇总信息并记帐 */
    for (i = 0; i < db_row_count(&rs); i++)
    {
        /* 汇总信息流水号组成 f+截至日期+顺序号 */
        sprintf(sTmp, "F%s%d", pJzDate, i);
        pKhAcct = db_cell_by_name(&rs, i, "acctno");
        memset(sSxf, 0, sizeof sSxf);
        strcpy(sSxf, db_cell_by_name(&rs, i, "je"));
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist where nodeid = %d "
                " and acctno = '%s' and workdate = '%s' and resflag1 = '1' and originator = '%s' ",
                OP_REGIONID, pKhAcct, pJzDate, pJhh);
        if(ret == E_DB_NORECORD) {
            /* 汇总信息工作日期为截至日期 */
            sprintf(sSqlStr, "insert into feelist values(%d, '%s', '%s', '%s', '%s',"
                    "'%s', '%s', '%s', %.2f, %s, '%s', '%s', '', '', '', '', '')",
                    OP_REGIONID, pJzDate, "1",
                    pJhh, sTmp, pSysDate, pKhAcct, "", atof(sSxf),
                    db_cell_by_name(&rs, i, "bs"), "0", "1");
            ret = db_exec(sSqlStr);
            if(ret) {
                BKINFO("%s 保存收费汇总信息失败", pKhAcct);
                continue;
            }
        }
        else if(ret) {
            BKINFO("%s 查询收费汇总信息失败", pKhAcct);
            continue;
        }

        if(atoi(retstr) == 1)   //手续费已收费成功的不再收费
            continue;
        /* 查询帐号扣手续费的协议号 */
        ret = Qry_Agreement(pKhAcct, sXyh);
        if(ret) {
            iFlag = 1;  
            continue;
        }

        /* 根据帐号查询户名 */
        ret = db_query_str(sAcctName, sizeof(sAcctName), "select name from acctinfo where acctid = '%s'", pKhAcct);
        if(ret) {
            BKINFO("%s 查询户名失败", pKhAcct);
            continue;
        }

        XMLSetNodeVal(opDoc, "//opTrdate", pSysDate); //系统工作日期
        XMLSetNodeVal(opDoc, "//opWorkdate", pJzDate);
        XMLSetNodeVal(opDoc, "//opRefid", sTmp);
        XMLSetNodeVal(opDoc, "//opOriginator", pJhh);
        XMLSetNodeVal(opDoc, "//opInnerBank", pJgh);
        XMLSetNodeVal(opDoc, "//opAgreement", sXyh);
        XMLSetNodeVal(opDoc, "//opPayacct", pKhAcct);
        XMLSetNodeVal(opDoc, "//opSettlamt", sSxf);
        XMLSetNodeVal(opDoc, "//opPayname", sAcctName);

        /* 向行内记帐 */
        ret = AcctToBank(opDoc, pKhAcct, "0002", "9130");
        if(ret != E_APP_ACCOUNTSUCC) 
            continue;
        /* 更新手续费信息表收费结果result=1收费成功 */
        ret = db_exec("update feelist set result = '1' where nodeid = %d and "
                " originator = '%s' and feedate = '%s' and acctno = '%s'",
                OP_REGIONID, pJhh, pSysDate, pKhAcct);
        if(ret)
            BKINFO("%s 行内记帐成功,本地更新失败", pKhAcct);

        /* 更新业务表中的收费标志feeflag=1已收手续费 */
        sprintf(sSqlStr, "update htrnjour set feeflag = '1' where nodeid = %d and workdate > '%s' and "
                " workdate <= '%s' and originator = '%s' and inoutflag = '1' and clearstate = 'C' and "
                " ((notetype in(select notetype from noteinfo where nodeid = %d and feepayer = '1') and dcflag = '2') "
                " or (notetype in(select notetype from noteinfo where nodeid = %d and feepayer = '2') and dcflag = '1')) "
                " and feeflag = '0' and resflag1 = '0' and ((dcflag = '1' and beneacct = '%s') or "
                " (dcflag = '2' and payingacct = '%s'))",
                OP_REGIONID, sQsDate, pJzDate, pJhh, OP_REGIONID, OP_REGIONID, pKhAcct, pKhAcct);
        ret = db_exec(sSqlStr);
        if(ret)
            BKINFO("%s 行内记帐成功,更新交易流水表失败", pKhAcct);
    }
    db_free_result(&rs);
    /* 更新此机构收费的最后日期也就是下一此收费的起始日期 */
    db_exec("update bankinfo set reserved = '%s' where bankid = '%s'", pJzDate, pJgh);
END:

    // 生成手续费收取情况报表
    if (ret == 0)
    {
        memset(filename, 0, sizeof(filename));
        BKINFO("Ready PrintFeeTransInfo()...");
        if ((ret = PrintFeeTransInfo(opDoc, filename)) == 0)
            XMLSetNodeVal(opDoc, "//opFilenames", filename);
        else
        {
            sprintf(sRemark, "生成手续费收取情况报表失败!");
            BKINFO("PrintFeeTransInfo() ret=[%d]", ret);
        }
    }

    BKINFO(sRemark);
    /*
       sprintf(sTmp, "%04d", ret);
       XMLSetNodeVal(opDoc, "//opTCRetcode", sTmp);
       XMLSetNodeVal(opDoc, "//opTCRetinfo", sRemark);
     */

    return ret;
}

/* 导出个人业务文件 123->154 */
int PF10_154(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char filename[256], cond[1024];
    char *opBankno, *pWorkDate, *pAcceptor;
    long lSerialno;
    double totamt;
    int totnum;
    FILE *fp;
    int rc, i;

    pWorkDate = XMLGetNodeVal(opDoc, "//opWorkdate");
    pAcceptor = XMLGetNodeVal(opDoc, "//opOriginator");
    opBankno = XMLGetNodeVal(opDoc, "//opInnerBank");

    sprintf(cond, "inoutflag='2' and acceptor='%s' and workdate='%s' and "
            "clearstate in('1','C') and dcflag='2' and resflag1 != '0'",
            pAcceptor, pWorkDate);

    rc = db_query(&rs, "select count(*), sum(settlamt) from trnjour"
            "where %s", cond);
    if (rc != 0)
        return rc;
    totnum = db_cell_i(&rs, 0, 0);
    totamt = db_cell_d(&rs, 0, 1);
    db_free_result(&rs);

    rc = db_query(&rs, "select beneacct,settlamt,fee,benename from trnjour "
            "where %s", cond);
    if (rc != 0)
        return rc;
    lSerialno = GenSerial("batch", 1, 999, 1);
    sprintf(filename, "%s/TC%s%06ld.TXT", getenv("FILES_DIR"), opBankno, 
            current_time());
    if ((fp = fopen(filename, "w")) == NULL)
    {
        db_free_result(&rs);
        return 999;
    }
    fprintf(fp, "%9s%06ld%03ld|%d|%.0lf|||||同城转入|\n", 
            opBankno, current_date()%1000000, lSerialno, totnum, totamt*100);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%d|%d|%s|%.0lf||%s|%s|||\n", 
                i+1, IsAcctType(db_cell(&rs, i, 0)), db_cell(&rs, i, 0), 
                db_cell_d(&rs, i, 1)*100, "10", db_cell(&rs, i, 3));
    }
    db_free_result(&rs);
    fclose(fp);

    return 0;
}

/* 打印手续费凭证 */
int PF10_762(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char sFeeDate[9];
    char amount[128], littl_amt[30];
    char acctserial[20], name[81];
    char printtime[30];
    char *chinesedate = NULL;
    char *acctno;
    FILE *fp;
    int i, rc;

    if (InitRptVar(opDoc) != 0)
        return E_OTHER;

    memset(sFeeDate, 0, sizeof(sFeeDate));
    strcpy(sFeeDate, XMLGetNodeVal(opDoc, "//opWorkdate"));

    chinesedate = ChineseDate(atol(sFeeDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/FeeNote.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.fn",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    rc = db_query(&rs, "select * from feelist where nodeid=%d "
            "and inoutflag='1' and workdate='%s' and originator='%s' "
            "and result='1' and resflag1='1'",
            OP_REGIONID, sFeeDate, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "");

    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    for (i = 0; i < db_row_count(&rs); i++)
    {
        // 查询记账流水
        db_query_str(acctserial, sizeof(acctserial),
                "select acctserial from acctjour "
                "where nodeid=%d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='3'", 
                OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                gs_originator, db_cell_by_name(&rs, i, "refid"));

        // 查询户名
        acctno = db_cell_by_name(&rs, i, "acctno");
        db_query_str(name, sizeof(name),
                "select name from acctinfo where acctid='%s' and nodeid=%d",
                acctno, OP_REGIONID);

        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "amount")));
        MoneyToChinese(db_cell_by_name(&rs, i, "amount"), amount);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                sFeeDate,
                acctno,
                name, 
                db_cell_by_name(&rs, i, "number"),
                littl_amt, 
                amount,
                printtime,
                gs_oper,
                acctserial);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }

    XMLSetNodeVal(opDoc, "//opFilenames", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

/*
   收妥清单
 */
int PrintAcceptNoteList(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char tbname[81];
    char printtime[30];
    char notetype_name[61];
    char *chinesedate = NULL;
    char sSTDate[9], sSTRound[3];
    char acctserial[20], sucbuf[40], failbuf[40];
    FILE *fp=NULL;
    int iSucSum, iFailSum;
    double dSucAmt, dFailAmt;
    int i, rc = 0;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    memset(sSTDate, 0, sizeof(sSTDate));
    strcpy(sSTDate, XMLGetNodeVal(xmlReq, "//opCleardate"));
    memset(sSTRound, 0, sizeof(sSTRound));
    strcpy(sSTRound, XMLGetNodeVal(xmlReq, "//opClearround"));
    if (DiffDate(sSTDate, GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    chinesedate = ChineseDate(atol(sSTDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/STList.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.st",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

    rc = db_query(&rs, "select * from %s where nodeid=%d and classid=1 "
            "and clearstate='C' and truncflag='0' and dcflag='1' and "
            "inoutflag='1' and exchgdate='%s' and exchground='%s' and "
            "originator='%s' and notetype not in('03','04') and tpflag='0'",
            tbname, OP_REGIONID, sSTDate, sSTRound, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, sSTRound,
            gs_bankid, gs_bankname);

    iSucSum = iFailSum = 0L;
    dSucAmt = dFailAmt = 0.0;
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if (atoi(db_cell_by_name(&rs, i, "stflag")) == 1)
        {
            // 收妥
            iSucSum++;
            dSucAmt += atof(db_cell_by_name(&rs, i, "settlamt"));
            db_query_str(acctserial, sizeof(acctserial),
                    "select acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and refid='%s' and inoutflag='1'", 
                    OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                    gs_originator, db_cell_by_name(&rs, i, "refid"));
        }
        else
        {
            iFailSum++;
            dFailAmt += atof(db_cell_by_name(&rs, i, "settlamt"));
            sprintf(acctserial, "未收妥");
        }
        db_query_str(notetype_name, sizeof(notetype_name),
                "select distinct name from noteinfo "
                "where notetype='%s'", db_cell_by_name(&rs, i, "notetype"));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "acceptor"), notetype_name, 
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "payer"),
                db_cell_by_name(&rs, i, "beneacct"),
                db_cell_by_name(&rs, i, "benename"),
                FormatMoney(db_cell_by_name(&rs, i, "settlamt")),
                acctserial);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);

    sprintf(sucbuf, "%.2lf", dSucAmt);
    sprintf(failbuf, "%.2lf", dFailAmt);
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    WriteRptFooter(fp, "%d;%s;%d;%s;%s;%s;\n", iSucSum, FormatMoney(sucbuf),
            iFailSum, FormatMoney(failbuf), printtime, gs_oper);
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }
    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

int PrintFeeTransInfo(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char tbname[81];
    char printtime[30];
    char *chinesedate = NULL;
    char sFeeDate[9];
    char acctserial[20], sucbuf[40], failbuf[40];
    FILE *fp=NULL;
    int iSucSum, iFailSum;
    double dSucAmt, dFailAmt;
    int i, rc = 0;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    memset(sFeeDate, 0, sizeof(sFeeDate));
    strcpy(sFeeDate, XMLGetNodeVal(xmlReq, "//opWorkdate"));
    /*
       memset(sSTDate, 0, sizeof(sSTDate));
       strcpy(sSTDate, GetTrnCtl("ExchgDate"));
       if (DiffDate(sSTDate, GetSysPara("ARCHIVEDATE")) <= 0)
       strcpy(tbname, "htrnjour");
       else
       strcpy(tbname, "trnjour");
     */

    chinesedate = ChineseDate(atol(sFeeDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/FeeInfo.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.fee",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

    rc = db_query(&rs, "select * from feelist where nodeid=%d "
            "and inoutflag='1' and workdate='%s' and originator='%s' "
            "and resflag1='1'",
            OP_REGIONID, sFeeDate, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, 
            GetWorkdate(), gs_bankid, gs_bankname);

    iSucSum = iFailSum = 0L;
    dSucAmt = dFailAmt = 0.0;
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if (atoi(db_cell_by_name(&rs, i, "result")) == 1)
        {
            // 收取成功
            iSucSum++;
            dSucAmt += atof(db_cell_by_name(&rs, i, "amount"));
            db_query_str(acctserial, sizeof(acctserial),
                    "select acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and refid='%s' and inoutflag='3'", 
                    OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                    gs_originator, db_cell_by_name(&rs, i, "refid"));
        }
        else
        {
            iFailSum++;
            dFailAmt += atof(db_cell_by_name(&rs, i, "amount"));
            sprintf(acctserial, "未收取");
        }

        fprintf(fp, "%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "refid"), 
                db_cell_by_name(&rs, i, "acctno"),
                db_cell_by_name(&rs, i, "number"),
                FormatMoney(db_cell_by_name(&rs, i, "amount")),
                acctserial);
    }
    db_free_result(&rs);
    sprintf(sucbuf, "%.2lf", dSucAmt);
    sprintf(failbuf, "%.2lf", dFailAmt);
    fprintf(fp, "合计;成功笔数:%d 金额: %s;;失败笔数: %d;金额: %s;\n", 
            iSucSum, FormatMoney(sucbuf), iFailSum, FormatMoney(failbuf));
    WriteRptRowCount(fp, i+1);

    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    WriteRptFooter(fp, "%s;%s;\n", printtime, gs_oper);
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }
    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

char *getClearState(char clearstate)
{
    switch(clearstate)
    {
        case CLRSTAT_SETTLED:   return "清算成功";
        case CLRSTAT_FAILED:    return "清算失败";
        case CLRSTAT_UNKNOW:    return "状态未知";
        case CLRSTAT_CHECKED:   return "已对账";
        case CLRSTAT_UNSETTLED: return "未清算";
        default: return "未知";
    }
}

/*
 * 打印记账清单
 * 输入:opDoc 平台报文 p 保留
 */
int PrintSettleList(void *opDoc, char *filename)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs, rs2;
    FILE *fp = NULL;
    char tbname[10] = "trnjour", tmp[1024] = {0};
    char caParaFile[256] = {0}, caDataFile[256] = {0}, caOutFile[256] = {0};
    char sOriginator[13], sInnerBank[13], sWorkDate[9];
    char acctstat[20];
    int classid, workround;
    int recordCount = 0, recordCount2 = 0;
    int i = 0, j = 0;
    int ret = 0;

    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opInnerBank", sInnerBank, sizeof(sInnerBank));
    XmlGetString(opDoc, "//opCleardate", sWorkDate, sizeof(sWorkDate));
    workround = XmlGetInteger(opDoc, "//opClearround");
    classid = XmlGetInteger(opDoc, "//opClassid");
    /*
       strcpy(settledate, GetSettledDateround());
       if ((cleardate = XMLGetNodeVal(doc, "//opCleardate")) == NULL)
       strcpy(currdate, getDate(0));
       else
       strcpy(currdate, cleardate);
       classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
       clearround = XmlGetInteger(doc, "//opClearround");

       BKINFO("种类:%d 场次:%d 已取对账日期场次:%s 系统日期:%s 机器日期:%s", 
       classid, clearround, settledate, GetWorkdate(), currdate);
       settledate[8] = 0;

    //当隔天第一场未对账时对账场次是前一天最后场次,当天晚上归档时将对账场次置0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, currdate) != 0) {
    BKINFO("[%d]>[%d] ? , settldate[%s]!=currdate[%s] ?",
    clearround, atoi(settledate+9),
    settledate, currdate);
    XMLSetNodeVal(doc, "//opTCRetinfo", "未对账,不允许打印记账清单");
    return 0;
    }
     */

    memset(tmp, 0, sizeof(tmp));
    if (strcmp(GetCBankno(), sOriginator))
        sprintf(tmp, "AND innerorganid='%s'", sInnerBank);
    ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
            "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
            "WHERE classid=%d %s AND workdate='%s' AND workround='%d' AND clearstate!='0' order by workround, clearstate, refid",
            tbname, classid, tmp, sWorkDate, workround);
    /*
       ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
       "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
       "WHERE classid=%d AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
       tbname, classid, currdate, clearround);
       ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
       "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
       "WHERE classid=%d AND ((originator='%s' and inoutflag='1') OR (acceptor='%s' and inoutflag='2'))"
       "  AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
       tbname, classid,
       XMLGetNodeVal(doc, "//opOriginator"), XMLGetNodeVal(doc, "//opOriginator"),
       currdate, clearround);
     */
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s;%s;%s;%s;%d;%d;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"),
            sWorkDate, classid, "所有场次", XMLGetNodeVal(doc, "//opOriginator"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++)
    {
        sprintf(tmp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                *db_cell(&rs, i, 4) == '1' ? "借" : "贷",
                db_cell(&rs, i, 5),
                db_cell(&rs, i, 6),
                db_cell(&rs, i, 9),
                db_cell(&rs, i, 11),
                FormatMoney(db_cell(&rs, i, 7)),
                getClearState(*db_cell(&rs, i, 13)));

        ret = db_query(&rs2, "SELECT acctserial, result FROM acctjour WHERE " 
                "nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%c'", 
                OP_REGIONID, sWorkDate, db_cell(&rs, i, 1), 
                sdpStringTrimHeadChar(db_cell(&rs, i, 0), '0'), *db_cell(&rs, i, 3));
        if (ret) {
            if (ret == E_DB_NORECORD) {
                fprintf(fp, "%s;%s;\n", tmp, "未记账");
                continue;
            } else {
                BKINFO("查询记帐流水失败,ret=%d", ret);
                db_free_result(&rs2);
                return ret;
            }
        }
        else { 
            if (db_cell_i(&rs2, j, 1) != 1)
                strcpy(acctstat, GetChineseName(acc_stat_list, db_cell_i(&rs2, j, 1)));
            else
                strcpy(acctstat, db_cell(&rs2, j, 0));
            fprintf(fp, "%s;%s;\n", tmp, acctstat);
        }
        db_free_result(&rs2);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, recordCount);
    WriteRptFooter(fp, "");
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/BankAcctList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("产生临时文件名或报表结果文件失败!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    sprintf(filename, "%s", basename(caOutFile));
    return 0;
}

/*
   交易查询行内账务处理
 */
int PF10_151(void *doc, char *p)
{
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sFlag[2], sInOutFlag[2];
    char sWorkDate[9], sOriginator[13], sRefId[17];
    int ret;

    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opWorkdate", sWorkDate, sizeof(sWorkDate));
    XmlGetString(opDoc, "//opRefid", sRefId, sizeof(sRefId));
    XmlGetString(opDoc, "//opInoutflag", sInOutFlag, sizeof(sInOutFlag));

    BKINFO("查询行内交易: workdate=[%s] originator=[%s] refid=[%s] io=[%s]...", 
            sWorkDate, sOriginator, sRefId, sInOutFlag);

    // 判断账户类型, 对私不向行内查询
    ret = db_query_str(sFlag, sizeof(sFlag), 
            "select resflag1 from trnjour where nodeid=%d "
            "AND workdate='%s' AND refid='%s' AND originator='%s' "
            "AND inoutflag='%s'", OP_REGIONID,
            sWorkDate, sRefId, sOriginator, sInOutFlag);
    if (ret != 0)
        return ret;
    /*
       if (*sFlag == '1') // 对私
       {
       XMLSetNodeVal(opDoc, "//opBKRetcode", "0");
       XMLSetNodeVal(opDoc, "//opTreserved1", "999");
       XMLSetNodeVal(opDoc, "//opTreserved2", "无" );
       XMLSetNodeVal(opDoc, "//opBKRetinfo", "对私业务不记账");
       return 0;
       }
     */

    /* 查询行内记帐状态 */
    ret = CallInBank8201(opDoc, sWorkDate, sOriginator, sRefId, sInOutFlag);

    if (ret == 0 && XmlGetInteger(opDoc, "//opBKRetcode") == 0 
            && XmlGetInteger(opDoc, "//opTreserved1") == 0) {
        db_exec("update acctjour set result='1', acctserial='%s' where "
                "nodeid = %d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='%s'",
                XMLGetNodeVal(opDoc, "//opTreserved2"), 
                OP_REGIONID, sWorkDate, sOriginator, sRefId, sInOutFlag);
    }

    return ret;
}
