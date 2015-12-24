/*********************************************
  PrintSvr.c
  前置机打印服务
  Chen Jie, May 2007
 *********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pubdef.h"
#include "pubfunc.h"
#include "XMLFunc.h"
#include "commbuff.h"
#include "errcode.h"
#include "dblib.h"
#include "tcpapi.h"
#include "trncode.h"
#include "printer.h"
#include "chinese.h"
#include "sequence.h"
#include <libgen.h>

#define PS_MAXROWS 4          // 附言最大行数
#define PS_MAXCOLS 90        // 附言最大列数

extern int PutExchgData(xmlDocPtr xmlReq, xmlDocPtr xmlRsp);
extern int CheckGB18030(char *buf, int len);
extern char* DateS2L(char *sDate);

extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq, xmlDocPtr xmlRsp);

extern int ChkPresAgent(char *orgid, char *presproxy);
extern int ChkAcptAgent(char *orgid, char *acptproxy);
// 打印提出计数单-PrintOutJSD.c
extern int PrintOutJSD(xmlDocPtr xmlReq, xmlDocPtr xmlRsp);
extern int PrintOutNoteJJQD(xmlDocPtr xmlReq, xmlDocPtr xmlRsp);

extern int GetExchgRoute(char *areacode, char *bankid);

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
        "OppBank", 
        "OppBankName", 
        "OppBankAddr", 
        "OppAcctId", 
        "OppCustomer",
        "OppCustAddr",
        "OriginAcct",
        "OriginCustName",
        "InterBank",
        "ChargesBearer",
        "SenderProxy",
        "RecverProxy",
        "PrintFlag",
        "TrnDetail",
        "PS",
        "SenderPS",
        "ValueDate",
        "ExchRate",
        "TrackInfo"
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
int PrintInNoteAddNew( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    result_set rs;
    result_set ex;
    FILE *fp=NULL;
    int iRet = 0;
    int iErrCode;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
    {
        strcpy(tbname, HDDB_TRNJOUR);
        strcpy(tbname_ex, HDDB_EXTRADATA);
    }
    else
    {
        strcpy(tbname, "trnjour");
        strcpy(tbname_ex, "extradata");
    }

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
    }
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteAddNew.para", getenv("HOME") );
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
            "  AND seqno BETWEEN %ld AND %ld AND %s "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            "  AND errcode in('%04d', '%04d', '%04d')",
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("SvcClass")),
            startrefid, endrefid, buf,
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount, GetTrnCtl("DCFlag"), tmp,
            E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
    iRet = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, seqno", tbname, condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提入补充凭证失败, condi=[%s][%s]", 
                __FILE__, __LINE__, tbname, condi );
        return -1;
    }

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    /*
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        SetError(E_GNR_FILE_OPEN);
        db_free_result(&rs);
        return -1;
    }
    fprintf(fp, "\n\n\n");
    fclose(fp); fp = NULL;
    */

    GetDateTime(datetime);
    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
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

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "errcode"))) != 0)
        {
            strcat(key, "  ");
            strcat(key, errmsg(atoi(db_cell_by_name(&rs, i, "errcode"))));
        }

        // 大写金额
        totamt += atof(db_cell_by_name(&rs, i, "settlamt"));
        MoneyToChinese(db_cell_by_name(&rs, i, "settlamt"), amount);

        memset(ps, 0, sizeof(ps));
        if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    db_cell_by_name(&rs, i, "workdate"), 
                    db_cell_by_name(&rs, i, "seqno"));
            if (iRet == 0)
            {
                GetExtraData(db_cell(&ex, 0, 0), ps);
                db_free_result(&ex);
            }
            else
            {
                SetError(0);
                iRet = db_query(&ex, "select extradata from %s "
                        "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
                        db_cell_by_name(&rs, i, "workdate"), 
                        db_cell_by_name(&rs,i,"seqno"));
                if (iRet == 0)
                {
                    GetExtraData(db_cell(&ex, 0, 0), ps);
                    db_free_result(&ex);
                }
                else
                    SetError(0);
            }
        }

        sql_result(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where notetype='%s' and dcflag='%s'",
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
                (atoi(db_cell_by_name(&rs, i, "trncode")) == T_TRAN_REFUND ? "退票" : ""),
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
                db_cell_by_name(&rs, i, "seqno"));
    }

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    db_free_result(&rs);
    fclose(fp);

    GetTmpFileName(caOutFile);
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);
    SetError(0);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

    return iRet;
}

int PrintInNoteAdd( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    char ps[PS_MAXROWS][256];
    char key[200];
    FILE *fp=NULL;
    int iRet = 0;
    int iErrCode;
    int i;

    // 使用新格式补充凭证
    if (*GetTrnCtl("NewRptPara") == '1')
        return PrintInNoteAddNew(xmlReq, xmlRsp);

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
    {
        strcpy(tbname, HDDB_TRNJOUR);
        strcpy(tbname_ex, HDDB_EXTRADATA);
    }
    else
    {
        strcpy(tbname, "trnjour");
        strcpy(tbname_ex, "extradata");
    }

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
    }
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteAdd.para", getenv("HOME") );
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
            "  AND seqno BETWEEN %ld AND %ld AND %s "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag='1' %s"
            "  AND errcode in('%04d', '%04d', '%04d')",
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("SvcClass")),
            startrefid, endrefid, buf,
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount, GetTrnCtl("DCFlag"), tmp,
            E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);
    iRet = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, seqno", tbname, condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提入补充凭证失败, condi=[%s][%s]", 
                __FILE__, __LINE__, tbname, condi );
        return -1;
    }

    year = atoi(GetTrnCtl("ClearDate"))/10000;
    month = atoi(GetTrnCtl("ClearDate")) % 10000 / 100;
    day = atoi(GetTrnCtl("ClearDate")) % 100;

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.add",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        SetError(E_GNR_FILE_OPEN);
        db_free_result(&rs);
        return -1;
    }
    fprintf(fp, "\n\n");
    fclose(fp); fp = NULL;
#if 0
    if ((fp = fopen(caOutFile, "w")) == NULL)
    {
        SetError(E_GNR_FILE_OPEN);
        return -1;
    }
    set_print_handle(fp);
    set_printer("init_printer", NULL, 0);
    /*
       ctl[0] = 22;
       set_printer("set_page_byline", ctl, 1);
       ctl[0] = 1; ctl[1] = 0; ctl[2] = 2; ctl[3] = 0;
       set_printer("set_page_format", ctl, 4);
       */
    set_printer("unset_page_slot", NULL, 0);
    /*
       set_printer("set_line_8_1", NULL, 0);
       set_printer("set_chinese", NULL, 0);
       */
    //ctl[0] = 0x00; ctl[1] = 0x01;
    //set_printer("set_col_spacing", ctl, 2);
    fclose(fp); fp = NULL;
#endif
    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if ((fp = fopen(caDataFile, "w")) == NULL)
        {
            db_free_result(&rs);
            SetError(E_GNR_FILE_OPEN);
            return -1;
        }
        WriteRptHeader(fp, "");
        // 密押字段
        sprintf(key, "%s", db_cell_by_name(&rs, i, "paykey"));

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "errcode"))) != 0)
        {
            strcat(key, "  ");
            strcat(key, errmsg(atoi(db_cell_by_name(&rs, i, "errcode"))));
        }

        // 大写金额
        totamt += atof(db_cell_by_name(&rs, i, "settlamt"));
        MoneyToChinese(db_cell_by_name(&rs, i, "settlamt"), amount);
        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "settlamt")));
        memset(ps, 0, sizeof(ps));
        if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    db_cell_by_name(&rs, i, "workdate"), 
                    db_cell_by_name(&rs, i, "seqno"));
            if (iRet == 0)
            {
                GetExtraData(db_cell(&ex, 0, 0), ps);
                db_free_result(&ex);
            }
            else
            {
                SetError(0);
                iRet = db_query(&ex, "select extradata from %s "
                        "where workdate='%s' and seqno=%s", HDDB_EXTRADATA,
                        db_cell_by_name(&rs, i, "workdate"), 
                        db_cell_by_name(&rs,i,"seqno"));
                if (iRet == 0)
                {
                    GetExtraData(db_cell(&ex, 0, 0), ps);
                    db_free_result(&ex);
                }
                else
                    SetError(0);
            }
        }

        org_name(db_cell_by_name(&rs, i, "originator"), tmp);
        split_2str(tmp, 30, pres_name1, pres_name2);
        org_name(db_cell_by_name(&rs, i, "acceptor"), tmp);
        split_2str(tmp, 30, acpt_name1, acpt_name2);
        split_2str(db_cell_by_name(&rs, i, "payer"), 40, payer1, payer2);
        split_2str(db_cell_by_name(&rs, i, "benename"), 40, bene1, bene2);
        sql_result(notetype_name, sizeof(notetype_name),
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
                 == T_TRAN_REFUND ? "退票" : ""),
                org_name(db_cell_by_name(&rs, i, "payingbank"), pbank),
                org_name(db_cell_by_name(&rs, i, "benebank"), bbank),
                gs_oper,
                atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                db_cell_by_name(&rs, i, "seqno"), 
                db_cell_by_name(&rs, i, "acceptor"),
                ps[0], ps[1]);
        WriteRptRowCount(fp, 1);
        WriteRptFooter(fp, "");
        fclose(fp); fp = NULL;

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(&rs);
            return -1;
        }
        if ((i+1) % 3 != 0)
        {
            if ((fp = fopen(caOutFile, "a")) == NULL)
            {
                SetError(E_GNR_FILE_OPEN);
                db_free_result(&rs);
                return -1;
            }
            fprintf(fp, "\n\n\n\n\n");
            fclose(fp); fp = NULL;
        }
        else
        {
            if ((fp = fopen(caOutFile, "a")) == NULL)
            {
                SetError(E_GNR_FILE_OPEN);
                db_free_result(&rs);
                return -1;
            }
            fprintf(fp, "\f\n\n");
            fclose(fp); fp = NULL;
        }
    }
    db_free_result(&rs);

    /*
       if ((fp = fopen(caOutFile, "a")) == NULL)
       {
       SetError(E_GNR_FILE_OPEN);
       return -1;
       }
       set_print_handle(fp);
       set_printer("init_printer", NULL, 0);
       fclose(fp); fp = NULL;
       */

    db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);
    SetError(0);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    sprintf(tmp, "本次打印: %d 笔, 合计金额: %.2lf", i, totamt);
    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/AppendInfo", tmp);
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

    return iRet;
}

// 前置打印补充凭证:提出借记回执
int PrintOutNoteAdd( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    char tbname[128];
    char tbname_ex[128];
    char condi[4096];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char pbank[13];
    char bbank[13];
    char startrefid[17];
    char endrefid[17];
    char amount[128];
    char buf[256];
    char notetype_name[61];
    char datetime[30];
    char ps[PS_MAXROWS][256];
    char key[200];
    char *p;
    result_set rs;
    result_set ex;
    FILE *fp=NULL;
    int iRet = 0;
    int iErrCode;
    int i;

    strcpy(startrefid, GetTrnCtl("StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0000000000");
    strcpy(endrefid, GetTrnCtl("EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "9999999999");

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
    {
        strcpy(tbname, HDDB_TRNJOUR);
        strcpy(tbname_ex, HDDB_EXTRADATA);
    }
    else
    {
        strcpy(tbname, "trnjour");
        strcpy(tbname_ex, "extradata");
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteAdd.para", getenv("HOME") );


    memset(buf, 0, sizeof(buf));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(buf, "and printnum=0");
            break;
        case '1':
            strcpy(buf, "and printnum>0");
            break;
        case '2':
        default:
            break;
    }
    memset(amount, 0, sizeof(amount));
    if (*GetTrnCtl("MaxAmount") != 0x00)
        sprintf(amount, "AND settlamt = %s", GetTrnCtl("MaxAmount"));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND cleardate = '%s'"
            "  AND classid = %d AND dcflag = '%d'"
            "  AND refid BETWEEN '%s' AND '%s' "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND truncflag LIKE '%s%%' %s"
            "  AND errcode ='0000'",
            RECONREC_PRES, GetTrnCtl("ClearDate"),
            atoi(GetTrnCtl("SvcClass")), PART_DEBIT,
            startrefid, endrefid, 
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount,
            GetTrnCtl("TruncFlag"), buf);

    iRet = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, seqno", tbname, condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提出借记失败, condi=[%s][%s]", 
                __FILE__, __LINE__, tbname, condi );
        return -1;
    }

    // 打印时间
    GetDateTime(datetime);

    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        db_free_result(&rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
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

        if ((iErrCode = atoi(db_cell_by_name(&rs, i, "errcode"))) != 0)
        {
            strcat(key, "  ");
            strcat(key, errmsg(atoi(db_cell_by_name(&rs, i, "errcode"))));
        }

        // 大写金额
        MoneyToChinese(db_cell_by_name(&rs, i, "settlamt"), buf);

        memset(ps, 0, sizeof(ps));
        if (atoi(db_cell_by_name(&rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    db_cell_by_name(&rs, i, "workdate"), 
                    db_cell_by_name(&rs, i, "seqno"));
            if (iRet == 0)
            {
                GetExtraData(db_cell(&ex, 0, 0), ps);
                db_free_result(&ex);
            }
            else
                SetError(0);
        }

        sql_result(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where notetype='%s' and dcflag='%s'",
                db_cell_by_name(&rs, i, "notetype"),
                db_cell_by_name(&rs, i, "dcflag"));
        fprintf(fp, "%s (%s);%s;%s;第%s场;%s;%s;%s %s;%s;%s;%s;%s %s;%s;%s;"
                "%s %s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;\n",
                db_cell_by_name(&rs, i, "originator"),
                db_cell_by_name(&rs, i, "termid"),
                db_cell_by_name(&rs, i, "acceptor"),
                db_cell_by_name(&rs, i, "workdate"),
                db_cell_by_name(&rs, i, "workround"),
                db_cell_by_name(&rs, i, "refid"),
                notetype_name,
                (atoi(db_cell_by_name(&rs, i, "trncode")) \
                 == T_TRAN_REFUND ? "退票" : ""),
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
                buf, 
                db_cell_by_name(&rs, i, "curcode"),
                FormatMoney(db_cell_by_name(&rs, i, "settlamt")), 
                db_cell_by_name(&rs, i, "purpose"),
                key,
                ps[0], ps[1], ps[2], ps[3],
                datetime,
                atoi(db_cell_by_name(&rs, i, "printnum"))+1,
                gs_oper);
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.oadd",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    db_exec("update %s set printnum=printnum+1 where %s", tbname, condi);
    SetError(0);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

    return iRet;
}

int PrintInQuery( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    char content[2048];
    char Originator[13];
    char Acceptor[13];
    char originame[81];
    char acptname[81];
    char WorkDate[9];
    char RefId[17];
    char NoteType[5];
    char NoteTypeName[41];
    char NoteNo[31];
    char DCFlag[3];
    char IssueDate[9];
    char SettlAmt[21];
    char PayingAcct[33];
    char Payer[81];
    char BeneAcct[33];
    char BeneName[81];
    char PayKey[30];
    char Agreement[128];
    char buf[128];
    char condi[2048];
    int iRet = 0, updflag = 1;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    sprintf(printdate, "%08ld", current_date());

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08ld", current_date());
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
            "%s/report/PrintQueryBook.para", getenv("HOME") );

    memset(buf, 0, sizeof(buf));
    switch(*GetTrnCtl("PrintState"))
    {
        case '0':
            strcpy(buf, "and readed='0'");
            updflag = 1;
            break;
        case '1':
            strcpy(buf, "and readed='1'");
            break;
        case '2':
        default:
            updflag = 1;
            break;
    }

    snprintf(condi, sizeof(condi),
            "recver='%s' AND recvdate BETWEEN '%s' AND '%s' "
            "AND mailid BETWEEN %ld AND %ld AND mailtype='%s' %s",
            GetMsgHdrRq("Originator"), startdate, enddate, 
            atol(startrefid), atol(endrefid), GetTrnCtl("MailType"), buf);
    iRet = db_query(&rs, "SELECT mailid, recver, sender, recvdate, title,"
            " content FROM recvbox WHERE %s order by recvdate, mailid", condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提入查询查复书失败",
                __FILE__, __LINE__ );
        goto err_handle;
    }

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s提入%s;%s;%s;%s;%s;", gs_sysname, 
                GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3));
        snprintf(content, sizeof(content), 
                "<?xml version=\"1.0\" encoding=\"GB18030\"?>%s",
                db_cell(&rs, i, 5));
        if ((doc = xmlParseMemory(content, strlen(content))) == NULL)
        {
            fclose(fp);
            db_free_result(&rs);
            SetError( E_TR_PRINT );
            return -1;
        }
        xmlGetVal(doc, "//Originator", Originator);
        xmlGetVal(doc, "//Acceptor", Acceptor);
        xmlGetVal(doc, "//NoteType", NoteType);
        xmlGetVal(doc, "//DCFlag", DCFlag);
        if (sql_result(NoteTypeName, sizeof(NoteTypeName), 
                    "select name from noteinfo where notetype='%s' and dcflag='%s'",
                    NoteType, DCFlag) != 0)
            SetError(0);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;",
                xmlGetVal(doc, "//WorkDate", WorkDate),
                xmlGetVal(doc, "//RefId", RefId),
                Originator,
                org_name(Originator, originame),
                Acceptor,
                org_name(Acceptor, acptname),
                NoteTypeName,
                xmlGetVal(doc, "//NoteNo", NoteNo),
                xmlGetVal(doc, "//SettlAmt", SettlAmt),
                xmlGetVal(doc, "//IssueDate", IssueDate),
                xmlGetVal(doc, "//PayingAcct", PayingAcct),
                XMLGetVal(doc, "//Payer", Payer),
                xmlGetVal(doc, "//BeneAcct", BeneAcct),
                XMLGetVal(doc, "//BeneName", BeneName),
                XMLGetVal(doc, "//PayKey", PayKey),
                XMLGetVal(doc, "//Agreement", Agreement),
                GetChineseName(clrstat_list, XmlGetInteger(doc,"//ClearState")),
                db_cell(&rs, i, 4),
                GetChineseName(dcflag_list, atoi(DCFlag)));

        fprintf(fp, "%s;%s;\n", printdate, gs_oper);

        xmlFreeDoc(doc);
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    if (updflag == 1)
        db_exec("update recvbox set readed='1' where %s", condi);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile));

err_handle:

    return iRet;
}

int PrintOutQuery( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    char content[2048];
    char Originator[13];
    char Acceptor[13];
    char originame[81];
    char acptname[81];
    char WorkDate[9];
    char RefId[17];
    char NoteType[5];
    char NoteTypeName[41];
    char NoteNo[31];
    char DCFlag[3];
    char IssueDate[9];
    char SettlAmt[21];
    char PayingAcct[33];
    char Payer[81];
    char BeneAcct[33];
    char BeneName[81];
    char PayKey[30];
    char Agreement[128];
    char condi[2048];
    char buf[128];
    int iRet = 0, updflag = 1;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    sprintf(printdate, "%08ld", current_date());

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08ld", current_date());
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
            "%s/report/PrintQueryBook.para", getenv("HOME") );

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
    snprintf(condi, sizeof(condi),
            "sender='%s' AND senddate BETWEEN '%s' AND '%s'"
            "  AND mailid BETWEEN %ld AND %ld AND mailtype='%s' %s",
            GetMsgHdrRq("Originator"), startdate, enddate, atol(startrefid), 
            atol(endrefid), GetTrnCtl("MailType"), buf);
    iRet = db_query(&rs, "SELECT mailid, recver, sender, senddate, title,"
            " content FROM sendbox WHERE %s order by senddate, mailid", condi);
    if ( iRet != 0 )
    {
        err_log("查询提出查询查复书失败");
        goto err_handle;
    }

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s提出%s;%s;%s;%s;%s;", gs_sysname, 
                GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3));
        snprintf(content, sizeof(content), 
                "<?xml version=\"1.0\" encoding=\"GB18030\"?>%s",
                db_cell(&rs, i, 5));
        if ((doc = xmlParseMemory(content, strlen(content))) == NULL)
        {
            fclose(fp);
            db_free_result(&rs);
            SetError( E_TR_PRINT );
            return -1;
        }
        xmlGetVal(doc, "//Originator", Originator);
        xmlGetVal(doc, "//Acceptor", Acceptor);
        xmlGetVal(doc, "//NoteType", NoteType);
        xmlGetVal(doc, "//DCFlag", DCFlag);
        if (sql_result(NoteTypeName, sizeof(NoteTypeName), 
                    "select name from noteinfo where notetype='%s' and "
                    "dcflag='%s'", NoteType, DCFlag) != 0)
            SetError(0);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;",
                xmlGetVal(doc, "//WorkDate", WorkDate),
                xmlGetVal(doc, "//RefId", RefId),
                Originator,
                org_name(Originator, originame),
                Acceptor,
                org_name(Acceptor, acptname),
                NoteTypeName,
                xmlGetVal(doc, "//NoteNo", NoteNo),
                xmlGetVal(doc, "//SettlAmt", SettlAmt),
                xmlGetVal(doc, "//IssueDate", IssueDate),
                xmlGetVal(doc, "//PayingAcct", PayingAcct),
                XMLGetVal(doc, "//Payer", Payer),
                xmlGetVal(doc, "//BeneAcct", BeneAcct),
                XMLGetVal(doc, "//BeneName", BeneName),
                XMLGetVal(doc, "//PayKey", PayKey),
                XMLGetVal(doc, "//Agreement", Agreement),
                GetChineseName(clrstat_list, XmlGetInteger(doc,"//ClearState")),
                db_cell(&rs, i, 4),
                GetChineseName(dcflag_list, atoi(DCFlag)));

        fprintf(fp, "%s;%s;\n", printdate, gs_oper);

        xmlFreeDoc(doc);
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }
    if (updflag == 1)
        db_exec("update sendbox set sended='2' where %s", condi);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile));

err_handle:

    return iRet;
}

int PrintInNoteJSDQD( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set banklist, rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char printtime[31];
    int ccount, dcount, t_ccount, t_dcount;
    double camt, damt, t_camt, t_damt;
    FILE *fp=NULL;
    int i, j, tot = 0;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (!IsNonEmptyStr(GetTrnCtl("WorkDate")))
    {
        SetError(E_GNR_DATA_REQUIRED);
        return -1;
    }
    gettime(printtime, sizeof(printtime), "%Y年 %m月 %d日 %H时 %M分");

    // 生成报表函数相关文件
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteJSDQD.para", getenv("HOME") );

    snprintf(condi, sizeof(condi), "acptbank='%s' and workdate='%s' and "
            "workround=%d and type='1'", GetMsgHdrRq("Originator"), 
            GetTrnCtl("WorkDate"), atoi(GetTrnCtl("WorkRound")));
    if (db_query(&banklist, "SELECT distinct presbank FROM baginfo "
                "WHERE %s order by presbank", condi) != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    t_ccount = t_dcount = 0;
    t_camt = t_damt = (double) 0;

    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&banklist);
        goto err_handle;
    }
    WriteRptHeader(fp, "%s;%s;%s;", gs_sysname, printtime, 
            GetTrnCtl("WorkRound"));
    for (i = 0; i < db_row_count(&banklist); i++)
    {
        if (db_query(&rs, "SELECT exchground,num,debitnum,debitamount,"
                    "creditnum,creditamount FROM baginfo WHERE presbank='%s'"
                    " and %s", db_cell(&banklist, i, 0), condi) != 0)
        {
            fclose(fp);
            db_free_result(&banklist);
            goto err_handle;
        }
        ccount = dcount = 0;
        camt = damt = (double) 0;
        for (j = 0; j < db_row_count(&rs); j++)
        {
            fprintf(fp, "%s;%s;%s;%s;%s;%s;\n",
                    db_cell(&banklist, i, 0), db_cell(&rs, j, 0),
                    db_cell(&rs, j, 2), db_cell(&rs, j, 3),
                    db_cell(&rs, j, 4), db_cell(&rs, j, 5));
            ccount += db_cell_i(&rs, j, 4);
            dcount += db_cell_i(&rs, j, 2);
            camt += db_cell_d(&rs, j, 5);
            damt += db_cell_d(&rs, j, 3);
            tot++;
        }
        db_free_result(&rs);
        fprintf(fp, " 小计;;%d;%.2lf;%d;%.2lf;\n", dcount, damt, ccount, camt);
        tot++;
        t_ccount += ccount;
        t_dcount += dcount;
        t_camt += camt;
        t_damt += damt;
    }
    db_free_result(&banklist);
    WriteRptRowCount(fp, tot);
    WriteRptFooter(fp, "%d;%.2lf;%d;%.2lf;%s;%s;%s;", t_dcount, t_damt, 
            t_ccount, t_camt, GetMsgHdrRq("Originator"), gs_bankname, 
            gs_oper);
    fclose(fp); fp = NULL;
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.jqd",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    if (PrintReportList(caParaFile, caDataFile, caOutFile) != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    return 0;
}

int PrintClearDiff(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *chinesedate;
    char *bankid;
    char buf[256];
    FILE *fp=NULL;
    int classid, clearround;
    int iRet = 0;
    int i;

    // 是否为清算行
    bankid = GetMsgHdrRq("Originator");
    /*
       snprintf(buf, sizeof(buf), "orgid='%s' and orglevel=1", bankid);
       if (!IsRecExist("organinfo", buf))
       {
       SetError(E_ORG_PERMIT);
       return -1;
       }
       */
    classid = atoi(GetTrnCtl("SvcClass"));
    clearround = atoi(GetTrnCtl("ClearRound"));

    iRet = db_query(&rs, "SELECT curcode, curtype, "
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
            classid, bankid, GetTrnCtl("ClearDate"), clearround);
    if ( iRet != 0 )
        return -1;

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/DiffNote.para", getenv("HOME") );
    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%s;%-30s", bankid, gs_bankname);

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
            getenv("FILES_DIR"),bankid,current_date()%100,current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

    return iRet;
}

int PrintDiffNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char bankname[81];
    char *chinesedate;
    char *bankid;
    char buf[256];
    char condi[256];
    FILE *fp=NULL;
    int classid, clearround;
    int iRet = 0;
    int printed=0, i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    // 是否为清算行
    bankid = GetMsgHdrRq("Originator");
    snprintf(buf, sizeof(buf), "orgid='%s' and orglevel=1", bankid);
    if (IsRecExist("organinfo", buf))
    {
        // 生成清算行差额
        if (PrintClearDiff(xmlReq, xmlRsp) != 0)
            return -1;
        // 仅打印清算行差额
        if (*GetTrnCtl("PrintFlag") == '1')
            return 0;
        sprintf(condi, "branchid like '%s%%'", GetTrnCtl("BankId"));
        printed = 1;
    }
    else
    {
        sprintf(condi, "branchid = '%s'", bankid);
    }

    classid = atoi(GetTrnCtl("SvcClass"));
    clearround = atoi(GetTrnCtl("ClearRound"));
    iRet = db_query(&rs, "SELECT branchid, curcode, curtype,"
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
    if ( iRet != 0 )
        return -1;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/DiffNote.para", getenv("HOME") );

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
                getenv("FILES_DIR"), gcaResponseFile);
    }
    else
    {
        snprintf( caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.dif",
                getenv("FILES_DIR"),bankid,current_date()%100,current_time());
    }
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    memset(gcaResponseFile, 0, sizeof(gcaResponseFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof(gcaResponseFile)-1);

    return iRet;
}

#if 0
int PrintOutNoteJSD( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set banklist;
    result_set rs;

    char tbname[128];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char buf[128];
    char tmp[128];
    char originame[81];
    char acptname[81];
    char *chinesedate = NULL;

    FILE *fp=NULL;
    double dDSumAmt, dCSumAmt;
    int iDCount, iCCount;
    int iRet = 0;
    int i, j, k;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteJSD.para", getenv("HOME") );

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    GetTmpFileName(caOutFile);

    sprintf(condi, "inoutflag = '%c'"
            "  AND workdate = '%s'"
            "  AND workround = %d"
            "  AND classid = %d"
            "  AND clearstate in ('1', 'C')"
            "  AND originator = '%s'"
            "  AND truncflag LIKE '%s%%'", 
            RECONREC_PRES,
            GetTrnCtl("WorkDate"),
            atoi(GetTrnCtl("WorkRound")), 
            atoi(GetTrnCtl("SvcClass")),
            GetMsgHdrRq("Originator"),
            GetTrnCtl("TruncFlag"));

    if (db_query(&banklist, 
                "SELECT DISTINCT originator,acceptor,curcode FROM %s"
                " WHERE %s AND acceptor LIKE '%s%%'", tbname, condi, 
                GetTrnCtl("Acceptor")) != 0)
    {
        return -1;
    }

    for (i = 0; i < db_row_count(&banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");
        org_name(db_cell(&banklist, i, 0), originame);
        org_name(db_cell(&banklist, i, 1), acptname);

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;%s;",
                gs_sysname,
                chinesedate,
                GetTrnCtl("WorkRound"),
                db_cell(&banklist, i, 2),
                db_cell(&banklist, i, 0), 
                originame,
                db_cell(&banklist, i, 1),
                acptname);

        iRet = db_query(&rs,
                "SELECT originator, refid, acceptor, notetype, dcflag, noteno,"
                "settlamt, beneacct, payingacct FROM %s "
                "WHERE %s AND acceptor='%s' AND curcode='%s' ORDER BY seqno",
                tbname,condi, db_cell(&banklist, i, 1), db_cell(&banklist, i, 2));
        if ( iRet != 0 )
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提出计数单失败", __FILE__, __LINE__);
            db_free_result(&banklist);
            goto err_handle;
        }

        dDSumAmt = dCSumAmt = (double)0;
        iDCount = iCCount = 0;
        for (j = 0; j < db_row_count(&rs); j++)
        {
            for (k = 0; k < db_col_count(&rs); k++)
            {
                if ( k == 6)
                {
                    if (db_cell_i(&rs, 0, 4) == PART_DEBIT)
                    {
                        dDSumAmt += db_cell_d(&rs, 0, k);
                        iDCount++;
                    }
                    else
                    {
                        dCSumAmt += db_cell_d(&rs, 0, k);
                        iCCount++;
                    }
                    strcpy(buf, db_cell(&rs, 0, k));
                    fprintf(fp, "%s;", FormatMoney(buf));
                }
                else
                {
                    fprintf(fp, "%s;", db_cell(&rs, 0, k));
                }
            }
            fprintf(fp, "\n");
        }
        db_free_result(&rs);

        WriteRptRowCount(fp, j);
        sprintf(tmp, "%.2lf", dDSumAmt);
        sprintf(buf, "%.2lf", dCSumAmt);
        WriteRptFooter(fp, "%d;%s;%d;%s;", 
                iDCount, FormatMoney(tmp), iCCount, FormatMoney(buf));
        fclose(fp);

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(&banklist);
            goto err_handle;
        }
    }
    db_free_result(&banklist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}
#endif

// 提出汇总单
int PrintOutNoteTotal( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    int iRet = 0;
    int i, j, k;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    else
    {
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));
        if (strcmp(GetTrnCtl("Originator"), GetMsgHdrRq("Originator")))
        {
            if (ChkPresAgent(GetTrnCtl("Originator"),GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
    }
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteTotal.para", getenv("HOME") );

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.otot",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());

#ifdef DB2
    iRet = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    iRet = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (iRet != 0)
        return -1;
    if (db_begin() != 0)
        return -1;
    // workdate,workround --> cleardate,clearround
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround = %d"
            " AND classid = %d AND clearstate in ('1', 'C')"
            " AND %s AND truncflag LIKE '%s%%'", tbname, RECONREC_PRES, 
            GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")), 
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("TruncFlag"));
    if (iRet != 0)
        goto err_handle;

    memset(truncname, 0, sizeof(truncname));
    if (!IsNonEmptyStr(GetTrnCtl("TruncFlag")))
        strcpy(truncname, "截留、不截留");
    else
        strcpy(truncname, (*GetTrnCtl("TruncFlag")=='0' ? "不截留" : "截留"));

    if (db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS
                " order by curcode") != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if (db_query(&orglist, "SELECT DISTINCT originator FROM " TMP_T_RS 
                    " WHERE curcode='%s' order by originator", 
                    db_cell(&curlist, i, 0)) != 0)
        {
            db_free_result(&curlist);
            goto err_handle;
        }
        for (k = 0; k < db_row_count(&orglist); k++)
        {
            if (db_query(&banklist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                        " WHERE curcode='%s' and originator='%s' "
                        "order by acceptor", db_cell(&curlist, i, 0),
                        db_cell(&orglist, k, 0)) != 0)
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
                if (db_query(&rs, "SELECT count(*)," IS_NULL 
                            "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                            "WHERE curcode='%s' AND originator='%s' "
                            "AND acceptor='%s' AND dcflag='1'", 
                            db_cell(&curlist, i, 0),
                            db_cell(&orglist, k, 0),
                            db_cell(&banklist,j,0)) != 0)
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
                if (db_query(&rs, "SELECT count(*)," IS_NULL 
                            "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                            "WHERE curcode='%s' AND originator='%s' "
                            "AND acceptor='%s' AND dcflag='2'", 
                            db_cell(&curlist, i, 0),
                            db_cell(&orglist, k, 0),
                            db_cell(&banklist,j,0)) != 0)
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

            iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (iRet != 0)
            {
                SetError( E_TR_PRINT );
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    db_commit();
    ifree(chinesedate);
    return iRet;
}

int PrintInNoteTotal(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
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
    long startrefid;
    long endrefid;
    int iRet = 0;
    int i, j, k;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");
    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    if (*GetTrnCtl("Acceptor") == 0x00)
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteTotal.para", getenv("HOME") );

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.itot",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());

#ifdef DB2
    iRet = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    iRet = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (iRet != 0)
        return -1;
    if (db_begin() != 0)
        return -1;
    // workdate, workround --> cleardate, clearround
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround = %d"
            " AND seqno BETWEEN %ld AND %ld AND classid = %d AND clearstate in"
            " ('1', 'C') AND %s AND truncflag LIKE '%s%%'", tbname,
            RECONREC_ACPT, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")), 
            startrefid, endrefid,
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("TruncFlag"));
    if (iRet != 0)
        return -1;

    memset(truncname, 0, sizeof(truncname));
    if (!IsNonEmptyStr(GetTrnCtl("TruncFlag")))
        strcpy(truncname, "截留、不截留");
    else
        strcpy(truncname, (*GetTrnCtl("TruncFlag")=='0' ? "不截留" : "截留"));

    if (db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS
                " order by curcode") != 0)
        return -1;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if (db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                    " WHERE curcode='%s' order by acceptor", 
                    db_cell(&curlist, i, 0)) != 0)
        {
            db_free_result(&curlist);
            return -1;
        }
        for (k = 0; k < db_row_count(&orglist); k++)
        {
            if (db_query(&banklist, "SELECT DISTINCT originator FROM " TMP_T_RS
                        " WHERE curcode='%s' and acceptor='%s' "
                        "order by originator", db_cell(&curlist, i, 0),
                        db_cell(&orglist, k, 0)) != 0)
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
                if (db_query(&rs, "SELECT count(*)," IS_NULL 
                            "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                            "WHERE curcode='%s' AND acceptor='%s' "
                            "AND originator='%s' AND dcflag='1'", 
                            db_cell(&curlist, i, 0),
                            db_cell(&orglist, k, 0),
                            db_cell(&banklist,j,0)) != 0)
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
                if (db_query(&rs, "SELECT count(*)," IS_NULL 
                            "(sum(settlamt),0.00) FROM " TMP_T_RS " "
                            "WHERE curcode='%s' AND acceptor='%s' "
                            "AND originator='%s' AND dcflag='2'", 
                            db_cell(&curlist, i, 0),
                            db_cell(&orglist, k, 0),
                            db_cell(&banklist,j,0)) != 0)
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

            iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (iRet != 0)
            {
                SetError( E_TR_PRINT );
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    db_commit();
    ifree(chinesedate);
    return iRet;
}

int PrintInNoteJHD( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set banklist, rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char areaname[81];
    char bankname[81];
    char condi[1024];
    char printtime[11];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int outroute, inroute;
    int i, j, tot = 0;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (!IsNonEmptyStr(GetTrnCtl("ExchgDate")))
    {
        SetError(E_GNR_DATA_REQUIRED);
        return -1;
    }
    chinesedate = ChineseDate(atol(GetTrnCtl("ExchgDate")));
    gettime(printtime, sizeof(printtime), "%H:%M:%S");

    // 生成报表函数相关文件
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteJHD.para", getenv("HOME") );
    GetTmpFileName(caDataFile);
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.ijsd",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());

    // 区域名称
    sql_result(areaname, sizeof(areaname), "select areaname from exchgarea "
            "where areacode='%s'", GetTrnCtl("ExchArea"));
    // 提入行线路号
    inroute = GetExchgRoute(GetTrnCtl("ExchArea"), GetMsgHdrRq("Originator"));

    snprintf(condi, sizeof(condi), "acptbank='%s' and exchgdate='%s' and "
            "exchground=%d and excharea='%s'", GetMsgHdrRq("Originator"), 
            GetTrnCtl("ExchgDate"), atoi(GetTrnCtl("ExchgRound")), 
            GetTrnCtl("ExchArea"));
    if (db_query(&banklist, "SELECT distinct presbank FROM baginfo "
                "WHERE %s order by presbank", condi) != 0)
        goto err_handle;
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&banklist);
        goto err_handle;
    }
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&banklist); i++)
    {
        if (db_query(&rs, "SELECT type,num,debitnum,debitamount,creditnum,"
                    "creditamount FROM baginfo WHERE presbank='%s' and %s "
                    "and type='1'", db_cell(&banklist, i, 0), condi) != 0)
        {
            db_free_result(&banklist);
            fclose(fp);
            goto err_handle;
        }
        org_name(db_cell(&banklist, i, 0), bankname);
        outroute = GetExchgRoute(GetTrnCtl("ExchArea"), db_cell(&banklist,i,0));
        for (j = 0; j < db_row_count(&rs); j++)
        {
            fprintf(fp, "(%s)%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                    areaname, gs_sysname, chinesedate, GetTrnCtl("ExchgRound"),
                    printtime, db_cell(&banklist, i, 0), bankname,
                    GetMsgHdrRq("Originator"), gs_bankname, 
                    db_cell(&rs, j, 2), db_cell(&rs, j, 3),
                    db_cell(&rs, j, 4), db_cell(&rs, j, 5), gs_oper);
            //inroute, outroute);
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
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    ifree(chinesedate);
    return 0;
}

// 提出清单
int PrintOutNoteList( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    int iRet = 0;
    int h, i, j, k;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Originator") == 0x00)
        sprintf(buf, "originator in (select orgid from organinfo "
                "where presproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    else
        sprintf(buf, "originator='%s'", GetTrnCtl("Originator"));

    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteList.para", getenv("HOME") );

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.olst",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());

#ifdef DB2
    iRet = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    iRet = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (iRet != 0)
        return -1;
    if (db_begin() != 0)
        return -1;
    // workdate, workround --> cleardate, clearround
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            "where inoutflag = '%c' AND cleardate='%s' AND clearround=%d"
            " AND classid = %d AND clearstate in ('1', 'C') AND %s"
            " AND dcflag ='%s' AND truncflag like '%s%%'", tbname,
            RECONREC_PRES, GetTrnCtl("ClearDate"), atoi(GetTrnCtl("ClearRound")),
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("DCFlag"),
            GetTrnCtl("TruncFlag"));
    if (iRet != 0)
        return -1;

    if (db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS) != 0)
        goto err_handle;

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if (db_query(&orglist, "SELECT DISTINCT originator FROM " TMP_T_RS
                    " WHERE curcode='%s' order by originator", 
                    db_cell(&curlist, i, 0)) != 0)
        {
            db_free_result(&curlist);
            return -1;
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

            if (db_query(&banklist, "SELECT distinct acceptor FROM " TMP_T_RS
                        " WHERE curcode='%s' and originator='%s' "
                        "ORDER BY acceptor", db_cell(&curlist, i, 0), 
                        db_cell(&orglist, h, 0)) != 0)
            {
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
            for (j = 0; j < db_row_count(&banklist); j++)
            {
                int ltot = 0;
                double ltotamt = (double)0;
                iRet = db_query(&rs,
                        "SELECT notetype, noteno, payingacct, payer, beneacct, "
                        "benename, settlamt, truncflag, refid, workdate "
                        "FROM " TMP_T_RS
                        " WHERE curcode='%s' and originator='%s' and "
                        "acceptor='%s' ORDER BY seqno", db_cell(&curlist,i,0),
                        db_cell(&orglist, h, 0), db_cell(&banklist, j, 0));
                if ( iRet != 0 )
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
                    sql_result(notetype_name, sizeof(notetype_name),
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

            iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (iRet != 0)
            {
                SetError( E_TR_PRINT );
                db_free_result(&curlist);
                db_free_result(&orglist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    db_commit();
    ifree(chinesedate);
    return iRet;
}

// 提入清单
int PrintInNoteList( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
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
    long startrefid;
    long endrefid;
    FILE *fp=NULL;
    int iSum, tot, alltot;
    double dSumAmt;
    int iRet = 0;
    int h, i, j, k;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    if (DiffDate(GetTrnCtl("ClearDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");
    chinesedate = ChineseDate(atol(GetTrnCtl("ClearDate")));

    if (*GetTrnCtl("StartRefId") == 0x00)
        startrefid = 0L;
    else
        startrefid = atol(GetTrnCtl("StartRefId"));
    if (*GetTrnCtl("EndRefId") == 0x00)
        endrefid = MAX_SEQUENCE_ID;
    else
        endrefid = atol(GetTrnCtl("EndRefId"));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteList.para", getenv("HOME") );
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.ilst",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());

    if (*GetTrnCtl("Acceptor") == 0x00)
    {
        sprintf(buf, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    }
    else
    {
        sprintf(buf, "acceptor='%s'", GetTrnCtl("Acceptor"));
        if (strcmp(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
    }

#ifdef DB2
    iRet = db_exec("declare global temporary table t_rs like trnjour "
            "on commit delete rows not logged");
#elif SYBASE
    iRet = db_exec("select * into #tmp_t_rs from trnjour where 1=2");
#endif
    if (iRet != 0)
        return -1;
    if (db_begin() != 0)
        return -1;
    // workdate, workround --> cleardate, clearround
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            " WHERE inoutflag = '%c' AND cleardate = '%s' AND clearround = %d"
            " AND seqno BETWEEN %ld AND %ld AND classid = %d AND clearstate in"
            " ('1', 'C') AND %s AND dcflag = '%s' AND truncflag like '%s%%'", 
            tbname, RECONREC_ACPT, GetTrnCtl("ClearDate"), 
            atoi(GetTrnCtl("ClearRound")), startrefid, endrefid,
            atoi(GetTrnCtl("SvcClass")), buf, GetTrnCtl("DCFlag"),
            GetTrnCtl("TruncFlag"));
    if (iRet != 0)
        return -1;

    if (db_query(&curlist, "SELECT DISTINCT curcode FROM " TMP_T_RS) != 0)
    {
        goto err_handle;
        return -1;
    }

    GetTmpFileName(caDataFile);
    for (i = 0; i < db_row_count(&curlist); i++)
    {
        if (db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                    " WHERE curcode='%s' order by acceptor", 
                    db_cell(&curlist, i, 0)) != 0)
        {
            db_free_result(&curlist);
            goto err_handle;
            return -1;
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

            if (db_query(&banklist, "SELECT distinct originator FROM " TMP_T_RS
                        " WHERE curcode='%s' and acceptor='%s' "
                        "ORDER BY originator", db_cell(&curlist, i, 0), 
                        db_cell(&orglist, h, 0)) != 0)
            {
                db_free_result(&curlist);
                db_free_result(&orglist);
                goto err_handle;
            }

            for (j = 0; j < db_row_count(&banklist); j++)
            {
                int ltot = 0;
                double ltotamt = (double)0;
                iRet = db_query(&rs,
                        "SELECT notetype, noteno, payingacct, payer, beneacct,"
                        "benename, settlamt, truncflag,seqno,workdate FROM "
                        TMP_T_RS " WHERE curcode='%s' and acceptor='%s' and "
                        "originator='%s' ORDER BY seqno", db_cell(&curlist,i,0),
                        db_cell(&orglist, h, 0), db_cell(&banklist, j, 0));
                if ( iRet != 0 )
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
                    sql_result(notetype_name, sizeof(notetype_name),
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

            iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
            if (iRet != 0)
            {
                SetError( E_TR_PRINT );
                db_free_result(&orglist);
                db_free_result(&curlist);
                goto err_handle;
            }
        }
        db_free_result(&orglist);
    }
    db_free_result(&curlist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    db_commit();
    ifree(chinesedate);
    return iRet;
}

int PrintOperSettle( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    result_set banklist;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char buf[256];
    char tmp[256];
    char presdate[11];
    double dDSumAmt, dCSumAmt;
    int iDCount, iCCount;
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j, k;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OperSettle.para", getenv("HOME") );

    memset(presdate, 0, sizeof(presdate));
    strcpy(presdate, GetTrnCtl("StartDate"));
    if (presdate[0] == 0)
        sprintf(presdate, "%08ld", current_date());

    chinesedate = ChineseDate(atol(presdate));

    GetTmpFileName(caOutFile);

    sprintf(condi, "presdate = '%s' AND classid = %d AND originator = '%s' "
            "AND clearstate in ('1', 'C') AND (acctoper='%s' OR auditor='%s')",
            presdate, atoi(GetTrnCtl("SvcClass")), GetMsgHdrRq("Originator"),
            gs_oper, gs_oper);
    if (db_query(&banklist, "SELECT DISTINCT curcode FROM %s WHERE %s"
                " order by curcode", HDDB_TRNJOUR, condi) != 0)
    {
        ifree(chinesedate);
        return -1;
    }

    for (i = 0; i < db_row_count(&banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname, chinesedate, 
                ChsName(curcode_list,db_cell(&banklist, i, 0)), 
                GetMsgHdrRq("Originator"), gs_bankname, gs_oper);

        iRet = db_query(&rs,
                "SELECT refid, acceptor, notetype, dcflag, noteno, "
                "payingacct, beneacct, settlamt FROM %s "
                "WHERE %s AND curcode='%s' ORDER BY seqno", 
                HDDB_TRNJOUR, condi, db_cell(&banklist, i, 0));
        if ( iRet != 0 )
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印柜员对账单失败", __FILE__, __LINE__);
            db_free_result(&banklist);
            goto err_handle;
        }

        iDCount = iCCount = 0;
        dDSumAmt = dCSumAmt = (double)0;
        for (j = 0; db_row_count(&rs); j++)
        {
            for (k = 0; k < db_col_count(&rs); k++)
            {
                if ( k == 3)
                {
                    if (db_cell_i(&rs, j, k) == PART_DEBIT)
                        fprintf(fp, "借;");
                    else if (db_cell_i(&rs, j, k) == PART_CREDIT)
                        fprintf(fp, "贷;");
                    else
                        fprintf(fp, ";");
                }
                else if ( k == 7)
                {
                    if (db_cell_i(&rs, j, 3) == PART_DEBIT)
                    {
                        dDSumAmt += db_cell_d(&rs, j, k);
                        iDCount++;
                    }
                    else
                    {
                        dCSumAmt += db_cell_d(&rs, j, k);
                        iCCount++;
                    }
                    strcpy(buf, db_cell(&rs, j, k));
                    fprintf(fp, "%s;", FormatMoney(buf));
                }
                else
                {
                    fprintf(fp, "%s;", db_cell(&rs, j, k));
                }
            }
            fprintf(fp, "\n");
        }
        db_free_result(&rs);
        WriteRptRowCount(fp, j);
        sprintf(tmp, "%.2lf", dDSumAmt);
        sprintf(buf, "%.2lf", dCSumAmt);
        WriteRptFooter(fp, "%d;%s;%d;%s;", 
                iDCount, FormatMoney(tmp), iCCount, FormatMoney(buf));
        fclose(fp); fp = NULL;

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(&banklist);
            goto err_handle;
        }
    }
    db_free_result(&banklist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

// 打印退票理由书(add by luokf 2009/09/24 in yangzhou)
int PrintTPExcuseBook( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    xmlDocPtr doc;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char inoutflag[1 + 1];   //接收标志
    char startdate[8 + 1];   //开始日期
    char enddate[8 + 1];     //结束日期
    char condi[128], printed[2];
    char originator[13], acceptor[13];
    char refid[17], dcflag[2], workdate[11];
    char notetype[3], notetype_name[41], noteno[21];
    char settlamt[31], reason[61];
    char printtime[31];
    char sender[13], sendname[81];
    char recver[13], recvname[81];
    char buf[2048];
    result_set rs;
    FILE *fp=NULL;
    int rc = 0;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    //对接收、发送的退票理由书进行处理
    XmlGetString(xmlReq, "/UFTP/TrnCtl/InOutFlag", 
            inoutflag, sizeof(inoutflag));
    //取开始日期
    XmlGetString(xmlReq, "/UFTP/TrnCtl/StartDate", 
            startdate, sizeof(startdate));
    //取结束日期
    XmlGetString(xmlReq, "/UFTP/TrnCtl/EndDate", 
            enddate, sizeof(enddate));

    //考虑开始日期或者结束日期为空的情况
    //起始日期为空(为空则当作当前日期处理)
    if (startdate[0] == 0)
        sprintf(startdate, "%08ld", current_date());
    //结束日期为空(也同样当作当前日期处理)
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    memset(condi, 0, sizeof(condi));
    XmlGetString(xmlReq, "/UFTP/TrnCtl/PrintState", printed, sizeof(printed));
    //判断是接收还是接收还是发送退票理由书 0 - 接收,1 - 发送
    if (inoutflag[0] == '0')
    {	
        if (printed[0] == '0' || printed[0] == '1')
            sprintf(condi, "AND readed='%s'", printed);
        //接收
        rc = db_query(&rs, "SELECT * FROM recvbox WHERE mailtype='5' "
                "AND recver='%s' %s AND recvdate BETWEEN '%s' AND '%s' "
                "order by recvdate desc,recvtime desc", 
                GetMsgHdrRq("Originator"), condi, startdate, enddate);
    }
    else if (inoutflag[0] == '1')
    {	
        if (printed[0] == '0' || printed[0] == '1')
            sprintf(condi, "AND sended='%c'", printed[0]+1);
        //发送	
        rc = db_query(&rs, "SELECT * FROM sendbox WHERE mailtype='5' "
                "AND sender='%s' %s AND senddate BETWEEN '%s' AND '%s' "
                "order by senddate desc,sendtime desc",
                GetMsgHdrRq("Originator"), condi, startdate, enddate);
    }
    else
    {
        SetError(E_GNR_DATAFMT);
        return -1;
    }
    //如果没有查到或者发生异常情况
    if ( rc != 0 )
    {
        err_log("没有你所要求的退票理由书.");
        return -1;
    }

    snprintf(caParaFile, sizeof(caParaFile),
            "%s/report/TPPrintExcuseBook.para", getenv("HOME"));
    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        db_free_result(&rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
    }
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");

    //循环取内容
    WriteRptHeader(fp, "");
    for (i = 0; i < db_row_count(&rs); i++)
    {
        memset(sender, 0, sizeof(sender));
        strcpy(sender, db_cell_by_name(&rs, i, "sender"));
        org_name(sender, sendname);
        memset(recver, 0, sizeof(recver));
        strcpy(recver, db_cell_by_name(&rs, i, "recver"));
        org_name(recver, recvname);

        snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding="
                "\"GB18030\"?>%s", db_cell_by_name(&rs, i, "content"));
        doc = xmlParseMemory(buf, strlen(buf));
        if (doc == NULL)
            continue;
        XmlGetString(doc, "/Content/WorkDate", workdate, sizeof(workdate));
        XmlGetString(doc, "/Content/Originator",originator, sizeof(originator));
        XmlGetString(doc, "/Content/Acceptor",acceptor, sizeof(acceptor));
        XmlGetString(doc, "/Content/RefId", refid, sizeof(refid));
        XmlGetString(doc, "/Content/DCFlag", dcflag, sizeof(dcflag));
        XmlGetString(doc, "/Content/NoteType", notetype, sizeof(notetype));
        XmlGetString(doc, "/Content/NoteNo", noteno, sizeof(noteno));
        XmlGetString(doc, "/Content/SettlAmt", settlamt, sizeof(settlamt));
        XmlGetString(doc, "/Content/Reason", reason, sizeof(reason));
        DateS2L(workdate);
        sql_result(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where notetype='%s' and dcflag='%s'",
                notetype, dcflag);
        fprintf(fp, "%s%s;%s(%s):;;%s;%s;%s;%s;%s;%s;%s;%s(%s);;%s;%s;%s;\n", 
                gs_sysname, (inoutflag[0] == '1' ? "提出":"提入"),recvname, 
                recver, workdate, notetype_name, refid, 
                noteno, FormatMoney(settlamt), (dcflag[0] =='1' ? "借记" : "贷记"), reason,
                sendname, sender, printtime, gs_oper, acceptor);

        xmlFreeDoc(doc);
    }
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    db_free_result(&rs);
    fclose(fp); fp = NULL;

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.tpr",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }
    if (inoutflag[0] == '0')
    {
        db_exec("update recvbox set readed='1' where mailtype='5' "
                "AND recver='%s' AND recvdate BETWEEN '%s' AND '%s'", 
                GetMsgHdrRq("Originator"), startdate, enddate);
    }
    else
    {
        db_exec("update sendbox set sended='2' where mailtype='5' "
                "AND sender='%s' AND senddate BETWEEN '%s' AND '%s'", 
                GetMsgHdrRq("Originator"), startdate, enddate);
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof(gcaResponseFile));

    return rc;
}

func_list uftp_call[ ] =
{
    { T_PRN_INNOTEADD,  NULL, PrintInNoteAdd,     NULL },
    { T_PRN_OUTNOTEADD, NULL, PrintOutNoteAdd,    NULL },
    { T_PRN_INQUERY,    NULL, PrintInQuery,       NULL },
    { T_PRN_OUTQUERY,   NULL, PrintOutQuery,      NULL },
    { T_PRN_EXCHGDIFF,  NULL, PrintDiffNote,      NULL },
    { T_PRN_OUTTOTAL,   NULL, PrintOutNoteTotal,  NULL },
    { T_PRN_INTOTAL,    NULL, PrintInNoteTotal,   NULL },
    //{ T_PRN_OUTJSD,     NULL, PrintOutNoteJSD,    NULL },
    { T_PRN_INJSDQD,    NULL, PrintInNoteJSDQD,   NULL },
    { T_PRN_OUTJJQD,    NULL, PrintOutNoteJJQD,   NULL },
    { T_PRN_OUTJHD,     NULL, PrintOutJSD,    NULL },
    { T_PRN_OUTLIST,    NULL, PrintOutNoteList,   NULL },
    { T_PRN_INJHD,      NULL, PrintInNoteJHD,     NULL },
    { T_PRN_INLIST,     NULL, PrintInNoteList,    NULL },
    { T_PRN_OPERSETTLE, NULL, PrintOperSettle,    NULL },
    { T_PRN_EXCUSEBOOK, NULL, PrintTPExcuseBook,  NULL },
    { -1, NULL, NULL, NULL }
};
