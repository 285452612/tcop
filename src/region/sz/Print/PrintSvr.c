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

#define GetTrnCtl(a) XmlGetStringDup(xmlReq, "/UFTP/TrnCtl/"a)
#define MAX_SEQUENCE_ID 999999999
#define TMP_T_RS "#tmp_t_rs"
#define HDDB_EXTRADATA          "hddb..extradata"

extern char gs_originator[13];
extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq);
extern char *GetTmpFileName(char *);
extern char *FormatMoney(char *str);

extern int ChkPresAgent(char *orgid, char *presproxy);
extern int ChkAcptAgent(char *orgid, char *acptproxy);

static char amttype[256];    //资金性质 define by luokf 2010-07-23 in suzhou
static char interbank[81];   //中间行   define by luokf 2010-07-23 in suzhou
static char oppbankname[81]; //汇入行名称 define by luokf 2010-07-23 in suzhou
static char oppcustaddr[81]; //收款夫地址 define by luokf 2010-07-23 in suzhou
static char printinfo1[1024]; //税票打印信息1
static char printinfo2[1024]; //税票打印信息2

int GetExchgRoute(char *areacode, char *bankid)
{
    char buf[10];

    /*
    if (db_query_str(buf, sizeof(buf), "select routeid from exchroute "
                "where bankid='%s' and areacode='%s'", bankid, areacode) != 0)
        return 0;
    else
        return atoi(buf);
    */
    return 0;
}

int GetExtraData(char *data, char (*ps)[256])
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


//判断提出提入交换行是否在同一个区域
//如果提出机构所在交换区域与要打印的区域相同,返回'0',不同则返回'1'
int ChkExchgArea(char *bankid, char *exchgarea )
{
    char region[7];

    memset(region, 0, sizeof(region));

    //取交易提出行所在的交换区域
    db_query_str(region, sizeof(region), "select region from organinfo "
            "where nodeid=%d and orgid='%s' and orglevel='2' ", 
            OP_REGIONID, bankid);
    if (region[0] == 0x00)
        return 0;
    if ( strcmp(region, exchgarea) )
        return 1;
    else
        return 0;   
}

int CharNums(char *data, int c)
{
    int i, t = 0;
    if (data == NULL)
        return t;
    for (i = 0; i < strlen(data); i++)
        if (*(data+i) == c)
            t++;
    return t;
}
// 提入补充凭证打印
int PrintInNoteAdd( xmlDocPtr xmlReq, char *filename )
{
    char tbname[128];
    char tbname_ex[128];
    char condi[4096];
    char tmp[256];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char pbank[81];
    char bbank[81];
    //long startrefid;
    //long endrefid;
    char startrefid[17];
    char endrefid[17];
    double totamt = (double)0;
    char refid[17];
    char acctserial[17];
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
    int n, i, j, k, len;
    char *xmlBuf = NULL;
    xmlDocPtr doc;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    strcpy(startrefid, GetTrnCtl("StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0");
    strcpy(endrefid, GetTrnCtl("EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "9999999999");
    /*
       if (*GetTrnCtl("StartRefId") == 0x00)
       startrefid = 0L;
       else
       startrefid = atol(GetTrnCtl("StartRefId"));
       if (*GetTrnCtl("EndRefId") == 0x00)
       endrefid = MAX_SEQUENCE_ID;
       else
       endrefid = atol(GetTrnCtl("EndRefId"));
     */

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        /*
           if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
           {
           if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
           {
           return E_OTHER;
           }
           }
         */
    }

    //取打印的业务类型,如果是外币业务则取与本币不同的打印格式文件
    if (XmlGetInteger(xmlReq, "/UFTP/TrnCtl/SvcClass" ) == CLASS_FOREIGN)
        snprintf( caParaFile, sizeof(caParaFile),
                "%s/dat/%d/InNoteAddNewFor.para", getenv("HOME"), TCOP_BANKID );
    else
    {
        //if (atoi(db_cell_by_name(&rs, i, "notetype")) == 41)
        snprintf( caParaFile, sizeof(caParaFile),
                "%s/dat/%d/InNoteAddNew.para", getenv("HOME"), TCOP_BANKID );
    }

    memset(tmp, 0, sizeof(tmp));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(tmp, "and printnum=0");
            break;
        case '1':
            strcpy(tmp, "and printnum>0");
            break;
        case '2':
        default:
            break;
    }

    memset(amount, 0, sizeof(amount));
    if (*GetTrnCtl("MaxAmount") != 0x00)
        sprintf(amount, "AND settlamt = %s", GetTrnCtl("MaxAmount"));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND workdate = '%s' AND classid = %d"
            "  AND convert(decimal,refid) BETWEEN %s AND %s "
            "  AND %s "
            "  AND notetype LIKE '%s%%'  AND notetype != '41' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            "  AND result in(%d, %d, %d)",
            RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("SvcClass")),
            startrefid, endrefid, buf,
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount, GetTrnCtl("DCFlag"), tmp,
            E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
    rc = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, prestime", tbname, condi);
    if ( rc != 0 )
    {
        err_log("查询提入补充凭证失败, condi=[%s][%s]", tbname, condi );
        return rc;
    }

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    fclose(fp);

    GetDateTime(datetime);

    //GetTmpFileName(caDataFile);
    if ((fp = fopen((char *)getFilesdirFile(caDataFile), "w")) == NULL)
        //if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    WriteRptHeader(fp, "");

    for (i = 0, j = 0; i < db_row_count(&rs); i++)
    {
        /*
           if ((fp = fopen((char *)getFilesdirFile(caDataFile), "w")) == NULL)
        //if ((fp = fopen(caDataFile, "w")) == NULL)
        {
        db_free_result(&rs);
        return E_OTHER;
        }
        WriteRptHeader(fp, "");
         */

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
                "where nodeid=%d and notetype='%s'", 
                OP_REGIONID, db_cell_by_name(&rs, i, "notetype"));
        // 取行内记账流水
        sprintf(refid, "%s", db_cell_by_name(&rs, i, "refid"));
        sdpStringTrimHeadChar(refid, '0');
        db_query_str(acctserial, sizeof(acctserial),
                "select acctserial from acctjour "
                "where nodeid=%d and workdate='%s' and originator='%s' "
                "and convert(decimal, refid)=%s and inoutflag='2'",
                OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                db_cell_by_name(&rs, i, "originator"), refid);

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
            rc = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    db_cell_by_name(&rs, i, "workdate"),
                    db_cell_by_name(&rs, i, "seqno"));
            if ( rc == 0)
            {
                //GetExtraData(db_cell(&ex, 0, 0), ps);
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
            else
            {
                SetError(0);
                rc = db_query(&ex, "select extradata from %s "
                        "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
                        db_cell_by_name(&rs, i, "workdate"),
                        db_cell_by_name(&rs,i,"seqno"));
                if (rc == 0)
                {
                    //GetExtraData(db_cell(&ex, 0, 0), ps);
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
                else
                    SetError(0);
            }
        }
REPORT:
        fprintf(fp, "%s;%s;%s;第%s场;%s;%s%s;%s;%s;%s;%s;%s %s;%s;%s;"
                "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%s;%s;%s;%s;\n",
                //"%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;\n",
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
                oppcustaddr);
                //printinfo1,
                //printinfo2);

        /*
           WriteRptRowCount(fp, 1);
           WriteRptFooter(fp, "");
           fclose(fp);

           rc = PrintReportList(caParaFile, caDataFile, caOutFile);
           if (rc != 0)
           {
           err_log("打印提入补充凭证失败[%d]", rc );
           return E_OTHER;
           }

           if ((fp = fopen(caOutFile, "a")) == NULL)
           {
           db_free_result(&rs);
           return E_SYS_CALL;
           }
           if (++j % 2 == 0)
           {
        //fprintf(fp, "\n\f");
        if( i / 10 != 0 )
        fprintf(fp, "\n\n\n\n\n\n\n");
        else
        fprintf(fp, "\n\n\n\n\n\n\n\n\n\n\n\n");
        }
        fclose(fp);
         */
    }

    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        err_log("打印提入补充凭证失败[%d]", rc );
        return E_OTHER;
    }

    if (i > 0)
        db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);

    /*sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
      XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);*/
    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}

// 提入补充凭证打印(cop平台发起)
int PrintInNoteAdd_COP( xmlDocPtr xmlReq, char *filename )
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

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;


    /*
       p = GetTrnCtl("CNKNBZ"); 
       if( strlen(p) )
       iTranFlag = atoi(p);
     */

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");


    /*格式文件*/
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteAddNewCop.para", getenv("HOME"), TCOP_BANKID );

    memset(tmp, 0, sizeof(tmp));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(tmp, "and printnum=0");
            break;
        case '1':
            strcpy(tmp, "and printnum>0");
            break;
        default:
            break;
    }
    memset(amount, 0, sizeof(amount));
    if (*GetTrnCtl("MaxAmount") != 0x00)
        sprintf(amount, "AND settlamt = %s", GetTrnCtl("MaxAmount"));
    memset(condi, 0, sizeof(condi));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND workdate = '%s' AND classid = %d"
            "  %s AND acceptor='%s' AND dcflag='2' AND notetype != '41'"
            //"  %s AND acceptor='%s' AND truncflag='1'"
            "  AND result in(%d, %d, %d) ",
            RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("SvcClass")),
            tmp, GetTrnCtl("Acceptor"), E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
    /*
       snprintf(condi, sizeof(condi), 
       "inoutflag='%c' AND workdate = '%s' AND classid = %d"
       "  AND convert(decimal,refid) BETWEEN %ld AND %ld AND %s "
       "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
       "  AND beneacct LIKE '%s%%' %s"
       "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
       "  AND result in(%d, %d, %d)",
       RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("SvcClass")),
       startrefid, endrefid, buf,
       GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
       GetTrnCtl("BeneAcct"), amount, GetTrnCtl("DCFlag"), tmp,
       E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
     */
    rc = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, prestime", tbname, condi);
    if ( rc != 0 )
    {
        err_log("查询提入补充凭证失败, condi=[%s][%s]", tbname, condi );
        return rc;
    }

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    GetDateTime(datetime);
    //GetTmpFileName(caOutFile);
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    fclose(fp);
    //GetTmpFileName(caDataFile);
    if ((fp = fopen((char *)getFilesdirFile(caDataFile), "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    WriteRptHeader(fp, "");

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
                "where nodeid=%d and notetype='%s'", 
                OP_REGIONID, db_cell_by_name(&rs, i, "notetype"));
        // 取行内记账流水
        sprintf(refid, "%s", db_cell_by_name(&rs, i, "refid"));
        sdpStringTrimHeadChar(refid, '0');
        memset( acctserial, 0, sizeof(acctserial) );
        memset( result, 0, sizeof(result) );
        memset(condi_acct, 0, sizeof(condi_acct));
        snprintf(condi_acct, sizeof(condi_acct), 
                "select acctserial, result from acctjour where inoutflag='%c' AND workdate = '%s' AND originator='%s'"
                " AND convert(decimal, refid)=%s AND nodeid=%d ",
                RECONREC_ACPT, db_cell_by_name(&rs, i, "workdate"), db_cell_by_name(&rs, i, "originator"),
                refid, OP_REGIONID);
        db_query_strs( condi_acct, acctserial, result );
        /*
           db_query_str(acctserial, sizeof(acctserial),
           "select acctserial from acctjour "
           "where nodeid=%d and workdate='%s' and originator='%s' "
           "and convert(decimal, refid)=%s and inoutflag='2'",
           OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
           db_cell_by_name(&rs, i, "originator"), refid);
         */

        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));

#if 0
        if( iTranFlag == -1 )//全部
        {
            iPrintFlag = 1;
#ifdef DEBUG_PRINT
            BKINFO("打印全部...");
#endif
        }
        else if( iTranFlag ==0 )//自动入账失败的
        {
            if( atoi(result) == 5 || atoi(result) == 9)
            {
                iPrintFlag = 1;
#ifdef DEBUG_PRINT
                BKINFO("打印自动入账失败...");
#endif
            }
        }
        else if( iTranFlag == 1 )//自动入账成功的及已退票的
        {
            if( atoi(result) == 1 || atoi(db_cell_by_name(&rs, i, "tpflag")) == 1)
            {
                iPrintFlag = 1;
#ifdef DEBUG_PRINT
                BKINFO("打印自动入账成功和已退票的...");
#endif
            }
        }
#endif

        memset(ps, 0, sizeof(ps));
#if 1 
        memset(amttype, 0, sizeof(amttype));
        memset(interbank, 0, sizeof(interbank));
        memset(oppbankname, 0, sizeof(oppbankname));
        memset(oppcustaddr, 0, sizeof(oppcustaddr));
#endif

        if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
        {
            rc = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    db_cell_by_name(&rs, i, "workdate"),
                    db_cell_by_name(&rs, i, "seqno"));
            if ( rc == 0)
            {
                //GetExtraData(db_cell(&ex, 0, 0), ps);
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
            else
            {
                SetError(0);
                rc = db_query(&ex, "select extradata from %s "
                        "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
                        db_cell_by_name(&rs, i, "workdate"),
                        db_cell_by_name(&rs,i,"seqno"));
                if (rc == 0)
                {
                    //GetExtraData(db_cell(&ex, 0, 0), ps);
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
                else
                    SetError(0);
            }
        }
REPORT:
        //if( iPrintFlag == 1 )
        {
            //j++;    //打印行数
            fprintf(fp, "%s;%s;%s;第%s场;%s;%s%s;%s;%s;%s;%s;%s %s;%s;%s;"
                    "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%s;%s;%s;%s;\n",
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
            /*
               WriteRptRowCount(fp, 1);
               WriteRptFooter(fp, "");
               fclose(fp);

               rc = PrintReportList(caParaFile, caDataFile, caOutFile);
               if (rc != 0)
               {
               err_log("打印提入补充凭证失败[%d]", rc );
               return E_OTHER;
               }

               if ((fp = fopen(caOutFile, "a")) == NULL)
               {
               db_free_result(&rs);
               return E_SYS_CALL;
               }
               if (++j % 2 == 0)
               {
            //fprintf(fp, "\n\f");
            if( i / 10 != 0 )
            fprintf(fp, "\n\n\n\n\n\n\n");
            else
            fprintf(fp, "\n\n\n\n\n\n\n\n\n\n\n\n");
            }
            fclose(fp);
             */
        }
    }

    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    //WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        err_log("打印提入补充凭证失败[%d]", rc );
        return E_OTHER;
    }

    if (i > 0)
        db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);

    /*sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
      XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);*/
    sprintf( filename, "file/%s", basename(caOutFile) );
    XMLSetNodeVal( xmlReq, "//Reserve", filename);

    return rc;
}

int PrintInQuery( xmlDocPtr xmlReq, char *filename)
{
    xmlDocPtr doc = NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startrefid[17];
    char endrefid[17];
    char startdate[9];
    char enddate[9];
    char printdate[9];
    result_set rs;
    FILE *fp=NULL;
    char *Originator, *Acceptor;
    char originame[81];
    char acptname[81];
    char NoteTypeName[41];
    char buf[128];
    char condi[2048];
    char state;
    int rc = 0, updflag = 1;
    int i;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;
    sprintf(printdate, "%08ld", current_date());

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08s", GetWorkdate());
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    strcpy(startrefid, GetTrnCtl("StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0");
    strcpy(endrefid, GetTrnCtl("EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "999999999");

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/PrintQueryBook.para", getenv("HOME"), TCOP_BANKID );

    memset(buf, 0, sizeof(buf));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(buf, "and readflag='0'");
            updflag = 1;
            break;
        case '1':
            strcpy(buf, "and readflag='1'");
            break;
        case '2':
        default:
            updflag = 1;
            break;
    }
    // state=0-未回复, 1-已回复
    state = *GetTrnCtl("MailType"); 
    snprintf(condi, sizeof(condi),
            "acceptor='%s' AND workdate BETWEEN '%s' AND '%s' and state='%c' "
            "AND convert(decimal,refid) BETWEEN %ld AND %ld "
            "AND inoutflag='2' %s",
            gs_originator, startdate, enddate, state,
            atol(startrefid), atol(endrefid), buf);
    rc = db_query(&rs, "SELECT * FROM queryinfo "
            "WHERE %s order by workdate, refid", condi);
    if ( rc != 0 )
    {
        if (rc != E_DB_NORECORD)
            err_log("查询提入查询查复书失败");
        return rc;
    }

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s提入%s;%s;%s;%s;%s;", gs_sysname, 
                GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
                db_cell_by_name(&rs, i, "refid"),
                db_cell_by_name(&rs, i, "acceptor"),
                db_cell_by_name(&rs, i, "originator"),
                db_cell_by_name(&rs, i, "workdate"));

        Originator = db_cell_by_name(&rs, i, "ooriginator");
        Acceptor = db_cell_by_name(&rs, i, "oacceptor");

        db_query_str(NoteTypeName, sizeof(NoteTypeName), 
                "select distinct name from noteinfo where notetype='%s'", 
                db_cell_by_name(&rs, i, "notetype"));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;",
                db_cell_by_name(&rs, i, "otrandate"),
                db_cell_by_name(&rs, i, "orefid"),
                db_cell_by_name(&rs, i, "ooriginator"),
                org_name(Originator, originame),
                db_cell_by_name(&rs, i, "oacceptor"),
                org_name(Acceptor, acptname),
                NoteTypeName,
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "settlamt"),
                db_cell_by_name(&rs, i, "issuedate"),
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "payer"),
                db_cell_by_name(&rs, i, "beneacct"),
                db_cell_by_name(&rs, i, "benename"),
                db_cell_by_name(&rs, i, "paykey"),
                db_cell_by_name(&rs, i, "agreement"),
                db_cell_by_name(&rs, i, "content"),
                db_cell_by_name(&rs, i, "replycontent"),
                GetChineseName(dcflag_list, 
                    atoi(db_cell_by_name(&rs, i, "dcflag"))));

        fprintf(fp, "%s;%s;\n", printdate, gs_oper);
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    rc = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (rc != 0)
    {
        return E_OTHER;
    }

    if (updflag == 1)
        db_exec("update queryinfo set readflag='1' where %s", condi);

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}

int PrintOutQuery( xmlDocPtr xmlReq, char *filename )
{
    xmlDocPtr doc = NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startrefid[17];
    char endrefid[17];
    char startdate[9];
    char enddate[9];
    char printdate[9];
    result_set rs;
    FILE *fp=NULL;
    char *Originator;
    char *Acceptor;
    char originame[81];
    char acptname[81];
    char NoteTypeName[41];
    char condi[2048];
    char buf[128];
    int rc = 0, updflag = 1;
    int i;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;
    sprintf(printdate, "%08ld", current_date());

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08s", GetWorkdate());
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    strcpy(startrefid, GetTrnCtl("StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0");
    strcpy(endrefid, GetTrnCtl("EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "999999999");

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/PrintQueryBook.para", getenv("HOME"),TCOP_BANKID );

    /*
       memset(buf, 0, sizeof(buf));
       switch(*GetTrnCtl("PrintState"))
       {
       case '0':
       strcpy(buf, "and sended='1'");
       updflag = 1;
       break;
       case '1':
       strcpy(buf, "and sended='2'");
       break;
       case '2':
       default:
       updflag = 1;
       break;
       }
     */
    snprintf(condi, sizeof(condi),
            "originator='%s' AND workdate BETWEEN '%s' AND '%s' AND state='%s'"
            " AND convert(decimal,refid) BETWEEN %ld AND %ld AND inoutflag='1'",
            gs_originator,startdate, enddate, GetTrnCtl("MailType"), 
            atol(startrefid), atol(endrefid));
    rc = db_query(&rs, "SELECT * FROM queryinfo "
            "WHERE %s order by workdate, refid", condi);
    if ( rc != 0 )
    {
        err_log("查询提出查询查复书失败");
        return rc;
    }

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s提出%s;%s;%s;%s;%s;", gs_sysname, 
                GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
                db_cell_by_name(&rs, i, "refid"),
                db_cell_by_name(&rs, i, "acceptor"),
                db_cell_by_name(&rs, i, "originator"),
                db_cell_by_name(&rs, i, "workdate"));

        Originator = db_cell_by_name(&rs, i, "ooriginator");
        Acceptor = db_cell_by_name(&rs, i, "oacceptor");

        db_query_str(NoteTypeName, sizeof(NoteTypeName), 
                "select distinct name from noteinfo where notetype='%s'", 
                db_cell_by_name(&rs, i, "notetype"));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;",
                db_cell_by_name(&rs, i, "otrandate"),
                db_cell_by_name(&rs, i, "orefid"),
                db_cell_by_name(&rs, i, "ooriginator"),
                org_name(Originator, originame),
                db_cell_by_name(&rs, i, "oacceptor"),
                org_name(Acceptor, acptname),
                NoteTypeName,
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "settlamt"),
                db_cell_by_name(&rs, i, "issuedate"),
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "payer"),
                db_cell_by_name(&rs, i, "beneacct"),
                db_cell_by_name(&rs, i, "benename"),
                db_cell_by_name(&rs, i, "paykey"),
                db_cell_by_name(&rs, i, "agreement"),
                db_cell_by_name(&rs, i, "content"),
                db_cell_by_name(&rs, i, "replycontent"),
                GetChineseName(dcflag_list, 
                    atoi(db_cell_by_name(&rs, i, "dcflag"))));

        fprintf(fp, "%s;%s;\n", printdate, gs_oper);
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    rc = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (rc != 0)
    {
        return E_OTHER;
    }

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}

// 提出汇总单
int PrintOutNoteTotal( xmlDocPtr xmlReq, char *filename )
{
    result_set banklist, orglist, curlist;
    result_set rs;
    char tbname[80];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[256];
    char bankname[81];
    char truncname[10];
    char *chinesedate = NULL;
    char printtime[31];
    FILE *fp=NULL;
    double totDSumAmt, totCSumAmt;
    char sDSumAmt[30], sCSumAmt[30];
    char sDAmt[30], sCAmt[30];
    char workround_cond[128];
    char workround_name[128];
    int iDCount, iCCount;
    int itotDCount, itotCCount;
    int rc = 0;
    int i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    //取请求报文的工作场次信息,为空则打印该工作日的所有工作场次数据
    memset(workround_cond, 0, sizeof(workround_cond));
    memset(workround_name, 0, sizeof(workround_name));
    strcpy(workround_name, "所有");
    if (IsNonEmptyStr(GetTrnCtl("WorkRound")) && *GetTrnCtl("WorkRound") != '0') 
    {
        //workround = atoi(GetTrnCtl("WorkRound"));
        memset(workround_name, 0, sizeof(workround_name));
        strcpy(workround_name, GetTrnCtl("WorkRound"));
        sprintf(workround_cond, "and workround='%d'", atoi(GetTrnCtl("WorkRound")));
    }

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));
        /*
           if (strcmp(GetTrnCtl("Originator"), gs_originator))
           {
           if (ChkPresAgent(GetTrnCtl("Originator"), gs_originator))
           {
        //SetError(E_ORG_PERMIT);
        return E_OTHER;
        }
        }
         */
    }
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutNoteTotal.para", getenv("HOME"), TCOP_BANKID);

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.otot",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

#ifdef DB2
    rc = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    rc = db_exec("select * into " TMP_T_RS " from trnjour where 1=2");
#endif
    if (rc != 0)
        return rc;
    if ((rc = db_begin()) != 0)
        return rc;
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE inoutflag = '%c' AND workdate = '%s' %s"
            " AND classid = %d AND clearstate in ('1', 'C')"
            " AND %s AND truncflag LIKE '%s%%'", tbname, RECONREC_PRES, 
            GetTrnCtl("WorkDate"), workround_cond,
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("TruncFlag"));
    if (rc != 0)
        goto err_handle;

    memset(truncname, 0, sizeof(truncname));
    if (!IsNonEmptyStr(GetTrnCtl("TruncFlag")))
        strcpy(truncname, "截留、不截留");
    else
        strcpy(truncname, (*GetTrnCtl("TruncFlag")=='0' ? "不截留" : "截留"));

    if ((rc = db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS
                    " order by curcode")) != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if ((rc = db_query(&orglist, "SELECT DISTINCT originator FROM " TMP_T_RS 
                        " WHERE curcode='%s' order by originator", 
                        db_cell(&curlist, i, 0))) != 0)
        {
            db_free_result(&curlist);
            goto err_handle;
        }
        for (k = 0; k < db_row_count(&orglist); k++)
        {
            if ((rc = db_query(&banklist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                            " WHERE curcode='%s' and originator='%s' "
                            "order by acceptor", db_cell(&curlist, i, 0),
                            db_cell(&orglist, k, 0))) != 0)
            {
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }

            fp = fopen(caDataFile, "w");
            WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, 
                    workround_name, db_cell(&curlist,i,0), truncname);

            itotDCount = itotCCount = 0;
            totDSumAmt = totCSumAmt = (double)0;
            for (j = 0; j < db_row_count(&banklist); j++)
            {
                // 统计借方笔数及金额
                if ((rc = db_query(&rs, "SELECT count(*)," IS_NULL 
                                "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                                "WHERE curcode='%s' AND originator='%s' "
                                "AND acceptor='%s' AND dcflag='1'", 
                                db_cell(&curlist, i, 0),
                                db_cell(&orglist, k, 0),
                                db_cell(&banklist,j,0))) != 0)
                {
                    err_log("打印提出汇总单失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                iDCount = db_cell_i(&rs, 0, 0);
                itotDCount += iDCount;
                strcpy(sDAmt, db_cell(&rs, 0, 1));
                totDSumAmt += atof(sDAmt);
                db_free_result(&rs);

                // 统计贷方笔数及金额
                if ((rc = db_query(&rs, "SELECT count(*)," IS_NULL 
                                "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                                "WHERE curcode='%s' AND originator='%s' "
                                "AND acceptor='%s' AND dcflag='2'", 
                                db_cell(&curlist, i, 0),
                                db_cell(&orglist, k, 0),
                                db_cell(&banklist,j,0))) != 0)
                {
                    err_log("打印提出汇总单失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                iCCount = db_cell_i(&rs, 0, 0);
                itotCCount += iCCount;
                strcpy(sCAmt, db_cell(&rs, 0, 1));
                totCSumAmt += atof(sCAmt);
                db_free_result(&rs);

                fprintf(fp, "%s;%d;%s;%d;%s;\n", db_cell(&banklist, j, 0), 
                        iDCount, FormatMoney(sDAmt), 
                        iCCount, FormatMoney(sCAmt));
            }
            db_free_result(&banklist);
            WriteRptRowCount(fp, j);
            sprintf(sDSumAmt, "%.2lf", totDSumAmt);
            sprintf(sCSumAmt, "%.2lf", totCSumAmt);
            gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
            WriteRptFooter(fp,"%d;%s;%d;%s;%s;%s;%s;%s;",
                    itotDCount, FormatMoney(sDSumAmt),
                    itotCCount, FormatMoney(sCSumAmt), 
                    db_cell(&orglist, k, 0), 
                    org_name(db_cell(&orglist, k, 0), bankname),
                    printtime, gs_oper);
            fclose(fp);

            rc = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (rc != 0)
            {
                rc = ERR_APP;
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    db_commit();
    ifree(chinesedate);
    return rc;
}

int PrintInNoteTotal(xmlDocPtr xmlReq, char *filename )
{
    result_set banklist, orglist, curlist;
    result_set rs;
    char tbname[80];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[256];
    char bankname[81];
    char truncname[10];
    char *chinesedate = NULL;
    char printtime[31];
    FILE *fp=NULL;
    double totDSumAmt, totCSumAmt;
    char sDSumAmt[30], sCSumAmt[30];
    char sDAmt[30], sCAmt[30];
    char workround_cond[128];
    char workround_name[128];
    int iDCount, iCCount;
    int itotDCount, itotCCount;
    int rc = 0;
    int i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
    //取请求报文的工作场次信息,为空则打印该工作日的所有工作场次数据
    memset(workround_cond, 0, sizeof(workround_cond));
    memset(workround_name, 0, sizeof(workround_name));
    strcpy(workround_name, "所有");
    if (IsNonEmptyStr(GetTrnCtl("WorkRound")) && *GetTrnCtl("WorkRound") != '0') 
    {
        sprintf(workround_name, "%s", GetTrnCtl("WorkRound"));
        sprintf(workround_cond, "and workround='%d'", atoi(GetTrnCtl("WorkRound")));
    }

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        /*
           if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
           {
           if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
           {
        //SetError(E_ORG_PERMIT);
        return E_OTHER;
        }
        }
         */
    }

    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteTotal.para", getenv("HOME"), TCOP_BANKID);

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.itot",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

#ifdef DB2
    rc = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    rc = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (rc != 0)
        return rc;
    if ((rc = db_begin()) != 0)
        return rc;
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            //"WHERE inoutflag = '%c' AND workdate = '%s' AND workround='%d'"
            "WHERE inoutflag = '%c' AND workdate = '%s' %s"
            " AND classid = %d AND clearstate in"
            " ('1', 'C') AND %s AND truncflag LIKE '%s%%'", tbname,
            //RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("WorkRound")), 
            RECONREC_ACPT, GetTrnCtl("WorkDate"), workround_cond, 
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("TruncFlag"));
    if (rc != 0)
        return rc;

    memset(truncname, 0, sizeof(truncname));
    if (!IsNonEmptyStr(GetTrnCtl("TruncFlag")))
        strcpy(truncname, "截留、不截留");
    else
        strcpy(truncname, (*GetTrnCtl("TruncFlag")=='0' ? "不截留" : "截留"));

    if ((rc = db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS
                    " order by curcode")) != 0)
        return rc;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if ((rc = db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                        " WHERE curcode='%s' order by acceptor", 
                        db_cell(&curlist, i, 0))) != 0)
        {
            db_free_result(&curlist);
            return rc;
        }
        for (k = 0; k < db_row_count(&orglist); k++)
        {
            if ((rc = db_query(&banklist, "SELECT DISTINCT originator FROM %s"
                            " WHERE curcode='%s' and acceptor='%s' "
                            "order by originator", TMP_T_RS, 
                            db_cell(&curlist, i, 0), db_cell(&orglist, k, 0))) != 0)
            {
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }

            fp = fopen(caDataFile, "w");
            WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, 
                    GetTrnCtl("WorkRound"), db_cell(&curlist,i,0), truncname);

            itotDCount = itotCCount = 0;
            totDSumAmt = totCSumAmt = (double)0;
            for (j = 0; j < db_row_count(&banklist); j++)
            {
                // 统计借方笔数及金额
                if ((rc = db_query(&rs, "SELECT count(*)," IS_NULL 
                                "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                                "WHERE curcode='%s' AND acceptor='%s' "
                                "AND originator='%s' AND dcflag='1'", 
                                db_cell(&curlist, i, 0),
                                db_cell(&orglist, k, 0),
                                db_cell(&banklist,j,0))) != 0)
                {
                    err_log("打印提入汇总单失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                iDCount = db_cell_i(&rs, 0, 0);
                itotDCount += iDCount;
                strcpy(sDAmt, db_cell(&rs, 0, 1));
                totDSumAmt += atof(sDAmt);
                db_free_result(&rs);

                // 统计贷方笔数及金额
                if ((rc = db_query(&rs, "SELECT count(*)," IS_NULL 
                                "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                                "WHERE curcode='%s' AND acceptor='%s' "
                                "AND originator='%s' AND dcflag='2'", 
                                db_cell(&curlist, i, 0),
                                db_cell(&orglist, k, 0),
                                db_cell(&banklist,j,0))) != 0)
                {
                    err_log("打印提入汇总单失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                iCCount = db_cell_i(&rs, 0, 0);
                itotCCount += iCCount;
                strcpy(sCAmt, db_cell(&rs, 0, 1));
                totCSumAmt += atof(sCAmt);
                db_free_result(&rs);

                fprintf(fp, "%s;%d;%s;%d;%s;\n", db_cell(&banklist, j, 0), 
                        iDCount, FormatMoney(sDAmt), 
                        iCCount, FormatMoney(sCAmt));
            }
            db_free_result(&banklist);
            WriteRptRowCount(fp, j);
            sprintf(sDSumAmt, "%.2lf", totDSumAmt);
            sprintf(sCSumAmt, "%.2lf", totCSumAmt);
            gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
            WriteRptFooter(fp,"%d;%s;%d;%s;%s;%s;%s;%s;",
                    itotDCount, FormatMoney(sDSumAmt),
                    itotCCount, FormatMoney(sCSumAmt), 
                    db_cell(&orglist, k, 0), 
                    org_name(db_cell(&orglist, k, 0), bankname),
                    printtime, gs_oper);
            fclose(fp);

            rc = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (rc != 0)
            {
                rc = ERR_APP;
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    db_commit();
    ifree(chinesedate);
    return rc;
}

// 提出清单
int PrintOutNoteList( xmlDocPtr xmlReq, char *filename )
{
    result_set curlist, orglist, banklist;
    result_set rs;
    result_set result;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[128];
    char tbname[81];
    char bankname[81];
    char printtime[30];
    char bankcode[8];
    char notetype[8];
    char notetype_name[61];
    char workround_cond[128];
    char workround_name[128];
    //char PayingAcctOrName[81],BeneAcctOrName[81];
    char *chinesedate = NULL;
    char result1[128]={0};
    char result2[128]={0};
    char result3[256]={0};
    char trncode[128]={0};
    FILE *fp=NULL;
    int iSum, tot, alltot;
    double dSumAmt;
    int rc = 0;
    int h, i, j, k, l;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
    //取请求报文的工作场次信息,为空则打印该工作日的所有工作场次数据
    memset(workround_cond, 0, sizeof(workround_cond));
    memset(workround_name, 0, sizeof(workround_name));
    strcpy(workround_name, "所有");
    if (IsNonEmptyStr(GetTrnCtl("WorkRound")) && *GetTrnCtl("WorkRound") != '0') 
    {
        sprintf(workround_name, "%s", GetTrnCtl("WorkRound"));
        sprintf(workround_cond, " and workround='%d'", atoi(GetTrnCtl("WorkRound")));
    }

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel='2')", gs_originator);
    else
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutNoteList.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.olst",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

#ifdef DB2
    rc = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    rc = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (rc != 0)
        return rc;
    if ((rc = db_begin()) != 0)
        return rc;
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            //"where inoutflag = '%c' AND workdate='%s' AND workround='%d'"
            "where inoutflag = '%c' AND workdate='%s' %s"
            " AND classid = %d AND clearstate in ('1', 'C') AND %s"
            " AND dcflag like '%%%s%%' AND truncflag like '%%%s%%'", tbname,
            //RECONREC_PRES, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("WorkRound")),
            RECONREC_PRES, GetTrnCtl("WorkDate"), workround_cond,
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("DCFlag"),
            GetTrnCtl("TruncFlag"));
    if (rc != 0)
        return rc;

    if ((rc = db_query(&curlist, 
                    "SELECT DISTINCT curcode FROM " TMP_T_RS)) != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if ((rc = db_query(&orglist, "SELECT DISTINCT originator FROM "
                        TMP_T_RS " WHERE curcode='%s' order by originator",
                        db_cell(&curlist, i, 0))) != 0)
        {
            db_free_result(&curlist);
            return rc;
        }
        for (h = 0; h < db_row_count(&orglist); h++)
        {
            iSum = tot = 0;
            dSumAmt = (double)0;
            alltot = 0;
            fp = fopen(caDataFile, "w");

            //WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;%s;",
            WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;",
                    gs_sysname,
                    //GetChineseName(dcflag_list, atoi(GetTrnCtl("DCFlag"))),
                    *GetTrnCtl("DCFlag")?(atoi(GetTrnCtl("DCFlag")) == PART_DEBIT ? "借方" : "贷方"):"所有",
                    chinesedate,
                    GetTrnCtl("WorkRound"),
                    ChsName(curcode_list, db_cell(&curlist, i, 0)),
                    db_cell(&orglist, h, 0),
                    org_name(db_cell(&orglist, h, 0), bankname));

            if ((rc = db_query(&banklist, "SELECT distinct acceptor FROM " TMP_T_RS
                            " WHERE curcode='%s' and originator='%s' "
                            "ORDER BY acceptor", db_cell(&curlist, i, 0), 
                            db_cell(&orglist, h, 0))) != 0)
            {
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
            for (j = 0; j < db_row_count(&banklist); j++)
            {
                int ltot = 0;
                double ltotamt = (double)0;
                rc = db_query(&rs,
                        "SELECT notetype, noteno, payingacct, payer, beneacct, "
                        "benename, settlamt, truncflag, refid, workdate,reserved3,clearstate,trncode, dcflag "
                        "FROM " TMP_T_RS
                        " WHERE curcode='%s' and originator='%s' and "
                        "acceptor='%s' ORDER BY workdate,prestime", 
                        db_cell(&curlist,i,0), db_cell(&orglist, h, 0), 
                        db_cell(&banklist, j, 0));
                if ( rc != 0 )
                {
                    err_log("打印提出凭证清单失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                for (k = 0; k < db_row_count(&rs); k++)
                {
                    err_log("REFID[%s]", db_cell(&rs,k,8) );
                    strcpy(notetype,db_cell(&rs,k,0));
                    if(strcmp(notetype,"71")==0||strcmp(notetype,"73")==0)
                        strcpy(trncode,"个人通存");
                    else if(strcmp(notetype,"72")==0||strcmp(notetype,"74")==0)
                        strcpy(trncode,"个人通兑");
                    else
                    {
                        switch(atoi(db_cell(&rs,k,12)))
                        {
                            case  1:strcpy(trncode,"借记业务");break;
                            case  2:strcpy(trncode,"贷记业务");break;
                            case  7:strcpy(trncode,"退票交易");break;
                            case 13:strcpy(trncode,"个人通存");break;
                            case 14:strcpy(trncode,"个人通兑");break;
                            default:break;
                        }
                    }

                    if(*db_cell(&rs,k,10)=='1')//reserved3==1
                    {
                        strcpy(result1,"记账成功");
                    }
                    else
                    {
                        err_log("REFID[%s]", db_cell(&rs,k,8) );
                        rc = db_query(&result,"select result,trncode from acctjour where originator ='%s' and workdate='%s' and refid='%s' and inoutflag='1'",
                                db_cell(&orglist, h, 0), db_cell(&rs,k,9), db_cell(&rs,k,8));
                        if(rc!=0)
                        {
                            if(rc==E_DB_NORECORD)
                            {
                                strcpy(result1,"未记账");
                            }
                            else
                            {
                                err_log("打印提入凭证明细表失败");
                                db_free_result(&banklist);
                                db_free_result(&orglist);
                                db_free_result(&curlist);
                                db_free_result(&rs);
                                goto err_handle;
                            }
                        }
                        else
                        {
                            do
                            {
                                l=0;
                                strcpy(bankcode,db_cell(&result, 0, 1));
                                if(db_row_count(&result)>1)
                                {
                                    if(strcmp(notetype,"71")==0)
                                    {
                                        if(equal(bankcode, "8963"))
                                            l=0;
                                        else
                                            l=1;
                                    }
                                    else if(equal(notetype,"72"))
                                    {
                                        if(equal(bankcode, "4202"))
                                            l=0;
                                        else
                                            l=1;
                                    }
                                    else
                                    {
                                        if(equal(bankcode, "8956"))
                                            l=0;
                                        else
                                            l=1;
                                    }
                                }
                                else
                                {
                                    if(equal(bankcode, "9579"))
                                    {
                                        strcpy(result1,"记账失败");
                                        break;
                                    }
                                }
                                switch(*db_cell(&result, l, 0))
                                {
                                    case '0':strcpy(result1,"状态未知");break;
                                    case '1':strcpy(result1,"记账成功");break;
                                    case '2':strcpy(result1,"已冲正");break;
                                    case '5':strcpy(result1,"已挂账");break;
                                    case '9':strcpy(result1,"记账失败");break;
                                    default:break;
                                }
                            }while(0);
                            db_free_result(&result);
                        }
                    }
                    switch(*db_cell(&rs,k,11))
                    {
                        case 'C':strcpy(result2,"已对账");break;
                        case '1':strcpy(result2,"交易成功");break;
                        case '9':strcpy(result2,"交易失败");break;
                        default:strcpy(result2,"状态未知");break;
                    }

                    sprintf(result3,"%s/%s",result1,result2);
                    /*
                       memset(BeneAcctOrName, 0, sizeof(BeneAcctOrName));
                       strcpy(BeneAcctOrName, db_cell(&rs, k, 4));
                       if(strlen(BeneAcctOrName) == 0)
                       strcpy(BeneAcctOrName, db_cell(&rs, k, 5));
                       memset(PayingAcctOrName, 0, sizeof(PayingAcctOrName));
                       strcpy(PayingAcctOrName, db_cell(&rs, k, 2));
                       if(strlen(PayingAcctOrName) == 0)
                       strcpy(PayingAcctOrName, db_cell(&rs, k, 3));
                     */
                    db_query_str(notetype_name, sizeof(notetype_name),
                            "select name from noteinfo where notetype='%s' "
                            "and dcflag='%s'", db_cell(&rs, k, 0), 
                            db_cell_by_name(&rs, k, "dcflag"));
                    //GetTrnCtl("DCFlag"));
                    sprintf(buf, "%02d/%02d", db_cell_i(&rs,k,9)%10000/100,
                            db_cell_i(&rs,k,9) % 100);
                    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                            db_cell(&banklist, j, 0), notetype_name, 
                            db_cell(&rs, k, 1),
                            db_cell(&rs, k, 2), db_cell(&rs, k, 3),
                            db_cell(&rs, k, 4), db_cell(&rs, k, 5),
                            FormatMoney(db_cell(&rs, k, 6)),
                            (db_cell_i(&rs, k, 7) == 0 ? "不截留" : "截留"),
                            db_cell(&rs, k, 8), buf, result3, trncode);

                    ltot += 1; ltotamt += db_cell_d(&rs, k, 6);
                    iSum += 1; dSumAmt += db_cell_d(&rs, k, 6);
                    tot += 1;
                }
                db_free_result(&rs);
                sprintf(buf, "%.2lf", ltotamt);
                fprintf(fp, ";;小计笔数:%11d;;;;                      "
                        "小计金额:;;;%s;;;;\n", ltot, FormatMoney(buf));
                tot += 1;
                alltot += ltot;
            }
            db_free_result(&banklist);

            WriteRptRowCount(fp, tot);
            sprintf(buf, "%.2lf", dSumAmt);

            gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
            WriteRptFooter(fp, "%d;%s;%s;%s;", alltot, FormatMoney(buf), 
                    printtime, gs_oper);
            fclose(fp);

            rc = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (rc != 0)
            {
                rc = ERR_APP;
                db_free_result(&curlist);
                db_free_result(&orglist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    db_commit();
    ifree(chinesedate);
    return rc;
}

// 提入清单
int PrintInNoteList( xmlDocPtr xmlReq, char *filename )
{
    result_set curlist, orglist, banklist;
    result_set rs;
    result_set result;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[128];
    char tbname[81];
    char bankname[81];
    char printtime[30];
    char notetype[8];
    char notetype_name[61];
    char workround_cond[128];
    char workround_name[128];
    char *chinesedate = NULL;
    char result1[128]={0};
    char result2[128]={0};
    char result3[256]={0};
    char trncode[128]={0};
    FILE *fp=NULL;
    int iSum, tot, alltot;
    double dSumAmt;
    int rc = 0;
    int h, i, j, k, l;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    //取请求报文的工作场次信息,为空则打印该工作日的所有工作场次数据
    memset(workround_cond, 0, sizeof(workround_cond));
    memset(workround_name, 0, sizeof(workround_name));
    strcpy(workround_name, "所有");
    if (IsNonEmptyStr(GetTrnCtl("WorkRound")) && *GetTrnCtl("WorkRound") != '0') 
    {
        sprintf(workround_name, "%s", GetTrnCtl("WorkRound"));
        sprintf(workround_cond, "and workround='%d'", atoi(GetTrnCtl("WorkRound")));
    }
    /*
       if (*GetTrnCtl("StartRefId") == 0x00)
       startrefid = 0L;
       else
       startrefid = atol(GetTrnCtl("StartRefId"));
       if (*GetTrnCtl("EndRefId") == 0x00)
       endrefid = MAX_SEQUENCE_ID;
       else
       endrefid = atol(GetTrnCtl("EndRefId"));
     */

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteList.para", getenv("HOME"), TCOP_BANKID);
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.ilst",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

    if (*GetTrnCtl("Acceptor") == 0x00)
    {
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    }
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        /*
           if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
           {
           if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
           {
        //SetError(E_ORG_PERMIT);
        return E_OTHER;
        }
        }
         */
    }

#ifdef DB2
    rc = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    rc = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (rc != 0)
        return rc;
    if ((rc = db_begin()) != 0)
        return rc;
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            //" WHERE inoutflag = '%c' AND workdate = '%s' AND workround='%d'"
            " WHERE inoutflag = '%c' AND workdate = '%s' %s"
            " AND classid = %d AND clearstate in('1', 'C') AND %s "
            "AND dcflag like '%%%s%%' AND truncflag like '%%%s%%'", 
            tbname, RECONREC_ACPT, GetTrnCtl("WorkDate"), 
            //atoi(GetTrnCtl("WorkRound")), atoi(GetTrnCtl("SvcClass")), buf, 
            workround_cond, atoi(GetTrnCtl("SvcClass")), buf, 
            GetTrnCtl("DCFlag"), GetTrnCtl("TruncFlag"));
    if (rc != 0)
        return rc;

    if ((rc = db_query(&curlist, 
                    "SELECT DISTINCT curcode FROM " TMP_T_RS)) != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if ((rc = db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                        " WHERE curcode='%s' order by acceptor", 
                        db_cell(&curlist, i, 0))) != 0)
        {
            db_free_result(&curlist);
            goto err_handle;
        }
        for (h = 0; h < db_row_count(&orglist); h++)
        {
            iSum = tot = 0;
            dSumAmt = (double)0;
            alltot = 0;
            fp = fopen(caDataFile, "w");

            WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;", gs_sysname,
                    *GetTrnCtl("DCFlag")?(atoi(GetTrnCtl("DCFlag")) == PART_DEBIT ? "借方" : "贷方"):"所有",
                    chinesedate, GetTrnCtl("WorkRound"), 
                    ChsName(curcode_list, db_cell(&curlist, i, 0)),
                    db_cell(&orglist, h, 0), 
                    org_name(db_cell(&orglist, h, 0), bankname));

            if ((rc = db_query(&banklist, "SELECT distinct originator FROM " TMP_T_RS
                            " WHERE curcode='%s' and acceptor='%s' "
                            "ORDER BY originator", db_cell(&curlist, i, 0), 
                            db_cell(&orglist, h, 0))) != 0)
            {
                db_free_result(&curlist);
                db_free_result(&orglist);
                goto err_handle;
            }

            for (j = 0; j < db_row_count(&banklist); j++)
            {
                int ltot = 0;
                double ltotamt = (double)0;
                rc = db_query(&rs,
                        "SELECT notetype, noteno, payingacct, payer, beneacct,"
                        "benename, settlamt, truncflag,refid,workdate,reserved3,clearstate,trncode,dcflag FROM "
                        TMP_T_RS " WHERE curcode='%s' and acceptor='%s' and "
                        "originator='%s' ORDER BY workdate,prestime", 
                        db_cell(&curlist,i,0), db_cell(&orglist, h, 0), 
                        db_cell(&banklist, j, 0));
                if ( rc != 0 )
                {
                    err_log("打印提入凭证明细表失败");
                    db_free_result(&banklist);
                    db_free_result(&orglist);
                    db_free_result(&curlist);
                    goto err_handle;
                }
                for (k = 0; k < db_row_count(&rs); k++)
                {
                    strcpy(notetype,db_cell(&rs,k,0));
                    if(strcmp(notetype,"71")==0||strcmp(notetype,"73")==0)
                        strcpy(trncode,"个人通存");
                    else if(strcmp(notetype,"72")==0||strcmp(notetype,"74")==0)
                        strcpy(trncode,"个人通兑");
                    else
                    {
                        switch(atoi(db_cell(&rs,k,12)))
                        {
                            case  1:strcpy(trncode,"借记业务");break;
                            case  2:strcpy(trncode,"贷记业务");break;
                            case  7:strcpy(trncode,"退票交易");break;
                            case 13:strcpy(trncode,"个人通存");break;
                            case 14:strcpy(trncode,"个人通兑");break;
                            default:break;
                        }
                    }

                    /*
                       if(*db_cell(&rs,k,10)=='1')//reserved3==1
                       {
                       strcpy(result1,"记账成功");
                       }
                       else
                     */
                    {
                        rc = db_query(&result,"select result from acctjour where originator ='%s' and workdate='%s' and refid='%s' and inoutflag='2'",
                                db_cell(&banklist, j, 0), db_cell(&rs,k,9), db_cell(&rs,k,8));
                        if(rc!=0)
                        {
                            if(rc==E_DB_NORECORD)
                            {
                                strcpy(result1,"未记账");
                            }
                            else
                            {
                                err_log("打印提入凭证明细表失败");
                                db_free_result(&banklist);
                                db_free_result(&orglist);
                                db_free_result(&curlist);
                                db_free_result(&rs);
                                goto err_handle;
                            }
                        }
                        else
                        {
                            l=0;
                            if(db_row_count(&result)>1)
                            {
                                if(*db_cell(&rs,k,13)=='2')
                                {
                                    if(strcmp(db_cell(&result, 0, 1),"8249")==0)
                                        l=0;
                                    else
                                        l=1;
                                }
                            }
                            switch(*db_cell(&result, l, 0))
                            {
                                case '0':strcpy(result1,"状态未知");break;
                                case '1':strcpy(result1,"记账成功");break;
                                case '2':strcpy(result1,"已冲正");break;
                                case '5':strcpy(result1,"已挂账");break;
                                case '9':strcpy(result1,"记账失败");break;
                                default:break;
                            }
                            db_free_result(&result);
                        }
                    }
                    switch(*db_cell(&rs,k,11))
                    {
                        case 'C':strcpy(result2,"已对账");break;
                        case '1':strcpy(result2,"交易成功");break;
                        case '9':strcpy(result2,"交易失败");break;
                        default:strcpy(result2,"状态未知");break;
                    }

                    sprintf(result3,"%s/%s",result1,result2);
                    /*
                       memset(BeneAcctOrName, 0, sizeof(BeneAcctOrName));
                       strcpy(BeneAcctOrName, db_cell(&rs, k, 4));
                       if(strlen(BeneAcctOrName) == 0)
                       strcpy(BeneAcctOrName, db_cell(&rs, k, 5));
                       memset(PayingAcctOrName, 0, sizeof(PayingAcctOrName));
                       strcpy(PayingAcctOrName, db_cell(&rs, k, 2));
                       if(strlen(PayingAcctOrName) == 0)
                       strcpy(PayingAcctOrName, db_cell(&rs, k, 3));
                     */
                    db_query_str(notetype_name, sizeof(notetype_name),
                            "select name from noteinfo where notetype='%s' and "
                            "dcflag='%s'", db_cell(&rs, k, 0), 
                            db_cell_by_name(&rs, k, "DCFlag"));
                    //GetTrnCtl("DCFlag"));
                    sprintf(buf, "%02d/%02d", db_cell_i(&rs,k,9)%10000/100,
                            db_cell_i(&rs,k,9) % 100);
                    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                            db_cell(&banklist, j, 0), notetype_name, 
                            db_cell(&rs, k, 1),
                            db_cell(&rs, k, 2), db_cell(&rs, k, 3),
                            db_cell(&rs, k, 4), db_cell(&rs, k, 5),
                            FormatMoney(db_cell(&rs, k, 6)),
                            (db_cell_i(&rs, k, 7) == 0 ? "不截留" : "截留"),
                            db_cell(&rs, k, 8), buf, result3, trncode);

                    ltot += 1; ltotamt += db_cell_d(&rs, k, 6);
                    iSum += 1; dSumAmt += db_cell_d(&rs, k, 6);
                    tot += 1;
                }
                db_free_result(&rs);
                sprintf(buf, "%.2lf", ltotamt);
                fprintf(fp, ";;小计笔数:%11d;;;;                      "
                        "小计金额:;;;%s;;;;\n", ltot, FormatMoney(buf));
                tot += 1;
                alltot += ltot;
            }
            db_free_result(&banklist);

            WriteRptRowCount(fp, tot);
            sprintf(buf, "%.2lf", dSumAmt);

            gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
            WriteRptFooter(fp, "%d;%s;%s;%s;", alltot, FormatMoney(buf), 
                    printtime, gs_oper);
            fclose(fp); fp = NULL;

            rc = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (rc != 0)
            {
                rc = ERR_APP;
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    sprintf(filename, "%s", basename(caOutFile));

err_handle:

    db_commit();
    ifree(chinesedate);
    return rc;
}

int PrintOutJHD( xmlDocPtr xmlReq, char *filename)
{
    result_set banklist, rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char acptname[81];
    char areaname[81];
    char condi[1024];
    char printtime[11];
    char *chinesedate = NULL;
    char spacearea[128];
    char inoutarea[8];
    char routearea[50];
    FILE *fp=NULL;
    int outroute, inroute;
    int i, j, tot = 0, cnt = 0;
    int rc = E_OTHER;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")) ||
            !IsNonEmptyStr(GetTrnCtl("ExchArea")) ||
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")) )
    {
        return E_OTHER;
    }
    chinesedate = ChineseDate(atol(GetTrnCtl("ExchgDate")));
    gettime(printtime, sizeof(printtime), "%H:%M:%S");

    // 生成报表函数相关文件
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutNoteJHD.para", getenv("HOME"), TCOP_BANKID);
    GetTmpFileName(caDataFile);
    GetTmpFileName(caOutFile);

    // 区域名称
    db_query_str(areaname, sizeof(areaname), "select areaname from exchgarea "
            "where areacode='%s'", GetTrnCtl("ExchArea"));
    if( strlen(areaname) == 0 )

        snprintf(condi, sizeof(condi), "presbank='%s' and acptbank like '%s%%' "
                "and exchgdate='%s' and exchground=%d and excharea='%s' and type='1'",
                gs_originator, GetTrnCtl("Acceptor"), GetTrnCtl("ExchgDate"), 
                atoi(GetTrnCtl("ExchgRound")), GetTrnCtl("ExchArea"));
    if ((rc = db_query(&banklist, "SELECT distinct acptbank FROM baginfo "
                    "WHERE %s order by acptbank", condi)) != 0)
        goto err_handle;

    outroute = GetExchgRoute(GetTrnCtl("ExchArea"), gs_originator);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&banklist);
        rc = ERR_APP;
        goto err_handle;
    }
    WriteRptHeader(fp, "");
    //判断交易提出区域与打印的区域是否在同一个区域
    //如果不在同一个区域，在计数单上加两根横线 add by luokf 2010-06-03
    memset(spacearea, 0, sizeof(spacearea));
    memset(inoutarea, 0, sizeof(inoutarea));
    memset(routearea, 0, sizeof(routearea));
    //如果为"0",则说明是本区域
    if (ChkExchgArea(gs_originator, GetTrnCtl("ExchArea")) == 0)
    {
        strcpy(spacearea, " ");
        strcpy(inoutarea, "本地区");
        strcpy(routearea, "←-------------------");
    }
    else
    {
        strcpy(spacearea,
                "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■");
        strcpy(inoutarea, "跨地区");
        //strcpy(routearea, "<=======跨地区======="); 
        strcpy(routearea, "=======跨地区======="); 
    }
    for (i = 0; i < db_row_count(&banklist); i++)
    {
        if (db_query(&rs, "SELECT " IS_NULL "(sum(num),0)," 
                    IS_NULL "(sum(debitnum),0),"
                    IS_NULL "(sum(debitamount),0),"
                    IS_NULL "(sum(creditnum),0),"
                    IS_NULL "(sum(creditamount),0) "
                    "FROM baginfo WHERE acptbank='%s' and %s",
                    db_cell(&banklist, i, 0), condi) != 0)
        {
            db_free_result(&banklist);
            fclose(fp);
            goto err_handle;
        }
        org_name(db_cell(&banklist, i, 0), acptname);
        inroute=GetExchgRoute(GetTrnCtl("ExchArea"), db_cell(&banklist,i,0));
        cnt = db_row_count(&rs);
        for (j = 0; j < cnt; j++)
        {
            /*
               fprintf(fp,
               "(%s)%s;%s;第 %s 场;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%d;\n",
               areaname, gs_sysname, chinesedate, GetTrnCtl("ExchgRound"),
               printtime, db_cell(&banklist, i, 0), acptname,
               gs_originator, gs_bankname, db_cell(&rs, j, 2), 
               db_cell(&rs, j, 3), db_cell(&rs, j, 4), db_cell(&rs, j, 5),
               gs_oper, outroute, inroute);
             */
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;[%d];[%d];%s;%s;%s;\n",
                    areaname, inoutarea,
                    acptname, chinesedate, GetTrnCtl("ExchgRound"),
                    db_cell(&banklist, i, 0), gs_originator,
                    db_cell(&rs, j, 1), db_cell(&rs, j, 2), db_cell(&rs, j, 3), db_cell(&rs, j, 4),
                    inroute, outroute, routearea, spacearea, spacearea
                   ); 
            tot++;
        }
        db_free_result(&rs);
    }
    db_free_result(&banklist);

    WriteRptRowCount(fp, tot);
    WriteRptFooter(fp, "");
    fclose(fp); fp = NULL;
    if (PrintReportList(caParaFile, caDataFile, caOutFile) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }

    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

int PrintOutJJQD( xmlDocPtr xmlReq, char *filename)
{
    result_set banklist, rs, rs1;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char areaname[81];
    char bankname[81];
    char condi[1024];
    char printtime[20];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int debit = 0, credit = 0;
    int innum = 0;
    double debitamt = 0;
    double creditamt = 0;
    int rc = 0;
    int routeid, i = 0;

    if (InitRptVar(xmlReq) != 0)
        return ERR_APP;
    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")) || 
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")) ||
            !IsNonEmptyStr(GetTrnCtl("ExchArea")))
    {
        return ERR_APP;
    }

    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")))
        chinesedate = ChineseDate(current_date());
    else
        chinesedate = ChineseDate(atol(GetTrnCtl("ExchgDate")));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/PrintOutJJQD.para", getenv("HOME"), TCOP_BANKID);

    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        return ERR_APP;
    }

    // 区域名称
    db_query_str(areaname, sizeof(areaname), "select areaname from exchgarea "
            "where areacode='%s'", GetTrnCtl("ExchArea"));
    // 线路号
    routeid = GetExchgRoute(GetTrnCtl("ExchArea"), gs_originator);

    // 写数据文件Header部分
    gettime(printtime, sizeof(printtime), "%Y年%m月%d日 %H时%M分");
    WriteRptHeader(fp, "(%s)%s;%s;%s;%d;%s;%s;%s;", areaname, gs_sysname, 
            chinesedate, GetTrnCtl("ExchgRound"), routeid,
            gs_originator, gs_bankname, printtime);

    snprintf(condi, sizeof(condi), "presbank='%s' and exchgdate='%s' and "
            "exchground=%d and excharea='%s' and type='1' ", gs_originator, GetTrnCtl("ExchgDate"), 
            atoi(GetTrnCtl("ExchgRound")), GetTrnCtl("ExchArea"));
    rc = db_query(&banklist, "SELECT distinct acptbank FROM baginfo "
            "WHERE %s", condi);
    if ( rc != 0 )
    {
        if (rc != E_DB_NORECORD)
        {
            fclose(fp);
            return rc;
        }
        fprintf(fp, "无;----;0;0;0;无;----;0;0;0;\n");
        i = 1;
    }
    else
    {
        for (i = 0; i < db_row_count(&banklist); i++)
        {
            // 场内包
            rc = db_query(&rs, "SELECT "IS_NULL "(sum(num),0),"
                    IS_NULL "(sum(debitnum),0),"
                    IS_NULL "(sum(creditnum),0),"
                    IS_NULL "(sum(debitamount),0),"
                    IS_NULL "(sum(creditamount),0) FROM baginfo"
                    " WHERE acptbank='%s' and %s and type = '1'", 
                    db_cell(&banklist, i, 0), condi);
            if (rc != 0)
            {
                db_free_result(&banklist);
                fclose(fp);
                return rc;
            }
            innum ++;

            /*
               fprintf(fp, "%s;%s;%s;%s;%d;%d;%d;\n", db_cell(&banklist, i, 0), 
               bankname, db_cell(&rs, 0, 1), db_cell(&rs, 0, 2), 
               db_cell_i(&rs, 0, 0), db_cell_i(&rs1, 0, 0), 
               db_cell_i(&rs, 0, 0)+db_cell_i(&rs1, 0, 0));
               debit += db_cell_i(&rs, 0, 1);
               credit += db_cell_i(&rs, 0, 2);
               db_free_result(&rs1);
             */
            org_name(db_cell(&banklist, i, 0), bankname);
            fprintf(fp, "%s;%s;%s;%s;%s;", db_cell(&banklist, i, 0), 
                    db_cell(&rs, 0, 1), 
                    db_cell(&rs, 0, 3), 
                    db_cell(&rs, 0, 2),
                    db_cell(&rs, 0, 4));
            if (i % 2 == 1)
                fprintf(fp, "\n");

            debit += db_cell_i(&rs, 0, 1);
            credit += db_cell_i(&rs, 0, 2);
            debitamt += db_cell_d(&rs, 0, 3);
            creditamt += db_cell_d(&rs, 0, 4);
            db_free_result(&rs);
        }
        if (db_row_count(&banklist) % 2 == 0)
            fprintf(fp, ";;;;;合计;%d;%.2lf;%d;%.2lf;\n",debit,debitamt,credit,creditamt);
        else
            fprintf(fp, "合计;%d;%.2lf;%d;%.2lf;\n",debit,debitamt,credit,creditamt);
        db_free_result(&banklist);
    }

    WriteRptRowCount(fp, innum/2+1);
    WriteRptFooter(fp, "%d;",innum);
    fclose(fp); fp = NULL;

    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.jjd",
            getenv("FILES_DIR"), gs_originator, current_date()%100,
            current_time());
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }

    sprintf(filename, "%s", basename(caOutFile));

err_handle:

    ifree(chinesedate);
    return rc;
}

int PrintInJJQD( xmlDocPtr xmlReq, char *filename)
{
    result_set banklist, rs, rs1;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char areaname[81];
    char bankname[81];
    char condi[1024];
    char printtime[20];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int debit = 0, credit = 0;
    int innum = 0;
    double debitamt = 0;
    double creditamt = 0;
    int rc = 0;
    int routeid, i = 0;

    if (InitRptVar(xmlReq) != 0)
        return ERR_APP;
    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")) ||
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")) ||
            !IsNonEmptyStr(GetTrnCtl("ExchArea")))
    {
        return ERR_APP;
    }

    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")))
        chinesedate = ChineseDate(current_date());
    else
        chinesedate = ChineseDate(atol(GetTrnCtl("ExchgDate")));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/PrintInJJQD.para", getenv("HOME"), TCOP_BANKID);

    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        return ERR_APP;
    }

    // 区域名称
    db_query_str(areaname, sizeof(areaname), "select areaname from exchgarea "
            "where areacode='%s'", GetTrnCtl("ExchArea"));
    // 线路号
    routeid = GetExchgRoute(GetTrnCtl("ExchArea"), gs_originator);

    // 写数据文件Header部分
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
    WriteRptHeader(fp, "(%s)%s;%s;%s;%d;%s;%s;%s;", areaname, gs_sysname, 
            chinesedate, GetTrnCtl("ExchgRound"), routeid,
            gs_originator, gs_bankname, printtime);

    snprintf(condi, sizeof(condi), "acptbank='%s' and exchgdate='%s' and "
            "exchground=%d and excharea='%s' and type='1' ", gs_originator, 
            GetTrnCtl("ExchgDate"), atoi(GetTrnCtl("ExchgRound")),
            GetTrnCtl("ExchArea"));
    rc = db_query(&banklist, "SELECT distinct presbank FROM baginfo "
            "WHERE %s", condi);
    if ( rc != 0 )
    {
        if (rc != E_DB_NORECORD)
        {
            fclose(fp);
            return rc;
        }
        fprintf(fp, "无;----;0;0;0;无;----;0;0;0;\n");
        i = 1;
    }
    else
    {
        for (i = 0; i < db_row_count(&banklist); i++)
        {
            // 场内包
            rc = db_query(&rs, "SELECT "IS_NULL "(sum(num),0),"
                    IS_NULL "(sum(debitnum),0),"
                    IS_NULL "(sum(creditnum),0),"
                    IS_NULL "(sum(debitamount),0),"
                    IS_NULL "(sum(creditamount),0) FROM baginfo"
                    " WHERE presbank='%s' and %s and type = '1'", 
                    db_cell(&banklist, i, 0), condi);
            if (rc != 0)
            {
                db_free_result(&banklist);
                fclose(fp);
                return rc;
            }
            innum ++;

            /*
               fprintf(fp, "%s;%s;%s;%s;%d;%d;%d;\n", db_cell(&banklist, i, 0), 
               bankname, db_cell(&rs, 0, 1), db_cell(&rs, 0, 2), 
               db_cell_i(&rs, 0, 0), db_cell_i(&rs1, 0, 0), 
               db_cell_i(&rs, 0, 0)+db_cell_i(&rs1, 0, 0));
               debit += db_cell_i(&rs, 0, 1);
               credit += db_cell_i(&rs, 0, 2);
               db_free_result(&rs1);
             */
            org_name(db_cell(&banklist, i, 0), bankname);
            fprintf(fp, "%s;%s;%s;%s;%s;", db_cell(&banklist, i, 0), 
                    db_cell(&rs, 0, 1), 
                    db_cell(&rs, 0, 3), 
                    db_cell(&rs, 0, 2),
                    db_cell(&rs, 0, 4));
            if (i % 2 == 1)
                fprintf(fp, "\n");

            debit += db_cell_i(&rs, 0, 1);
            credit += db_cell_i(&rs, 0, 2);
            debitamt += db_cell_d(&rs, 0, 3);
            creditamt += db_cell_d(&rs, 0, 4);
            db_free_result(&rs);
        }
        if (db_row_count(&banklist) % 2 == 0)
            fprintf(fp, ";;;;;合计;%d;%.2lf;%d;%.2lf;\n",debit,debitamt,credit,creditamt);
        else
            fprintf(fp, "合计;%d;%.2lf;%d;%.2lf;\n",debit,debitamt,credit,creditamt);
        db_free_result(&banklist);
    }

    WriteRptRowCount(fp, innum/2+1);
    WriteRptFooter(fp, "%d;",innum);
    fclose(fp); fp = NULL;

    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.jjd",
            getenv("FILES_DIR"), gs_originator, current_date()%100,
            current_time());
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }

    sprintf(filename, "%s", basename(caOutFile));

err_handle:

    ifree(chinesedate);
    return rc;
}

int PrintClearDiff(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *chinesedate;
    char buf[256];
    FILE *fp=NULL;
    int classid, clearround;
    int rc = 0;
    int i;
    char cmd[256]={0};

    classid = atoi(GetTrnCtl("SvcClass"));
    clearround = atoi(GetTrnCtl("WorkRound"));
    if( clearround != 0 )
        sprintf( cmd, " and workround=%d", clearround );

    rc = db_query(&rs, "SELECT curcode, curtype, "
            IS_NULL "(sum(pres_credit_num),0),"
            IS_NULL "(sum(pres_credit_total), 0.0),"
            IS_NULL "(sum(pres_debit_num), 0),"
            IS_NULL "(sum(pres_debit_total), 0.0),"
            IS_NULL "(sum(acpt_debit_num), 0),"
            IS_NULL "(sum(acpt_debit_total), 0.0),"
            IS_NULL "(sum(acpt_credit_num), 0),"
            IS_NULL "(sum(acpt_credit_total), 0.0),"
            IS_NULL "(sum(pres_credit_num + acpt_debit_num), 0) as pay_n,"
            IS_NULL "(sum(pres_credit_total + acpt_debit_total),0.0) as pay_t,"
            IS_NULL "(sum(pres_debit_num + acpt_credit_num),0) as bene_n,"
            IS_NULL "(sum(pres_debit_total+acpt_credit_total),0.0) as bene_t,"
            IS_NULL "(sum(balance),0.0) FROM reconinfo "
            "WHERE svcclass=%d and bankid='%s' "
            //"and workdate='%s' and workround=%d "
            "and workdate='%s' %s "
            "group by curcode, curtype ORDER BY curcode, curtype",
            //classid, gs_originator, GetTrnCtl("WorkDate"), clearround);
       classid, gs_originator, GetTrnCtl("WorkDate"), cmd);
    if ( rc != 0 )
        return rc;

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/DiffNote.para", getenv("HOME"), TCOP_BANKID);
    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s;%-30s", gs_originator, gs_bankname);

        if (classid == CLASS_FOREIGN)
        {
            fprintf(fp, "  外币:%s %s;",
                    ChsName(curcode_list, db_cell(&rs, i, 0)),
                    GetChineseName(curtype_list, db_cell_i(&rs, i, 1)));
        }
        else
        {
            fprintf(fp, "  人民币;" );
        }
        fprintf(fp, "%s;第%d场;", chinesedate, clearround);

        if (*db_cell(&rs, i, 14) == '-')
            sprintf(buf, "%s; ;", FormatMoney(db_cell(&rs, i, 14) + 1));
        else
            sprintf(buf, " ;%s;", FormatMoney(db_cell(&rs, i, 14)));

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s%s;清算行;\n", 
                db_cell(&rs, i, 2), FormatMoney(db_cell(&rs, i, 3)),
                db_cell(&rs, i, 4), FormatMoney(db_cell(&rs, i, 5)),
                db_cell(&rs, i, 6), FormatMoney(db_cell(&rs, i, 7)),
                db_cell(&rs, i, 8), FormatMoney(db_cell(&rs, i, 9)),
                db_cell(&rs, i, 10), FormatMoney(db_cell(&rs, i, 11)),
                db_cell(&rs, i, 12), FormatMoney(db_cell(&rs, i, 13)), 
                buf, gs_oper);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.dif",
            getenv("FILES_DIR"),gs_originator,current_date()%100,current_time());
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
        return ERR_APP;

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}

int PrintDiffNote( xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char bankname[81];
    char *chinesedate;
    char clearfile[256];
    char buf[256];
    char condi[256];
    FILE *fp=NULL;
    int classid, clearround;
    int rc = 0;
    int printed=0, i;
    char cmd[256]={0};

    if (InitRptVar(xmlReq) != 0)
        return ERR_APP;

    // 是否为清算行
    snprintf(buf, sizeof(buf), "orgid='%s' and orglevel='1'", gs_originator);
    if (db_hasrecord("organinfo", buf))
    {
        // 生成清算行差额
        if ((rc = PrintClearDiff(xmlReq, clearfile)) != 0)
            return rc;
        // 仅打印清算行差额
        if (*GetTrnCtl("PrintFlag") == '1')
            return 0;
        sprintf(condi, "branchid like '%s%%'", GetTrnCtl("BankId"));
        printed = 1;
    }
    else
    {
        sprintf(condi, "branchid = '%s'", gs_originator);
    }

    classid = atoi(GetTrnCtl("SvcClass"));
    clearround = atoi(GetTrnCtl("WorkRound"));
    if(clearround != 0 )
        sprintf( cmd, " AND workround=%d", clearround );
    rc = db_query(&rs, "SELECT branchid, curcode, curtype,"
            IS_NULL "(sum(pres_credit_num),0),"
            IS_NULL "(sum(pres_credit_total), 0.0),"
            IS_NULL "(sum(pres_debit_num), 0),"
            IS_NULL "(sum(pres_debit_total), 0.0),"
            IS_NULL "(sum(acpt_debit_num), 0),"
            IS_NULL "(sum(acpt_debit_total), 0.0),"
            IS_NULL "(sum(acpt_credit_num), 0),"
            IS_NULL "(sum(acpt_credit_total), 0.0),"
            IS_NULL "(sum(pres_credit_num + acpt_debit_num), 0) as pay_n,"
            IS_NULL "(sum(pres_credit_total + acpt_debit_total),0.0) as pay_t,"
            IS_NULL "(sum(pres_debit_num + acpt_credit_num),0) as bene_n,"
            IS_NULL "(sum(pres_debit_total+acpt_credit_total),0.0) as bene_t,"
            IS_NULL "(sum(balance),0.0) FROM ebanksumm "
            //"WHERE svcclass=%d AND %s AND workdate='%s' AND workround=%d "
            "WHERE svcclass=%d AND %s AND workdate='%s' "
            "group by branchid, curcode, curtype "
            "ORDER BY branchid, curcode, curtype",
            //classid, condi, GetTrnCtl("WorkDate"), clearround);
       classid, condi, GetTrnCtl("WorkDate"), cmd);
    if ( rc != 0 )
        return rc;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/DiffNote.para", getenv("HOME"), TCOP_BANKID);

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));
    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s;%-30s", db_cell(&rs,i,0), 
                org_name(db_cell(&rs,i,0), bankname));

        if (classid == CLASS_FOREIGN)
        {
            fprintf(fp, "  外币:%s %s;",
                    ChsName(curcode_list, db_cell(&rs, i, 1)),
                    GetChineseName(curtype_list, db_cell_i(&rs, i, 2)));
        }
        else
        {
            fprintf(fp, "  人民币;" );
        }
        fprintf(fp, "%s;第%d场;", chinesedate, clearround);

        if (*db_cell(&rs, i, 15) == '-')
            sprintf(buf, "%s; ;", FormatMoney(db_cell(&rs, i, 15) + 1));
        else
            sprintf(buf, " ;%s;", FormatMoney(db_cell(&rs, i, 15)));

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s%s;交换行;\n", 
                db_cell(&rs, i, 3), FormatMoney(db_cell(&rs, i, 4)),
                db_cell(&rs, i, 5), FormatMoney(db_cell(&rs, i, 6)),
                db_cell(&rs, i, 7), FormatMoney(db_cell(&rs, i, 8)),
                db_cell(&rs, i, 9), FormatMoney(db_cell(&rs, i, 10)),
                db_cell(&rs, i, 11), FormatMoney(db_cell(&rs, i, 12)),
                db_cell(&rs, i, 13), FormatMoney(db_cell(&rs, i, 14)), 
                buf, gs_oper);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    // 打印输出文件已经存在
    if (printed == 1)
    {
        snprintf( caOutFile, sizeof(caOutFile), "%s/%s", 
                getenv("FILES_DIR"), clearfile);
    }
    else
    {
        snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.dif",
                getenv("FILES_DIR"), gs_originator,
                current_date()%100, current_time());
    }
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
        return ERR_APP;

    sprintf(filename, "%s", basename(caOutFile));
    return rc;
}

int PrintSettleResult( xmlDocPtr xmlReq, char *filename)
{
    FILE *fp;
    char caOutFile[256];
    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%s_%s_%s.result",
            getenv("FILES_DIR"),XmlGetStringDup(xmlReq,"//Originator"),GetTrnCtl("WorkDate"),GetTrnCtl("WorkRound"),GetTrnCtl("SvcClass"));
    INFO("打印对账结果报告单[%s]",caOutFile);
    fp=fopen(caOutFile,"r");
    if(fp==NULL)
    {
        *filename='\0';
    }
    else
    {
        fclose(fp);
        sprintf(filename, "%s", basename(caOutFile));
    }
    return 0;
}
//清算业务量统计表
int PrintClearTotal_PF( xmlDocPtr xmlReq, char *filename )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char bankname[81];
    char *chinesedate1, *chinesedate2;
    char clearfile[256];
    char buf[256];
    char condi[256];
    FILE *fp=NULL;
    int classid, clearround;
    int rc = 0;
    int printed=0, i;
    char cmd[256]={0};
    char startdate[9]={0};
    char enddate[9]={0};
    char tmp[256]={0};
    double  totCSumAmt1, totCSumAmt2, totCSumAmt3, totCSumAmt4;
    char  sCSumAmt1[30], sCSumAmt2[30], sCSumAmt3[30], sCSumAmt4[30];
    char sCAmt1[30],  sCAmt2[30],  sCAmt3[30],  sCAmt4[30];
    int iCCount1=0, iCCount2=0, iCCount3=0, iCCount4=0;
    int itotCCount1=0, itotCCount2=0, itotCCount3=0, itotCCount4=0;


    if (InitRptVar(xmlReq) != 0)
        return ERR_APP;

    /*
    //按凭证种类统计
    if (*GetTrnCtl("PrintFlag") == '1')
    return PrintNoteTotal_PF( xmlReq, filename); 
     */

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08s", GetWorkdate());
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    // 是否为清算行
    snprintf(buf, sizeof(buf), "orgid='%s' and orglevel='1'", gs_originator);
    if (db_hasrecord("organinfo", buf))
    {
        sprintf(condi, "branchid like '%s%%'", GetTrnCtl("BankId"));
    }
    else
    {
        sprintf(condi, "branchid = '%s'", gs_originator);
    }

    classid = atoi(GetTrnCtl("SvcClass"));

    rc = db_query(&rs, "SELECT branchid, curcode, curtype,"
            IS_NULL "(sum(pres_credit_num),0),"
            IS_NULL "(sum(pres_credit_total), 0.0),"
            IS_NULL "(sum(pres_debit_num), 0),"
            IS_NULL "(sum(pres_debit_total), 0.0),"
            IS_NULL "(sum(acpt_credit_num), 0),"
            IS_NULL "(sum(acpt_credit_total), 0.0),"
            IS_NULL "(sum(acpt_debit_num), 0),"
            IS_NULL "(sum(acpt_debit_total), 0.0),"
            IS_NULL "(sum(balance),0.0) FROM ebanksumm "
            "WHERE svcclass=%d AND %s AND workdate between '%s' and '%s' "
            "group by branchid, curcode, curtype "
            "ORDER BY branchid, curcode, curtype",
            //classid, condi, GetTrnCtl("WorkDate"), clearround);
       classid, condi, startdate, enddate );
    if ( rc != 0 )
        return rc;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/ClearTotal_PF.para", getenv("HOME"), TCOP_BANKID);
    fp = fopen(GetTmpFileName(caDataFile), "w");

    chinesedate1 = ChineseDate(atol(startdate));
    chinesedate2 = ChineseDate(atol(enddate));

    WriteRptHeader(fp, "%s;%s;%s;%s;",
            gs_originator, 
            org_name(gs_originator, bankname),
            chinesedate1,
            chinesedate2 
            );

    for (i = 0; i < db_row_count(&rs); i++)
    {

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 3), FormatMoney(db_cell(&rs, i, 4)),
                db_cell(&rs, i, 5), FormatMoney(db_cell(&rs, i, 6)),
                db_cell(&rs, i, 7), FormatMoney(db_cell(&rs, i, 8)),
                db_cell(&rs, i, 9), FormatMoney(db_cell(&rs, i, 10))
               );

        //提出贷记
        iCCount1 = db_cell_i(&rs, i, 3);
        itotCCount1 += iCCount1;
        strcpy(sCAmt1, db_cell(&rs, i, 4));
        totCSumAmt1 += atof(sCAmt1);
        //提出借记
        iCCount2 = db_cell_i(&rs, i, 5);
        itotCCount2 += iCCount2;
        strcpy(sCAmt2, db_cell(&rs, i, 6));
        totCSumAmt2 += atof(sCAmt2);
        //提入贷记
        iCCount3 = db_cell_i(&rs, i, 7);
        itotCCount3 += iCCount3;
        strcpy(sCAmt3, db_cell(&rs, i, 8));
        totCSumAmt3 += atof(sCAmt3);
        //提入借记
        iCCount4 = db_cell_i(&rs, i, 9);
        itotCCount4 += iCCount4;
        strcpy(sCAmt4, db_cell(&rs, i, 10));
        totCSumAmt4 += atof(sCAmt4);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);

    sprintf(sCSumAmt1, "%.2lf", totCSumAmt1);
    sprintf(sCSumAmt2, "%.2lf", totCSumAmt2);
    sprintf(sCSumAmt3, "%.2lf", totCSumAmt3);
    sprintf(sCSumAmt4, "%.2lf", totCSumAmt4);
    WriteRptFooter(fp, "%d;%s;%d;%s;%d;%s;%d;%s",
            itotCCount1, FormatMoney(sCSumAmt1),
            itotCCount2, FormatMoney(sCSumAmt2),
            itotCCount3, FormatMoney(sCSumAmt3),
            itotCCount4, FormatMoney(sCSumAmt4) 
            );
    fclose(fp);

    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.ctpf",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
        return ERR_APP;

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}
//清算业务种类统计表
int PrintNoteTotal_PF( xmlDocPtr xmlReq, char *filename )
{
    result_set rs;
    result_set rs1;
    result_set rs_note;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char bankname[81];
    char *chinesedate1, *chinesedate2;
    char clearfile[256];
    char buf[256];
    char condi[256];
    FILE *fp=NULL;
    int classid, clearround;
    int rc = 0;
    int rc1 = 0;
    int printed=0, i;
    char cmd[256]={0};
    char startdate[9]={0};
    char enddate[9]={0};
    char tmp[256]={0};
    double  totCSumAmt1, totCSumAmt2, totCSumAmt3, totCSumAmt4;
    char  sCSumAmt1[30], sCSumAmt2[30], sCSumAmt3[30], sCSumAmt4[30];
    char sCAmt1[30],  sCAmt2[30],  sCAmt3[30],  sCAmt4[30];
    int iCCount1, iCCount2, iCCount3, iCCount4;
    int itotCCount1, itotCCount2, itotCCount3, itotCCount4;
    char notetype_name[61]={0};
    char notetype[3]={0};


    if (InitRptVar(xmlReq) != 0)
        return ERR_APP;

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08s", GetWorkdate());
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    /*
       if (*GetTrnCtl("BankId") == 0x00)
       {
       sprintf(buf, "originator like '%s%%' ", GetTrnCtl("BankId"));
       }
       else
       {
       sprintf(buf, "originator='%s' or acceptor='%s'", 
       GetTrnCtl("BankdId"), GetTrnCtl("BankdId"));
       }
     */
    sprintf(condi, "originator like '%s%%' or acceptor like '%s%%'", 
            GetTrnCtl("BankdId"), GetTrnCtl("BankdId"));
    classid = atoi(GetTrnCtl("SvcClass"));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/NoteTotal_PF.para", getenv("HOME"), TCOP_BANKID);
    fp = fopen(GetTmpFileName(caDataFile), "w");

    chinesedate1 = ChineseDate(atol(startdate));
    chinesedate2 = ChineseDate(atol(enddate));

    WriteRptHeader(fp, "%s;%s;%s;%s;",
            gs_originator, 
            org_name(gs_originator, bankname),
            chinesedate1,
            chinesedate2 
            );

    strcpy(notetype, GetTrnCtl("NoteType"));
    //所有凭证
    if( notetype[0] == 0 )
    {
        rc = db_query(&rs_note, "SELECT  distinct notetype From noteinfo where nodeid=%d ",
                OP_REGIONID);

        for (i = 0; i < db_row_count(&rs_note); i++)
        {
            rc = db_query(&rs, "SELECT  count(*)," IS_NULL "(sum(settlamt), 0.0) "
                    " FROM htrnjour"
                    " WHERE nodeid =%d AND workdate between '%s' and '%s' "
                    " And notetype='%s' AND inoutflag='1' "
                    " And clearstate in('1', 'C') "
                    "group by notetype "
                    "ORDER BY notetype",
                    //OP_REGIONID, classid, startdate, enddate, db_cell(&rs_note, i, 0));
               OP_REGIONID,  startdate, enddate, db_cell(&rs_note, i, 0));
            /*
               if ( rc != 0 )
               {
               db_free_result(&rs_note);
               return rc;
               }
             */


            rc1 = db_query(&rs1, "SELECT  count(*)," IS_NULL "(sum(settlamt), 0.0) "
                    " FROM htrnjour"
                    " WHERE nodeid=%d And  workdate between '%s' and '%s' "
                    " And notetype='%s' AND inoutflag='2' "
                    " And clearstate in('1', 'C') "
                    "group by notetype "
                    "ORDER BY notetype",
                    //OP_REGIONID,classid, startdate, enddate, db_cell(&rs_note, i, 0) );
                OP_REGIONID, startdate, enddate, db_cell(&rs_note, i, 0) );
            /*
               if ( rc != 0 )
               {
               db_free_result(&rs_note);
               db_free_result(&rs);
               return rc;
               }
             */

            // 取凭证名称
            db_query_str(notetype_name, sizeof(notetype_name),
                    "select distinct name from noteinfo "
                    "where nodeid=%d and notetype='%s'", 
                    OP_REGIONID, db_cell(&rs_note, i, 0));

            if( rc == 0 )
            {
                //笔数
                iCCount1 = db_cell_i(&rs, 0, 0);
                strcpy(sCAmt1, db_cell(&rs, 0, 1));
            }
            else//找不到
            {
                iCCount1 =0;
                strcpy(sCAmt1, "0.00");
            }
            if( rc1 == 0 )
            {
                iCCount2 = db_cell_i(&rs1, 0, 0);
                strcpy(sCAmt2, db_cell(&rs1, 0, 1));
            }
            else
            {
                iCCount2 =0;
                strcpy(sCAmt2, "0.00");
            }
            itotCCount1 = iCCount1+iCCount2; //总笔数
            itotCCount2 += iCCount1; //提出笔数
            itotCCount3 += iCCount2; //提入笔数
            itotCCount4 += itotCCount1; //所有笔数
            //金额
            totCSumAmt1 = atof(sCAmt1) + atof(sCAmt2); //总金额
            sprintf(sCSumAmt1, "%.2lf", totCSumAmt1);

            totCSumAmt2 += atof(sCAmt1); //提出金额
            totCSumAmt3 += atof(sCAmt2); //提入金额
            totCSumAmt4 += totCSumAmt1; //所有金额

            fprintf(fp, "%s;%s;%d;%s;%d;%s;%d;%s;\n", 
                    db_cell(&rs_note, i, 0), notetype_name,  
                    iCCount1, FormatMoney(sCAmt1),
                    iCCount2, FormatMoney(sCAmt2),
                    //db_cell(&rs, 0, 0), FormatMoney(db_cell(&rs, 0, 1)),
                    //db_cell(&rs1, 0, 0), FormatMoney(db_cell(&rs1, 0, 1)),
                    itotCCount1, FormatMoney(sCSumAmt1)
                   );

        }
        db_free_result(&rs_note);
    }
    else
    {
        rc = db_query(&rs, "SELECT  count(*)," IS_NULL "(sum(settlamt), 0.0) "
                " FROM htrnjour"
                " WHERE nodeid=%d AND workdate between '%s' and '%s' "
                " And notetype='%s' AND inoutflag='1' "
                " And clearstate in('1', 'C') "
                "group by notetype "
                "ORDER BY notetype",
                OP_REGIONID, startdate, enddate, GetTrnCtl("NoteType") );
        if ( rc != 0 )
            return rc;


        rc1 = db_query(&rs1, "SELECT  count(*)," IS_NULL "(sum(settlamt), 0.0) "
                " FROM htrnjour"
                " WHERE   nodeid=%d AND workdate between '%s' and '%s' "
                " And notetype='%s' AND inoutflag='2' "
                " And clearstate in('1', 'C') "
                "group by notetype "
                "ORDER BY notetype",
                OP_REGIONID, startdate, enddate, GetTrnCtl("NoteType") );
        if ( rc != 0 )
        {
            db_free_result(&rs);
            return rc;
        }

        // 取凭证名称
        db_query_str(notetype_name, sizeof(notetype_name),
                "select distinct name from noteinfo "
                "where nodeid=%d and notetype='%s'", 
                OP_REGIONID, notetype);

        if( rc == 0 )
        {
            iCCount1 = db_cell_i(&rs, 0, 0);
            strcpy(sCAmt1, db_cell(&rs, 0, 1));
        }
        else
        {
            iCCount1 =0;
            strcpy(sCAmt1, "0.00");
        }
        if( rc1 == 0 )
        {
            iCCount2 = db_cell_i(&rs1, 0, 0);
            strcpy(sCAmt2, db_cell(&rs1, 0, 1));
        }
        else
        {
            iCCount2 =0;
            strcpy(sCAmt2, "0.00");
        }
        //笔数
        itotCCount1 = iCCount1+iCCount2; //总笔数
        itotCCount2 = iCCount1; //提出笔数
        itotCCount3 = iCCount2; //提入笔数
        itotCCount4 = itotCCount1; //所有笔数
        //金额
        totCSumAmt1 = atof(sCAmt1) + atof(sCAmt2); //总金额
        sprintf(sCSumAmt1, "%.2lf", totCSumAmt1);

        totCSumAmt2 = atof(sCAmt1); //提出金额
        totCSumAmt3 = atof(sCAmt2); //提入金额
        totCSumAmt4 = totCSumAmt1; //所有金额

        fprintf(fp, "%s;%s;%d;%s;%d;%s;%d;%s;\n", 
                notetype, notetype_name,  
                iCCount1, FormatMoney(sCAmt1),
                iCCount2, FormatMoney(sCAmt2),
                //db_cell(&rs, 0, 0), FormatMoney(db_cell(&rs, 0, 1)),
                //db_cell(&rs1, 0, 0), FormatMoney(db_cell(&rs1, 0, 1)),
                itotCCount1, FormatMoney(sCSumAmt1)
               );
        i=1;  //只有一条记录

    }

REPROT:
    db_free_result(&rs);
    db_free_result(&rs1);
    WriteRptRowCount(fp, i);

    sprintf(sCSumAmt2, "%.2lf", totCSumAmt2);
    sprintf(sCSumAmt3, "%.2lf", totCSumAmt3);
    sprintf(sCSumAmt4, "%.2lf", totCSumAmt4);
    WriteRptFooter(fp, "%d;%s;%d;%s;%d;%s",
            itotCCount2, FormatMoney(sCSumAmt2),
            itotCCount3, FormatMoney(sCSumAmt3),
            itotCCount4, FormatMoney(sCSumAmt4) 
            );
    fclose(fp);

    snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.ntpf",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
        return ERR_APP;

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}
//来账扣税凭证打印
int PrintInNoteAdd_KS( xmlDocPtr xmlReq, char *filename )
{
    char tbname[128];
    char tbname_ex[128];
    char condi[4096];
    char tmp[256];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char pbank[81];
    char bbank[81];
    //long startrefid;
    //long endrefid;
    char startrefid[17];
    char endrefid[17];
    double totamt = (double)0;
    char refid[17];
    char acctserial[17];
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
    int n, i, j, k, len;
    char *xmlBuf = NULL;
    xmlDocPtr doc;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    strcpy(startrefid, GetTrnCtl("StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0");
    strcpy(endrefid, GetTrnCtl("EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "9999999999");

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
    }

    /*
       snprintf( caParaFile, sizeof(caParaFile),
       "%s/dat/%d/InNoteAddNew.para", getenv("HOME"), TCOP_BANKID );
     */

    memset(tmp, 0, sizeof(tmp));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(tmp, "and printnum=0");
            break;
        case '1':
            strcpy(tmp, "and printnum>0");
            break;
        case '2':
        default:
            break;
    }

    memset(amount, 0, sizeof(amount));
    if (*GetTrnCtl("MaxAmount") != 0x00)
        sprintf(amount, "AND settlamt = %s", GetTrnCtl("MaxAmount"));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND workdate = '%s' AND classid = %d"
            "  AND convert(decimal,refid) BETWEEN %s AND %s "
            "  AND %s "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            "  AND result in(%d, %d, %d)",
            RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("SvcClass")),
            startrefid, endrefid, buf,
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount, GetTrnCtl("DCFlag"), tmp,
            E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
    rc = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, prestime", tbname, condi);
    if ( rc != 0 )
    {
        err_log("查询提入补充凭证(协议扣税)失败, condi=[%s][%s]", tbname, condi );
        return rc;
    }

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    fclose(fp);

    GetDateTime(datetime);
    GetTmpFileName(caDataFile);
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    fputs("\n", fp);
    for (i = 0, j = 0; i < db_row_count(&rs); i++)
    {
        snprintf( caParaFile, sizeof(caParaFile),
                "%s/dat/%d/InNoteAddNew_KS.para", getenv("HOME"), TCOP_BANKID );
        // 密押字段
        p = db_cell_by_name(&rs, i, "agreement");
        if (*p != 0)
            sprintf(key, "协议号:%s", p);

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "result"))) != 0)
        {
            strcat(key, "  ");
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
                "where nodeid=%d and notetype='%s'", 
                OP_REGIONID, db_cell_by_name(&rs, i, "notetype"));
        // 取行内记账流水
        sprintf(refid, "%s", db_cell_by_name(&rs, i, "refid"));
        sdpStringTrimHeadChar(refid, '0');
        db_query_str(acctserial, sizeof(acctserial),
                "select acctserial from acctjour "
                "where nodeid=%d and workdate='%s' and originator='%s' "
                "and convert(decimal, refid)=%s and inoutflag='2'",
                OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                db_cell_by_name(&rs, i, "originator"), refid);

        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));
        snprintf(content, sizeof(content),
                "%s;%s;%s;第%s场;%s;%s%s;%s;%s;%s;%s;%s %s;%s;%s;"
                //"%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;",
                "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;",
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
                db_cell_by_name(&rs, i, "agreement"),
                printinfo1,
                datetime,
                atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                gs_oper,
                acctserial
                    );

        memset(out, 0, sizeof(out));
        GenFormatData(caParaFile, content, out);
        fputs(out, fp);
        /*
        if (++j % 2 == 0)
        {
            //fputs("\n\n\n\n\n\n\n\n\n\n\n", fp);
            fputs("\f", fp);
        }
        */
        fputs("\n", fp);

        if (*printinfo2 != 0x00)
        {
            snprintf(content, sizeof(content),
                    "%s;%s;%s;第%s场;%s;%s%s;%s;%s;%s;%s;%s %s;%s;%s;"
                    //"%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%s;%s;",
                    "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;",
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
                    "接上页", 
                    db_cell_by_name(&rs, i, "curcode"),
                    "接上页",
                    db_cell_by_name(&rs, i, "purpose"),
                    db_cell_by_name(&rs, i, "agreement"),
                    printinfo2,
                    datetime,
                    atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                    gs_oper,
                    acctserial
                        );
            memset(out, 0, sizeof(out));
            GenFormatData(caParaFile, content, out);
            fputs(out, fp);
            if (++j % 2 == 0)
                fputs("\f", fp);
            fputs("\n", fp);
        }
    }

    db_free_result(&rs);
    fclose(fp);

    if (i > 0)
        db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);

    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}
