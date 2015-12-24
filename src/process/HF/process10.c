#include "interface.h"
#include "sz_const.h"

#define GenAccountSerial() GenSerial("account", 1, 999999999, 1)

static char GSQL[SQLBUFF_MAX] = {0};
static int ret = 0;

#define ACCOUNT_OUTCREDIT               9001    //提出贷
#define ACCOUNT_INTRUNCDEBIT            9101    //提入借截留
#define ACCOUNT_INNOTRUNCDEBIT          9201    //提入借非截留
#define ACCOUNT_INNOTRUNCDEBIT_CHECK    9301    //提入借非截留确认
#define ACCOUNT_OUTDEBIT_TP             9401    //提入借非截留作退票
#define ACCOUNT_ZZFEE                   9501    //转账收手续费
#define ACCOUNT_CASHFEE                 9601    //现金收手续费
#define ACCOUNT_INCREDIT_TP             9701    //提出贷被退票自动处理
#define ACCOUNT_INCREDIT_TPCHK          9801    //提出贷被退票确认处理
#define ACCOUNT_CREDIT_REOUT            9901    //提出贷被退票重提出

#define ACCOUNT_OUTCASH_SAVE            8001    //提出现金通存
#define ACCOUNT_OUTZZ_SAVE              8101    //提出转账通存
#define ACCOUNT_IN_WITHDRAW             8201    //提入通兑

#define ACCOUNT_INCREDIT                9002    //提入贷自动或确认入账
#define ACCOUNT_OUTTRUNCDEBIT           9102    //提出借截留或即付入账
#define ACCOUNT_OUTNOTRUNCDEBIT         9202    //提出借非截留
#define ACCOUNT_OUTCREDIT_TPA           9302    //提入贷作退票A
#define ACCOUNT_OUTCREDIT_TPB           9502    //提入贷作退票B
#define ACCOUNT_OUTDEBIT_ST             9402    //提出借收妥
#define ACCOUNT_INDEBIT_TP              9602    //提出借被退票自动处理
#define ACCOUNT_INDEBIT_TPCHK           9702    //提出借被退票确认处理

#define ACCOUNT_IN_SAVE                 8002    //提入通存
#define ACCOUNT_OUTCASH_WITHDRAW        8102    //提出现金通兑
#define ACCOUNT_OUTZZ_WITHDRAW          8202    //提出转账通兑

#define ACCOUNT_CREDITCZ                9010    //贷记（存款）的冲正
#define ACCOUNT_DEBITCZ                 9011    //借记（扣款）的冲正

char *getAccountDesc(char *accttype)
{
    switch (atoi(accttype)) {
        case ACCOUNT_OUTCREDIT:             return "提出贷";
        case ACCOUNT_INTRUNCDEBIT:          return "提入借截";
        case ACCOUNT_INNOTRUNCDEBIT:        return "提入借非截";
        case ACCOUNT_INNOTRUNCDEBIT_CHECK:  return "提入借确认";
        case ACCOUNT_OUTDEBIT_TP:           return "提入借被退";
        case ACCOUNT_ZZFEE:                 return "转账收费";
        case ACCOUNT_CASHFEE:               return "现金收费";
        case ACCOUNT_INCREDIT_TP:           return "提入退票";
        case ACCOUNT_INCREDIT_TPCHK:        return "贷被退确认";
        case ACCOUNT_CREDIT_REOUT:          return "贷被退重提";

        case ACCOUNT_OUTCASH_SAVE:          return "提出现存";
        case ACCOUNT_OUTZZ_SAVE:            return "提出转存";
        case ACCOUNT_IN_WITHDRAW:           return "提入通兑";

        case ACCOUNT_INCREDIT:              return "提入贷确认";
        case ACCOUNT_OUTTRUNCDEBIT:         return "提出借截留";
        case ACCOUNT_OUTNOTRUNCDEBIT:       return "提出借非截";
        case ACCOUNT_OUTCREDIT_TPA:         return "入贷退票A";
        case ACCOUNT_OUTCREDIT_TPB:         return "入贷退票B";
        case ACCOUNT_OUTDEBIT_ST:           return "提出借收妥";
        case ACCOUNT_INDEBIT_TP:            return "借被退自动";
        case ACCOUNT_INDEBIT_TPCHK:         return "借被退确认";

        case ACCOUNT_IN_SAVE:               return "提入通存";
        case ACCOUNT_OUTCASH_WITHDRAW:      return "提出现兑";
        case ACCOUNT_OUTZZ_WITHDRAW:        return "提出转兑";

        case ACCOUNT_CREDITCZ:              return "贷记冲正";
        case ACCOUNT_DEBITCZ:               return "借记冲正";
        default: return "未知";
    }
}

int tpHandle(char dcflag, xmlDoc *doc, char *p);

static int FmtInsert(char *dbtable, char *fields[], int fieldnum)
{
    char sqlstr[8192];
    int i;

    memset(sqlstr, 0, sizeof(sqlstr));

    for (i = 0; i < fieldnum; i++)
    {
        strcat(sqlstr, "'");
        if (fields[i] != NULL)
            strcat(sqlstr, fields[i]);
        strcat(sqlstr, "',");
    }
    sqlstr[strlen(sqlstr)-1] = 0;

    if (db_exec("INSERT INTO %s VALUES(%s)", dbtable, sqlstr) != 0)
        return -1;

    return 0;
}

static int getcols( char *line, char *words[], int maxwords, int delim )
{
    char *p = line, *p2;
    int nwords = 0;
    int append = 0;

    if (line[strlen(line)-1] == delim)
        append = 1;

    while ( *p != '\0' )
    {
        words[nwords++] = p;
        if ( nwords >= maxwords )
            return nwords;

        while(1)
        {
            p2 = strchr( p, delim );
            if ( p2 == NULL )
                break;

            // 如果 delim字符前有斜杠则忽略
            if (p2-1 == NULL || *(p2-1) != '\\')
                break;

            memmove(p2-1, p2, strlen(p2)+1);
            p = p2;
        }
        if (p2 == NULL)
            break;

        *p2 = '\0';
        p = p2 + 1;
    }

    if (append == 1)
    {
        words[nwords] = strdup(" ");
        *(words[nwords]) = 0;
    }

    return nwords + append;
}

// 金额大小写转换
static int  MoneyToChinese ( char *money, char * chinese )
{
    int len, zerotype, i, unit_num , allzero = 1 ;
    char fundstr [ 51 ];
    char *numberchar [ ] =
    {
        "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"
    };
    char *rmbchar [ ] =
    {
        "分", "角", "", "元", "拾", "佰", "仟", "万", "拾", "佰", "仟", 
        "亿", "拾", "佰", "仟"
    };

    sprintf ( fundstr, "%.2lf", atof( money ) );
    len = strlen ( fundstr );
    unit_num = sizeof ( rmbchar ) / sizeof ( rmbchar [ 0 ] );

    for ( i = zerotype = 0, chinese [ 0 ] = '\0' ; i < len ; i++ )
    {
        switch ( fundstr [ i ] )
        {
            case '-':
                {
                    strcat ( chinese, "负" );
                    break;
                }
            case '.':
                {
                    if ( chinese [ 0 ] == '\0' )
                        strcat ( chinese, numberchar [ 0 ] );
                    if ( zerotype == 1 )
                        strcat ( chinese, rmbchar [ 3 ] );
                    zerotype = 0;
                    break;
                }
            case '0':
                {
                    if ( len - i  == 12 && 11 < unit_num )
                        strcat ( chinese, rmbchar [ 11 ] );
                    if ( len - i  == 8 && 7 < unit_num )
                    {
                        if( !allzero )
                            strcat ( chinese, rmbchar [ 7 ] );
                    }
                    zerotype = 1;
                    break;
                }
            default:
                {
                    if ( len - i  < 12 )
                        allzero = 0 ;
                    if ( zerotype == 1 )
                        strcat ( chinese, numberchar [ 0 ] );
                    strcat ( chinese, numberchar [ fundstr [ i ] - '0' ] );
                    if ( len - i - 1 < unit_num )
                        strcat ( chinese, rmbchar [ len - i - 1 ] );
                    zerotype = 0;
                    break;
                }
        }
    }

    if ( memcmp( fundstr + len -2 , "00", 2 ) == 0 ) strcat ( chinese, "整" );
    return 0;
}

//根据交易结果判断中心清算状态
int isClearstateUnknowTran(int tcResult)
{
    switch(tcResult)
    {
        //交易结果不明确,清算状态不确定
        case 201: case 202: case 203: case 204: case 205: case 207:
        case 3002: case 3003: case 8043: case 8048: case 8202: case 8203:
        case 8204: case 8205: return 1; 
        default: return 0;
    }
}

//返回是否截留凭证 失败返回-1
int getTruncflag(int notetype, char dcflag)
{
    char tmp[256] = {0};

    if (ret = db_query_str(tmp, sizeof(tmp),
                "SELECT truncflag FROM noteinfo WHERE nodeid=%d AND notetype='%02d' AND dcflag='%c'", 
                OP_REGIONID, notetype, dcflag)) {
        BKINFO("查询借贷[%c]票据[%02d]截留标志失败!ret=[%d]", dcflag, notetype, ret);
        return -1;
    }

    return atoi(tmp);
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

//根据凭证计算单笔手续费:成功 > 0 失败 < 0 不需收费 0
double feeCalculate(char dcflag, int notetype)
{
    char tmp[64] = {0};

    BKINFO("计算单笔手续费,凭证种类[%d]借贷[%c]", notetype, dcflag);
    
    ret = db_query_str(tmp, sizeof(tmp),
            "SELECT feepayer FROM noteinfo WHERE nodeid=%d AND dcflag='%c' AND notetype='%02d'", 
            OP_REGIONID, dcflag, notetype);
    if (ret) {
        BKINFO("查询凭证收费方向信息失败");
        return -1;
    }

    //贷记且付费方为付款方或借记且付费方为收款方
    if ((dcflag == '2' && atoi(tmp) == 1) || (dcflag == '1' && atoi(tmp) == 2)) {
        if (ret = db_query_str(tmp, sizeof(tmp), "SELECT value FROM feetype WHERE nodeid=%d AND typeid=1", OP_REGIONID)) {
            BKINFO("根据收费类型查询收费值失败");
            return -2;
        }
        return atof(tmp);
    } 
    return 0;
}

//收单笔手续费(0:成功 其它:失败)
int fetchFee(int bktcode, xmlDoc *doc, int isHtrnjour)
{
    char where[1024] = {0};
    char feeResult[4] = {0};

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='1'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), 
            XMLGetNodeVal(doc, "//opRefid"), 
            XMLGetNodeVal(doc, "//opOriginator"));

    if ((ret = db_query_str(feeResult, sizeof(feeResult), "SELECT result FROM feelist %s", where)) != 0) {
        if (ret != E_DB_NORECORD)
            return ret;
        //记录手续费流水
        if ((ret = InsertTableByID(doc, "feelist", 0)) != 0)
            return ret;
    } else if (feeResult[0] == '1') //已收费
        goto EXIT;

    XMLSetNodeVal(doc, "//opWorkdate", getDate(0));
    if ((ret = Accounting(bktcode, doc)) != E_APP_ACCOUNTSUCC)
        return ret;

    //更新收费流水
    if ((ret = db_exec("UPDATE feelist SET result='1',reserved1='%s' %s", 
                    XMLGetNodeVal(doc, "//opHostSerial"), where)) != 0)
        return ret;

EXIT:
    //更新交易流水
    db_exec("UPDATE %s SET feeflag='1', bankfee=%s %s", 
            isHtrnjour ? "htrnjour" : "trnjour", 
            XMLGetNodeVal(doc, "//opSettlamt"), where);

    return 0;
}

//记账并记录记账流水
int Accounting(int bktcode, xmlDoc *doc)
{
    char *p = NULL;

    BKINFO("进入记账程序流程...");

    p = XMLGetNodeVal(doc, "//opCurcode");
    // Modified by chenjie, add 'bktcode != ACCOUNT_OUTCREDIT'
    if (*p != 0 && strcmp("CNY", p) && bktcode != ACCOUNT_OUTCREDIT) {
        BKINFO("除提出贷外, 外币[%s]不记账", p);
        return 0;
    }
    if (bktcode == ACCOUNT_OUTCREDIT)
    {
        if (strcmp("USD", p) == 0)
            bktcode = 7001;
        else if (strcmp("GBP", p) == 0)
            bktcode = 7101;
        else if (strcmp("HKD", p) == 0)
            bktcode = 7201;
        else if (strcmp("JPY", p) == 0)
            bktcode = 7301;
        else if (strcmp("EUR", p) == 0)
            bktcode = 7401;
        else
            ;
    }

    p = sdpXmlSelectNodeText(doc, "//opNoteno");
    if (p != NULL && strlen(p) > 8) {
        BKINFO("凭证号太长,截断[%s->%s]", p, p+strlen(p)-8);
        XMLSetNodeVal(doc, "//opNoteno", p+strlen(p)-8); //行内凭证号最大8位(截最后8位)
    }

    XMLSetNodeVal(doc, "//opTrcode", vstrcat("%d", bktcode));
    XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%ld", GenAccountSerial()));

    if (ret = callInterface(bktcode, doc))
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
        return E_APP_ACCOUNTFAIL;

    if ((ret = InsertAcctjour(doc)) == 0)
        return E_APP_ACCOUNTSUCC;

    return ret;
}

//判断记账流水并记账
int CheckAccounting(int bktcode, xmlDoc *doc)
{
    char tmp[8] = {0};
    char where[1024] = {0};

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"));

    ret = db_query_str(tmp, sizeof(tmp), "SELECT count(1) FROM acctjour %s AND result='1' AND trncode='%d'", 
            where, bktcode);

    if (ret == E_DB_NORECORD) 
        ret = Accounting(bktcode, doc);
    return ret;
}

//冲正并更新记账流水
int AccountCancle(int bktcode, xmlDoc *doc)
{
    char acctResult[8] = {0};
    char acctSerial[20] = {0};
    char acctReserved1[20] = {0};
    char where[1024] = {0};
    char *p = NULL;

    sprintf(where, " WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"));

    sprintf(GSQL, "SELECT acctserial, result, reserved1 FROM acctjour %s", where);

    if (ret = db_query_strs(GSQL, acctSerial, acctResult, acctReserved1)) {
        BKINFO("查询原记帐流水失败");
        return E_DB_SELECT;
    }

    if (atoi(acctResult) != 1) {
        BKINFO("此交易无需冲正");
        return 0;
    }

    XMLSetNodeVal(doc, "//opHreserved1", acctReserved1); //原系统参考号
    XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%ld", GenAccountSerial()));

    if ((ret = callInterface(bktcode, doc)) != 0)
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
        return E_APP_CZFAIL;

    //冲正时产生的唯一交易流水保存在reserved2中,reserved1为记账时保留唯一交易流水
    ret = db_exec("UPDATE acctjour SET revserial='%s',result='2',reserved2='%s' %s",
            XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opHreserved5"), where);

    return 0;
}

/*
 * 复核记帐
 * 输入:opDoc 平台报文 p 保留
 * 返回:平台错误码
 */
int PF10_102(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char tmp[128] = {0};
    char *ptmp = NULL;
    char dcflag = 0;
    int tcResult = 0;
    int notetype = 0;
    int truncflag = 0;

    BKINFO("与中心通讯前后标志:%s", p);

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 

    if (OP_TCTCODE == 7)
        return tpHandle(dcflag, doc, p);

    if (p[0] == COMMTOPH_BEFORE[0])    //与中心通讯前
    {
        if (dcflag == OP_CREDITTRAN)
        {
            ptmp = XMLGetNodeVal(doc, "//opPayacct");
            if (ptmp != NULL && ptmp[0] == '*')
                XMLSetNodeVal(doc, "//opPayacct", ptmp+1);
            ret = Accounting(ACCOUNT_OUTCREDIT, doc);  //提出贷记
            XMLSetNodeVal(doc, "//opPayacct", ptmp);
        }
        else if (dcflag == OP_DEBITTRAN)
            BKINFO("提出借记发送中心前不记账");
    } 
    else if (p[0] == COMMTOPH_AFTER[0])  //与中心通讯后
    {
        tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
        if (!tcResult)
        { 
            double fee = 0;
            ptmp = XMLGetNodeVal(doc, dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct");

            BKINFO("判断提出是否个人账号[%s]借贷[%c]", ptmp, dcflag);
            if (ptmp != NULL && ptmp[0] == '*')
                XMLSetNodeVal(doc, (dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct"), ptmp+1);

            if (ptmp != NULL && strlen(ptmp) > 1 && (*ptmp == '1' || (*ptmp == '*' && ptmp[1] == '1'))) { 
                //提出个人账号实时收手续费
                if ((fee = feeCalculate(dcflag, atoi(XMLGetNodeVal(doc, "//opNotetype")))) > 0) {
                    sprintf(tmp, "%.2lf", fee);
                    XMLSetNodeVal(doc, "//opBankFee", tmp);
                }
            }
        }
        if (dcflag == OP_CREDITTRAN) {
            if (isClearstateUnknowTran(tcResult))
                return E_SYS_COMM;
            if (tcResult) {
                if ((ret = AccountCancle(ACCOUNT_DEBITCZ, doc)) == 0) //提出贷记冲正
                    return E_APP_ACCOUNTANDCZ;
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            else
                BKINFO("提出贷发送中心后中心处理成功,行内不处理账务");
        } 
        else if (dcflag == OP_DEBITTRAN) {
            if (tcResult) {
                BKINFO("提出借中心处理失败,行内不处理");
                return E_APP_NONEEDACCOUNT;
            } else {
                notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
                if (notetype == 3 || notetype == 4) { //银行本票,三省一市银行汇票
                    ret = Accounting(ACCOUNT_OUTTRUNCDEBIT, doc); //提出借即付票据
                } else {
                    if ((truncflag = getTruncflag(notetype, dcflag)) < 0)
                        return E_DB_SELECT;

                    if (truncflag)
                        ret = Accounting(ACCOUNT_OUTTRUNCDEBIT, doc); //提出截留借记
                    else
                        ret = Accounting(ACCOUNT_OUTNOTRUNCDEBIT, doc); //提出非截留借记
                }
                //should return 0 -- add by zhaoxq
                return 0;
            }
        }
    } else 
    {
        BKINFO("与中心通讯前后标志不合法");
        return E_SYS_CALL;
    }
    return ret;
}

/*
 * 提入记帐
 * 输入:opDoc 平台报文 p 保留
 * 返回:平台错误码
 */
int PF10_104(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char dcflag = 0;
    int truncflag = 0;
    int notetype = 0;
    char tmp[128] = {0};
    char *pp = NULL;

    if (*XMLGetNodeVal(doc, "//opClassid") == '2') //个人业务
        return PF10_108(opDoc, p);

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 

    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
    if ((truncflag = getTruncflag(notetype, dcflag)) < 0)
        return E_DB_SELECT;

    if (dcflag == OP_DEBITTRAN) 
        strcpy(tmp, XMLGetNodeVal(doc, "//opPayacct"));
    else if (dcflag == OP_CREDITTRAN)
        strcpy(tmp, XMLGetNodeVal(doc, "//opBeneacct"));

    if ((pp = strchr(tmp, '-')) != NULL) {
        memcpy(pp, pp+1, strlen(pp+1));
        tmp[strlen(tmp)-1] = 0;
    }

    if (tmp[0] == '*')
        XMLSetNodeVal(doc, (dcflag == OP_DEBITTRAN ? "//opPayacct" : "//opBeneacct"), tmp+1);

    if (OP_TCTCODE == 7)
        return tpHandle(dcflag, doc, NULL);

    if (dcflag == OP_CREDITTRAN) //提入贷记直接返回成功
    {
        if (!truncflag) //非截留贷记不处理(保留)
            return 0;
        if (XMLGetNodeVal(doc, "//opAcctCheck")[0] == '1')
        {
            if (ret = callInterface(2004, doc)) {
                BKINFO("提入贷查询账户信息失败");
                return 0;
            }
            if (strcmp(XMLGetNodeVal(doc, "//opOreserved1"), XMLGetNodeVal(doc, "//opBenename"))) {
                BKINFO("提入贷检查户名不一致:行内[%s]同城[%s]", 
                        XMLGetNodeVal(doc, "//opOreserved1"), XMLGetNodeVal(doc, "//opBenename"));
                return 0;
            }
        }
        Accounting(ACCOUNT_INCREDIT, doc); //贷记存款交易
        return 0;
    } 
    else if (dcflag == OP_DEBITTRAN)
    {
        // 地税发送过来的缴税交易
        if(notetype == 41)
        {
            if ((ret = ChkAgreement(XMLGetNodeVal(opDoc, "//opAgreement")))!=0)
                return ret;
        }

        //提入借要区分本票汇票专用科目
        if (truncflag)
            ret = Accounting(ACCOUNT_INTRUNCDEBIT, doc);  //提入借截留
        else
            ret = Accounting(ACCOUNT_INNOTRUNCDEBIT, doc);  //提入借非截留
    }

    return ret;
}

//提出提入退票处理
int tpHandle(char dcflag, xmlDoc *doc, char *p)
{
    char tblName[16] = "trnjour";
    char *ptmp = NULL;
    int tcResult;

    BKINFO("开始提出提入退票处理,借贷[%c]p[%s]", dcflag, p);

    //更新原交易被退票标志
    if (strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");
    ptmp = XMLGetNodeVal(doc, "//opAgreement");

    ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
            "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c'",
            tblName, OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, p == NULL ? '1' : '2');

    if (p == NULL) { //提入
        if (dcflag == OP_DEBITTRAN) { //提入借退票
            ret = Accounting(ACCOUNT_INDEBIT_TP, doc);
        } else if (dcflag == OP_CREDITTRAN) { //提入贷退票
            ret = Accounting(ACCOUNT_INCREDIT_TP, doc);
        }
    } else if (p[0] == COMMTOPH_BEFORE[0]) {   //与中心通讯前
        if (dcflag == OP_CREDITTRAN) { //提出贷退票
            if ((ret = Accounting(ACCOUNT_OUTCREDIT_TPA, doc)) == E_APP_ACCOUNTSUCC)
                ret = Accounting(ACCOUNT_OUTCREDIT_TPB, doc);
        }/* else if (dcflag == OP_DEBITTRAN) { //提出借退票
            ret = Accounting(ACCOUNT_OUTDEBIT_TP, doc);
        }*/
    }
    else if (p[0] == COMMTOPH_AFTER[0])  { //与中心通讯后
        tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
        if (!tcResult)
        {
            if (dcflag == OP_DEBITTRAN) { //提出借退票
                ret = Accounting(ACCOUNT_OUTDEBIT_TP, doc);
            }
        }
    }

    return ret;
}

/*
 * 现金转账通存通兑(查询缴税)
 * 输入:opDoc 平台报文 p 保留
 * 返回:平台错误码
 */
int PF10_108(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    int notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
    char pwd[40] = {0};
    char tmp[64] = {0};
    char dcflag = 0;
    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 

    BKINFO("直接提出提入交易:凭证[%d]通讯前后标志[%s]", notetype, p);

    if (notetype == 14) { //查询缴税
        if (p[0] == COMMTOPH_BEFORE[0])
            return Accounting(ACCOUNT_OUTCREDIT, doc);
        return 0;
    }

    if (p == NULL) { //提入
        if (notetype == 71 || notetype == 73) { //通存
            if (strcmp(XMLGetNodeVal(doc, "//opMac"), vstrcat("%d", E_SYS_SYDDECRYPT)) == 0)
                return E_SYS_SYDDECRYPT;
            ret = Accounting(ACCOUNT_IN_SAVE, doc);
        } else if (notetype == 72 || notetype == 74) { //通兑
#ifndef SYD_ENC
            DBUG("Ready SJL05Decrypt()...");
            if (ret = SJL05Decrypt(XMLGetNodeVal(doc, "//opAgreement"), XMLGetNodeVal(doc, "//opTrackInfo2"), pwd))
                return ret;
#else
            DBUG("Ready SYDDecrypt()...");
            if (ret = SYDDecrypt(XMLGetNodeVal(doc, "//opAgreement"), XMLGetNodeVal(doc, "//opTrackInfo2"), pwd))
                return ret;
#endif
            DBUG("Ready BankPwdEncrypt()...");
            if (ret = BankPwdEncrypt(pwd, tmp))
                return ret;
            XMLSetNodeVal(doc, "//opEXBKBankPwd", tmp);
            DBUG("Ready Accounting()...");
            ret = Accounting(ACCOUNT_IN_WITHDRAW, doc);
        }
        DBUG("Accouting ret=[%d] ...", ret);
        if (isSuccess(ret))
            return 0;
        DBUG("Accounting fail, ret=[%d]", ret);
    } else if (p[0] == COMMTOPH_BEFORE[0]) { //与中心通讯前
        if (notetype == 71) //现金通存
            ret = Accounting(ACCOUNT_OUTCASH_SAVE, doc);
        else if (notetype == 73) { //个人转账贷
            OFP_Decrypt(XMLGetNodeVal(doc, "//opAgreement"), pwd);
            if (ret = BankPwdEncrypt(pwd, tmp))
                return ret;
            XMLSetNodeVal(doc, "//opEXBKBankPwd", tmp);
            ret = Accounting(ACCOUNT_OUTZZ_SAVE, doc);
        } else if (notetype == 72 || notetype == 74) {
            OFP_Decrypt(XMLGetNodeVal(doc, "//opAgreement"), pwd);
            //DBUG("个人密码:[%s]", pwd);
#ifndef SYD_ENC
            if (ret = SJL05Encrypt(XMLGetNodeVal(doc, "//opTrackInfo2"), pwd, tmp))
                return ret;
#else
            if (ret = SYDEncrypt(XMLGetNodeVal(doc, "//opTrackInfo2"), pwd, tmp))
                return ret;
#endif
            //DBUG("个人密码加密结果:[%s]", tmp);
            XMLSetNodeVal(doc, "//opAgreement", tmp);
        }
    } else if (p[0] == COMMTOPH_AFTER[0])  { //与中心通讯后
        if (atoi(XMLGetNodeVal(doc, "//opTCRetcode")))
            return E_APP_NONEEDACCOUNT;
        if (notetype == 72) //现金通兑
            ret = Accounting(ACCOUNT_OUTCASH_WITHDRAW, doc);
        else if (notetype == 74) //个人转账借
            ret = Accounting(ACCOUNT_OUTZZ_WITHDRAW, doc);

        double fee = 0;
        if ((fee = feeCalculate(dcflag, atoi(XMLGetNodeVal(doc, "//opNotetype")))) > 0) {
            sprintf(tmp, "%.2lf", fee);
            XMLSetNodeVal(doc, "//opBankFee", tmp);
        }
    }

    return ret;
}

/*
 * 提入借贷(退票)确认
 * 输入 opDoc 平台报文 p 保留
 */
int PF10_123(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char where[1024] = {0};
    char workdate[9] = {0};
    char dcflag = 0;
    char tmp[20] = {0};

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 

    strcpy(workdate, XMLGetNodeVal(doc, "//opWorkdate"));
    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='2'",
            OP_REGIONID, workdate, XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'));

    BKINFO("提入确认:借贷[%c]", dcflag);

    XMLSetNodeVal(doc, "//opInoutflag", "2");
    // 记账使用当天系统日期
    XMLSetNodeVal(doc, "//opWorkdate", getDate(0));

    if (*XMLGetNodeVal(doc, "//opReserved") == '7') { //提入退票确认
        if (dcflag == OP_DEBITTRAN)
            ret = Accounting(ACCOUNT_INDEBIT_TPCHK, doc);
        else if (dcflag == OP_CREDITTRAN)
            ret = Accounting(ACCOUNT_INCREDIT_TPCHK, doc);
    } else {
        if (dcflag == OP_DEBITTRAN) { //提入借非截留应该已经记过9201再确认记账
            ret = db_query_str(tmp, sizeof(tmp), "SELECT count(1) FROM acctjour %s AND result='1' AND trncode in('%d', '%d')", 
                    where, ACCOUNT_INNOTRUNCDEBIT, ACCOUNT_INNOTRUNCDEBIT_CHECK);
            if (ret == 0) {
                if (atoi(tmp) == 1)
                    ret = Accounting(ACCOUNT_INNOTRUNCDEBIT_CHECK, doc);
                else if (atoi(tmp) == 2)
                    return E_APP_ACCOUNTSUCC; // modified by chenjie, E_DB_NORECORD->E_APP_ACCOUNTSUCC
                else if (atoi(tmp) == 0) // add by chenjie
                    return E_DB_NORECORD;
            }
        } else if (dcflag == OP_CREDITTRAN) { //提入贷未记账可以再确认记账
            ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s", where);
            if (ret == E_DB_NORECORD)
                ret = Accounting(ACCOUNT_INCREDIT, doc);
            else
                return E_DB_NORECORD;
        } else
            return E_DB_NORECORD;
    }

    if (ret == E_APP_ACCOUNTSUCC)
        db_exec("UPDATE %s SET chkflag='1' %s", 
                strcmp(workdate, GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);

    return ret;
}

/*
 * 提出借收妥 
 * 输入 opDoc 平台报文 p 保留
 */
int PF10_122(void *opDoc, char *p) 
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set tpRS, ywRS;
    const char *pSTDate = XMLGetNodeVal(doc, "//opWorkdate");
    const char *pSTRound = XMLGetNodeVal(doc, "//opWorkround");
    const char *pOriginator = XMLGetNodeVal(doc, "//opOriginator");
    const char *pInfo = "";
    const char *ptmp = NULL;
    char tblName[10] = "trnjour";
    int recordCount = 0;
    int i = 0;

    if (strcmp(pSTDate, GetWorkdate()) > 0) {
        pInfo = "收妥日期不能大于当前工作日期";
        goto EXIT;
    }

    if (strcmp(pSTDate, GetExchgdate()) == 0) {
        if (strcmp(pSTRound, GetExchground()) >= 0) {
            pInfo = "收妥场次必需小于当前交换场次";
            goto EXIT;
        }
        if (atoi(GetExchground()) - atoi(pSTRound) == 1 && *GetSysStat() == '1') {
            pInfo = "交换场次大于收妥场次1场且系统为工作状态不允许收妥";
            goto EXIT;
        }
        if (*GetExchground() == '1' && *GetSysStat() == '1') {
            pInfo = "收妥日期等于当前交换日期且当前交换场次为1且系统为工作状态不能收妥";
            goto EXIT;
        }
    }

    if (strcmp(pSTDate, GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");

    //查询提入退票交易(是否指定提入机构?按机构进行收妥?)
    ret = db_query(&tpRS, "SELECT acceptor, agreement FROM %s WHERE "
            "nodeid=%d and classid=1 and workdate='%s' and inoutflag='2' and workround='%s' and trncode='7'",
            tblName, OP_REGIONID, pSTDate, pSTRound);
    if (ret && ret != E_DB_NORECORD)
        goto EXIT;

    recordCount = db_row_count(&tpRS);
    for (i = 0; i < recordCount; i++)
    {
        ptmp = db_cell_by_name(&tpRS, i, "agreement");
        ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                "nodeid=%d and classid=1 and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='1'",
                tblName, OP_REGIONID, db_cell_by_name(&tpRS, i, "acceptor"), ptmp+9, ptmp);
    }

    //查询已对账提出非截留借记且非本汇票交易进行收妥
    ret = db_query(&ywRS, "SELECT * FROM %s WHERE nodeid=%d and classid=1 and clearstate='C' and truncflag='0' "
            "and dcflag='1' and inoutflag='1' and workdate='%s' and workround='%s' "
            "and originator='%s' and notetype not in('03', '04') and stflag!='1'",
            tblName, OP_REGIONID, pSTDate, pSTRound, pOriginator);
    if (ret)
        goto EXIT;

    recordCount = db_row_count(&ywRS);
    for (i = 0; i < recordCount; i++)
    {
        XMLSetNodeVal(doc, "//opSettlamt",   db_cell_by_name(&ywRS, i, "settlamt"));
        XMLSetNodeVal(doc, "//opNodeid",     db_cell_by_name(&ywRS, i, "nodeid"));
        XMLSetNodeVal(doc, "//opWorkdate",   db_cell_by_name(&ywRS, i, "workdate"));
        XMLSetNodeVal(doc, "//opRefid",      db_cell_by_name(&ywRS, i, "refid"));
        XMLSetNodeVal(doc, "//opInoutflag",  "1");
        XMLSetNodeVal(doc, "//opOriginator", db_cell_by_name(&ywRS, i, "originator"));
        XMLSetNodeVal(doc, "//opBankno",     db_cell_by_name(&ywRS, i, "innerorganid"));
        XMLSetNodeVal(doc, "//opOperid",     db_cell_by_name(&ywRS, i, "acctoper"));
        XMLSetNodeVal(doc, "//opCurcode",    db_cell_by_name(&ywRS, i, "curcode"));
        XMLSetNodeVal(doc, "//opCurtype",    db_cell_by_name(&ywRS, i, "curtype"));
        XMLSetNodeVal(doc, "//opPayname",    db_cell_by_name(&ywRS, i, "payer"));
        XMLSetNodeVal(doc, "//opPaybank",    db_cell_by_name(&ywRS, i, "payingbank"));
        XMLSetNodeVal(doc, "//opBeneacct",   db_cell_by_name(&ywRS, i, "beneacct"));
        XMLSetNodeVal(doc, "//opBenename",   db_cell_by_name(&ywRS, i, "benename"));
        XMLSetNodeVal(doc, "//opNotetype",   db_cell_by_name(&ywRS, i, "notetype"));
        XMLSetNodeVal(doc, "//opNoteno",     db_cell_by_name(&ywRS, i, "noteno"));

        ret = Accounting(ACCOUNT_OUTDEBIT_ST, doc);
        if (ret == E_APP_ACCOUNTSUCC)
        {
            db_exec("UPDATE %s SET stflag='1' WHERE "
                    "nodeid=%d and inoutflag='1' and originator='%s' and refid='%s' and workdate='%s'",
                    tblName, OP_REGIONID, pOriginator, 
                    XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opWorkdate"));
        }
    }

EXIT:
    if (pInfo != NULL)
        XMLSetNodeVal(doc, "//opTCRetinfo", (char *)pInfo);
    return ret;
}

/*
 * 转账或现金单笔手续费
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_152(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char tmp[64], pwd[64];
    int isHist;

    if (strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) > 0)
        isHist = 0;
    else
        isHist = 1;
    //收费方式1:现金 2:转账
    if (*XMLGetNodeVal(doc, "//opEXBKTaxtype") == '1') 
        ret = fetchFee(ACCOUNT_CASHFEE, doc, isHist);
    else
    {
        OFP_Decrypt(XMLGetNodeVal(doc, "//opAgreement"), pwd);
        //pwd = XMLGetNodeVal(doc, "//opAgreement");
        memset(tmp, 0, sizeof(tmp));
        DBUG("Ready BankPwdEncrypt(%s)...", pwd);
        if (ret = BankPwdEncrypt(pwd, tmp))
            return ret;
        XMLSetNodeVal(doc, "//opEXBKBankPwd", tmp);
        ret = fetchFee(ACCOUNT_ZZFEE, doc, isHist);
    }

    return ret;
}

/*
 * 批量手续费
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_153(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set ywRS;
    double fee = 0;
    char feeDate[9] = {0}, lastFeedate[9] = {0};
    int succFlag = 1, forbitSuccFlag = 0;
    int recordCount = 0, i = 0;

    forbitSuccFlag = atoi(XMLGetNodeVal(doc, "//opReserved")); //强制成功标志
    strcpy(feeDate, XMLGetNodeVal(doc, "//opWorkdate")); //收费截止日期
    strcpy(lastFeedate, GetLastFeedate());

    if (strcmp(feeDate, GetArchivedate()) > 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "收费截止日期不能大于归档日期");
        return 0;
    }
    if (strcmp(feeDate, lastFeedate) <= 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "收费截止日期必需大于上次收费截止日期");
        return 0;
    }

    ret = db_query(&ywRS, "SELECT workdate,refid,inoutflag,dcflag,originator,innerorganid,acctoper,"
            "curcode,curtype,payingacct,payer,payingbank,beneacct,benename,benebank,notetype,noteno FROM htrnjour WHERE "
            "nodeid=%d AND workdate between '%s' and '%s' AND inoutflag='1' AND feeflag!='1' AND clearstate='C'",
            OP_REGIONID, lastFeedate, feeDate);

    XMLSetNodeVal(doc, "//opNodeid", vstrcat("%d", OP_REGIONID));

    recordCount = db_row_count(&ywRS);
    for (i = 0; i < recordCount; i++)
    {
        char dcflag = *db_cell(&ywRS, i, 3);

        if ((fee = feeCalculate(dcflag, atoi(db_cell(&ywRS, i, 15)))) < 0)
            return E_DB_SELECT;
        if (fee == 0)
            continue;

        XMLSetNodeVal(doc, "//opSettlamt",   vstrcat("%.2lf", fee));
        XMLSetNodeVal(doc, "//opWorkdate",   db_cell(&ywRS, i, 0));
        XMLSetNodeVal(doc, "//opRefid",      db_cell(&ywRS, i, 1));
        XMLSetNodeVal(doc, "//opInoutflag",  db_cell(&ywRS, i, 2));
        XMLSetNodeVal(doc, "//opOriginator", db_cell(&ywRS, i, 4));
        XMLSetNodeVal(doc, "//opBankno",     db_cell(&ywRS, i, 5));
        XMLSetNodeVal(doc, "//opOperid",     db_cell(&ywRS, i, 6));
        XMLSetNodeVal(doc, "//opCurcode",    db_cell(&ywRS, i, 7));
        XMLSetNodeVal(doc, "//opCurtype",    db_cell(&ywRS, i, 8));
        XMLSetNodeVal(doc, "//opPayacct",    dcflag == '1' ? db_cell(&ywRS, i, 12) : db_cell(&ywRS, i, 9));
        XMLSetNodeVal(doc, "//opPayname",    dcflag == '1' ? db_cell(&ywRS, i, 13) : db_cell(&ywRS, i, 10));
        XMLSetNodeVal(doc, "//opPaybank",    dcflag == '1' ? db_cell(&ywRS, i, 14) : db_cell(&ywRS, i, 11));
        XMLSetNodeVal(doc, "//opBenename",   dcflag == '1' ? db_cell(&ywRS, i, 10) : db_cell(&ywRS, i, 13));
        XMLSetNodeVal(doc, "//opNotetype",   db_cell(&ywRS, i, 15));
        XMLSetNodeVal(doc, "//opNoteno",     db_cell(&ywRS, i, 16));

        if ((ret = fetchFee(ACCOUNT_ZZFEE, doc, 1)) != 0) {
            succFlag = 0;
            BKINFO("收手续费失败,ret=[%d]refid=[%s]", ret, XMLGetNodeVal(doc, "//opRefid"));
            continue;
        }
    }

    if (succFlag || forbitSuccFlag) {
        //更新最后批量收费日期
        if ((ret = UpdLastFeedate(feeDate)) == 0) {
            if (recordCount == 0)
                return E_DB_NORECORD;
        }
    }

    return ret;
}

/*
 * 批量录入
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_154(void *opDoc, char *p)
{
    result_set rs;
    xmlDoc *doc = (xmlDoc *)opDoc;
    int rc;
    int i;
    char line[8192];
    char outline[8192] = {0};
    char *fields[256];
    char caOutFile[256] = {0};
    int  fieldnum;
    char fname[64];
    sprintf(fname, "%s/%s", getenv("FILES_DIR"), XMLGetNodeVal(doc, "//opReserved"));
    FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
        SDKerrlog(ERRLOG , "打开批量录入文件失败");
        return -1;
    }

    if (getFilesdirFile(caOutFile) == NULL) 
    {
        BKINFO("生成临时文件失败");
        return -1;
    }
    FILE *fpout = fopen(caOutFile, "w");

    while(fgets(line, sizeof(line), fp) != NULL)
    {
        if (strlen(line) <= 1)
            continue;

        fieldnum = getcols(line, fields, 256, ',');

        rc = db_query(&rs, "select * from noteinfo where dcflag='%s' "
                "and notetype='%s'", fields[0], fields[3]);
        if (rc != 0)
        {
            continue;
        }


        //保存到数据库表中
        if (db_exec("INSERT INTO trnjour (dcflag, originator, acceptor, notetype, noteno,"
            " issuedate, payingacct, payer, payingbank, beneacct, benename, benebank,"
            " settlamt, purpose,curcode,curtype,refid, clearstate, result, presdate, prestime, inoutflag,"
            " workdate,nodeid,trncode,classid) VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s',"
            " '%s','%s','%s',%s,'%s','%s','%s','%s','0',0,'%s','%s','1', '%s',10,'501',1)", 
            fields[0],fields[1],fields[2],fields[3],fields[4],
            fields[5],fields[6],fields[7],fields[8],fields[9],fields[10],fields[11],fields[12],
            fields[13],fields[14],fields[15],fields[16],"now","yeye",GetWorkdate()) != 0)
        {
            SDKerrlog(ERRLOG , "Insert %s (%s) fail.", "trnjour", line);
            fclose(fp);
            return -1;
        }

        memset(outline, 0, sizeof outline);
        for (i = 0; i < fieldnum; i++)
        {
            if (fields[i] != NULL)
                strcat(outline, fields[i]);
            strcat(outline, ",");
        }
        outline[strlen(outline)-1] = '\n';

        if (fputs(outline, fpout) == EOF)
        {
            BKINFO("写入临时文件失败");
            continue;
        }

        /*
        if (FmtInsert("trnjour", fields, fieldnum) != 0)
        {
            SDKerrlog(ERRLOG , "FmtInsert %s (%s) fail.", "trnjour", line);
            fclose(fp);
            return -1;
        }
        */
    }
    fclose(fp);
    fclose(fpout);

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));
    return 0;
}

/*
 * 凭证消耗
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_155(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sAutoOrg[12+1]={0}, sAutoOper[12+1]={0};
    char sName1[81]={0}, sName2[81]={0};
    char sSqlStr[1024]={0};
    char *q;

    ret = callInterface(1607, opDoc);
    if(ret)
        return ret;

    return 0;
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
 * 打印批量手续费
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_780(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs, rs2;
    FILE *fp = NULL;
    char tmp[1024] = {0};
    char caParaFile[256] = {0}, caDataFile[256] = {0}, caOutFile[256] = {0};
    char startdate[9] = {0}, enddate[9] = {0};
    int recordCount = 0;
    double feeSum = 0;
    int i = 0;

    strcpy(startdate, XMLGetNodeVal(doc, "//opStartdate"));
    strcpy(enddate, XMLGetNodeVal(doc, "//opEnddate"));

    BKINFO("起终日期:%s-%s 系统日期:%s 机器日期:%s", startdate, enddate, GetWorkdate(), getDate(0));

    if (startdate[0] == 0 || enddate[0] == 0)
        return E_PACK_GETVAL;

    ret = db_query(&rs, "SELECT workdate,refid,originator,acctno,amount,"
            "result,a.reserved1,name FROM feelist a,noteinfo "
            "WHERE feedate BETWEEN '%s' AND '%s' AND a.reserved2=notetype "
            "ORDER BY workdate", startdate, enddate);
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s-%s;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++) {
        feeSum += atof(db_cell(&rs, i, 4));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3),
                db_cell(&rs, i, 4),
                db_cell(&rs, i, 7),
                db_cell(&rs, i, 6),
                *db_cell(&rs, i, 5) == '1' ? "成功" : "失败");
    }

    WriteRptRowCount(fp, recordCount);
    sprintf(tmp, "%.2lf", feeSum);
    WriteRptFooter(fp, "%d;%s;", recordCount, FormatMoney(tmp));
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/FeeList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("产生临时文件名或报表结果文件失败!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
 * 打印记账清单
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_781(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs, rs2;
    FILE *fp = NULL;
    char tbname[10] = "trnjour", tmp[1024] = {0};
    char caParaFile[256] = {0}, caDataFile[256] = {0}, caOutFile[256] = {0};
    char settledate[16] = {0}, currdate[16] = {0};
    int classid = 0, clearround = 0;
    int recordCount = 0, recordCount2 = 0;
    int i = 0, j = 0;

    strcpy(settledate, GetSettledDateround());
    strcpy(currdate, getDate(0));
    classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
    clearround = atoi(XMLGetNodeVal(doc, "//opClearround"));

    BKINFO("种类:%d 场次:%d 已取对账日期场次:%s 系统日期:%s 机器日期:%s", 
            classid, clearround, settledate, GetWorkdate(), currdate);
    settledate[8] = 0;

    //当隔天第一场未对账时对账场次是前一天最后场次,当天晚上归档时将对账场次置0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, currdate) != 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "未对账,不允许打印记账清单");
        return 0;
    }

    ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
            "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
            "WHERE classid=%d AND ((originator='%s' and inoutflag='1') OR (acceptor='%s' and inoutflag='2'))"
            "  AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
            tbname, classid,
            XMLGetNodeVal(doc, "//opOriginator"), XMLGetNodeVal(doc, "//opOriginator"),
            currdate, clearround);
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s;%s;%s;%s;%d;%d;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"),
            settledate, classid, clearround, XMLGetNodeVal(doc, "//opOriginator"));

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

        ret = db_query(&rs2, "SELECT acctserial, trncode, result FROM acctjour WHERE " 
                "nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%c'", 
                OP_REGIONID, currdate, db_cell(&rs, i, 1), 
                sdpStringTrimHeadChar(db_cell(&rs, i, 0), '0'), *db_cell(&rs, i, 3));
        if (ret) {
            if (ret == E_DB_NORECORD) {
                fprintf(fp, "%s;%s;%s;%s;%s;%s;\n", tmp, "", "未记账", "否", "", "");
                continue;
            } else {
                BKINFO("查询记帐流水失败,ret=%d", ret);
                return ret;
            }
        }

        recordCount2 = db_row_count(&rs2);
        for (j = 0; j < recordCount2; j++)
        { 
            fprintf(fp, "%s;%s;%s;%s;%s;%s;\n", tmp,
                    db_cell(&rs2, j, 0), 
                    getAccountDesc(db_cell(&rs2, j, 1)),
                    recordCount2 > 1 ? "是" : "否",
                    db_cell(&rs2, j, 1), "");
        }
    }

    WriteRptRowCount(fp, recordCount);
    WriteRptFooter(fp, "");
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/BankAcctList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("产生临时文件名或报表结果文件失败!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
 * 打印批量手续费明细
 * 输入:opDoc 平台报文 p 保留
 */
int PF10_782(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs, rs1, rs2;
    FILE *fp = NULL;
    char tmp[1024] = {0};
    char subject[4] = {0};
    char chinese[128] = {0};
    char caParaFile[256] = {0}, caDataFile[256] = {0}, caOutFile[256] = {0};
    char startdate[9] = {0}, enddate[9] = {0};
    int recordCount = 0;
    double feeSum = 0;
    int i = 0;

    strcpy(startdate, XMLGetNodeVal(doc, "//opStartdate"));
    strcpy(enddate, XMLGetNodeVal(doc, "//opEnddate"));

    BKINFO("起终日期:%s-%s 系统日期:%s 机器日期:%s", startdate, enddate, GetWorkdate(), getDate(0));

    if (startdate[0] == 0 || enddate[0] == 0)
        return E_PACK_GETVAL;

    ret = db_query(&rs, "SELECT workdate,refid,inoutflag,originator,acctno,amount,"
            "result,a.reserved1,name FROM feelist a,noteinfo "
            "WHERE feedate BETWEEN '%s' AND '%s' AND a.reserved2=notetype AND result='1' "
            "ORDER BY workdate", startdate, enddate);
    if (ret != 0)
        return ret;


    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s-%s;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++) {
        ret = db_query(&rs1, "SELECT dcflag, noteno, payer, benename, acctoper "
                " FROM htrnjour "
                " WHERE workdate='%s' AND refid='%s' AND inoutflag='%s' AND originator='%s' "
                " ORDER BY workdate", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3)
                );

        strncpy(subject, &db_cell(&rs, i, 4)[10, 12], 3);
        MoneyToChinese(db_cell(&rs, i, 5), chinese);
        feeSum += atof(db_cell(&rs, i, 5));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 0),
                subject,
                *db_cell(&rs1, 0, 0) == '1' ? db_cell(&rs1, 0, 3) : db_cell(&rs1, 0, 2),
                db_cell(&rs, i, 4),
                db_cell(&rs1, 0, 1),
                db_cell(&rs, i, 5), 
                chinese,
                db_cell(&rs1, 0, 4),
                db_cell(&rs, i, 5)); 
    }

    WriteRptRowCount(fp, recordCount);
    sprintf(tmp, "%.2lf", feeSum);
    WriteRptFooter(fp, "%d;%s;", recordCount, FormatMoney(tmp));
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/DetailFeeList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("产生临时文件名或报表结果文件失败!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
   交易查询行内账务处理
 */
int PF10_151(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs;
    int recordCount = 0;
    int i = 0;
    char acctDesc[1024] = {0};

    ret = db_query(&rs, "SELECT acctserial, trncode, result FROM acctjour WHERE %s", 
            GetSigleTrnjourWhere(doc));
    if (ret)
        return ret;

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++)
    { 
        sprintf(acctDesc+strlen(acctDesc), "|%s,%s", getAccountDesc(db_cell(&rs, i, 1)), db_cell(&rs, i, 0));
    }
    XMLSetNodeVal(doc, "//opTreserved2", acctDesc+1);

    return ret;
}

/*
 * 提入冲正
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_157(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;

    XMLSetNodeVal(opDoc, "//opInoutflag", "2");
    if ((ret = AccountCancle(ACCOUNT_DEBITCZ, doc)) == 0) //提出贷记冲正
        return E_APP_ACCOUNTANDCZ;
    else 
        return E_APP_ACCOUNTNOCZ;

    return 0;
}

/*
 * 票据合法性检查
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_158(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;

    XMLSetNodeVal(opDoc, "//opBKRetcode", "0000");
    XMLSetNodeVal(opDoc, "//opBKRetinfo", "交易成功");

    return 0;
}

/*
 * 账户查询
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_405(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sAutoOrg[12+1]={0}, sAutoOper[12+1]={0};
    char sName1[81]={0}, sName2[81]={0};
    char sSqlStr[1024]={0};
    char *q;

    sprintf(sSqlStr,"select autoorg, autooper from bankinfo where exchno='%s'", 
            XMLGetNodeVal(doc, "//opInnerBank"));
    ret = db_query_strs(sSqlStr, sAutoOrg, sAutoOper);
    if (ret != 0)
        return E_DB;

    if ((q = XMLGetNodeVal(opDoc, "//opAcctName")) != NULL)
        strcpy(sName1, q);
    XMLSetNodeVal(opDoc, "//opInnerBank", sAutoOrg);
    XMLSetNodeVal(opDoc, "//opOperid", sAutoOper);
    ret = callInterface(1004, opDoc);
    if(ret)
        return ret;

    if ((q = XMLGetNodeVal(opDoc, "//opAcctName")) != NULL)
        strcpy(sName2, q);

    BKINFO("name1=[%s] name2=[%s]", sName1, sName2);
    if (sName1[0] == 0x00 || strcmp(sName1, sName2))
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "户名不符");
        return 0;
    }

    return 0;
}

/*
 * 差额查询
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_903(void *doc, char *p)
{
    result_set rs;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *workdate, *bankid;
    int workround, i, rc = 0;
    char settledate[20], condi[256];
    int trtype_list[] = { 1101, 1102, 1103, 1104 };

    workdate = XMLGetNodeVal(opDoc, "//opCleardate");
    workround = atoi(XMLGetNodeVal(opDoc, "//opClearround"));
    bankid = XMLGetNodeVal(opDoc, "//opOriginator");

    strcpy(settledate, GetSettledDateround());
    settledate[8] = 0;
    if ((workround> atoi(settledate+9)) || strcmp(settledate, workdate) != 0)
    {
        XMLSetNodeVal(doc, "//opTCRetcode", "8999");
        XMLSetNodeVal(doc, "//opTCRetinfo", "未与中心对账,不允许与行内清算!");
        return 0;
    }

    rc = db_query(&rs, "SELECT "
            "isnull" "(sum(pres_debit_total), 0.0),"
            "isnull" "(sum(acpt_credit_total), 0.0),"
            "isnull" "(sum(acpt_debit_total), 0.0),"
            "isnull" "(sum(pres_credit_total), 0.0),"
            "isnull" "(sum(balance), 0.0) FROM ebanksumm "
            "WHERE svcclass in (1, 2) and branchid='%s' "
            "and workdate='%s' and workround=%d",
            bankid, workdate, workround);
    if ( rc != 0 )
        return rc;

    // 插入待清算队列
    for (i = 0; i < 4; i++)
    {
        sprintf(condi, "workdate='%s' and workround=%d and bankid='%s' "
                "and trtype='%d'", workdate, workround, bankid, trtype_list[i]);
        if (db_hasrecord("bankclear", condi) == 0)
        {
            rc = db_exec("insert into bankclear values"
                    "('%s', %d, '%s', %.2lf, '%d', '0')", workdate, workround, 
                    bankid, db_cell_d(&rs, 0, i), trtype_list[i]);
            if ( rc != 0 )
                return rc;
        }
    }

    XMLSetNodeVal(opDoc, "//opOreserved1", db_cell(&rs, 0, 0));
    XMLSetNodeVal(opDoc, "//opOreserved2", db_cell(&rs, 0, 1));
    XMLSetNodeVal(opDoc, "//opOreserved3", db_cell(&rs, 0, 2));
    XMLSetNodeVal(opDoc, "//opOreserved4", db_cell(&rs, 0, 3));
    XMLSetNodeVal(opDoc, "//opOreserved5", db_cell(&rs, 0, 4));

    return 0;
}

/*
 * 与行内资金清算
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_904(void *doc, char *pp)
{
    result_set rs;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *workdate, *bankid;
    char path[100];
    char hostserial[256];
    int workround, i, ret = 0;
    char *p;
    memset(hostserial, 0, sizeof(hostserial));

    workdate = XMLGetNodeVal(opDoc, "//opCleardate");
    workround = atoi(XMLGetNodeVal(opDoc, "//opClearround"));
    bankid = XMLGetNodeVal(opDoc, "//opOriginator");

    ret = db_query(&rs, "select trtype, settlamt, flag from bankclear "
            "where workdate='%s' and workround=%d and bankid='%s'", 
            workdate, workround, bankid);
    if (ret != 0)
        return ret;

    for (i = 0; i < db_row_count(&rs); i++)
    {
        sprintf(path, "//opOreserved%d", db_cell_i(&rs, i, 0)%10);

        if (db_cell_i(&rs, i, 2) == 1)
        {
            XMLSetNodeVal(opDoc, path, "已经清算成功");
            continue;
        }
        if (db_cell_d(&rs, i, 1) < 0.01)
        {
            db_exec("update bankclear set flag='1' where workdate='%s'"
                    " and workround=%d and bankid='%s' and trtype='%s'", 
                    workdate, workround, bankid, db_cell(&rs, i, 0));
            XMLSetNodeVal(opDoc, path, "清算成功");
            continue;
        }

        // 转账金额
        XMLSetNodeVal(opDoc, "//opSettlamt", db_cell(&rs, i, 1));
        // 转账流水
        XMLSetNodeVal(opDoc, "//opHreserved5", vstrcat("%ld", GenAccountSerial()));

        // 记账
        if (callInterface(db_cell_i(&rs, i, 0), doc)) 
            XMLSetNodeVal(opDoc, path, "通迅失败");
        else
        {
            p = XMLGetNodeVal(doc, "//opBKRetcode");
            if (strlen(p) && !sdpStringIsAllChar(p, '0'))
                XMLSetNodeVal(opDoc, path, XMLGetNodeVal(doc, "//opBKRetinfo"));
            else
            {
                db_exec("update bankclear set flag='1' where workdate='%s'"
                        " and workround=%d and bankid='%s' and trtype='%s'", 
                        workdate, workround, bankid, db_cell(&rs, i, 0));
                XMLSetNodeVal(opDoc, path, "清算成功");
                strcat(hostserial, XMLGetNodeVal(doc, "//opHostSerial"));
                strcat(hostserial, "-");
            }
        }
    }
    XMLSetNodeVal(opDoc, "//opHostSerial", hostserial);
    db_free_result(&rs);

    return 0;
}
