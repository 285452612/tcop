#include "comm.h"
#include "interface.h"
#include "chinese.h"
#include "udb.h"
#include "interface.h"
#include "errcode.h"
#include "Public.h"

#define DEBUG_PRINT 
#define PS_MAXROWS 4          // 附言最大行数
#define PS_MAXCOLS 90        // 附言最大列数

extern char gs_originator[13];
extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern char *FormatMoney(char *str);
#define GetTrnCtl(a) XmlGetStringDup(xmlReq, "/UFTP/TrnCtl/"a)


static char amttype[256];    //资金性质 define by luokf 2010-07-23 in suzhou
static char interbank[81];   //中间行   define by luokf 2010-07-23 in suzhou
static char oppbankname[81]; //汇入行名称 define by luokf 2010-07-23 in suzhou
static char oppcustaddr[81]; //收款夫地址 define by luokf 2010-07-23 in suzhou
static char printinfo1[1024]; //税票打印信息1
static char printinfo2[1024]; //税票打印信息2

static int CharNums(char *data, int c)
{
    int i, t = 0;
    if (data == NULL)
        return t;
    for (i = 0; i < strlen(data); i++)
        if (*(data+i) == c)
            t++;
    return t;
}
//附加域
static int GetExtraData(char *data, char (*ps)[256])
{
    xmlDocPtr doc;
    char *xmlBuf = NULL;
    char path[100];
    char buf[2048];
    char field[1024];
    int  len = 0;
    int  i, j, n, pos;
    char FieldList[][30] = {
        "opOppBank", 
        //"opOppBankName", 
        "opOppBankAddr", 
        "opOppAcctId", 
        "opOppCustomer",
        //"OpOppCustAddr",
        "opOriginAcct",
        "opOriginCustName",
        //"opInterBank",
        "opChargesBearer",
        "opSenderProxy",
        "opRecverProxy",
        "opPrintFlag",
        "opTrnDetail",
        "opPS",
        "opSenderPS",
        "opValueDate",
        "opExchRate",
        "opTrackInfo"
    };

    memset(amttype, 0, sizeof(amttype));
    memset(interbank, 0, sizeof(interbank));
    memset(oppbankname, 0, sizeof(oppbankname));
    memset(oppcustaddr, 0, sizeof(oppcustaddr));
    memset(printinfo1, 0, sizeof(printinfo1));
    memset(printinfo2, 0, sizeof(printinfo2));

    xmlBuf = (char *)encoding_conv(data, "GB18030", "UTF-8");
    if (xmlBuf == NULL)
        return 0;
    doc = xmlParseMemory(xmlBuf, strlen(xmlBuf));
    if (doc == NULL)
        return 0;

    //从附加信息中取"资金性质"
    XmlGetString(doc, "//opAmtType", amttype, sizeof(amttype));
    //从附加信息中取"中间行"
    XmlGetString(doc, "//opInterBank", interbank, sizeof(interbank));
    //从附加信息中取"汇入行名称"
    XmlGetString(doc, "//opOppBankName", oppbankname, sizeof(oppbankname));
    //从附加信息中取"付款地址"
    XmlGetString(doc, "//opOppCustAddr", oppcustaddr, sizeof(oppcustaddr));
    // 打印信息
    XmlGetString(doc, "//opPrintInfo1", printinfo1, sizeof(printinfo1));
    XmlGetString(doc, "//opPrintInfo2", printinfo2, sizeof(printinfo2));

    // 小于10行补足换行
    if ((n = CharNums(printinfo1, '\n')) < 10)
    {
        n = 10 - n;
        while (n--)
            strcat(printinfo1, "                                                                                                    \n");
    }
    if (*printinfo2 != 0x00)
    {
        // 小于10行补足换行
        if ((n = CharNums(printinfo2, '\n')) < 10)
        {
            n = 10 - n;
            while (n--)
                strcat(printinfo2, "                                                                                                    \n");
        }
    }

    memset(buf, 0, sizeof(buf));
    for (i = 0; i < sizeof(FieldList)/sizeof(FieldList[0]); i++)
    {
        sprintf(path, "//%s", FieldList[i]);
        XMLGetVal(doc, path, field);
        if (field[0] == 0x00)
            continue;
        strcat(buf, field);
        strcat(buf, ",");
    }
    xmlFreeDoc(doc);
    free(xmlBuf);

    len = strlen(buf);
    pos = 0;
    j = 0;
    while (len > 0)
    {
        if (j > PS_MAXROWS)
            break;

        if (CheckGB18030(buf + pos, PS_MAXCOLS) != 0)
        {
            memcpy(*(ps+(j++)), buf + pos, PS_MAXCOLS-1);
            pos += PS_MAXCOLS - 1;
            len -= PS_MAXCOLS - 1;
        }
        else
        {
            memcpy(*(ps+(j++)), buf + pos, PS_MAXCOLS);
            pos += PS_MAXCOLS;
            len -= PS_MAXCOLS;
        }
    }

    return 0;
}

int main( int argc, char *argv[])
{
    char tbname[128];
    char tbname_ex[128];
    char condi[4096];
    char condi_acct[4096];
    char tmp[256];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char pbank[81];
    char bbank[81];
    long startrefid;
    long endrefid;
    double totamt = (double)0;
    char refid[17];
    char acctserial[17], result[2];
    char amount[128], littl_amt[30];
    char buf[256];
    char datetime[30];
    char notetype_name[61];
    char ps[PS_MAXROWS][256];
    char key[200];
    char *p;
    char content[8192], out[8192];
    result_set rs, ex;
    FILE *fp=NULL;
    int rc = 0;
    int iErrCode;
    int n, i, j=0, k, len;
    int iTranFlag = -1, iPrintFlag = -1;
    char *xmlBuf = NULL;
    xmlDocPtr doc;
    char sWorkDate[8+1]={0};

    memset( sWorkDate, 0, sizeof(sWorkDate) );
    db_query_str( sWorkDate, sizeof(sWorkDate), "select paraval from syspara "
            " where nodeid=10  and paraname='WORKDATE' ");
    memset(condi, 0, sizeof(condi));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND workdate = '%s' " 
            " AND dcflag='2' AND notetype != '41'"
            " AND truncflag='1'  AND result in(%d, %d, %d) ",
            RECONREC_ACPT, sWorkDate, E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);

    rc = db_query(&rs, "SELECT * FROM trnjour  WHERE %s "
            "ORDER BY classid, curcode, prestime", condi);

    if ( rc != 0 )
    {
        err_log("查询提入补充凭证失败, condi=[%s][%s]", "trnjour", condi );
        return rc;
    }

    //取当前时间
    GetDateTime(datetime);
    //打印数据文件(新建other_file目录)
    sprintf(caDataFile, "%s/other_file/Other_Print_%s.out", 
            getenv("HOME"), sWorkDate);

    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    for (i = 0, j = 0; i < db_row_count(&rs); i++)
    {

        // 密押字段
        p = db_cell_by_name(&rs, i, "agreement");
        if (*p != 0)
            sprintf(key, "支付密码:%s", p);
        else
            sprintf(key, "银行密押:%s", db_cell_by_name(&rs, i, "testkey"));

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "result"))) != 0)
        {
            strcat(key, "  ");
            //strcat(key, errmsg(atoi(db_cell_by_name(&rs, i, "result"))));
        }

        // 大写金额
        totamt += atof(db_cell_by_name(&rs, i, "settlamt"));
        MoneyToChinese(db_cell_by_name(&rs, i, "settlamt"), amount);
        // 附加信息
        memset(ps, 0, sizeof(ps));
        sprintf(content, "<?xml version=\"1.0\" encoding=\"GB18030\"?>%s",
                db_cell_by_name(&rs, i, "extraxml"));
        GetExtraData(content, ps);
        // 取凭证名称
        db_query_str(notetype_name, sizeof(notetype_name),
                "select distinct name from noteinfo "
                "where nodeid=10 and notetype='%s'", 
                 db_cell_by_name(&rs, i, "notetype"));
        // 取行内记账流水
        sprintf(refid, "%s", db_cell_by_name(&rs, i, "refid"));
        sdpStringTrimHeadChar(refid, '0');
        memset( acctserial, 0, sizeof(acctserial) );
        memset( result, 0, sizeof(result) );
        memset(condi_acct, 0, sizeof(condi_acct));
        snprintf(condi_acct, sizeof(condi_acct), 
                "select acctserial, result from acctjour where inoutflag='%c' AND workdate = '%s' AND originator='%s'"
                " AND convert(decimal, refid)=%s AND nodeid=10 ",
                RECONREC_ACPT, db_cell_by_name(&rs, i, "workdate"), db_cell_by_name(&rs, i, "originator"),
                refid );
        db_query_strs( condi_acct, acctserial, result );

        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));

        memset(ps, 0, sizeof(ps));
#if 1 
        memset(amttype, 0, sizeof(amttype));
        memset(interbank, 0, sizeof(interbank));
        memset(oppbankname, 0, sizeof(oppbankname));
        memset(oppcustaddr, 0, sizeof(oppcustaddr));
#endif

        if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
        {
            rc = db_query(&ex, "select extradata from extradata "
                    "where workdate='%s' and seqno=%s", 
                    db_cell_by_name(&rs, i, "workdate"),
                    db_cell_by_name(&rs, i, "seqno"));
            if ( rc == 0)
            {
                xmlBuf = (char *)encoding_conv(db_cell(&ex, 0, 0), "GB18030", "UTF-8");
                if (xmlBuf == NULL)
                {
                    err_log("encoding_conv error!");
                    db_free_result(&ex);
                    goto REPORT;
                }
                doc = xmlParseMemory(xmlBuf, strlen(xmlBuf));
                if (doc == NULL)
                {
                    err_log("xmlParseMemory error!");
                    free(xmlBuf);
                    db_free_result(&ex);
                    goto REPORT;
                }

                //从附加信息中取"资金性质"
                XmlGetString(doc, "//AmtType", amttype, sizeof(amttype));
                //从附加信息中取"中间行"
                XmlGetString(doc, "//InterBank", interbank, sizeof(interbank));
                //从附加信息中取"汇入行名称"
                XmlGetString(doc, "//OppBankName", oppbankname, sizeof(oppbankname));
                //从附加信息中取"付款夫地址"
                XmlGetString(doc, "//OppCustAddr", oppcustaddr, sizeof(oppcustaddr));

                xmlFreeDoc(doc);
                free(xmlBuf);
                db_free_result(&ex);
            }
        }
REPORT:
        fprintf(fp, "%s %s %s %s %s %s%s %s %s %s %s %s %s %s %s "
                "%s %s %s %s %s %s %s %s %s %s %s %s %s %d %s %s %s %s %s %s\n",
                db_cell_by_name(&rs, i, "originator"),
                //db_cell_by_name(&rs, i, "termid"),
                db_cell_by_name(&rs, i, "acceptor"),
                db_cell_by_name(&rs, i, "workdate"),
                db_cell_by_name(&rs, i, "workround"),
                db_cell_by_name(&rs, i, "refid"),
                notetype_name,
                (atoi(db_cell_by_name(&rs, i, "trncode")) == 7 ? "退票" : ""),
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "issuedate"),
                db_cell_by_name(&rs, i, "payer"),
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "payingbank"),
                org_name(db_cell_by_name(&rs, i, "payingbank"), pbank),
                db_cell_by_name(&rs, i, "benename"),
                db_cell_by_name(&rs, i, "beneacct"),
                db_cell_by_name(&rs, i, "benebank"),
                org_name(db_cell_by_name(&rs, i, "benebank"), bbank),
                ChsName(curcode_list, db_cell_by_name(&rs, i, "curcode")),
                amount, 
                db_cell_by_name(&rs, i, "curcode"),
                littl_amt,
                db_cell_by_name(&rs, i, "purpose"),
                key,
                ps[0], ps[1], ps[2], ps[3],
                datetime,
                atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                gs_oper,
                acctserial,
                amttype,
                interbank,
                oppbankname,
                oppcustaddr );
    }

    db_free_result(&rs);
    fclose(fp);

    return 0;
}
