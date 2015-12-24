#include "comm.h"
#include "interface.h"
#include "chinese.h"
#include "udb.h"
#include "errcode.h"
#include "Public.h"

#define PS_MAXROWS 4          // 附言最大行数
#define PS_MAXCOLS 90        // 附言最大列数

#define GetTrnCtl(a) XmlGetStringDup(xmlReq, "/UFTP/TrnCtl/"a)
#define MAX_SEQUENCE_ID 999999999
#define TMP_T_RS "#tmp_t_rs"

extern char gs_originator[13];
extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq);
extern char *GetTmpFileName(char *);
extern char *FormatMoney(char *str);

extern int ChkPresAgent(char *orgid, char *presproxy);
extern int ChkAcptAgent(char *orgid, char *acptproxy);

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
    int  i, j, pos;
    char FieldList[][30] = {
        "opOppBank", 
        "opOppBankName", 
        "opOppBankAddr", 
        "opOppAcctId", 
        "opOppCustomer",
        "OpOppCustAddr",
        "opOriginAcct",
        "opOriginCustName",
        "opInterBank",
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

    xmlBuf = (char *)encoding_conv(data, "GB18030", "UTF-8");
    if (xmlBuf == NULL)
        return 0;
    doc = xmlParseMemory(xmlBuf, strlen(xmlBuf));
    if (doc == NULL)
        return 0;

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

// 提入补充凭证打印
int PrintInNoteAddNew( xmlDocPtr xmlReq, char *filename )
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
    long startrefid;
    long endrefid;
    double totamt = (double)0;
    char amount[128], littl_amt[30];
    char buf[256];
    char datetime[30];
    char notetype_name[61];
    char ps[PS_MAXROWS][256];
    char key[200];
    char *p;
    char content[4096];
    result_set rs;
    result_set ex;
    FILE *fp=NULL;
    int rc = 0;
    int iErrCode;
    int i;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
            {
                return E_OTHER;
            }
        }
    }
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteAddNew.para", getenv("HOME"), TCOP_BANKID );
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
    // workdate --> cleardate
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND cleardate = '%s' AND classid = %d"
            "  AND refid BETWEEN '%ld' AND '%ld' AND %s "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            "  AND result in(%d, %d, %d)",
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("SvcClass")),
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

    GetDateTime(datetime);
    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&rs); i++)
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

        memset(ps, 0, sizeof(ps));
        sprintf(content, "<?xml version=\"1.0\" encoding=\"GB18030\"?>%s",
                db_cell_by_name(&rs, i, "extraxml"));
        GetExtraData(content, ps);
        /*
           if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
           {
           rc = db_query(&ex, "select extradata from %s "
           "where workdate='%s' and seqno=%s", tbname_ex,
           db_cell_by_name(&rs, i, "workdate"), 
           db_cell_by_name(&rs, i, "seqno"));
           if (rc == 0)
           {
           GetExtraData(db_cell(&ex, 0, 0), ps);
           db_free_result(&ex);
           }
           else
           {
           rc = db_query(&ex, "select extradata from %s "
           "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
           db_cell_by_name(&rs, i, "workdate"), 
           db_cell_by_name(&rs,i,"seqno"));
           if (rc == 0)
           {
           GetExtraData(db_cell(&ex, 0, 0), ps);
           db_free_result(&ex);
           }
           }
           }
         */

        db_query_str(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where nodeid=%d and notetype='%s'"
                " and dcflag='%s'", OP_REGIONID, 
                db_cell_by_name(&rs, i, "notetype"), 
                db_cell_by_name(&rs, i, "dcflag"));
        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));
        fprintf(fp, "%s (%s);%s;%s;第%s场;%s;%s%s;%s;%s;%s;%s;%s %s;%s;%s;"
                "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;\n",
                db_cell_by_name(&rs, i, "originator"),
                db_cell_by_name(&rs, i, "termid"),
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
                db_cell_by_name(&rs, i, "refid"));
    }

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    db_free_result(&rs);
    fclose(fp);

    GetTmpFileName(caOutFile);
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        return E_OTHER;
    }

    db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);

    /*sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
      XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);*/
    sprintf(filename, "%s", basename(caOutFile));

    return rc;
}

int PrintInNoteAdd( xmlDocPtr xmlReq, char *filename )
{
    result_set rs, ex;
    char tbname[128], tbname_ex[128];
    char condi[4096];
    char tmp[256];
    char caParaFile[256], caDataFile[256], caOutFile[256];
    double totamt = (double)0;
    int year, month, day;
    char pres_name1[31], acpt_name1[31];
    char pres_name2[31], acpt_name2[31];
    char payer1[41], payer2[41];
    char bene1[41], bene2[41];
    char pbank[81], bbank[81];
    long startrefid, endrefid;
    char amount[128], littl_amt[30];
    char buf[256];
    char notetype_name[61];
    char content[4096];
    char ps[PS_MAXROWS][256];
    char key[200];
    FILE *fp=NULL;
    int rc = 0;
    int iErrCode;
    int i;

    // 使用新格式补充凭证
    if (*GetTrnCtl("NewRptPara") == '1')
        return PrintInNoteAddNew(xmlReq, filename);

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
            {
                return E_OTHER;
            }
        }
    }
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteAdd.para", getenv("HOME"), TCOP_BANKID );
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
    // workdate --> cleardate
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND cleardate='%s' AND classid = %d"
            " AND refid BETWEEN '%ld' AND '%ld' AND %s "
            " AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            " AND beneacct LIKE '%s%%' %s"
            " AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            " AND result in(%d, %d, %d)",
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("SvcClass")),
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

    year = atoi(GetTrnCtl("ClearDate"))/10000;
    month = atoi(GetTrnCtl("ClearDate")) % 10000 / 100;
    day = atoi(GetTrnCtl("ClearDate")) % 100;

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }
    fprintf(fp, "\n\n");
    fclose(fp); fp = NULL;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if ((fp = fopen(caDataFile, "w")) == NULL)
        {
            db_free_result(&rs);
            return E_OTHER;
        }
        WriteRptHeader(fp, "");
        // 密押字段
        sprintf(key, "%s", db_cell_by_name(&rs, i, "paykey"));

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "result"))) != 0)
        {
            strcat(key, "  ");
            //strcat(key, errmsg(atoi(db_cell_by_name(&rs, i, "result"))));
        }

        // 大写金额
        totamt += atof(db_cell_by_name(&rs, i, "settlamt"));
        MoneyToChinese(db_cell_by_name(&rs, i, "settlamt"), amount);
        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));
        memset(ps, 0, sizeof(ps));
        sprintf(content, "<?xml version=\"1.0\" encoding=\"GB18030\"?>%s",
                db_cell_by_name(&rs, i, "extraxml"));
        GetExtraData(content, ps);
        /*
           if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
           {
           rc = db_query(&ex, "select extradata from %s "
           "where workdate='%s' and seqno=%s", tbname_ex,
           db_cell_by_name(&rs, i, "workdate"), 
           db_cell_by_name(&rs, i, "seqno"));
           if (rc == 0)
           {
           GetExtraData(db_cell(&ex, 0, 0), ps);
           db_free_result(&ex);
           }
           else
           {
           rc = db_query(&ex, "select extradata from %s "
           "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
           db_cell_by_name(&rs, i, "workdate"), 
           db_cell_by_name(&rs,i,"seqno"));
           if (rc == 0)
           {
           GetExtraData(db_cell(&ex, 0, 0), ps);
           db_free_result(&ex);
           }
           }
           }
         */

        org_name(db_cell_by_name(&rs, i, "originator"), tmp);
        split_2str(tmp, 30, pres_name1, pres_name2);
        org_name(db_cell_by_name(&rs, i, "acceptor"), tmp);
        split_2str(tmp, 30, acpt_name1, acpt_name2);
        split_2str(db_cell_by_name(&rs, i, "payer"), 40, payer1, payer2);
        split_2str(db_cell_by_name(&rs, i, "benename"), 40, bene1, bene2);
        db_query_str(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where notetype='%s' and dcflag='%s'",
                db_cell_by_name(&rs, i, "notetype"), 
                db_cell_by_name(&rs, i, "dcflag"));
        fprintf(fp, "%s;%04d;%02d;%02d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;"
                "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;%s;%d;%s;%s;%s;%s;\n",
                db_cell_by_name(&rs, i, "refid"), year, month, day,
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "originator"), 
                pres_name1, pres_name2,
                db_cell_by_name(&rs, i, "acceptor"),
                acpt_name1, acpt_name2,
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "beneacct"),
                payer1, payer2, bene1, bene2,
                amount, 
                //ChsName(curcode_list, db_cell_by_name(&rs,i,"curcode")),
                db_cell_by_name(&rs, i, "curcode"),
                littl_amt, db_cell_by_name(&rs, i, "purpose"),
                key, db_cell_by_name(&rs, i, "testkey"),
                notetype_name,
                (atoi(db_cell_by_name(&rs, i, "trncode")) \
                 == 7 ? "退票" : ""),
                org_name(db_cell_by_name(&rs, i, "payingbank"), pbank),
                org_name(db_cell_by_name(&rs, i, "benebank"), bbank),
                gs_oper,
                atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                db_cell_by_name(&rs, i, "refid"), 
                db_cell_by_name(&rs, i, "acceptor"),
                ps[0], ps[1]);
        WriteRptRowCount(fp, 1);
        WriteRptFooter(fp, "");
        fclose(fp); fp = NULL;

        rc = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (rc != 0)
        {
            db_free_result(&rs);
            return E_OTHER;
        }
        if ((i+1) % 3 != 0)
        {
            if ((fp = fopen(caOutFile, "a")) == NULL)
            {
                db_free_result(&rs);
                return E_OTHER;
            }
            fprintf(fp, "\n\n\n\n\n");
            fclose(fp); fp = NULL;
        }
        else
        {
            if ((fp = fopen(caOutFile, "a")) == NULL)
            {
                db_free_result(&rs);
                return E_OTHER;
            }
            fprintf(fp, "\f\n\n");
            fclose(fp); fp = NULL;
        }
    }
    db_free_result(&rs);

    db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);

    /*sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
      XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);*/
    sprintf(filename, "%s", basename(caOutFile));

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
    int iDCount, iCCount;
    int itotDCount, itotCCount;
    int rc = 0;
    int i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));
        if (strcmp(GetTrnCtl("Originator"), gs_originator))
        {
            if (ChkPresAgent(GetTrnCtl("Originator"), gs_originator))
            {
                //SetError(E_ORG_PERMIT);
                return E_OTHER;
            }
        }
    }
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutNoteTotal.para", getenv("HOME"), TCOP_BANKID);

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

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
    // workdate,workround --> cleardate,clearround
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround='%d'"
            " AND classid = %d AND clearstate in ('1', 'C')"
            " AND %s AND truncflag LIKE '%s%%'", tbname, RECONREC_PRES, 
            GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")), 
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
                    GetTrnCtl("ClearRound"), db_cell(&curlist,i,0), truncname);

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
    int iDCount, iCCount;
    int itotDCount, itotCCount;
    int rc = 0;
    int i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
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

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
            {
                //SetError(E_ORG_PERMIT);
                return E_OTHER;
            }
        }
    }

    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNoteTotal.para", getenv("HOME"), TCOP_BANKID);

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

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
    // workdate, workround --> cleardate, clearround
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround='%d'"
            " AND classid = %d AND clearstate in"
            " ('1', 'C') AND %s AND truncflag LIKE '%s%%'", tbname,
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")), 
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
                    GetTrnCtl("ClearRound"), db_cell(&curlist,i,0), truncname);

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
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[128];
    char tbname[81];
    char bankname[81];
    char printtime[30];
    char notetype_name[61];
    //char PayingAcctOrName[81],BeneAcctOrName[81];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iSum, tot, alltot;
    double dSumAmt;
    int rc = 0;
    int h, i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel='2')", gs_originator);
    else
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));
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
    // workdate, workround --> cleardate, clearround
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            "where inoutflag = '%c' AND cleardate='%s' AND clearround='%d'"
            " AND classid = %d AND clearstate in ('1', 'C') AND %s"
            " AND dcflag ='%s' AND truncflag like '%s%%'", tbname,
            RECONREC_PRES, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")),
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

            WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;",
                    gs_sysname,
                    GetChineseName(dcflag_list, atoi(GetTrnCtl("DCFlag"))),
                    chinesedate,
                    GetTrnCtl("ClearRound"),
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
                        "benename, settlamt, truncflag, refid, workdate "
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
                            GetTrnCtl("DCFlag"));
                    sprintf(buf, "%02d/%02d", db_cell_i(&rs,k,9)%10000/100,
                            db_cell_i(&rs,k,9) % 100);
                    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                            db_cell(&banklist, j, 0), notetype_name, 
                            db_cell(&rs, k, 1),
                            db_cell(&rs, k, 2), db_cell(&rs, k, 3),
                            db_cell(&rs, k, 4), db_cell(&rs, k, 5),
                            FormatMoney(db_cell(&rs, k, 6)),
                            (db_cell_i(&rs, k, 7) == 0 ? "不截留" : "截留"),
                            db_cell(&rs, k, 8), buf);

                    ltot += 1; ltotamt += db_cell_d(&rs, k, 6);
                    iSum += 1; dSumAmt += db_cell_d(&rs, k, 6);
                    tot += 1;
                }
                db_free_result(&rs);
                sprintf(buf, "%.2lf", ltotamt);
                fprintf(fp, ";;小计笔数:%11d;;;;                      "
                        "小计金额:;;;%s;\n", ltot, FormatMoney(buf));
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
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[128];
    char tbname[81];
    char bankname[81];
    char printtime[30];
    char notetype_name[61];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iSum, tot, alltot;
    double dSumAmt;
    int rc = 0;
    int h, i, j, k;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

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
        if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
            {
                //SetError(E_ORG_PERMIT);
                return E_OTHER;
            }
        }
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
    // workdate, workround --> cleardate, clearround
    rc = db_exec("insert into " TMP_T_RS " select * from %s "
            " WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround='%d'"
            " AND classid = %d AND clearstate in('1', 'C') AND %s "
            "AND dcflag = '%s' AND truncflag like '%s%%'", 
            tbname, RECONREC_ACPT, GetTrnCtl("ClearDate"), 
            atoi(GetTrnCtl("ClearRound")), atoi(GetTrnCtl("SvcClass")), buf, 
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
                    (atoi(GetTrnCtl("DCFlag")) == PART_DEBIT ? "借方" : "贷方"),
                    chinesedate, GetTrnCtl("ClearRound"), 
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
                        "benename, settlamt, truncflag,refid,workdate FROM "
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
                            GetTrnCtl("DCFlag"));
                    sprintf(buf, "%02d/%02d", db_cell_i(&rs,k,9)%10000/100,
                            db_cell_i(&rs,k,9) % 100);
                    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                            db_cell(&banklist, j, 0), notetype_name, 
                            db_cell(&rs, k, 1),
                            db_cell(&rs, k, 2), db_cell(&rs, k, 3),
                            db_cell(&rs, k, 4), db_cell(&rs, k, 5),
                            FormatMoney(db_cell(&rs, k, 6)),
                            (db_cell_i(&rs, k, 7) == 0 ? "不截留" : "截留"),
                            db_cell(&rs, k, 8), buf);

                    ltot += 1; ltotamt += db_cell_d(&rs, k, 6);
                    iSum += 1; dSumAmt += db_cell_d(&rs, k, 6);
                    tot += 1;
                }
                db_free_result(&rs);
                sprintf(buf, "%.2lf", ltotamt);
                fprintf(fp, ";;小计笔数:%11d;;;;                      "
                        "小计金额:;;;%s;\n", ltot, FormatMoney(buf));
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
    FILE *fp=NULL;
    int outroute, inroute;
    int i, j, tot = 0;
    int rc;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")) || 
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")))
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

    snprintf(condi, sizeof(condi), "presbank='%s' and acptbank like '%s%%' "
            "and exchgdate='%s' and exchground=%d and type='1'",
            gs_originator, GetTrnCtl("Acceptor"),
            GetTrnCtl("ExchgDate"), atoi(GetTrnCtl("ExchgRound")));
    if (db_query(&banklist, "SELECT distinct acptbank FROM baginfo "
                "WHERE %s order by acptbank", condi) != 0)
        goto err_handle;

    outroute = GetExchgRoute(GetTrnCtl("ExchArea"), gs_originator);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&banklist);
        rc = ERR_APP;
        goto err_handle;
    }
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&banklist); i++)
    {
        if (db_query(&rs, "SELECT type,num,debitnum,debitamount,creditnum,"
                    "creditamount FROM baginfo WHERE acptbank='%s' and %s",
                    db_cell(&banklist, i, 0), condi) != 0)
        {
            db_free_result(&banklist);
            fclose(fp);
            goto err_handle;
        }
        org_name(db_cell(&banklist, i, 0), acptname);
        inroute=GetExchgRoute(GetTrnCtl("ExchArea"), db_cell(&banklist,i,0));
        for (j = 0; j < db_row_count(&rs); j++)
        {
            fprintf(fp,
                    "(%s)%s;%s;第 %s 场;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%d;\n",
                    areaname, gs_sysname, chinesedate, GetTrnCtl("ExchgRound"),
                    printtime, db_cell(&banklist, i, 0), acptname,
                    gs_originator, gs_bankname, db_cell(&rs, j, 2), 
                    db_cell(&rs, j, 3), db_cell(&rs, j, 4), db_cell(&rs, j, 5),
                    gs_oper, outroute, inroute);
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
    return 0;
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
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")))
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
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
    WriteRptHeader(fp, "(%s)%s;%s;%s;%d;%s;%s;%s;", areaname, gs_sysname, 
            chinesedate, GetTrnCtl("ExchgRound"), routeid,
            gs_originator, gs_bankname, printtime);

    snprintf(condi, sizeof(condi), "presbank='%s' and exchgdate='%s' and "
            "exchground=%d", gs_originator, GetTrnCtl("ExchgDate"), 
            atoi(GetTrnCtl("ExchgRound")));
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

    WriteRptRowCount(fp, i);
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
            !IsNonEmptyStr(GetTrnCtl("ExchgRound")))
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
            "exchground=%d", gs_originator, GetTrnCtl("ExchgDate"), 
            atoi(GetTrnCtl("ExchgRound")));
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

    WriteRptRowCount(fp, i);
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

    classid = atoi(GetTrnCtl("SvcClass"));
    clearround = atoi(GetTrnCtl("ClearRound"));

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
            "and cleardate='%s' and clearround=%d "
            "group by curcode, curtype ORDER BY curcode, curtype",
            classid, gs_originator, GetTrnCtl("ClearDate"), clearround);
    if ( rc != 0 )
        return rc;

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

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
    clearround = atoi(GetTrnCtl("ClearRound"));
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
            "WHERE svcclass=%d AND %s AND cleardate='%s' AND clearround=%d "
            "group by branchid, curcode, curtype "
            "ORDER BY branchid, curcode, curtype",
            classid, condi, GetTrnCtl("ClearDate"), clearround);
    if ( rc != 0 )
        return rc;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/DiffNote.para", getenv("HOME"), TCOP_BANKID);

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));
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

