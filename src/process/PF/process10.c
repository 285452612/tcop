#include "interface.h" 
#include "errcode.h" 
#include "sz_const.h" 
#define GenAccountSerial() GenSerial("account", 1, 9999999, 1)
//#define TEST_PF
//#define TEST_PF_PT
#define DEBUG_PF 

static char GSQL[SQLBUFF_MAX] = {0};
static int ret = 0;
//个人业务
#define ACCOUNT_OUTZZ_PERSON            8227    //个人业务(转账)7708
#define ACCOUNT_OUTCASH_PERSON          8227    //个人业务(现金)7710
#define ACCOUNT_TRAN_PERSON             8227    //个人业务
#define ACCOUNT_TRAN_PERSON_KS          8227    //协议扣税个人账户
#define ACCOUNT_OUTCASH_SAVE            4201    //提出现金通存 
#define ACCOUNT_OUTCASH_WITHDRAW        4202    //提出现金通兑
//特殊业务
#define ACCOUNT_CHECK_ZFMM              8299    //校验支付密码
#define ACCOUNT_GET_PH                  7130    //获取支付密码批号
#define ACCOUNT_CHECK_BPMY              9989    //校验本票密押(sz61)
#define ACCOUNT_INCREDIT_FS             9960    //非裞(sz60)
#define ACCOUNT_CREDITCZ                7704    //贷记（存款）的冲正  OK
#define ACCOUNT_DEBITCZ                 7704    //借记（扣款）的冲正  OK
#define ACCOUNT_OPERCZ                  9038    //按柜员流水冲正 
#define ACCOUNT_EFBPCZ                  9928    //EF14,EF16交易冲正
#define ACCOUNT_ACCTINFO_PERSON         8982    //个人账户信息查询
#define ACCOUNT_ACCTINFO_PUB            9982    //对公账户信息查询
#define ACCOUNT_ACCTINFO_PERSON_KS      3982    //个人账户信息查询(协议扣税使用)
#define ACCOUNT_ACCTINFO_PUB1           4302    //对公账户信息查询(账户管理使用)
#define ACCOUNT_QUERY_BPINFO            8307    //本票(汇票)登记薄查询(确认)
#define ACCOUNT_QUERY_BPINFO_IN         9307    //本票(汇票)登记薄查询(提入)
#define ACCOUNT_QUERY_ACCOUNTTYPE       8992    //查询交易的入帐方式
#define ACCOUNT_QUERY_ORG               9328    //查询营业机构
#define ACCOUNT_CATCH_FEE               9579    //扣客户手续费
#define ACCOUNT_GET_PERSONMX            8263    //获取个人业务核心记账明细
#define ACCOUNT_GET_TERMNO              8978    //获取柜员终端号
#define ACCOUNT_GET_DATE                1650    //获取行内系统日期(e650)
#define ACCOUNT_QUERY_BPMY              8359    //查询总行本票密押
//对公业务
#define ACCOUNT_OUTCREDIT               8963    //提出贷  
#define ACCOUNT_OUTCREDIT_PERSON        9963    //提出贷(个人业务,同8963)
#define ACCOUNT_OUTTRUNCDEBIT           8953    //提入借截留
#define ACCOUNT_OUTNOTRUNCDEBIT         8951    //提出借非截留  OK
#define ACCOUNT_OUTNOTRUNCDEBIT_PERSON  9951    //提出借非截留(个人业务,同8951) 
#define ACCOUNT_INCREDIT                8248    //提入贷自动或提出贷被退票 
#define ACCOUNT_INDEBITRUNC_BP          9984    //提入本票(截流)

/*****************交易确认业务******************
  1、   本票类型调用8984交易
  2、   银行汇票类型调用8983交易
  3、   承兑汇票类型调用EF14交易
  4、   商业汇票类型调用EF16交易
  5、   其他借记调用9953==8953交易
  6、   贷记确认用8249交易
*/
#define ACCOUNT_INDEBITCHK_BP           8984    //提入本票确认
#define ACCOUNT_INDEBITCHK_HP           8983    //提入汇票确认
#define ACCOUNT_INDEBITCHK_OTHER        9953    //提入其他借记确认
#define ACCOUNT_INDEBITCHK_CDHP         9914    //提入承兑汇票确认EF14
#define ACCOUNT_INDEBITCHK_SYHP         9916    //提入商业汇票确认EF16
#define ACCOUNT_INCREDIT_CHK            8249    //提入贷记确认入账 

//退票业务
#define ACCOUNT_OUTCREDIT_TPA           8247    //提入贷作退票A OK
#define ACCOUNT_OUTCREDIT_TPB           8956    //提入贷作退票B OK
#define ACCOUNT_OUTDEBIT_TPA            8955    //提入借作退票A 
#define ACCOUNT_OUTDEBIT_TPA_BP         1955    //本票被退票
#define ACCOUNT_OUTDEBIT_TPB            8956    //提入借作退票B 
#define ACCOUNT_INCREDIT_TP             8248    //提出贷被退票 

//收妥(手工处理,平台不处理,暂不启用)
#define ACCOUNT_OUTDEBIT_ST             9402    //提出借收妥

//以下未使用
#define ACCOUNT_INTRUNCDEBIT            9101    //提入借截留
#define ACCOUNT_INNOTRUNCDEBIT          9201    //提入借非截留
#define ACCOUNT_INNOTRUNCDEBIT_CHECK    9000    //提入借非截留确认 包含下列交易 
#define ACCOUNT_INCREDIT_TPCHK          9801    //提出贷被退票确认处理
#define ACCOUNT_CREDIT_REOUT            9901    //提出贷被退票重提出
#define ACCOUNT_INDEBIT_TP              9602    //提出借被退票自动处理
#define ACCOUNT_INDEBIT_TPCHK           9702    //提出借被退票确认处理

#define ACCOUNT_OUTDEBIT_TP             9401    //提入借非截留作退票
#define ACCOUNT_ZZFEE                   9501    //转账收手续费
#define ACCOUNT_CASHFEE                 9601    //现金收手续费

#define ACCOUNT_IN_SAVE                 8002    //提入通存
#define ACCOUNT_OUTZZ_WITHDRAW          8202    //提出转账通兑
//结束

int equal(char *s1, char *s2)
{
    return strcmp(s1,s2)?0:1;
}

//获取个人业务核心记帐数据
int GetBankPersonMX( void *opDoc )
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char *p=NULL;

    BKINFO("取个人交易核心记账明细[%d]...",ACCOUNT_GET_PERSONMX);

    XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate() );
    if (ret = callInterface(ACCOUNT_GET_PERSONMX, doc))
        return ret;
    p = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", p);
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        BKINFO("取个人交易核心记账明细失败...");
        return -1;
    }

    p = XMLGetNodeVal(doc, "//opBAOBLJ");
    BKINFO("取个人交易核心记账明细文件[%s]...", p);
    if( strlen(p) )
    {
        BKINFO("取个人交易核心记账明细失败[%s]...", p);
        return -1;
    }

    return ret;
}
//保存税票凭证的行内交易信息
void SaveTaxNote(void *opDoc)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs,rs_serial;
    char file[256] = {0};
    FILE *fp = NULL;
    char PrintBuf[20480] = {0};
    char where[256] = {0};
    int rc = 0,i = 0,len = 0;
    char tmp[256]={0};

    sprintf(where, "workdate='%s' and refid='%s' and originator='%s' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opOriginator"));

    rc = db_query(&rs,"select payingacct,payer,payingbank,benebank,noteno,beneacct, benename from trnjour where %s", where);
    if(rc != 0)
    {
        BKINFO("保存税票凭证客户信息失败[%s]", where);
        return;
    }

    rc = db_query(&rs_serial, "select acctserial from acctjour where %s", where);
    if(rc != 0)
    {
        BKINFO("保存税票凭证客户信息失败[%s]", where);
        db_free_result(&rs);
        return;
    }

    sprintf(file,"%s/consoleprint/%s.tax",getenv("HOME"), db_cell(&rs,0,4));
    //if((fp=fopen(file, "a"))==NULL)
    if((fp=fopen(file, "r+"))==NULL)
    {
        BKINFO("无法打开文件[%s]", file);
        db_free_result(&rs);
        db_free_result(&rs_serial);
        return;
    }

    //fseek(fp, 0L, SEEK_SET);
    len=0;
    len+=sprintf(PrintBuf+len,"\n");
    len+=sprintf(PrintBuf+len,"  付款帐号:%-32s   付款户名:%-60s\n", db_cell(&rs,0,0), db_cell(&rs,0, 1));
    len+=sprintf(PrintBuf+len,"  付款行号:%-20s   付款行名:%-60s\n", 
            XMLGetNodeVal(doc, "//opOriginator"), org_name(XMLGetNodeVal(doc, "//opOriginator"), tmp));
    len+=sprintf(PrintBuf+len,"  收款帐号:%-32s   收款户名:%-60s\n", db_cell(&rs,0,5), db_cell(&rs,0, 6));
    len+=sprintf(PrintBuf+len,"  收款行号:%-20s   收款行名:%-60s\n", 
            XMLGetNodeVal(doc, "//opAcceptor"), org_name(XMLGetNodeVal(doc, "//opAcceptor"), tmp));
    len+=sprintf(PrintBuf+len,"  行内流水:%-20s\n", db_cell(&rs_serial,0, 0));
    fwrite(PrintBuf, len,1, fp);
    //fprintf(fp, "%s", PrintBuf);

    db_free_result(&rs);
    db_free_result(&rs_serial);
    fclose(fp);

    BKINFO("保存税票打印文件[%s]客户信息成功", file);
}
//保存个人凭证
void SavePersonNote(void *opDoc)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs,rs_serial;
    FILE *fp = NULL;
    int rc = 0,i = -1,len = 0;
    char file[256] = {0};
    char where[256] = {0};
    char wheretest[256] = {0};
    char notetype[4] = {0};
    char PrintBuf[20480] = {0};

    strcpy(notetype, XMLGetNodeVal(doc, "//opNotetype"));
    sprintf(where, "workdate='%s' and refid='%s' and originator='%s' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opOriginator"));
    sprintf(wheretest, "workdate='%s' and refid='%s' and originator='622' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"));
    rc = db_query(&rs,"select beneacct,benename,payingacct,payer,settlamt,acctoper,payingbank,benebank from trnjour where %s", where);
    if(rc != 0)
    {
        BKINFO("保存个人凭证失败[%s]", where);
        return;
    }

#ifdef TEST_PF
    if(equal(notetype, "73") || equal(notetype, "74"))
        rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", where);
    else
        rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", wheretest);
#else
    rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", where);
#endif
    if(rc != 0)
    {
        /*
           if(equal(notetype, "73") || equal(notetype, "74"))
           BKINFO("保存个人凭证失败[%s]", where);
           else
           BKINFO("保存个人凭证失败[%s]", wheretest);
         */
        BKINFO("保存个人凭证失败[%s]", where);
        db_free_result(&rs);
        return;
    }

    if(db_row_count(&rs_serial) > 1)
    {
        if(equal(notetype, "71"))
        {
            if(equal(db_cell(&rs_serial,0,1), "9963"))
                i=0;
            else
                i=1;
        }
        else if(equal(notetype, "72"))
        {
            if(equal(db_cell(&rs_serial,0,1), "4202"))
                i=0;
            else
                i=1;
        }
        if( i == 0 )
            sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,0,0));
        else
            sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,1,0));
    }
    else
        sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,0,0));


    if((fp=fopen(file,"w")) == NULL)
    {
        BKINFO("保存个人凭证失败:无法以写入方式打开文件[%s]",file);
        db_free_result(&rs);
        db_free_result(&rs_serial);
        return;
    }

    if(equal(notetype,"72") || equal(notetype,"74"))
        len+=sprintf(PrintBuf+len, "\n\n\n\n\n\n\n\n\n\n\n");
    len+=sprintf(PrintBuf+len, "\n\n\n");
    len+=sprintf(PrintBuf+len, "          收款行号:%s\n", db_cell(&rs,0,7));
    len+=sprintf(PrintBuf+len, "          收款账号:%s\n", db_cell(&rs,0,0));
    len+=sprintf(PrintBuf+len, "          收款人名称:%s\n", db_cell(&rs,0,1));
    len+=sprintf(PrintBuf+len, "          付款行号:%s\n", db_cell(&rs,0,6));
    len+=sprintf(PrintBuf+len, "          付款账号:%s\n", db_cell(&rs,0,2));
    len+=sprintf(PrintBuf+len, "          付款人名称:%s\n", db_cell(&rs,0,3));
    len+=sprintf(PrintBuf+len, "          交易金额:%s\n", db_cell(&rs,0,4));
    len+=sprintf(PrintBuf+len, "          柜 员 号:%s\n", db_cell(&rs,0,5));
    len+=sprintf(PrintBuf+len, "          柜员流水:%s\n", db_cell(&rs_serial, 0,0));
    len+=sprintf(PrintBuf+len, "                                                  客户签名:");

    fwrite(PrintBuf, len, 1, fp);

    fclose(fp);
    db_free_result(&rs);
    db_free_result(&rs_serial);

    BKINFO("保存个人凭证成功:[%s]", file);
}

char *getAccountResult(char *acctresult)
{
    switch (atoi(acctresult)) {
        case 0:   return "状态未知";
        case 1:   return "记账成功";
        case 2:   return "已冲正";
        case 5:   return "已挂账";
        case 9:   return "记账失败";
        default: return "未知";
    }
}
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
        case ACCOUNT_INCREDIT_TPCHK:        return "贷被退确认";
        case ACCOUNT_CREDIT_REOUT:          return "贷被退重提";

        case ACCOUNT_OUTCASH_SAVE:          return "提出现存";

        case ACCOUNT_INCREDIT_CHK:          return "提入贷确认";
        case ACCOUNT_OUTTRUNCDEBIT:         return "提出借截留";
        case ACCOUNT_OUTNOTRUNCDEBIT:       return "提出借非截";
        case ACCOUNT_OUTCREDIT_TPA:         return "提出贷退票A";
        case ACCOUNT_OUTCREDIT_TPB:         return "提入退票(提出交易被退回)";
        case ACCOUNT_OUTDEBIT_ST:           return "提出借收妥";
        case ACCOUNT_INDEBIT_TP:            return "借被退自动";
        case ACCOUNT_INDEBIT_TPCHK:         return "借被退确认";

        case ACCOUNT_IN_SAVE:               return "提入通存";
        case ACCOUNT_OUTCASH_WITHDRAW:      return "提出现兑";
        case ACCOUNT_OUTZZ_WITHDRAW:        return "提出转兑";

        case ACCOUNT_CREDITCZ:              return "冲正";
                                            //case ACCOUNT_CREDITCZ:              return "贷记冲正";
                                            //case ACCOUNT_DEBITCZ:               return "借记冲正";
        default: return "未知";
    }
}

static int getcols( char *line, char *words[], int maxwords, int delim )
{
    char *p = line, *p2 = NULL;
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
int ChkAgreement(char *id, char *payacct)
{
    result_set rs;
    int rc;

    rc = db_query(&rs, "select state from agreement where nodeid=%d "
            "and agreementid='%s' and payingacct='%s' ", OP_REGIONID, id, payacct);
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

//记账并记录记账流水
int Accounting(int bktcode, xmlDoc *doc)
{
    char *p = NULL;
    char where[1024]={0};
    char *workdate = NULL;
    char *exchgdate= NULL;
    char *chkflag = NULL;
    char tmp[24]={0};
    char sql[512]={0};
    char sAcctSerial[14]={0};
    int result;

    BKINFO("进入记账程序流程...");

    p= XMLGetNodeVal(doc, "//opCurcode");
    if (*p!= 0 && strcmp("CNY", p) && bktcode != ACCOUNT_OUTCREDIT) {
        BKINFO("除提出贷外, 外币[%s]不记账", p);
        return 0;
    }

    p= sdpXmlSelectNodeText(doc, "//opNoteno");
    //43非裞不需要截断
    if (p!= NULL && strlen(p) > 8 && bktcode != ACCOUNT_INCREDIT_FS ) {
        BKINFO("凭证号太长,截断[%s->%s]", p, p+strlen(p)-8);
        XMLSetNodeVal(doc, "//opNoteno", p+strlen(p)-8); //行内凭证号最大8位(截最后8位)
    }

    XMLSetNodeVal(doc, "//opTrcode", vstrcat("%d", bktcode));
    /*********************************************
     *提出交易取前台产生的流水,平台不产生记帐流水
     *提入交易平台产生流水
     *********************************************/
    p = XMLGetNodeVal(doc, "//opPDQTLS");
    if( strlen(p) )
        XMLSetNodeVal(doc, "//opHreserved5", XMLGetNodeVal(doc, "//opPDQTLS"));
    else {
        /*集中业务平台发起的交易流水前面加机构代码*/
        p = XMLGetNodeVal(doc, "//opSYSMACSBNO");
        if( strlen(p) )
        {
            sprintf( sAcctSerial, "%4s%08ld", p, GenAccountSerial());
            XMLSetNodeVal(doc, "//opHreserved5", sAcctSerial);
        }
        else
            XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));
    }

#if 1
    chkflag = XMLGetNodeVal(doc, "//opChkflag");
    //确认交易
    if(  chkflag != NULL && atoi(chkflag) == 1 )
    {
        workdate= XMLGetNodeVal(doc, "//opWorkdate");
        exchgdate= XMLGetNodeVal(doc, "//opExchgdate");
        BKINFO("交易确认CHKFLAG[%s], TrnCode[%d], WorkDate[%s], ExchgDate[%s]", chkflag, bktcode, workdate, exchgdate);
        sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opExchgdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
        /*插acctjour为原交易日期为工作日期*/
        XMLSetNodeVal(doc, "//opWorkdate", exchgdate);
    }
    else
    {
        sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
    }
#endif

    /*
    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
            */

    //ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s AND result='1' AND trncode='%d'", where, bktcode);
    ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s", where );

    if (ret == E_DB_NORECORD)
    {
        if ((ret = InsertAcctjour(doc)) != 0)
        {
            BKINFO("保存记帐信息失败");
            return E_APP_ACCOUNTFAIL;
        }
    }
    else if(ret)
        return E_APP_ACCOUNTFAIL;

#if 0 
    /*还原工作日期*/
    if(  chkflag != NULL && atoi(chkflag) == 1 )
        XMLSetNodeVal(doc, "//opWorkdate", workdate);
#endif
    /*********************************************************
     *1:已经记帐的则返回记帐成功
     *0:交易结果不明确的则返回行内通讯故障
     *2:交易被冲正
     *9:交易失败
     *5:提入贷记挂账(非裞,户名不符合,8248交易行内挂账不入客户帐)
     *acctserial:柜员流水,revserial:同城提出号,reserved1:平台流水
     ************************************************************/
    if(atoi(tmp) == 1)   
        return E_APP_ACCOUNTSUCC;
    else if( strlen(tmp) && atoi(tmp) == 0)
        return E_SYS_COMM_BANK;

    if (ret = callInterface(bktcode, doc))
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", p);
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        //XMLSetNodeVal(doc, "//opBKRetcode", "");
        /*行内返回明确错误信息*/
        sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"9", where );
        db_exec( sql );
        return E_APP_ACCOUNTFAIL;
    }

    /*更新状态*/
    if( bktcode == ACCOUNT_INCREDIT )
    {
        p = XMLGetNodeVal(doc, "//opTISHXI");
        if( strlen(p) && !sdpStringIsAllChar(p, '0') )
        {
            BKINFO("提入贷记入帐挂账,行内返回[%s]", p);
            sprintf( sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                    XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"5", where );
            goto EXIT;
        }
    }

    /*贷记退票8247,贷记确认8249不需要保存同城提出号,与原记账的同城提出号一样*/
    if( bktcode == ACCOUNT_OUTCREDIT_TPA || bktcode == ACCOUNT_INCREDIT_CHK )
    {
        sprintf(sql, "update acctjour set acctserial = '%s', result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), "1", where );
    }
    else
    {
        sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"1", where );
    }
    /*
       sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
       XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"1", where );
     */

    /*税票返回税票号码*/
    p = XMLGetNodeVal(doc, "//opNotetype");
    if( atoi(p) == 14 )
        XMLSetNodeVal( doc, "//opOreserved3", XMLGetNodeVal(doc, "//opXINX03") );
EXIT:
    ret = db_exec( sql );
    if (ret  == 0)
        return E_APP_ACCOUNTSUCC;
    else 
    {
        BKINFO("交易成功,更新交易状态失败[%d]", ret);
        return ret;
    }
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
int AccountCancel(int bktcode, int flag, xmlDoc *doc)
{
    char acctResult[8] = {0};
    char acctSerial[20] = {0};
    char acctReserved1[20] = {0};
    char where[1024] = {0};
    char sAcctSerial[14]={0};
    char *p = NULL;

    /*如果终端为空则为集中业务发起的退票,需要去行内查询柜员的终端号*/
    p = XMLGetNodeVal(doc, "//opPDWSNO");
    if( strlen(p) == 0 )
    {
        ret = callInterface(ACCOUNT_GET_TERMNO, doc);
        if( ret )
        {
            BKINFO("查询集中业务发起柜员的终端号失败[%d]...", ret );
            return ret;
        }
    }

    if( bktcode == ACCOUNT_CREDITCZ || bktcode == ACCOUNT_DEBITCZ )
    {
        sprintf(where, " WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' ",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag") );
    }
    else
    {
        sprintf(where, " WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' and trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
    }

    sprintf(GSQL, "SELECT acctserial, result, reserved1 FROM acctjour %s", where);

    if (ret = db_query_strs(GSQL, acctSerial, acctResult, acctReserved1)) {
        BKINFO("查询原记帐流水失败");
        return E_DB_SELECT;
    }

    if (atoi(acctResult) != 1) {
        BKINFO("此交易无需冲正");
        return 0;
    }

    if( flag == 3 )
        XMLSetNodeVal(doc, "//opHostSerial", acctSerial); //行内返回柜员流水
    else
        XMLSetNodeVal(doc, "//opHostSerial", acctReserved1); //原系统参考号

    /*********************************************
     *1:柜面发起的交易,取前台流水, 7704
     *2:平台自动发起的冲正,平台产生流水, 7704
     *3:两个分录冲正, 9038按柜员流水冲正
     *如果为EF14,EF16交易则发起EF28(9928)冲正
     *********************************************/
    if( flag == 1 )
    {
        p = XMLGetNodeVal(doc, "//opPDQTLS");
        if( strlen(p) )
            XMLSetNodeVal(doc, "//opHreserved5", XMLGetNodeVal(doc, "//opPDQTLS"));
        else {
            /*集中业务平台发起的交易流水前面加机构代码*/
            p = XMLGetNodeVal(doc, "//opSYSMACSBNO");
            if( strlen(p) )
            {
                sprintf( sAcctSerial, "%4s%08ld", p, GenAccountSerial());
                XMLSetNodeVal(doc, "//opHreserved5", sAcctSerial);
            }
            else
                XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));
        }
    }
    else if( flag == 2 || flag == 3 )
        XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));

     
#if 0 
    /*如果为手工冲正交易则还原系统工作日期*/
    p = XMLGetNodeVal(doc, "//opCZFLAG");
    if( p != NULL && atoi(p) == 1 )
    {
        BKINFO("冲正标志[%s]", p);
        XMLSetNodeVal(doc, "//opWorkdate", XMLGetNodeVal(doc, "//opJIOHRQ"));
    }
#endif

     
#if 0 
    /*如果为手工冲正交易则还原系统工作日期*/
    p = XMLGetNodeVal(doc, "//opCZFLAG");
    if( p != NULL && atoi(p) == 1 )
    {
        BKINFO("冲正标志[%s]", p);
        XMLSetNodeVal(doc, "//opWorkdate", XMLGetNodeVal(doc, "//opJIOHRQ"));
    }
#endif

    XMLSetNodeVal(doc, "//opBKRetcode", "");
    if( flag == 3 )
        ret = callInterface(ACCOUNT_OPERCZ, doc);
    else if( bktcode == ACCOUNT_INDEBITCHK_CDHP || bktcode == ACCOUNT_INDEBITCHK_SYHP )
        ret = callInterface(ACCOUNT_EFBPCZ, doc);
    else
        ret = callInterface(ACCOUNT_CREDITCZ, doc);
    if( ret )
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        BKINFO("冲正失败[%s]...", p);
        XMLSetNodeVal(doc, "//opBKRetcode", "");
        return E_APP_CZFAIL;
    }
    BKINFO("opBKRetcode=[%s]", p);

    /****************************************
     *revserial:冲正交易主机流水
     *reserved2:冲正交易平台流水
     *****************************************/
    ret = db_exec("UPDATE acctjour SET revserial='%s',result='2',reserved2='%s' %s",
            XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opHreserved5"), where);

    return 0;
}

/*
 * 复核记帐(不使用,只作为参考)
 * 输入:opDoc 平台报文 p 保留
 * 返回:平台错误码
 */
#if 0
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
                if ((ret = AccountCancel(ACCOUNT_DEBITCZ, doc)) == 0) //提出贷记冲正
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
#endif

/*************************************
flag:
1-提出交易去掉'*'
2-提入交易不去掉'*'
 **************************************/
void ProcAcctNo(char *newacct, char *oldacct, int flag)
{
    char acct[40];
    char *p = NULL;

    sprintf(acct, oldacct);
    if ((p = strchr(acct, '-')) != NULL) {
        memcpy(p, p+1, strlen(p+1));
        acct[strlen(acct)-1] = 0;
    }
    if (flag == 1)
    {

        if (acct[0] == '*')
            strcpy(newacct, acct+1);
        else
            strcpy(newacct, acct);
    }

    strcpy(newacct, acct);
    return;
}

/**************************************
  判断帐户性质
  1--对公帐户
  2--个人账户
  ************************************/
int IsAcctType( char *acctno )
{
    char sBankPer[6]={0};

    sprintf( sBankPer, "%s", GetBankPerFlag() );
    BKINFO( "个人账户标志[%s]....", sBankPer );

    if( (strncmp( acctno, sBankPer, 3 ) == 0) || (strncmp( acctno, "6225", 4 ) == 0) )
        return 2;
    else
        return 1;
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
    int  truncflag = 0;
    int  notetype = 0, classid = 0;
    int  bktcode, opcode;
    char sql[512] = {0};
    char buf[128] = {0};
    char tmp[128] = {0};
    char tmp1[128] = {0};
    char *pp = NULL, *pAcctPath = NULL;
    char *acctno = NULL, *pwd = NULL, *orgid = NULL, *exchno = NULL;
    char sAutoOrg[12+1]={0}, sAutoOper[12+1]={0};
    char sSqlStr[1024]={0};
    char sLimitAmt[16]={0};
    char *pSettlAmt;
    char noteno1[9]={0};
    char noteno2[9]={0};
    int  iAcctFlag = -1;
    char fs_Acct[36] = {0};

    /*退票交易*/
    opcode = atoi(XMLGetNodeVal(doc, "//opTctcode"));
    if( opcode == 7 )
    {
        BKINFO("提入退票交易...");
        return  TP_Process(opDoc, p);
    }

    BKINFO("提入交易...");

    dcflag   = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
    classid  = atoi(XMLGetNodeVal(doc, "//opClassid"));

    // 查找对应提入的核心交易码, 及截留标志
    sprintf(sql, "select distinct truncflag, reserved2 from notetypemap "
            "where tctype='%02d' and dcflag='%c' and nodeid=%d",
            notetype, dcflag, OP_REGIONID);
    if (db_query_strs(sql, buf, tmp) != 0)
        return E_DB_SELECT;

    truncflag = atoi(buf);
    bktcode = atoi(tmp);
    BKINFO("***truncflag=[%d]|bktcode=[%d]***",truncflag, bktcode);

    // 提入借记交易付款账号为本行账号, 提入贷记交易收款账号为本行账号
    pAcctPath = (dcflag == OP_DEBITTRAN ? "//opPayacct" : "//opBeneacct");
    ProcAcctNo(tmp, XMLGetNodeVal(doc, pAcctPath), 2);
    iAcctFlag = IsAcctType(tmp);
    // 重置报文中的账号
    XMLSetNodeVal(doc, pAcctPath, tmp);

    /************************************************************
     * 个人业务查询默认的记账机构和记账柜员
     ************************************************************/
    if( classid == 2 )
    {
        XMLSetNodeVal(doc, "//opInnerBank", GetDefOrg()); //内部机构
        XMLSetNodeVal(doc, "//opOperid", GetDefOper()); //柜员
    }

    if (notetype == 71 || notetype == 73)
    {
#if 0 
        if (strcmp(XMLGetNodeVal(doc, "//opMac"), 
                    vstrcat("%d", E_SYS_SYDDECRYPT)) == 0)
            return E_SYS_SYDDECRYPT;
#endif

#ifdef TEST_PF
        XMLSetNodeVal(doc, "//opPayacct", "95010134900000011"); //清算账户(3490==2881)
#else
        memset( sql, 0, sizeof(sql) );
        memset( tmp, 0, sizeof(tmp) );
        /*送3490内部帐户*/
        sprintf(sql, "select distinct clearacct from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp) != 0)
            return E_DB_SELECT;
        BKINFO("个人转账内部帐户[%s]...", tmp);
        XMLSetNodeVal(doc, "//opPayacct", tmp); //内部帐户
#endif
        XMLSetNodeVal(doc, "//opZZCZLX", "9"); //转出帐号类型 9-内部帐户
        XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-不校验交易密码
        XMLSetNodeVal(doc, "//opHUILUU", "3"); //固定3
        XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-非贷销,1-贷销
        XMLSetNodeVal(doc, "//opJIOHLX", "2"); //2-提入贷方

        XMLSetNodeVal(doc, "//opZJZZLX", "1"); //转入帐号类型 1-卡
        XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //转入帐号性质
        XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //转出帐号性质
    }
    //else if(notetype == 41) // 地税发送过来的缴税交易, 检查协议号
    /*修改于2012-9-4，增加对公积金凭证(33)的判断*/
    else if((notetype == 41) || (notetype == 33 )) // 地税发送过来的缴税交易, 检查协议号
    {
        if ((ret = ChkAgreement(XMLGetNodeVal(doc, "//opAgreement"), XMLGetNodeVal(doc, "//opPayacct")))!=0)
        {
            BKINFO("缴税凭证校验协议号[%s]付款帐号[%s]失败[%d]", XMLGetNodeVal(doc, "//opAgreement"), 
                    XMLGetNodeVal(doc, "//opPayacct"), ret);
            return ret;
        }
        //个人帐户
        if( iAcctFlag == 2 )
        {
            //个人账户先查询户名
            ret = callInterface(ACCOUNT_ACCTINFO_PERSON_KS, doc);
            if ( XMLGetNodeVal(doc, "//opCardName") != NULL)
                XMLSetNodeVal(doc, "//opPayname", XMLGetNodeVal(doc, "//opCardName") );
            BKINFO("个人账号户名[%s]", XMLGetNodeVal(doc, "//opPayname")); 
#ifdef TEST_PF
            XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //内部账户
#else
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            sprintf(sql, "select distinct clearacct from bankinfo "
                    "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
            if (db_query_strs(sql, tmp) != 0)
                return E_DB_SELECT;
            BKINFO("个人转账内部帐户[%s]...", tmp);
            XMLSetNodeVal(doc, "//opBeneacct", tmp); //内部帐户
#endif

            XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-不校验交易密码
            XMLSetNodeVal(doc, "//opZZCZLX", "1"); //转出帐号类型 1-卡
            XMLSetNodeVal(doc, "//opZJZZLX", "9"); //转入帐号类型 9-内部帐户
            XMLSetNodeVal(doc, "//opHUILUU", "3"); //固定3
            XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-非贷销,1-贷销
            XMLSetNodeVal(doc, "//opJIOHLX", "3"); //3-提入借方
            XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //转入帐号性质
            XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //转出帐号性质

            bktcode = ACCOUNT_TRAN_PERSON_KS;
            BKINFO("协议缴税个人账户扣款BKCODE[%d]", bktcode);
        }
    }
    else if( notetype == 43 )//先调9960(sz60),查询裞号信息,再8248自动入帐
    {
        if( (ret = Accounting(ACCOUNT_INCREDIT_FS, doc)) != E_APP_ACCOUNTSUCC )
        {
            BKINFO("非裞业务[%d]失败", ACCOUNT_INCREDIT_FS);
            return ret;
        }
        pp = XMLGetNodeVal(doc, "//opTIOZLX");
        if( pp != NULL )
        {
            if( strlen(pp) == 0 )
            {
                BKINFO("非裞业务[%d]失败[%s]", ACCOUNT_INCREDIT_FS,pp);
                return E_APP_ACCOUNTFAIL;
            }
            else if( pp[0] != '0' )
            {
                BKINFO("非裞业务[%d]失败[%s]", ACCOUNT_INCREDIT_FS,pp);
                return E_APP_ACCOUNTFAIL;
            }
        }
        else
            return E_APP_ACCOUNTFAIL;
#if 0
        /*非税业务查询内部帐户入账*/
        memset( tmp, 0, sizeof(tmp) );
        memset( tmp1, 0, sizeof(tmp1) );
        memset( sql, 0, sizeof(sql) );
        sprintf(sql, "select creditacct, reserved2 from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp, tmp1) != 0)
            return E_DB_SELECT;
        BKINFO("非税内部帐户[%s]户名[%s]...", tmp, tmp1);
        XMLSetNodeVal(doc, "//opBeneacct", tmp);  //非税内部帐户
        XMLSetNodeVal(doc, "//opBenename", tmp1); //非税内部帐户户名
#endif
    }
    else if (notetype == 72 || notetype == 74)
    {
        strcpy( sLimitAmt, GetAmtLimit());
        pSettlAmt = XMLGetNodeVal(doc, "//opSettlamt");

        BKINFO("***sLimitAmt=[%s]|pSettlAmt=[%s]***", sLimitAmt, pSettlAmt);
        if( atof(pSettlAmt) > atof(sLimitAmt) )
        {
            BKINFO("个人通兑超过取款限额...");
            XMLSetNodeVal(doc, "//opBKRetinfo", "超过取款限额");
            return E_APP_ACCOUNTFAIL;
        }
        // 转换个人密码到行内
        memset( tmp, 0, sizeof(tmp) );
        acctno = XMLGetNodeVal(doc, "//opPayacct");
        pwd    = XMLGetNodeVal(doc, "//opAgreement");

        BKINFO("***AcctNo=[%s]|BankPwd=[%s]***",acctno, pwd);
        if( ret = ConvertPersonKey( 2, acctno, acctno, pwd, NULL, tmp) )
        {
            BKINFO("个人密码转加密失败[%d]...", ret);
            return ret;
        }

        XMLSetNodeVal(doc, "//opAgreement", tmp); //个人密码
        XMLSetNodeVal(doc, "//opSYSPINSEED", acctno); 
#ifdef TEST_PF
        XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //内部账户
#else
        memset( sql, 0, sizeof(sql) );
        memset( tmp, 0, sizeof(tmp) );
        sprintf(sql, "select distinct clearacct from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp) != 0)
            return E_DB_SELECT;
        BKINFO("个人转账内部帐户[%s]...", tmp);
        XMLSetNodeVal(doc, "//opBeneacct", tmp); //内部帐户
#endif
        XMLSetNodeVal(doc, "//opMMJYFS", "2"); //2-校验交易密码
        XMLSetNodeVal(doc, "//opZZCZLX", "1"); //转出帐号类型 1-卡
        XMLSetNodeVal(doc, "//opZJZZLX", "9"); //转入帐号类型 9-内部帐户
        XMLSetNodeVal(doc, "//opHUILUU", "3"); //固定3
        XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-非贷销,1-贷销
        XMLSetNodeVal(doc, "//opJIOHLX", "3"); //3-提入借方
        XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //转入帐号性质
        XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //转出帐号性质
    }

    if (dcflag == OP_CREDITTRAN)
    {
        //查询发起行行名
        /*
           memset( sql, 0, sizeof(sql) );
           memset( tmp, 0, sizeof(tmp) );
           sprintf(sql, "select distinct orgname  from organinfo "
           "where nodeid=%d and orgid='%s' and orglevel='3'", OP_REGIONID, XMLGetNodeVal(doc, "//opOriginator"));
           BKINFO("发起行行名[%s]...", tmp);
           XMLSetNodeVal(doc, "//opPJFKHM", tmp); //发起行行名
         */
        org_name(XMLGetNodeVal(doc, "//opOriginator"), tmp);
        XMLSetNodeVal(doc, "//opPJFKHM", tmp); //发起行行名
        /*非税帐号不自动入账*/
        acctno = XMLGetNodeVal(doc, "//opBeneacct");
        sprintf(fs_Acct, "%s", GetFSAcctNo() );
        BKINFO("非税账户[%s].....", fs_Acct);
        //if( !strcmp(acctno, "89010155260000408") )
        if( !strcmp(acctno, fs_Acct) )
        {
            BKINFO("提入贷记(非税账户)行内挂账");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
        }
        else if( notetype == 43 || !truncflag )  //自动扣款标志
        {
            /*非裞(43)业务行内做挂账处理*/
            BKINFO("提入贷记(不截流或非税)行内挂账");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
        }
        else
        {
            BKINFO("提入贷记行内自动记帐");
            XMLSetNodeVal(doc, "//opZDKZBZ", "1");
        }

        XMLSetNodeVal(doc, "//opJIOHLX", "2");
    }
    else if(dcflag == OP_DEBITTRAN)
    {
        if( notetype == 31 )
        {
            if(strlen(XMLGetNodeVal(doc, "//opNoteno")) > 8)
            {
                /*记账和核验时,凭证号用后8位(共16位), 查询批号时分别放到2个字段里
                 */
                if(strlen(XMLGetNodeVal(doc, "//opNoteno")) == 17 )
                {
                    sprintf(noteno1,"%.8s",XMLGetNodeVal(doc, "//opNoteno"));
                    sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+9);
                }
                else
                {
                    sprintf(noteno1,"%.8s",XMLGetNodeVal(doc, "//opNoteno"));
                    sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+8);
                }
                XMLSetNodeVal(doc, "//opTJJXTS", noteno1);
                XMLSetNodeVal(doc, "//opNoteno", noteno2);
            }
        }
        if( notetype == 31 ) 
            ret = CheckNoteInfo(ACCOUNT_CHECK_ZFMM, opDoc); //校验支付密码
        else if( notetype == 32 )
        {
            if( ret = callInterface(ACCOUNT_QUERY_BPINFO_IN, doc) )
            {
                BKINFO("查询登记薄失败[%d]...", ret);
                return ret;
            }

            //获取本票类型
            pp = XMLGetNodeVal(doc, "//opBENPLX");
            if( pp != NULL )
            {
                if( strlen(pp) == 0 )
                {
                    BKINFO("查询本票类型失败[%s]...", pp);
                    return E_APP_ACCOUNTFAIL;
                }
                BKINFO("本票(汇票)类型[%s]柜员流水[%s]生效日期[%s]...", 
                        pp, XMLGetNodeVal(doc, "//opGUIYLS"), XMLGetNodeVal(doc, "//opSXIORQ"));
                //查询总行本票密押
                ret = callInterface( ACCOUNT_QUERY_BPMY, opDoc );
                if( ret )
                {
                    BKINFO("查询本票总行密押败[%d]...", ret);
                    return ret;
                }
                //pp = XMLGetNodeVal(doc, "//opHUIPMY");

                ret = CheckNoteInfo(ACCOUNT_CHECK_BPMY, opDoc); //校验本票密押
            }
        }

        if( ret )
        {
            BKINFO("票据校验失败");
            return ret;
        }
        //测试
#ifdef TEST_PF_PT
        if( notetype == 31 )
            XMLSetNodeVal(doc, "//opPNGZPH", "g");
        else if( notetype == 32 )
            XMLSetNodeVal(doc, "//opPNGZPH", "c");
#else 
        if( notetype == 31 || notetype == 32 )
        {
            /*凭证批号查询*/
            BKINFO("开始获取支付密码支票批号...");
            if (ret = callInterface(ACCOUNT_GET_PH, opDoc))
            {
                BKINFO("获取批号失败");
                return ret;
            }
            pp = XMLGetNodeVal(doc, "//opSHFOBZ");
            if( pp != NULL )
            {
                if( !strlen(pp) || atoi(pp) != 1 )
                {
                    BKINFO("查询支付密码支票批号失败SHFOBZ[%s]", pp);
                    return E_APP_ACCOUNTFAIL; 
                }
            }
            else
                return E_APP_ACCOUNTFAIL; 
            /*还原凭证类型, add by lt@sunyard.com 2011-7-12*/
            if( notetype == 31 )
                XMLSetNodeVal(doc, "//opNotetype", "31");
            else if( notetype == 32 )
                XMLSetNodeVal(doc, "//opNotetype", "32");
        }
#endif
        if (!truncflag) 
        {
            BKINFO("提入非截流借记人工处理");
            return 0;
        }
    }
    ret = Accounting(bktcode, doc); 

    if (dcflag == OP_CREDITTRAN)//贷记存款交易返回成功
        ret = 0;

    return ret;
}

//提出提入退票处理
int TP_Process(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char dcflag = 0;
    char tblName[16] = "trnjour";
    char *ptmp = NULL;
    int tcResult;
    char tmp[50];
    char *q= NULL, *q1=NULL, *q2=NULL; //
    char sworkdate[9]={0};
    char sAutoOrg[13]={0},sAutoOper[13]={0};
    char sql[1024]={0};

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    BKINFO("开始提出提入退票处理,借贷[%c]", dcflag);

    ptmp = XMLGetNodeVal(doc, "//opAgreement");
    memcpy( sworkdate, ptmp, 8 );

    if (strcmp(sworkdate, GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");


    if (p == NULL) { //提入(提出交易)
        /**************提出交易被退票**************************************
         *更新原交易流水为已退票
         *提出贷记被退票调用8248(交换类型=5,自动扣账标志=0,需要人工确认入账
         ******************************************************************
         *提出借记被退票则先发起8992查询入帐方式:
         *1:直接入帐(本票)走8955宕账,其他走8956退票
         ****************************************************************/
        ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c'",
                tblName, OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '1');

        sprintf( sql, "select autooper, autoorg from bankinfo where nodeid=%d and exchno='%s'", 
                OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor") );
        ret = db_query_strs( sql, sAutoOper, sAutoOrg );
        XMLSetNodeVal(doc, "//opInnerBank", sAutoOrg);
        XMLSetNodeVal(doc, "//opInnerorganid", sAutoOrg);
        XMLSetNodeVal(doc, "//opOperid", sAutoOper);
        /*提入把收付款帐号还原成原交易的帐号*/
        XMLSetNodeVal(doc, "//opSHKRZH", XMLGetNodeVal(doc, "//opPayacct"));
        XMLSetNodeVal(doc, "//opSHKRXM", XMLGetNodeVal(doc, "//opPayname"));
        XMLSetNodeVal(doc, "//opFUKRZH", XMLGetNodeVal(doc, "//opBeneacct"));
        XMLSetNodeVal(doc, "//opFUKRXM", XMLGetNodeVal(doc, "//opBenename"));

        XMLSetNodeVal(doc, "//opInoutflag", "2");
        if (dcflag == OP_CREDITTRAN) { //提出贷被退票
            BKINFO("提出贷记被退票...");
            XMLSetNodeVal(doc, "//opJIOHLX", "5");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
            XMLSetNodeVal(doc, "//opBEIZXX", XMLGetNodeVal(doc, "//opPurpose") );
            ret = Accounting(ACCOUNT_INCREDIT_TP, doc);
        } else if (dcflag == OP_DEBITTRAN) { //提出借被退票
            BKINFO("提出借记被退票...");
            //提入退票送当天工作日期及帐(行内交换日期)
            XMLSetNodeVal(doc, "//opExchgdate", XMLGetNodeVal(doc, "//opWorkdate"));
            memset(tmp, 0, sizeof(tmp));
            ret = db_query_str(tmp, sizeof(tmp), "SELECT revserial FROM acctjour where "
                    "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c' and trncode !='%d'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '1', ACCOUNT_CATCH_FEE);
            BKINFO("同城提出号[%s]...",tmp);
            XMLSetNodeVal(doc, "//opTCHTCH", tmp);
            if (ret = callInterface(ACCOUNT_QUERY_ACCOUNTTYPE, doc))
            {
                BKINFO("查询入帐方式失败[%d]...",ret);
                return ret;
            } 
            q = XMLGetNodeVal(doc, "//opTJRZFS");
            if( q != NULL )
            {
                if( strlen(q) )
                {
                    BKINFO("入帐方式[%s]...",q);
                    //直接入帐(本票)走8955宕账,其他走8956退票
                    if( q[0] == '1' ) 
                    {
                        BKINFO("直接入帐提出借记退票...");
                        XMLSetNodeVal(doc, "//opTUIPLX", "0");
                        //XMLSetNodeVal(doc, "//opOriginator", XMLGetNodeVal(doc, "//opAcceptor"));
                        ret = Accounting(ACCOUNT_OUTDEBIT_TPA_BP, doc);
                    }
                    else
                        ret = Accounting(ACCOUNT_OUTDEBIT_TPB, doc);
                }
                else
                {
                    BKINFO("查询入帐方式失败[%s]...",q);
                    return E_APP_ACCOUNTFAIL;
                }
            }
            else
                return E_APP_ACCOUNTFAIL;
        }
    } else if (p[0] == COMMTOPH_BEFORE[0]) {   //与中心通讯前(提入交易)
        /***************************************
         *提入贷记先送8247宕帐,成功后8956退票,
         * 8956不成功发起8247的冲正,成功功后发送人行
         *人行失败则先做8956的冲正,再作8247的冲正
         *人行成功更新退票标志=1,行内不做处理
         *****************************************
         *提入借记先送8955宕帐,成功后8956退票,
         * 8956不成功发起8955的冲正,成功功后发送人行
         *人行失败则先做8956的冲正,再作8955的冲正
         *人行成功更新退票标志=1,行内不做处理
         ***************************************/

#if 1 
        /*************************************************************
         * 退票交易由于行内记两套分录,冲正交易9038必须要以行内日期为准
         * 如果人行日期与行内日期不一致, 则提出退票采用行内日期记帐
         ***********************************************************/


        ret = callInterface( ACCOUNT_GET_DATE, doc );
        q=XMLGetNodeVal(doc, "//opOreserved1");
        if( q != NULL )
        {
            if( strlen(q) )
            {
                BKINFO("行内系统日期[%s]", q );
                XMLSetNodeVal(doc, "//opWorkdate", q);
            }
        }
#endif

        /********************************************************
          按提入行(退票交易的发起行)的行内机构记账
          分行可以代理下属机构退票,根据提出机构转为行内机构退票
          冲正还要发起机构去冲正
         *********************************************************/
        /*
           memset(tmp, 0, sizeof(tmp));
           q=XMLGetNodeVal(doc, "//opOriginator");
           q1=XMLGetNodeVal(doc, "//opInnerBank");
           db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", OP_REGIONID, q);
           XMLSetNodeVal(doc, "//opInnerBank",tmp);
         */

        if (dcflag == OP_CREDITTRAN) { //提出贷退票
            BKINFO("提入贷记退票...");
            //贷记送当天工作日期及帐(行内交换日期)
            XMLSetNodeVal(doc, "//opExchgdate", XMLGetNodeVal(doc, "//opWorkdate"));
            if ((ret = Accounting(ACCOUNT_OUTCREDIT_TPA, doc)) == E_APP_ACCOUNTSUCC)
            {
                XMLSetNodeVal(doc, "//opTCHTCH", XMLGetNodeVal(doc, "//opOreserved3"));
                BKINFO("提入贷记退票,行内[%d]成功,同城提出号[%s],开始[%d]...", ACCOUNT_OUTCREDIT_TPA, 
                        XMLGetNodeVal(doc, "//opTCHTCH"),ACCOUNT_OUTCREDIT_TPB);

                ret = Accounting(ACCOUNT_OUTCREDIT_TPB, doc);
                /*8956不成功则发起8247的冲正*/
                if( ret != E_APP_ACCOUNTSUCC )
                {
                    //XMLSetNodeVal(doc, "//opInnerBank",q1); //还原机构
                    BKINFO("提入贷记退票,行内[%d]失败,开始[%d]冲正...", ACCOUNT_OUTCREDIT_TPB, ACCOUNT_OUTCREDIT_TPA);

                    if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPA, 2, doc)) == 0) //冲正(8247)
                        return E_APP_ACCOUNTANDCZ;
                    else
                    {
                        BKINFO("[%d]冲正失败[%d]...", ACCOUNT_OUTCREDIT_TPA, ret);
                        return E_APP_ACCOUNTNOCZ;
                    }
                }
            }
        } else if (dcflag == OP_DEBITTRAN) { //提出借退票
            /**********************************************
             * 根据柜员查询所属于的营业机构
             **************************************************************
             *退票机构规则修改    2011-04-06
             *根据89区分,如果为89开头则送本机构记账,送人行为本机构同城机构
             *否则直接送发起机构的上级机构记账,送人行为上级机构同城机构
             ***************************************************************
             *以上规则region里面处理  OP_DoInit
             *如果为8955则送原交易的接收机构去行内记账
             *8956送上面规则的机构记账
             **************************************************************/
            BKINFO("提入借记退票...");
            //送原交易的工作日期后台记帐(交换日期)
            XMLSetNodeVal(doc, "//opExchgdate", sworkdate);

            q=XMLGetNodeVal(doc, "//opQISHHH");
            q1=XMLGetNodeVal(doc, "//opInnerBank");
            q2=XMLGetNodeVal(doc, "//opOriginator");

            //OP_DoInit 处理过,用原交易提入机构记账
            memset(tmp, 0, sizeof(tmp));
            db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", OP_REGIONID, q);

            BKINFO("提入借记被退票,同城机构[%s],送行内机构[%s]去核心[%d]记账...", q, tmp, ACCOUNT_OUTDEBIT_TPA);
            XMLSetNodeVal(doc, "//opInnerBank",tmp);
            XMLSetNodeVal(doc, "//opOriginator",XMLGetNodeVal(doc, "//opQISHHH"));
            if ((ret = Accounting(ACCOUNT_OUTDEBIT_TPA, doc)) == E_APP_ACCOUNTSUCC)
            {
                XMLSetNodeVal(doc, "//opInnerBank",q1);
                XMLSetNodeVal(doc, "//opOriginator",q2);
                XMLSetNodeVal(doc, "//opTCHTCH", XMLGetNodeVal(doc, "//opOreserved3"));
                BKINFO("提入借记退票,行内[%d]成功,同城提出号[%s]开始[%d]...", ACCOUNT_OUTDEBIT_TPA,
                        XMLGetNodeVal(doc, "//opTCHTCH"),ACCOUNT_OUTDEBIT_TPB);
#if 0
                if( ret = callInterface(ACCOUNT_QUERY_ORG, doc) )
                {
                    BKINFO("查询营业机构失败[%d]...", ret);
                    return ret;
                }

                q=XMLGetNodeVal(doc, "//opHostSerial");
                if(strncmp(q,"89",2))
                {
                    BKINFO("集中业务虚拟柜员[%s]行内机构[%s]...", XMLGetNodeVal(doc, "//opOperid"), q);
                    memset(tmp, 0, sizeof(tmp));
                    ret = db_query_str(tmp, sizeof(tmp), "SELECT parent FROM bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, q1 );
                    BKINFO("[%s]同城支行,送[%s]退票...", q, tmp);
                    XMLSetNodeVal(doc, "//opInnerBank", tmp);
                }
                else
                    XMLSetNodeVal(doc, "//opInnerBank",q);
#endif
                ret = Accounting(ACCOUNT_OUTDEBIT_TPB, doc);
                /*8956不成功则发起8955的冲正  ????*/
                if( ret !=  E_APP_ACCOUNTSUCC )
                {
                    XMLSetNodeVal(doc, "//opInnerBank", tmp); //还原机构
                    XMLSetNodeVal(doc, "//opOriginator",q);
                    BKINFO("提入借记退票,行内[%d]失败,开始[%d]冲正...", ACCOUNT_OUTDEBIT_TPB, ACCOUNT_OUTDEBIT_TPA);
                    if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPA, 2, doc)) == 0) //冲正(8955)
                        return E_APP_ACCOUNTANDCZ;
                    else
                        return E_APP_ACCOUNTNOCZ;
                }
            }
        }
    }
    else if (p[0] == COMMTOPH_AFTER[0])  { //与中心通讯后
        tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
        BKINFO("提出退票人行返回[%d]", tcResult);
        if (!tcResult)
        {
            BKINFO("提出退票人行成功,更新退票标志,行内不作处理");
            ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                    "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c'",
                    tblName, OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '2');
        }
        else
        {
            if(dcflag == OP_CREDITTRAN)//贷记退票中心失败则做冲正
            {
                BKINFO("提入贷记退票人行失败,行内冲正...");
                if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPB, 3, doc)) == 0) //冲正(8956)
                {
                    BKINFO("提入贷记退票人行失败,行内[%d]冲正成功,开始[%d]冲正...", ACCOUNT_OUTCREDIT_TPB, ACCOUNT_OUTCREDIT_TPA);
                    if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPA, 3, doc)) == 0) //冲正(8247)
                        return E_APP_ACCOUNTANDCZ;
                    else 
                        return E_APP_ACCOUNTNOCZ;
                }
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            else if( dcflag == OP_DEBITTRAN ) //借记退票中心失败
            {
                BKINFO("提入借记退票人行失败,行内冲正...");
                if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPB, 3, doc)) == 0) //冲正(8956)
                {
                    BKINFO("提入借记退票人行失败,行内[%d]冲正成功,开始[%d]冲正...", ACCOUNT_OUTDEBIT_TPB, ACCOUNT_OUTDEBIT_TPA);
                    memset(tmp, 0, sizeof(tmp));
                    db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", 
                            OP_REGIONID, XMLGetNodeVal(doc, "//opQISHHH"));

                    XMLSetNodeVal(doc, "//opInnerBank",tmp);
                    XMLSetNodeVal(doc, "//opOriginator",XMLGetNodeVal(doc, "//opQISHHH"));
                    if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPA, 3, doc)) == 0) //冲正(8955)
                        return E_APP_ACCOUNTANDCZ;
                    else 
                        return E_APP_ACCOUNTNOCZ;
                }
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
        }
    }

    return ret;
}


int BeforeTransOut(int bktcode, char dcflag, xmlDocPtr doc)
{
    char pwd[128];
    char tmp[256];
    char *acctno = NULL;
    int  iFeeFlag=-1;
    int  classid=-1;
    char *p =NULL;
    char *acctname = NULL;

    if( strlen(XMLGetNodeVal(doc, "//opSHFEBZ")) )
        iFeeFlag = atoi(XMLGetNodeVal(doc, "//opSHFEBZ"));
    classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
    if (dcflag == OP_DEBITTRAN)
    {
        //提出借记对公业务要去校验本行账户名称是否正确
        if( classid == 1 )
        {
            XMLSetNodeVal(doc, "//opZHANGH", XMLGetNodeVal(doc, "//opBeneacct")); //收款帐户
            if( ret = callInterface(ACCOUNT_ACCTINFO_PUB, doc) )
                return ret;
            acctname = XMLGetNodeVal(doc, "//opKEHZWM");
            /*
               XMLSetNodeVal(doc, "//opAcctNo", XMLGetNodeVal(doc, "//opBeneacct")); //收款帐户
               if( ret = callInterface(ACCOUNT_ACCTINFO_PERSON, doc) )
               return ret;
               acctname = XMLGetNodeVal(doc, "//opCardName");
             */

            p= XMLGetNodeVal(doc, "//opBenename");
            if( acctname != NULL )
            {
                if( strcmp( acctname, p ) )
                {
                    sprintf( tmp, "户名不相符:柜面提交[%s],行内[%s]", p, acctname );
                    BKINFO( "%s...", tmp);
                    XMLSetNodeVal(doc, "//opBKRetinfo", tmp);
                    return E_APP_NONEEDACCOUNT;
                }
            }
            else
            {
                sprintf( tmp, "帐户户名查询失败" );
                BKINFO( "%s...", tmp);
                XMLSetNodeVal(doc, "//opBKRetinfo", tmp);
                return E_APP_NONEEDACCOUNT;
            }
        }

        if( iFeeFlag == 1 )
        {
            BKINFO("提出借记发送中心前先扣客户手续费...");
            if( (ret = Accounting( ACCOUNT_CATCH_FEE, doc ) )!= E_APP_ACCOUNTSUCC )
            {
                BKINFO("扣客户手续费失败[%d]...", ret);
                return ret;
            }
        }
        BKINFO("提出借记发送中心前行内不记账...");
        return 0;
    }

    // 提出贷记交易付款账号为本行账号, 去掉'*'号等
    acctno = XMLGetNodeVal(doc, "//opPayacct");
    ProcAcctNo(tmp, acctno, 1 );
    XMLSetNodeVal(doc, "//opPayacct", tmp);

    ret = Accounting(bktcode, doc);

    // 填回原账号
    XMLSetNodeVal(doc, "//opPayacct", acctno);

    return ret;
}

int AfterTransOut(int bktcode, char dcflag, xmlDocPtr doc)
{
    char tmp[40]   = {0};
    char tmp1[40]  = {0};
    char sql[512]  = {0};
    int  tcResult  = 0;
    int  truncflag = 0;
    int  ret       = 0;
    int  iFeeFlag  = -1;
    int  notetype, oldflag=-1;
    char *orgid    = NULL, *acctno = NULL, *acctname = NULL;
    char *p        = NULL;

    tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    if (dcflag == OP_CREDITTRAN)
    {
        if (isClearstateUnknowTran(tcResult))
            return E_SYS_COMM;
        ret = tcResult;
        p = XMLGetNodeVal(doc, "//opReserved");
        if( p != NULL )
            oldflag= atoi(p);

        if (tcResult) {
            if(oldflag == 1)
            {
                //return 0;
                //modify by lt@sunyard.com 2012-6-19
                return E_APP_ACCOUNTFAIL;
            }
            if ((ret = AccountCancel(ACCOUNT_CREDITCZ, 2, doc)) == 0) //提出贷记冲正(8963)
                return E_APP_ACCOUNTANDCZ;
            else 
                return E_APP_ACCOUNTNOCZ;
        }

        /*老通城业务发送人行成功后更新标志*/
        if( oldflag == 1 )
        {
            BKINFO("发送人行成功,更新为老同城业务..." );
            memset( sql, 0, sizeof(sql) );
            sprintf(sql, "update trnjour set reserved3='1' WHERE nodeid=%d AND workdate='%s' AND "
                    "originator='%s' AND convert(decimal, refid)=%s AND inoutflag='1'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"),XMLGetNodeVal(doc, "//opOriginator"),
                    XMLGetNodeVal(doc, "//opRefid"));
            ret = db_exec( sql );
            return ret;
        }
        /*个人现金通存(71)送人行成功后行内8963记账*/
        if( notetype == 71 )
        {
#ifdef TEST_PF 
            XMLSetNodeVal(doc, "//opPayacct", "95010133150000098"); //内部帐户
            orgid  = XMLGetNodeVal(doc, "//opOriginator");
            XMLSetNodeVal(doc, "//opOriginator", "622"); //交换机构
            /*内部户户名查询*/
            XMLSetNodeVal(doc, "//opZHANGH", "95010133150000098"); //内部帐户
            XMLSetNodeVal(doc, "//opPayname", "待划转汇款"); //内部帐户
#else
            /*查询个人转账内部帐户(3315) returnacct:帐号, reserved1:户名*/
            orgid  = XMLGetNodeVal(doc, "//opOriginator");
            sprintf(sql, "select distinct returnacct, reserved1 from bankinfo where nodeid=%d and  exchno='%s' ", OP_REGIONID, orgid );
            if (db_query_strs(sql, tmp, tmp1) != 0)
                return E_DB_SELECT;

            BKINFO("个人转账内部帐户[%s]户名[%s]...", tmp, tmp1);
            XMLSetNodeVal(doc, "//opPayacct", tmp); //内部帐户
            XMLSetNodeVal(doc, "//opPayname", tmp1); //内部帐户户名
            /*
            //内部户户名查询
            XMLSetNodeVal(doc, "//opZHANGH", tmp); //内部帐户
            if( ret = callInterface(ACCOUNT_ACCTINFO_PUB, doc) )
            return ret;
            acctname = XMLGetNodeVal(doc, "//opKEHZWM");
            if( acctname != NULL )
            {
            if ( acctname[0] != '\0' )
            {
            BKINFO("内部帐户户名[%s]...", acctname);
            XMLSetNodeVal(doc, "//opPayname", XMLGetNodeVal(doc, "//opKEHZWM") ); //内部帐户户名
            }
            else
            {
            BKINFO("内部帐户户名[%s]查询失败...", acctname);
            return -1;
            }
            }
            else
            {
            BKINFO("内部帐户户名[%s]查询失败...", acctname);
            return -1;
            }
             */
#endif

            XMLSetNodeVal(doc, "//opPNGZZL", "zz"); //凭证种类
            BKINFO("个人业务中心处理成功,行内[%d]记帐...", ACCOUNT_OUTCREDIT);
            if( (ret = Accounting(ACCOUNT_OUTCREDIT_PERSON, doc)) != E_APP_ACCOUNTSUCC )
            {
                BKINFO("个人转账提出贷记行内[%d]失败...", ACCOUNT_OUTCREDIT_PERSON);
#ifdef TEST_PF
                //还原发起机构(测试使用)
                XMLSetNodeVal(doc, "//opOriginator", orgid); //交换机构
#endif
                return ret;
            }
#ifdef TEST_PF
            //还原发起机构(测试使用)
            XMLSetNodeVal(doc, "//opOriginator", orgid); //交换机构
#endif
        }
        else
            BKINFO("提出贷中心处理成功,行内不处理");
    } 
    else if (dcflag == OP_DEBITTRAN) 
    {
        if( strlen(XMLGetNodeVal(doc, "//opSHFEBZ")) )
            iFeeFlag = atoi(XMLGetNodeVal(doc, "//opSHFEBZ"));
        if (tcResult != 0) 
        {
            if( iFeeFlag == 1 )
            {
                BKINFO("提出借中心处理失败,行内[%d]冲正", ACCOUNT_CATCH_FEE);
                if ((ret = AccountCancel(ACCOUNT_CATCH_FEE, 2, doc)) == 0) //手续费冲正(9579)
                    return E_APP_ACCOUNTANDCZ;
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            BKINFO("提出借中心处理失败,行内不处理");
            return E_APP_NONEEDACCOUNT;
        } 

        /******************************
          72(通兑)-先8951,后4202
          74(转账)-8227
         ******************************/
        if(notetype == 74 || notetype == 72)
        {
#ifdef TEST_PF
            if(notetype==74)
                XMLSetNodeVal(doc, "//opPayacct", "95010134900000011"); //清算账户
            else
                XMLSetNodeVal(doc, "//opBeneacct", "95010133150000098"); //内部账户
#else
            /*查询个人转账内部帐户*/
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            memset( tmp1, 0, sizeof(tmp1) );
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
            /*72送3315,74送3490(2881)*/
            if(notetype == 72 )
                sprintf(sql, "select distinct returnacct, reserved1 from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            else
                sprintf(sql, "select distinct clearacct, clearacct from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            if (db_query_strs(sql, tmp, tmp1) != 0)
                return E_DB_SELECT;

            BKINFO("个人转账内部帐户[%s]户名[%s]...", tmp, tmp1);
            if(notetype==74)
                XMLSetNodeVal(doc, "//opPayacct", tmp); //清算账户
            else
            {
                XMLSetNodeVal(doc, "//opBeneacct", tmp); //内部账户
                XMLSetNodeVal(doc, "//opBenename", tmp1); //内部账户户名
            }
#endif
            /*先8951内部帐户收款,4202-现金兑*/
            if(notetype == 72 )
            {
#ifdef TEST_PF
                orgid  = XMLGetNodeVal(doc, "//opOriginator");
                XMLSetNodeVal(doc, "//opOriginator", "622"); //交换机构
                XMLSetNodeVal(doc, "//opZHANGH", "95010133150000098"); //内部帐户
                XMLSetNodeVal(doc, "//opBenename", "待划转汇款"); //内部帐户
#endif
                XMLSetNodeVal(doc, "//opPNGZZL", "zz"); //凭证种类
                XMLSetNodeVal(doc, "//opTJRZFS", "1");  //1-直接入帐
                BKINFO("个人提出借记中心处理成功,行内先[%d]记帐...", ACCOUNT_OUTNOTRUNCDEBIT_PERSON);
                if( (ret = Accounting(ACCOUNT_OUTNOTRUNCDEBIT_PERSON, doc)) != E_APP_ACCOUNTSUCC )
                {
                    BKINFO("个人转账提出借记行内[%d]失败...", ACCOUNT_OUTNOTRUNCDEBIT_PERSON );
                    return ret;
                }
#ifdef TEST_PF
                //还原发起机构(测试使用)
                XMLSetNodeVal(doc, "//opOriginator", orgid); //交换机构
#endif

#ifdef TEST_PF
                XMLSetNodeVal(doc, "//opPayacct", "95010133150000098"); //内部账户
#else
                /*4202变成付款帐户为内部帐户*/
                XMLSetNodeVal(doc, "//opPayacct", tmp); //内部账户
#endif
            }
            else
            {

                XMLSetNodeVal(doc, "//opZZCZLX", "9"); //转出帐号类型 9-内部
                XMLSetNodeVal(doc, "//opHUILUU", "3"); //固定3
                XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-非贷销,1-贷销
                XMLSetNodeVal(doc, "//opJIOHLX", "1"); //固定1-提出借方
                XMLSetNodeVal(doc, "//opZJZZLX", "1"); //转入帐号类型 1-卡
                XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-不校验交易密码
                XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //转入帐号性质
            }
        }
        ret = Accounting(bktcode, doc); 
    }

    return ret;
}

// 收取手续费
int CollectFee(int notetype, char dcflag, xmlDocPtr doc)
{
    double fee         = 0;
    char   *pAcctPath  = NULL;
    char   acctno[40]  ={0};
    char   tmp[20]     ={0};

    pAcctPath = (dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct");
    ProcAcctNo(acctno, XMLGetNodeVal(doc, pAcctPath), 1);

    BKINFO("判断提出是否个人账号[%s]借贷[%c]", acctno, dcflag);
    if (acctno != NULL && strlen(acctno) > 1 
            && (*acctno == '1' || (*acctno == '*' && acctno[1] == '1'))) 
    {
        //提出个人账号实时收手续费
        if ((fee = feeCalculate(dcflag, notetype)) > 0) {
            sprintf(tmp, "%.2lf", fee);
            XMLSetNodeVal(doc, "//opBankFee", tmp);
        }
    }

    return 0;
}

/*
 * 提出交易
 * 输入:opDoc 平台报文 p 保留
 * 返回:平台错误码
 */
int PF10_108(void *opDoc, char *p)
{
    xmlDoc *doc      = (xmlDoc *)opDoc;
    char   sql[512]  = {0};
    char   tmp[128]  = {0}; 
    char   tmp1[128] = {0};
    char   dcflag;
    char   *acctno   = NULL, *oacctno = NULL, *dacctno = NULL;
    char   *pwd      = NULL, *orgid = NULL, *noteno = NULL, *pin = NULL;
    int    notetype;
    int    bktcode, oldflag, opcode;
    char   *pzFlag   =NULL;

    /*退票交易*/
    opcode = atoi(XMLGetNodeVal(doc, "//opTctcode"));
    if( opcode == 7 )
    {
        BKINFO("提出退票...");
        return  TP_Process(opDoc, p);
    }

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    // 查找对应的提出核心交易码//add distinct @ 2011.01.20
    sprintf(sql, "select distinct reserved1 from notetypemap "
            "where tctype='%02d' and dcflag='%c' and nodeid=%d",
            notetype, dcflag, OP_REGIONID);
    if (db_query_strs(sql, tmp) != 0)
        return E_DB_SELECT;
    bktcode = atoi(tmp);

    if (p[0] == COMMTOPH_BEFORE[0]) 
    { 
        BKINFO("处理提出交易: 借贷标志=[%c] 票据种类=[%02d] 与中心通讯前...",
                dcflag, notetype);
        /*提出贷记 行内密码前台加密,平台直接发送行内后台记帐
          提出借记 转加密个人密码(平台调用行内加密机)
         */

        oldflag= atoi(XMLGetNodeVal(doc, "//opReserved"));
        if( oldflag == 1 )
        {
            BKINFO("老同城业务,行内不处理..." );
            return 0;
        }

        if (notetype == 72 || notetype == 74)
        {
            /*******************************************************
             *提出个人转账(74)调8227行内记账
             *******************************************************
             *提出个人通存(72)发送人行处理成功后先调8951行内记账(内部帐户)
             *成功后再4202记账(内部帐户)
             *******************************************************/
            memset(tmp, 0, sizeof(tmp));
            dacctno = XMLGetNodeVal(doc, "//opPayacct");
            pwd     = XMLGetNodeVal(doc, "//opAgreement");
            orgid   = XMLGetNodeVal(doc, "//opInnerBank");
            oacctno = XMLGetNodeVal(doc, "//opSYSPINSEED");

            if( ret = ConvertPersonKey( 1, oacctno, dacctno, pwd, orgid, tmp) )
                return ret;
            XMLSetNodeVal(doc, "//opAgreement", tmp);
        }
        else if (notetype == 73 || notetype == 71)//个人提出贷记
        {
            /*******************************************************
             *提出个人转账(73)调8227行内记账
             *******************************************************
             *提出个人通存(71)先调4201行内记账(过渡帐户3315)
             *人行成功则再8963记账(内部帐户3315到3490)
             *******************************************************/
            memset(tmp, 0, sizeof(tmp));
            acctno = XMLGetNodeVal(doc, "//opBeneacct");
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
            pwd    = XMLGetNodeVal(doc, "//opAgreement");
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
#ifdef TEST_PF
            if( notetype == 71 )
                XMLSetNodeVal(doc, "//opBeneacct", "95010133150000098"); //内部帐户(3315)
            else 
                XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //内部帐户(3490)
#else
            /*询个人转账内部帐户*/
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            /*通存(71)送3315的过渡帐户,转账(73)送3490的内部帐户*/
            if( notetype == 71 )
                sprintf(sql, "select distinct returnacct from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            else
                sprintf(sql, "select distinct clearacct  from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            if (db_query_strs(sql, tmp) != 0)
                return E_DB_SELECT;
            BKINFO("个人转账内部帐户[%s]...", tmp);
            XMLSetNodeVal(doc, "//opBeneacct", tmp); //内部帐户
            XMLSetNodeVal(doc, "//opPurpose", acctno); //保存收款帐户到临时帐户
#endif
            XMLSetNodeVal(doc, "//opZZCZLX", "1"); //转出帐号类型 1-卡
            XMLSetNodeVal(doc, "//opZJZZLX", "9"); //转入帐号类型 9-内部帐户
            XMLSetNodeVal(doc, "//opMMJYFS", "2"); //2-校验交易密码
            XMLSetNodeVal(doc, "//opHUILUU", "3"); //固定3
            XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-非贷销,1-贷销
            XMLSetNodeVal(doc, "//opJIOHLX", "0"); //0-提出贷方
            XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //转入帐号性质
            XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //转出帐号性质
        }
        else if( notetype == 43 ) 
        {
            //凭证号码
            XMLSetNodeVal(doc, "//opPNGZZL", "zz");
            XMLSetNodeVal(doc, "//opNoteno", XMLGetNodeVal(doc, "//opBEIZXX"));
        }
        else if( notetype == 42 )
        {
            //当为g6的时候才用内部账户扣款
            pzFlag = XMLGetNodeVal(doc, "//opPNG1ZL");
            if( !strcmp(pzFlag, "g6") )
            {
                //海关税票送内部帐户扣款,reserved:户名,debitacct:内部帐户
                orgid   = XMLGetNodeVal(doc, "//opInnerBank");
                memset( sql, 0, sizeof(sql) );
                sprintf(sql, "select distinct debitacct, reserved from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
                if (db_query_strs(sql, tmp, tmp1) != 0)
                    return E_DB_SELECT;
                BKINFO("海关税票内部帐户[%s]户名[%s]...", tmp, tmp1);
                XMLSetNodeVal(doc, "//opPayacct", tmp); //内部帐户
                XMLSetNodeVal(doc, "//opPayname", tmp1); //内部帐户户名
            }
        }

        ret = BeforeTransOut(bktcode, dcflag, doc);
    } 
    else if (p[0] == COMMTOPH_AFTER[0])  
    { 
        BKINFO("处理提出交易: 借贷标志=[%c] 票据种类=[%d] 与中心通讯后...",
                dcflag, notetype);

        if (notetype == 73 || notetype == 71)//个人提出贷记
        {
            acctno = XMLGetNodeVal(doc, "//opPurpose");
            XMLSetNodeVal(doc, "//opBeneacct", acctno); //还原收款帐户
            BKINFO("个人同存提出还原收款帐户[%s]", acctno);
        }
        ret = AfterTransOut(bktcode, dcflag, doc);

        if(*XMLGetNodeVal(doc, "//opClassid")=='2')
        {
            BKINFO("开始保存个人交易信息到文件");
            SavePersonNote(doc);
        }
        if( notetype == 14 )
        {
            BKINFO("开始保存税票信息(交易信息)到文件...");
            SaveTaxNote(doc);
            BKINFO("保存税票信息(交易信息)到文件结束...");
        }
        /*
        // 收取手续费
        if (atoi(XMLGetNodeVal(doc, "//opTCRetcode")) == 0)
        CollectFee(notetype, dcflag, doc);
         */
    }

    return ret;
}

/*
 * 提入借贷(退票)确认
 * 输入 opDoc 平台报文 p 保留
 */
int PF10_123(void *opDoc, char *p)
{
    xmlDoc *doc           = (xmlDoc *)opDoc;
    char   where[1024]    = {0};
    char   where1[1024]   = {0};
    char   workdate[9]    = {0};
    char   dcflag         = 0;
    char   tmp[20]        = {0};
    char   bktype[3]      = {0};
    char   *q             = NULL;
    char   noteno[14]     = {0};
    char   *dfamount      = NULL;
    char   exchgdate[9]   = {0}; 
    char   exchground[2]  = {0};

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    strcpy(bktype,XMLGetNodeVal(doc, "//opPNGZZL"));

    strcpy(workdate, XMLGetNodeVal(doc, "//opWorkdate"));
    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='2'",
            OP_REGIONID, workdate, XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'));

    BKINFO("提入确认:借贷[%c]行内凭证种类[%s]", dcflag, bktype);

    XMLSetNodeVal(doc, "//opInoutflag", "2");
    /*确认交易送平台工作日期,行内交换日期(平台原交易的工作日期),交换场次*/
    XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate());
    //确认记帐用原交易日期保存记帐流水
    XMLSetNodeVal(doc, "//opChkflag", "1");
    sprintf( GSQL, "select exchgdate, exchground from %s %s", 
            strcmp(workdate, GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);
    ret = db_query_strs( GSQL, exchgdate, exchground );
    BKINFO("原交易的交换日期[%s],交换场次[%s]", exchgdate, exchground );
    if( ret == 0 )
    {
        XMLSetNodeVal(doc, "//opExchgdate", workdate);
        XMLSetNodeVal(doc, "//opExchground", exchground);
    }

    /*
       ret = callInterface( ACCOUNT_GET_DATE, doc );
       q=XMLGetNodeVal(doc, "//opOreserved1");
       if( strlen(q) )
       {
       BKINFO("行内系统日期[%s]", q );
       XMLSetNodeVal(doc, "//opWorkdate", q);
       }
     */

    if (*XMLGetNodeVal(doc, "//opReserved") == '7') 
    { //提入退票确认
        if (dcflag == OP_DEBITTRAN)
            ret = Accounting(ACCOUNT_INDEBIT_TPCHK, doc);
        else if (dcflag == OP_CREDITTRAN)
            ret = Accounting(ACCOUNT_INCREDIT_TPCHK, doc);
    } else {
        if (dcflag == OP_DEBITTRAN) {
            ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s AND result='1'", where);
            if (ret == E_DB_NORECORD)
            {
                /*****************************************
                 *本票,汇票先用8307查询类型,再送行内记帐
                 *承兑汇票用ef14
                 *商业汇票用ef16
                 *根据同城提入行查找行内机构
                 ****************************************/
                memset( tmp, 0, sizeof(tmp) );
                ret = db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
                BKINFO( "*****EXCHNO[%s]-->BANKID[%s]*****", XMLGetNodeVal(doc, "//opAcceptor"), tmp );
                XMLSetNodeVal( doc, "//opInnerorganid", tmp );
                XMLSetNodeVal( doc, "//opInnerBank", tmp );

                if(strcmp(bktype,"12")==0 )
                {
                    if( ret = callInterface(ACCOUNT_QUERY_BPINFO, doc) )
                    {
                        BKINFO("查询登记薄失败[%d]...", ret);
                        return ret;
                    }

                    q = XMLGetNodeVal(doc, "//opBENPLX");
                    if( strlen(q) == 0 )
                    {
                        BKINFO("查询本票类型失败[%s]...", q);
                        return E_APP_ACCOUNTFAIL;
                    }
                    BKINFO("本票(汇票)类型[%s]...", q);
                    ret = Accounting(ACCOUNT_INDEBITCHK_BP, doc);
                }
                else if(strcmp(bktype,"07")==0)//银行汇票
                {
                    dfamount = XMLGetNodeVal(doc, "//opDUIFJE");
                    if(strlen(dfamount) && atof(dfamount)>0.00)
                    {
                        XMLSetNodeVal(doc, "//opDUIFJE", XMLGetNodeVal(doc, "//opSettlamt"));
                        XMLSetNodeVal(doc, "//opSettlamt", dfamount);
                    }
                    ret = Accounting(ACCOUNT_INDEBITCHK_HP, doc);
                }
                else if(strcmp(bktype,"08")==0)//银行承兑汇票
                {

                    q = sdpXmlSelectNodeText(doc, "//opNoteno");
                    if (q != NULL && strlen(q) > 8) {
                        BKINFO("凭证号太长,截断[%s->%s]", q, p+strlen(q)-8);
                        XMLSetNodeVal(doc, "//opNoteno", q+strlen(q)-8); //行内凭证号最大8位(截最后8位)
                    }
                    q = XMLGetNodeVal(doc, "//opPNGZPH");
                    sprintf(noteno,"%4s%1s%8s","0899", q == NULL ? "0":q, 
                            XMLGetNodeVal(doc, "//opNoteno"));
                    //FREE(q);
                    XMLSetNodeVal(doc, "//opPNGZHH", noteno);
                    ret = Accounting(ACCOUNT_INDEBITCHK_CDHP, doc);
                }
                else if(strcmp(bktype,"10")==0)//银行商业汇票
                    ret = Accounting(ACCOUNT_INDEBITCHK_SYHP, doc);
                else//其他
                    ret = Accounting(ACCOUNT_INDEBITCHK_OTHER, doc);
            }
            else
            {
                return E_DB_NORECORD;
            }
        } else if (dcflag == OP_CREDITTRAN) { //提入贷未记账可以再确认记账
            ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s and trncode='%d'", where, ACCOUNT_INCREDIT);
            if( ret == 0 )
            {
                /*只有明确8248挂账才可以确认入帐*/
                if( atoi(tmp) != 5 )
                    return E_DB_NORECORD;
                ret = Accounting(ACCOUNT_INCREDIT_CHK, doc);
            }
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
    xmlDoc      *doc          = (xmlDoc *)opDoc;
    result_set  tpRS, ywRS;
    const char  *pSTDate      = XMLGetNodeVal(doc, "//opWorkdate");
    const char  *pSTRound     = XMLGetNodeVal(doc, "//opWorkround");
    const char  *pOriginator  = XMLGetNodeVal(doc, "//opOriginator");
    const char  *pInfo        = "";
    const char  *ptmp         = NULL;
    char        tblName[10]   = "trnjour";
    int         recordCount   = 0;
    int         i             = 0;

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

    //查询已对账提出非截留借记且非本汇票交易进行收妥(未被退票)
    ret = db_query(&ywRS, "SELECT * FROM %s WHERE nodeid=%d and classid=1 and clearstate='C' and truncflag='0' "
            "and dcflag='1' and inoutflag='1' and workdate='%s' and workround='%s' "
            "and originator='%s' and notetype not in('03', '04') and stflag!='1' and tpflag!='1'",
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
    xmlDoc *doc    = (xmlDoc *)opDoc;
    char   tmp[64] = {0};
    char   pwd[64] = {0};
    int    isHist;

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
        BKINFO("Ready BankPwdEncrypt(%s)...", pwd);
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
    xmlDoc     *doc           = (xmlDoc *)opDoc;
    result_set ywRS;
    double     fee            = 0;
    char       feeDate[9]     = {0}; 
    char       lastFeedate[9] = {0};
    int        succFlag       = 1;
    int        forbitSuccFlag = 0;
    int        recordCount    = 0, i = 0;

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
    result_set  rs;
    xmlDoc      *doc           = (xmlDoc *)opDoc;
    int         rc;
    int         i;
    char        line[8192]     = {0};
    char        outline[8192]  = {0};
    char        *fields[256];
    char        caOutFile[256] = {0};
    int         fieldnum;
    char        fname[64];
    sprintf(fname, "%s/%s", getenv("FILES_DIR"), XMLGetNodeVal(doc, "//opReserved"));
    FILE        *fp            = fopen(fname, "r");

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
    int            ret               = 0;
    xmlDoc         *opDoc            = (xmlDoc *)doc;
    char           sAutoOrg[12+1]    = {0}; 
    char           sAutoOper[12+1]   = {0};
    char           sName1[81]        = {0};
    char           sName2[81]        = {0};
    char           sSqlStr[1024]     = {0};
    char           *q                = NULL;

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
    xmlDoc         *doc            = (xmlDoc *)opDoc;
    result_set     rs, rs2;
    FILE           *fp             = NULL;
    char           tmp[1024]       = {0};
    char           caParaFile[256] = {0}; 
    char           caDataFile[256] = {0};
    char           caOutFile[256]  = {0};
    char           startdate[9]    = {0}; 
    char           enddate[9]      = {0};
    int            recordCount     = 0;
    double         feeSum          = 0;
    int            i               = 0;

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
    xmlDoc            *doc              = (xmlDoc *)opDoc;
    result_set        rs, rs2;
    FILE              *fp               = NULL;
    char              tbname[10]        = "trnjour"; 
    char              tmp[1024]         = {0};
    char              caParaFile[256]   = {0};
    char              caDataFile[256]   = {0}; 
    char              caOutFile[256]    = {0};
    char              settledate[16]    = {0};
    char              currdate[16]      = {0};
    int               classid           = 0; 
    int               clearround        = 0;
    int               recordCount       = 0;
    int               recordCount2      = 0;
    int               i                 = 0;
    int               j                 = 0;

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
    xmlDoc         *doc                  = (xmlDoc *)opDoc;
    result_set     rs, rs1, rs2;
    FILE           *fp                   = NULL;
    char           tmp[1024]             = {0};
    char           subject[4]            = {0};
    char           chinese[128]          = {0};
    char           caParaFile[256]       = {0};
    char           caDataFile[256]       = {0};
    char           caOutFile[256]        = {0};
    char           startdate[9]          = {0};
    char           enddate[9]            = {0};
    int            recordCount           = 0;
    double         feeSum                = 0;
    int            i                     = 0;

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
    xmlDoc        *doc              = (xmlDoc *)opDoc;
    result_set    rs;
    int           recordCount       = 0;
    int           i                 = 0;
    char          acctDesc[1024]    = {0};

    ret = db_query(&rs, "SELECT acctserial, trncode, result FROM acctjour WHERE %s", 
            GetSigleTrnjourWhere(doc));
    if (ret)
        return ret;

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++)
    { 
        sprintf(acctDesc+strlen(acctDesc), "|[%s]|[%s]", db_cell(&rs, i, 0), getAccountResult(db_cell(&rs, i, 2)) );
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
    int         ret              = 0;
    xmlDoc      *opDoc           = (xmlDoc *)doc;
    char          sql[512]         = {0};
    char          where[512]         = {0};
    char          refid[20]        = {0}; 
    char          originator[12]   = {0};
    char          acctorg[12]      = {0};
    char          acctoper[12]     = {0};
    char          trncode[6]       = {0};
    char          clearstate[2]       = {0};

    sprintf( where, " where nodeid=%d and originator='%s' and workdate='%s' and refid='%s' and inoutflag='2'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opOriginator"),
            XMLGetNodeVal(opDoc, "//opWorkdate"),XMLGetNodeVal(opDoc, "//opRefid")
           );
    memset( sql, 0, sizeof(sql) );
    sprintf( sql, "select clearstate from trnjour %s", where );
    ret =  db_query_str(clearstate, sizeof(clearstate), sql );
    if (ret != 0)
    {
        if( ret == E_DB_NORECORD )
            return E_APP_CZSUCC;
        else
            return E_DB;
    }
    else
    {
        if( atoi(clearstate) != 1 )
            return E_APP_CZSUCC;
    }

    memset( sql, 0, sizeof(sql) );
    sprintf( sql, "select  acctorg, acctoper,trncode from acctjour where nodeid=%d and  workdate='%s' and refid='%s' and inoutflag='%s' and originator='%s'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opRefid"),
            "2",XMLGetNodeVal(opDoc, "//opOriginator"));

    ret = db_query_strs( sql, acctorg, acctoper,trncode);
    if (ret != 0)
    {
        if( ret == E_DB_NORECORD )
            return E_APP_CZSUCC;
        else
            return E_DB;
    }

    BKINFO("****提入冲正的交易:日期[%s], 流水号[%s], 行内机构[%s], 柜员[%s], 交易代码[%s]*****", 
            XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opRefid"), acctorg,acctoper,trncode );

    //提入行内机构
    XMLSetNodeVal( opDoc, "//opInnerBank", acctorg); 
    XMLSetNodeVal( opDoc, "//opInnerorganid", acctorg); 
    //记账柜员
    XMLSetNodeVal( opDoc, "//opPDWSNO", acctoper); 
    XMLSetNodeVal( opDoc, "//opOperid", acctoper); 


    XMLSetNodeVal(opDoc, "//opInoutflag", "2");
    if ((ret = AccountCancel(ACCOUNT_CREDITCZ, 2, doc)) == 0) 
    {
        //更新流水为失败
        db_exec( "update trnjour set clearstate='9' %s", where );
        return 0;
    }
    else 
        return E_APP_CZFAIL;

    return 0;
}

/*
 * 提出冲正
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF10_111(void *doc, char *p)
{
    int           ret=0;
    xmlDoc        *opDoc           = (xmlDoc *)doc;
    char          sql[512]         = {0};
    char          refid[20]        = {0}; 
    char          originator[12]   = {0};
    char          inoutflag[20]    = {0}; 
    char          trncode[6]       = {0};
    char          termserial[20]   = {0};
    char          workdate[10]     = {0};
    char          hworkdate[10]    = {0};
    char          *pSerial         = NULL;
    char          *pBank           = NULL;

    /****************************************************************
      cop如果发起冲正的交易为集中业务发起的业务, 则机构设置为"8987", 
      add by lt@sunyard.com 2011-07-05
     ****************************************************/
    pSerial = XMLGetNodeVal(opDoc, "//opRefid");
    if( pSerial != NULL && !strncmp(pSerial, "99975", 5))
    {
        pBank = XMLGetNodeVal(opDoc, "//opInnerBank");
        if( strcmp(pBank, "8987") != 0 )
            XMLSetNodeVal( opDoc, "//opInnerBank", "8987"); 
    }
    //add end by lt@sunyard.com 2011-07-05
    //原交易工作日期
    strcpy( hworkdate, XMLGetNodeVal(opDoc, "//opJIOHRQ") );
    //系统工作日期
    strcpy( workdate, XMLGetNodeVal(opDoc, "//opWorkdate") );
    sprintf( sql, "select refid, originator, inoutflag, trncode from acctjour where workdate='%s' and acctserial='%s'",
            //XMLGetNodeVal(opDoc, "//opJIOHRQ"), XMLGetNodeVal(opDoc, "//opRefid") );
        hworkdate, XMLGetNodeVal(opDoc, "//opRefid") );

    ret = db_query_strs( sql, refid, originator, inoutflag, trncode, termserial );
    if (ret != 0)
        return E_DB;

    BKINFO("*****手工冲正的交易:WorkDate[%s], RefId[%s], Originator[%s], InOutFlag[%s], TrnCode[%s]*****", 
            workdate, refid, originator, inoutflag, trncode );

    XMLSetNodeVal( opDoc, "//opOriginator", originator ); 
    XMLSetNodeVal( opDoc, "//opInoutflag", inoutflag ); 
    XMLSetNodeVal( opDoc, "//opRefid", refid); 
    //原交易日期作为工作日期,工作日期保存到opJIOHRQ字段
    XMLSetNodeVal( opDoc, "//opWorkdate", hworkdate); 
    XMLSetNodeVal( opDoc, "//opJIOHRQ", workdate); 
    XMLSetNodeVal( opDoc, "//opCZFLAG", "1"); 

    if ((ret = AccountCancel(atoi(trncode), 1, doc)) == 0) //提出贷记冲正
        return E_APP_CZSUCC;
    else 
        return E_APP_CZFAIL;

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
    int         ret             = 0;
    int         notetype        = 0;
    char        *orgid          = NULL;
    char        sSqlStr[1024]   = {0}; 
    char        sAutoOrg[12+1]  = {0};
    char        sAutoOper[12+1] = {0};
    xmlDoc      *opDoc          = (xmlDoc *)doc;

    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    sprintf(sSqlStr,"select autoorg, autooper from bankinfo where nodeid=%d and exchno='%s'", 
            OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
    ret = db_query_strs(sSqlStr, sAutoOrg, sAutoOper);
    if (ret != 0)
        return E_DB;

    XMLSetNodeVal(opDoc, "//opInnerBank", sAutoOrg);
    XMLSetNodeVal(opDoc, "//opOperid", sAutoOper);

    if( notetype == 31 ) 
        ret = CheckNoteInfo(ACCOUNT_CHECK_ZFMM, opDoc); //校验支付密码
    else if( notetype == 32 )
        ret = CheckNoteInfo(ACCOUNT_CHECK_BPMY, opDoc); //校验本票密押

    /*
       XMLSetNodeVal(opDoc, "//opBKRetcode", "0000");
       XMLSetNodeVal(opDoc, "//opBKRetinfo", "交易成功");
     */

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
int PF10_405(void *doc, char *p)
{
    int         ret                  = 0;
    int         classid              = 0;
    xmlDoc      *opDoc               = (xmlDoc *)doc;
    char        sAutoOrg[12+1]       = {0};
    char        sAutoOper[12+1]      = {0};
    char        sName1[81]           = {0}; 
    char        sName2[81]           = {0};
    char        sSqlStr[1024]        = {0};
    char        *q                   = NULL;

    if ((q = XMLGetNodeVal(opDoc, "//opAcctName")) != NULL)
        strcpy(sName1, q);


    if ((q = XMLGetNodeVal(opDoc, "//opClassid")) != NULL)
        classid  = atoi(q);
    if( classid == 1 )
    {
        sprintf(sSqlStr,"select autoorg, autooper from bankinfo where nodeid=%d and exchno='%s'", 
                OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
        ret = db_query_strs(sSqlStr, sAutoOrg, sAutoOper);
        if (ret != 0)
            return E_DB;
        XMLSetNodeVal(opDoc, "//opInnerBank", sAutoOrg);
        XMLSetNodeVal(opDoc, "//opOperid", sAutoOper);
        ret = callInterface(ACCOUNT_ACCTINFO_PUB1, opDoc);
    }
    if( classid == 2 )
    {
        XMLSetNodeVal(doc, "//opInnerBank", GetDefOrg()); //内部机构
        XMLSetNodeVal(doc, "//opOperid", GetDefOper()); //柜员
        ret = callInterface(ACCOUNT_ACCTINFO_PERSON, opDoc);
    }

    if(ret)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "户名不符");
        return ret;
    }

    q = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", q);
    if (strlen(q) && !sdpStringIsAllChar(q, '0'))
    {
        return 0;
    }
    if ((q = XMLGetNodeVal(opDoc, "//opCardName")) != NULL)
        strcpy(sName2, q);
    else
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "户名不符");
        return 0;
    }

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
    result_set          rs;
    xmlDoc              *opDoc           = (xmlDoc *)doc;
    char                *workdate        = NULL; 
    char                *bankid          = NULL;
    int                 workround, i, rc = 0;
    char                settledate[20]   = {0}; 
    char                condi[256]       = {0};
    int                 trtype_list[]    = { 1101, 1102, 1103, 1104 };

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
int PF10_904(void *doc, char *p)
{
    result_set            rs;
    xmlDoc                *opDoc            = (xmlDoc *)doc;
    char                  *workdate         = NULL; 
    char                  *bankid           = NULL;
    char                  path[100]         = {0};
    char                  hostserial[256]   = {0};
    int                   workround, i, ret = 0;
    char                  *pp               = NULL;

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
            pp = XMLGetNodeVal(doc, "//opBKRetcode");
            if (strlen(pp) && !sdpStringIsAllChar(pp, '0'))
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
/*
 * 集中业务下载登记簿交易
 * 输入 doc 平台报文
 */
int PF10_654(void *opdoc, char *p)
{
    result_set          rs;
    result_set          rstmp;
    char                tblName[16]             = "trnjour";
    xmlDoc              *doc                    = (xmlDoc *)opdoc;
    //char workdate[9]={0};
    char                *workdate               = NULL;
    FILE                *fpout                  = NULL;
    char                path[100]               = {0};
    char                outline[8192]           = {0};
    char                caOutFile[256]          = {0};
    char                bankname[80]            = {0};
    int                 i, j, ret               = 0;
    char                dcflag                  = 0;
    char                exchground              = 0;
    char                filename[128]           = {0};
    char                TmpCmd[128]             = {0};

    BKINFO("集中业务登记薄下载...");
    //caOutFile 返回文件路径(文件名是随机的: tmpXXXXXX)
    if (getFilesdirFile(caOutFile) == NULL)
    {
        BKINFO("生成临时文件失败");
        return -1;
    }
    fpout = fopen(caOutFile, "w");

    if (strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");

    workdate = XMLGetNodeVal(doc, "//opWorkdate");
    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0];
    exchground = XMLGetNodeVal(doc, "//opExchground")[0];


    ret = db_query(&rs, "select workdate, exchground, refid, originator, "
            "originator,acceptor, notetype, noteno, dcflag, issuedate, "
            "payingacct, payer, payingbank, beneacct, benename, benebank, "
            "benebank, curcode, settlamt, agreement, purpose  from %s "
            "where workdate='%s' and dcflag='%c' and exchground like '%%%c%%' and inoutflag='2'"
            " and trncode != '7' and trncode!= '0007' and truncflag='0' ", 
            tblName, workdate, dcflag, exchground);
    if (ret != 0)
        return ret;

    for (i = 0; i < db_row_count(&rs); i++)
    {

        ret = db_query(&rstmp,"select result from acctjour where nodeid=10 and inoutflag='2'"
                " and originator='%s' and workdate='%s' and refid='%s'",
                db_cell_by_name(&rs,i,"originator"), db_cell_by_name(&rs,i,"workdate"), db_cell_by_name(&rs,i,"refid"));
        if(ret!=0 && ret!=E_DB_NORECORD)
        {
            db_free_result(&rs);
            return -1;
        }
        if(ret!=E_DB_NORECORD && *db_cell(&rstmp,0,0)!='2' && *db_cell(&rstmp,0,0)!='9')
        {
            db_free_result(&rstmp);
            continue;
        }
        db_free_result(&rstmp);
        for(j=0; j < db_col_count(&rs); j++)
        {
            if (j == 17)
            {
                db_query_str(bankname, sizeof(bankname), "select bankname from bankinfo "
                        "where nodeid=%d and exchno='%s'",OP_REGIONID, db_cell_by_name(&rs, i, "benebank"));
                fprintf(fpout, "%s|%s|", bankname,db_cell(&rs, i, j));
            }
            else if( j == 18 )
            {
                fprintf(fpout, "%.0lf|", atof(db_cell(&rs, i, j))*100);
            }
            else if ( j == db_col_count(&rs)-1)
                fprintf(fpout, "%s", db_cell(&rs, i, j));
            else
                fprintf(fpout, "%s|", db_cell(&rs, i, j));
        }
        fprintf(fpout, "\n");
    }
    fclose(fpout);

    sprintf( filename, "file/%s", basename(caOutFile) );
    BKINFO("集中业务登记薄下载文件[%s]...", filename);
    XMLSetNodeVal(doc, "//opReserved", filename);
    sprintf( TmpCmd, "tftclient -dup -h3 -r%s %s -tFHIPP", filename, filename );
    system( TmpCmd );

    return 0;
}
/*校验支付密码*/
int CheckNoteInfo(int bkcode, xmlDoc *doc)
{
    int             ret         = 0;
    char            *p          = NULL;
    char            noteno1[10], noteno2[10]; 

    p= sdpXmlSelectNodeText(doc, "//opNoteno");
    if (p!= NULL && strlen(p) > 8 ) {
        BKINFO("凭证号太长,截断[%s->%s]", p, p+strlen(p)-8);
        XMLSetNodeVal(doc, "//opNoteno", p+strlen(p)-8); //行内凭证号最大8位(截最后8位)
    }
    /*
       if(strlen(XMLGetNodeVal(doc, "//opNoteno")) > 8)
       {
       sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+8);
       }
       XMLSetNodeVal(doc, "//opNoteno", noteno2);
     */

    if (ret = callInterface(bkcode, doc))
    {
        BKINFO("校验密码失败");
        XMLSetNodeVal(doc, "//opBKRetinfo", "支付密码错误");
        return E_MNG_PAYPWD;
    }

    if( bkcode == ACCOUNT_CHECK_ZFMM )
    {
        /*opYYCWDM:从第4位开始,不为"0010"则为失败*/
        p = XMLGetNodeVal(doc, "//opYYCWDM");
        if( p )
        {
            if( strncmp( p+3, "0010", 4 ) )
            {
                BKINFO("校验支付密码错误opYYCWDM[%s]", p);
                XMLSetNodeVal(doc, "//opBKRetcode", "8999");
                if ( strlen(XMLGetNodeVal(doc, "//opOreserved2")) )
                    XMLSetNodeVal(doc, "//opBKRetinfo", XMLGetNodeVal(doc, "//opOreserved2"));
                else
                    XMLSetNodeVal(doc, "//opBKRetinfo", "支付密码错误");
                return E_MNG_PAYPWD;
            }
        }
        else
            return E_MNG_PAYPWD;
        BKINFO("校验支付密码校验成功opYYCWDM[%s]", p);
    }
    else if( bkcode == ACCOUNT_CHECK_BPMY )
    {
        p = XMLGetNodeVal(doc, "//opBKRetcode");
        BKINFO("opBKRetcode=[%s]", p);
        if( p )
        {
            if (strlen(p) && strcmp(p, "SUCCESS"))
            {
                XMLSetNodeVal(doc, "//opBKRetcode", "");
                XMLSetNodeVal(doc, "//opBKRetinfo", "支付密码错误");
                return E_MNG_PAYPWD;
            }
            BKINFO("校验本票密押成功!");
        }
        else
            return E_MNG_PAYPWD;
    }

    return ret;
}
