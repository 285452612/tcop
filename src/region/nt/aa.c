/*********************************************
  PrintSvr.c
  前置机打印服务
  Chen Jie, May 2007
 *********************************************/

#include <stdio.h>
#include <string.h>
#include "pubdef.h"
#include "pubfunc.h"
#include "XMLFunc.h"
#include "commbuff.h"
#include "errcode.h"
#include "dblib.h"
#include "tcpapi.h"
#include "trncode.h"
#include "chinese.h"
#include <libgen.h>


#define notetype_name(notetype) \
    get_tb_value("select name from noteinfo where notetype='%s'", notetype)
#define org_name(orgid) \
    get_tb_value("select orgname from organinfo where orgid='%s' and orgtype='1'", orgid)

#define PS_MAXROWS 4          // 附言最大行数
#define PS_MAXCOLS 90        // 附言最大列数

extern int CheckGB18030(char *buf, int len);
#if 0
int GetExtraData(char *data, char (*ps)[256])
{
    xmlDocPtr doc;
    char *xmlBuf = NULL;
    char path[100];
    char buf[1024];
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
    j = 0;
    for (i = 0; i < sizeof(FieldList)/sizeof(FieldList[0]); i++)
    {
        sprintf(path, "//%s", FieldList[i]);
        XMLGetVal(doc, path, field);
        if (field[0] == 0x00)
            continue;
        // 单个字段大于 PS_MAXCOLS 则换多行打印到结束为止
        if (strlen(field) > PS_MAXCOLS)
        {
            if (len > 0) // 先换行
                j++; 
            len = strlen(field);
            pos = 0;
            while (len > PS_MAXCOLS)
            {
                if (j > PS_MAXROWS)
                {
                    xmlFreeDoc(doc);
                    free(xmlBuf);
                    return 0;
                }
                if (CheckGB18030(field + pos, PS_MAXCOLS) != 0)
                {
                    memcpy(*(ps+j), field + pos, PS_MAXCOLS-1);
                    pos += PS_MAXCOLS - 1;
                }
                else
                {
                    memcpy(*(ps+j), field + pos, PS_MAXCOLS);
                    pos += PS_MAXCOLS;
                }
                len -= PS_MAXCOLS;
                j++;
            }
            if ( len > 0)
            {
                // 最后剩下不足一行, 继续打印下一个字段
                strcpy(*(ps + j), field + pos);
                strcat(*(ps + j), ",");
                len++;
            }
            continue;
        }
        // 达到单行最大PS_MAXCOLS换行
        len += strlen(field) + 1;
        if (len > PS_MAXCOLS)
        {
            len = strlen(field)+1;
            if (++j > PS_MAXROWS)
                break;
        }
        strcat(*(ps + j), field);
        strcat(*(ps + j), ",");
    }

    xmlFreeDoc(doc);
    free(xmlBuf);

    return 0;
}
#endif
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
    strcpy(buf, *ps);
    for (i = 0; i < sizeof(FieldList)/sizeof(FieldList[0]); i++)
    {
        sprintf(path, "//%s", FieldList[i]);
        XMLGetVal(doc, path, field);
        if (field[0] == 0x00)
            continue;
        if ((strcmp(FieldList[i],"PrintFlag") ==0)&&(field[0] == '1'))
            strcat(buf, "回单代发");
        else
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
int PrintInNoteAdd( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    char tbname[128];
    char tbname_ex[128];
    char condi[4096];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startrefid[17];
    char endrefid[17];
    char amount[128];
    char buf[256];
    char datetime[30];
    char ps[PS_MAXROWS][256];
    char key[200];
    char *p;
    result_set *rs=NULL;
    result_set *ex=NULL;
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

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
    {
        strcpy(tbname, "hddb..trnjour");
        strcpy(tbname_ex, "hddb..extradata");
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
            "inoutflag='%c' AND workdate = '%s' AND classid = %d"
            "  AND refid BETWEEN '%s' AND '%s' AND acceptor LIKE '%s%%'"
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag LIKE '%s%%' %s"
            "  AND errcode in('%04d', '%04d', '%04d')",
            RECONREC_ACPT, GetTrnCtl("WorkDate"), atoi(GetTrnCtl("SvcClass")),
            startrefid, endrefid, GetTrnCtl("Acceptor"), 
            GetTrnCtl("NoteType"), GetTrnCtl("NoteNo"),
            GetTrnCtl("BeneAcct"), amount,
            GetTrnCtl("DCFlag"), GetTrnCtl("TruncFlag"), buf,
            E_SUCC, E_WAITING_CONFIRMATION, E_GNR_TESTKEY_CHK);

    iRet = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, seqno", tbname, condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提入补充凭证失败, condi=[%s][%s]", 
                __FILE__, __LINE__, tbname, condi );
        return -1;
    }

    // 打印时间
    GetDateTime(datetime);

    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        db_free_result(rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
    }

    WriteRptHeader(fp, "");
    for (i = 0; i < rs->row_count; i++)
    {
        // 密押字段
        p = db_cell_by_name(rs, i, "agreement");
        if (*p != 0)
            sprintf(key, "支付密码:%s", p);
        else
            sprintf(key, "银行密押:%s", db_cell_by_name(rs, i, "testkey"));

        if ((iErrCode = atoi(db_cell_by_name(rs, i, "errcode"))) != 0)
        {
            strcat(key, "  ");
            strcat(key, errmsg(atoi(db_cell_by_name(rs, i, "errcode"))));
        }

        // 大写金额
        MoneyToChinese(db_cell_by_name(rs, i, "settlamt"), buf);

        memset(ps, 0, sizeof(ps));
        if (atof(db_cell_by_name(rs, i, "issueamt")) > 0)
           sprintf(ps[0], "签发金额: %s ",db_cell_by_name(rs, i, "issueamt"));

        if (atoi(db_cell_by_name(rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    GetTrnCtl("WorkDate"), db_cell_by_name(rs, i, "seqno"));
            if (iRet == 0)
            {
                GetExtraData(db_cell(ex, 0, 0), ps);
                db_free_result(ex);
            }
            else
                SetError(0);
        }

        fprintf(fp, "%s (%s);%s;%s;第%s场;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;%s %s;%s;"
                "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;\n",
                db_cell_by_name(rs, i, "originator"),
                db_cell_by_name(rs, i, "termid"),
                db_cell_by_name(rs, i, "acceptor"),
                db_cell_by_name(rs, i, "workdate"),
                db_cell_by_name(rs, i, "workround"),
                db_cell_by_name(rs, i, "refid"),
                notetype_name(db_cell_by_name(rs, i, "notetype")),
                db_cell_by_name(rs, i, "noteno"),
                db_cell_by_name(rs, i, "issuedate"),
                db_cell_by_name(rs, i, "payer"),
                db_cell_by_name(rs, i, "payingacct"),
                db_cell_by_name(rs, i, "payingbank"),
                org_name(db_cell_by_name(rs, i, "payingbank")),
                db_cell_by_name(rs, i, "benename"),
                db_cell_by_name(rs, i, "beneacct"),
                db_cell_by_name(rs, i, "benebank"),
                org_name(db_cell_by_name(rs, i, "benebank")),
                ChsName(curcode_list, db_cell_by_name(rs, i, "curcode")),
                buf, 
                db_cell_by_name(rs, i, "curcode"),
                FormatMoney(db_cell_by_name(rs, i, "settlamt")), 
                db_cell_by_name(rs, i, "purpose"),
                key,
                ps[0], ps[1], ps[2], ps[3],
                datetime,
                atoi(db_cell_by_name(rs, i, "printnum"))+1,
                GetMsgHdrRq("AcctOper"));
    }
    db_free_result(rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    GetTmpFileName(caOutFile);
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    DBExec("update %s set printnum=printnum+1 where %s", tbname, condi);
    SetError(0);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
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
    char startrefid[17];
    char endrefid[17];
    char amount[128];
    char buf[256];
    char datetime[30];
    char ps[PS_MAXROWS][256];
    char key[200];
    char *p;
    result_set *rs=NULL;
    result_set *ex=NULL;
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

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
    {
        strcpy(tbname, "hddb..trnjour");
        strcpy(tbname_ex, "hddb..extradata");
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
            "inoutflag='%c' AND workdate = '%s'"
            "  AND classid = %d AND dcflag = '%d'"
            "  AND refid BETWEEN '%s' AND '%s' "
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND truncflag LIKE '%s%%' %s"
            "  AND errcode ='0000'",
            RECONREC_PRES, GetTrnCtl("WorkDate"),
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
        db_free_result(rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
    }

    WriteRptHeader(fp, "");
    for (i = 0; i < rs->row_count; i++)
    {
        // 密押字段
        p = db_cell_by_name(rs, i, "agreement");
        if (*p != 0)
            sprintf(key, "支付密码:%s", p);
        else
            sprintf(key, "银行密押:%s", db_cell_by_name(rs, i, "testkey"));

        if ((iErrCode = atoi(db_cell_by_name(rs, i, "errcode"))) != 0)
        {
            strcat(key, "  ");
            strcat(key, errmsg(atoi(db_cell_by_name(rs, i, "errcode"))));
        }

        // 大写金额
        MoneyToChinese(db_cell_by_name(rs, i, "settlamt"), buf);

        memset(ps, 0, sizeof(ps));
        if (atof(db_cell_by_name(rs, i, "issueamt")) > 0)
           sprintf(ps[0], "签发金额: %s ",db_cell_by_name(rs, i, "issueamt"));

        if (atoi(db_cell_by_name(rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    GetTrnCtl("WorkDate"), db_cell_by_name(rs, i, "seqno"));
            if (iRet == 0)
            {
                GetExtraData(db_cell(ex, 0, 0), ps);
                db_free_result(ex);
            }
            else
                SetError(0);
        }

        fprintf(fp, "%s (%s);%s;%s;第%s场;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;%s %s;%s;"
                "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;\n",
                db_cell_by_name(rs, i, "originator"),
                db_cell_by_name(rs, i, "termid"),
                db_cell_by_name(rs, i, "acceptor"),
                db_cell_by_name(rs, i, "workdate"),
                db_cell_by_name(rs, i, "workround"),
                db_cell_by_name(rs, i, "refid"),
                notetype_name(db_cell_by_name(rs, i, "notetype")),
                db_cell_by_name(rs, i, "noteno"),
                db_cell_by_name(rs, i, "issuedate"),
                db_cell_by_name(rs, i, "payer"),
                db_cell_by_name(rs, i, "payingacct"),
                db_cell_by_name(rs, i, "payingbank"),
                org_name(db_cell_by_name(rs, i, "payingbank")),
                db_cell_by_name(rs, i, "benename"),
                db_cell_by_name(rs, i, "beneacct"),
                db_cell_by_name(rs, i, "benebank"),
                org_name(db_cell_by_name(rs, i, "benebank")),
                ChsName(curcode_list, db_cell_by_name(rs, i, "curcode")),
                buf, 
                db_cell_by_name(rs, i, "curcode"),
                FormatMoney(db_cell_by_name(rs, i, "settlamt")), 
                db_cell_by_name(rs, i, "purpose"),
                key,
                ps[0], ps[1], ps[2], ps[3],
                datetime,
                atoi(db_cell_by_name(rs, i, "printnum"))+1,
                GetMsgHdrRq("AcctOper"));
    }
    db_free_result(rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    GetTmpFileName(caOutFile);
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    DBExec("update %s set printnum=printnum+1 where %s", tbname, condi);
    SetError(0);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
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
    char operno[7];
    result_set *rs=NULL;
    FILE *fp=NULL;
    char *content = NULL;
    char Originator[13];
    char Acceptor[13];
    char WorkDate[9];
    char RefId[17];
    char NoteType[5];
    char NoteNo[31];
    char DCFlag[3];
    char IssueDate[9];
    char SettlAmt[21];
    char IssueAmt[21];
    char PayingAcct[33];
    char Payer[81];
    char BeneAcct[33];
    char BeneName[81];
    char PayKey[30];
    char Agreement[128];
    char ClearState[5];
    char buf[128];
    char condi[2048];
    char purpose_str[1025];
    char purpose[6][101];
    int iRet = 0, updflag = 1;
    int j;
    int i,k,pos;

    sprintf(printdate, "%08ld", current_date());
    strcpy(operno, GetMsgHdrRq("AcctOper"));

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
            "%s/report/PrintInQuery.para", getenv("HOME") );

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
            break;
    }

    snprintf(condi, sizeof(condi),
            "recver = '%s'"
            "  AND recvdate BETWEEN '%s' AND '%s'"
            "  AND mailid BETWEEN %ld AND %ld"
            "  AND mailtype LIKE '%s%%' %s",
            GetTrnCtl("Acceptor"), startdate, enddate, 
            atol(startrefid), atoi(endrefid), GetTrnCtl("MailType"), buf);
    /*
    iRet = db_DeclareCur("prtinqrycur", CURSOR_NORMAL, 
            "SELECT (case when mailtype='1' then '查询书' "
            "when mailtype='2' then '查复书' "
            "when mailtype='3' then '通知书' end) as typename, sender, writer,"
            " content, title, recvdate, recvtime, mailid FROM recvbox "
            "WHERE %s order by recvdate, mailid", condi);
     */
    SDKerrlog( ERRLOG, "%s|%d, condi=[%s]",__FILE__,__LINE__,condi);

    iRet = db_DeclareCur("prtinqrycur", CURSOR_NORMAL, 
            "SELECT mailid, recver, sender, recvdate, content FROM recvbox "
            "WHERE %s order by recvdate, mailid", condi);
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, 查询提入查询查复书失败",
                __FILE__, __LINE__ );
        goto err_handle;
    }

    // 打开游标
    db_OpenCur("prtinqrycur");
        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");
    j = 0;
    for (;;j++)
    {
        char *orgname = NULL;
        iRet = db_FetchCur("prtinqrycur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        if (iRet < 0)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            goto err_handle;
        }
        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        fprintf(fp, "%s;%s;%s;%s;", 
                db_cell(rs, 0, 0),
                db_cell(rs, 0, 1),
                db_cell(rs, 0, 2),
                db_cell(rs, 0, 3));

        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

        content = encoding_conv(db_cell(rs, 0, 4), "GB18030", "UTF-8");
        if (content == NULL)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            db_free_result(rs);
            SetError( E_TR_PRINT );
            return -1;
        }
        doc = xmlParseMemory(content, strlen(content));
        if (doc == NULL)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            db_free_result(rs);
            ifree(content);
            SetError( E_TR_PRINT );
            return -1;
        }

        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        memset(purpose , 0, sizeof(purpose));
        memset(purpose_str , 0, sizeof(purpose_str));
        XMLGetVal(doc, "//Purpose", purpose_str);
        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

        for(k = 0, pos = 0; k<6; )
        {
            if(strlen(purpose_str) - pos <= PS_MAXCOLS)
            {
                strncpy(purpose[k], purpose_str+pos, strlen(purpose_str)-pos);
                pos = strlen(purpose_str);
                break;
            }

            for( i = 0; i <PS_MAXCOLS ;i++)
            {

                if( CheckGB18030(purpose_str, PS_MAXCOLS -i) == 0)
                {
                    strncpy(purpose[k], purpose_str + pos, PS_MAXCOLS -i);
                    pos = pos + PS_MAXCOLS -i;
                    k++;
                    break;
                }
            }
            if(pos >= strlen(purpose_str))
                break;
        }

        SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
                xmlGetVal(doc, "//NoteType", NoteType),
                xmlGetVal(doc, "//NoteNo", NoteNo),
                xmlGetVal(doc, "//SettlAmt", SettlAmt),
                xmlGetVal(doc, "//IssueDate", IssueDate),
                xmlGetVal(doc, "//TradeDate", WorkDate),
                XMLGetVal(doc, "//Agreement", Agreement),
                XMLGetVal(doc, "//TestKey", PayKey),
                xmlGetVal(doc, "//PayingAcct", PayingAcct),
                XMLGetVal(doc, "//Payer", Payer),
                xmlGetVal(doc, "//BeneAcct", BeneAcct),
                XMLGetVal(doc, "//BeneName", BeneName),
                purpose[0],purpose[1],purpose[2],
                purpose[3],purpose[4],purpose[5]);

        xmlFreeDoc(doc);
        free(content);
        db_free_result(rs);
    }
    db_CloseCur("prtinqrycur");

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ( j == 0)
    {
        SetError( E_GNR_RECNOTFOUND );
        goto err_handle;
    }

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    if (updflag == 1)
        DBExec("update recvbox set readed='1' where %s", condi);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    return iRet;
}

int PrintDiffNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *rs=NULL;
    char sysname[128];
    char tbname[128];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char buf[256];
    char *originame;
    FILE *fp=NULL;
    int iRet = 0;
    int i;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/DiffNote.para", getenv("HOME") );

    strcpy(sysname, GetSysPara("SYSNAME"));
    if (sysname[0] == 0)
        strcpy(sysname, SYS_NAME);

    originame=org_name(GetMsgHdrRq("Originator"));

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, "hddb..ebanksumm");
    else
        strcpy(tbname, "ebanksumm");

    iRet = db_query(&rs, 
            "SELECT workdate, workround, svcclass, curcode, curtype, "
            "pres_credit_num,pres_credit_total,pres_debit_num,pres_debit_total,"
            "acpt_debit_num,acpt_debit_total,acpt_credit_num,acpt_credit_total,"
            "(pres_credit_num + acpt_debit_num) as paynum, "
            "(pres_credit_total + acpt_debit_total) as paytotal, "
            "(pres_debit_num + acpt_credit_num) as benenum, "
            "(pres_debit_total + acpt_credit_total) as benetotal, "
            "balance FROM %s "
            "WHERE workdate = '%s'"
            "  AND workround = %d"
            "  AND svcclass = %d"
            "  AND branchid = '%s'"
            "  AND curcode LIKE '%s%%'"
            "  AND curtype LIKE '%s%%'"
            " ORDER BY branchid",
            tbname,
            GetTrnCtl("WorkDate"),
            atoi(GetTrnCtl("WorkRound")),
            atoi(GetTrnCtl("SvcClass")),
            GetTrnCtl("Originator"),
            GetTrnCtl("CurCode"),
            GetTrnCtl("CurType"));
    if ( iRet != 0 )
    {
        free(originame);
        return -1;
    }

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");

    for (i = 0; i < db_row_count(rs); i++)
    {
        fprintf(fp, "%s;%s", 
                GetMsgHdrRq("Originator"), originame);

        if (db_cell_i(rs, i, 2) == CLASS_FOREIGN)
        {
            fprintf(fp, "  外币:%s %s;",
                    ChsName(curcode_list, db_cell(rs, i, 3)),
                    GetChineseName(curtype_list, db_cell_i(rs, i, 4)));
        }
        else
        {
            fprintf(fp, "  人民币;" );
        }
        fprintf(fp, "%s;第%s场;", ChineseDate(db_cell_i(rs, i, 0)),
                db_cell(rs, i, 1));

        if (*db_cell(rs, i, 17) == '-')
            sprintf(buf, "%s; ;", FormatMoney(db_cell(rs, i, 17) + 1));
        else
            sprintf(buf, " ;%s;", FormatMoney(db_cell(rs, i, 17)));

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(rs, i, 5), FormatMoney(db_cell(rs, i, 6)),
                db_cell(rs, i, 7), FormatMoney(db_cell(rs, i, 8)),
                db_cell(rs, i, 9), FormatMoney(db_cell(rs, i, 10)),
                db_cell(rs, i, 11), FormatMoney(db_cell(rs, i, 12)),
                db_cell(rs, i, 13), FormatMoney(db_cell(rs, i, 14)),
                db_cell(rs, i, 15), FormatMoney(db_cell(rs, i, 16)), buf);
    }
    db_free_result(rs);
    free(originame);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

    return iRet;
}

int PrintOutNoteJSD( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *banklist=NULL;
    result_set *rs=NULL;

    char tbname[128];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char buf[128];
    char tmp[128];
    char *originame = NULL;
    char *acptname = NULL;
    char *chinesedate = NULL;

    FILE *fp=NULL;
    double dDSumAmt, dCSumAmt;
    int iDCount, iCCount;
    int iRet = 0;
    int i, j, k;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteJSD.para", getenv("HOME") );

    if (DiffDate(GetTrnCtl("WorkDate"), GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, "hddb..trnjour");
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

    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");
        originame = org_name(db_cell(banklist, i, 0));
        acptname = org_name(db_cell(banklist, i, 1));

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;%s;",
                GetSysPara("SYSNAME"), 
                chinesedate,
                GetTrnCtl("WorkRound"),
                db_cell(banklist, i, 2),
                db_cell(banklist, i, 0), 
                originame,
                db_cell(banklist, i, 1),
                acptname);

        iRet = db_DeclareCur("outjsdcur", CURSOR_NORMAL, 
                "SELECT originator, refid, acceptor, notetype, dcflag, noteno,"
                "settlamt, beneacct, payingacct FROM %s "
                "WHERE %s AND acceptor='%s' AND curcode='%s' ORDER BY seqno",
                tbname,condi, db_cell(banklist, i, 1), db_cell(banklist, i, 2));
        if ( iRet != 0 )
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提出计数单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            free(originame);
            free(acptname);
            goto err_handle;
        }

        // 打开游标
        db_OpenCur("outjsdcur");

        j = 0;
        dDSumAmt = dCSumAmt = (double)0;
        iDCount = iCCount = 0;
        for (;;j++)
        {
            iRet = db_FetchCur("outjsdcur", &rs);
            if (iRet == SQLNOTFOUND)
                break;
            if (iRet < 0)
            {
                fclose(fp);
                db_free_result(banklist);
                goto err_handle;
            }

            for (k = 0; k < rs->col_count; k++)
            {
                if ( k == 6)
                {
                    if (db_cell_i(rs, 0, 4) == PART_DEBIT)
                    {
                        dDSumAmt += db_cell_d(rs, 0, k);
                        iDCount++;
                    }
                    else
                    {
                        dCSumAmt += db_cell_d(rs, 0, k);
                        iCCount++;
                    }
                    strcpy(buf, db_cell(rs, 0, k));
                    fprintf(fp, "%s;", FormatMoney(buf));
                }
                else
                {
                    fprintf(fp, "%s;", db_cell(rs, 0, k));
                }
            }
            fprintf(fp, "\n");
            db_free_result(rs);
        }
        db_CloseCur("outjsdcur");

        WriteRptRowCount(fp, j);
        sprintf(tmp, "%.2lf", dDSumAmt);
        sprintf(buf, "%.2lf", dCSumAmt);
        WriteRptFooter(fp, "%d;%s;%d;%s;", 
                iDCount, FormatMoney(tmp), iCCount, FormatMoney(buf));
        fclose(fp);

        if ( j == 0)
            continue;

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

// 提出汇总单
int PrintOutNoteTotal( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *banklist=NULL;
    result_set *rs=NULL;

    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char *originame = NULL;
    char *acptname = NULL;
    char *sysname = NULL;
    char *chinesedate = NULL;

    FILE *fp=NULL;
    char sDSumAmt[30], sCSumAmt[30];
    int iDCount, iCCount;
    int iRet = 0;
    int i;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteTotal.para", getenv("HOME") );

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
                "SELECT DISTINCT acceptor,curcode FROM hddb..trnjour WHERE %s "
                "AND acceptor LIKE '%s%%' order by acceptor, curcode", 
                condi, GetTrnCtl("Acceptor")) != 0)
    {
        return -1;
    }

    sysname = GetSysPara("SYSNAME"); 
    originame=org_name(GetMsgHdrRq("Originator"));
    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");
        acptname=org_name(db_cell(banklist, i, 0));

        WriteRptHeader(fp, "");

        // 统计借方笔数及金额
        if (db_query(&rs, "SELECT count(*), sum(settlamt) FROM hddb..trnjour WHERE"
                    " %s AND acceptor='%s' AND curcode='%s' AND dcflag='1'",
                    condi, db_cell(banklist, i, 0), 
                    db_cell(banklist, i, 1)) != 0)
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提出汇总单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            free(originame);
            free(acptname);
            goto err_handle;
        }
        iDCount = db_cell_i(rs, 0, 0);
        strcpy(sDSumAmt, db_cell(rs, 0, 1));
        db_free_result(rs);

        // 统计贷方笔数及金额
        if (db_query(&rs, "SELECT count(*), sum(settlamt) FROM hddb..trnjour WHERE"
                    " %s AND acceptor='%s' AND curcode='%s' AND dcflag='2'",
                    condi, db_cell(banklist, i, 0), 
                    db_cell(banklist, i, 1)) != 0)
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提出汇总单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            free(originame);
            free(acptname);
            goto err_handle;
        }
        iCCount = db_cell_i(rs, 0, 0);
        strcpy(sCSumAmt, db_cell(rs, 0, 1));
        db_free_result(rs);

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%d;%s;;;",
                sysname, chinesedate, GetTrnCtl("WorkRound"), 
                db_cell(banklist, i, 1), db_cell(banklist, i, 0), acptname,
                GetMsgHdrRq("Originator"), originame,
                iDCount, FormatMoney(sDSumAmt), iCCount, FormatMoney(sCSumAmt));

        WriteRptRowCount(fp, 1);
        WriteRptFooter(fp, "");
        fclose(fp);

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

// 提出清单
int PrintOutNoteList( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *banklist=NULL;
    result_set *rs=NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char buf[128];
    char caOrgName[256];
    char PayingAcctOrName[81],BeneAcctOrName[81];
    char *chinesedate = NULL;

    FILE *fp=NULL;
    double dSumAmt;
    int iRet = 0;
    int i, j, k;

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));
    memset(caOrgName, 0, sizeof(caOrgName));
    strcpy(caOrgName, org_name(GetMsgHdrRq("Originator")));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNoteList.para", getenv("HOME") );

    GetTmpFileName(caOutFile);

    sprintf(condi, "inoutflag = '%c'"
            "  AND workdate = '%s'"
            "  AND workround = %d"
            "  AND classid = %d"
            "  AND clearstate in ('1', 'C')"
            "  AND originator = '%s'"
            "  AND dcflag = '%s'"
            "  AND truncflag LIKE '%s%%'", 
            RECONREC_PRES,
            GetTrnCtl("WorkDate"),
            atoi(GetTrnCtl("WorkRound")), 
            atoi(GetTrnCtl("SvcClass")),
            GetMsgHdrRq("Originator"),
            GetTrnCtl("DCFlag"),
            GetTrnCtl("TruncFlag"));

    if (db_query(&banklist, "SELECT DISTINCT curcode FROM hddb..trnjour WHERE %s", 
                condi) != 0)
    {
        return -1;
    }

    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;",
                GetSysPara("SYSNAME"),
                (atoi(GetTrnCtl("DCFlag")) == PART_DEBIT ? "借方" : "贷方"),
                chinesedate,
                GetTrnCtl("WorkRound"),
                ChsName(curcode_list, db_cell(banklist, i, 0)),
                GetMsgHdrRq("Originator"),
                caOrgName);

        iRet = db_DeclareCur("outlistcur", CURSOR_NORMAL, 
                "SELECT refid, acceptor, notetype, noteno, payingacct, payer, "
                "beneacct, benename, settlamt, acctoper FROM hddb..trnjour "
                "WHERE %s and curcode='%s' ORDER BY seqno", condi,
                db_cell(banklist, i, 0));
        if ( iRet != 0 )
        {
            SDKerrlog(ERRLOG, "%s|%d, 打印提出凭证清单失败",__FILE__, __LINE__);
            db_free_result(banklist);
            goto err_handle;
        }

        // 打开游标
        db_OpenCur("outlistcur");

        j = 0;
        dSumAmt = (double)0;
        for (; ; j++)
        {
            iRet = db_FetchCur("outlistcur", &rs);
            if (iRet == SQLNOTFOUND)
                break;
            if (iRet < 0)
            {
                fclose(fp);
                db_free_result(banklist);
                goto err_handle;
            }

            strcpy(BeneAcctOrName, db_cell(rs, 0, 6));
            all_trim(BeneAcctOrName);
            if(strlen(BeneAcctOrName) == 0)
            {
                strcpy(BeneAcctOrName, db_cell(rs, 0, 7));
                all_trim(BeneAcctOrName);
                BeneAcctOrName[32] = 0;
            }
            strcpy(PayingAcctOrName, db_cell(rs, 0, 4));
            all_trim(PayingAcctOrName);
            if(strlen(PayingAcctOrName) == 0)
            {
                strcpy(PayingAcctOrName, db_cell(rs, 0, 5));
                all_trim(PayingAcctOrName);
                PayingAcctOrName[32] = 0;
            }

            dSumAmt += db_cell_d(rs, 0, 8);
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                    db_cell(rs, 0, 0),
                    db_cell(rs, 0, 1),
                    db_cell(rs, 0, 2),
                    db_cell(rs, 0, 3),
                    PayingAcctOrName,
                    BeneAcctOrName,
                    FormatMoney(db_cell(rs, 0, 8)),
                    db_cell(rs, 0, 9));
            db_free_result(rs);
        }
        db_CloseCur("outlistcur");

        WriteRptRowCount(fp, j);
        sprintf(buf, "%.2lf", dSumAmt);
        WriteRptFooter(fp, "%d;%s;", j, FormatMoney(buf));
        fclose(fp);

        if ( j == 0)
            continue;

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

// 提入汇总单
int PrintInNoteTotal( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *banklist=NULL;
    result_set *rs=NULL;

    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char *originame = NULL;
    char *acptname = NULL;
    char *sysname = NULL;
    char *chinesedate = NULL;

    FILE *fp=NULL;
    char sDSumAmt[30], sCSumAmt[30];
    int iDCount, iCCount;
    int iRet = 0;
    int i;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteTotal.para", getenv("HOME") );

    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    GetTmpFileName(caOutFile);

    sprintf(condi, "inoutflag = '%c'"
            "  AND workdate = '%s'"
            "  AND workround = %d"
            "  AND classid = %d"
            "  AND clearstate in ('1', 'C')"
            "  AND acceptor = '%s'"
            "  AND truncflag LIKE '%s%%'", 
            RECONREC_ACPT,
            GetTrnCtl("WorkDate"),
            atoi(GetTrnCtl("WorkRound")), 
            atoi(GetTrnCtl("SvcClass")),
            GetMsgHdrRq("Originator"),
            GetTrnCtl("TruncFlag"));

    if (db_query(&banklist, 
                "SELECT DISTINCT originator,curcode FROM hddb..trnjour WHERE %s "
                "AND originator LIKE '%s%%' order by originator, curcode", 
                condi, GetTrnCtl("Originator")) != 0)
    {
        return -1;
    }

    sysname = GetSysPara("SYSNAME"); 
    acptname=org_name(GetMsgHdrRq("Originator"));
    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");
        originame=org_name(db_cell(banklist, i, 0));

        WriteRptHeader(fp, "");

        // 统计借方笔数及金额
        if (db_query(&rs, "SELECT count(*), sum(settlamt) FROM hddb..trnjour WHERE"
                    " %s AND originator='%s' AND curcode='%s' AND dcflag='1'", 
                    condi, db_cell(banklist, i, 0), 
                    db_cell(banklist, i, 1)) != 0)
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提入汇总单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            free(originame);
            free(acptname);
            goto err_handle;
        }
        iDCount = db_cell_i(rs, 0, 0);
        strcpy(sDSumAmt, db_cell(rs, 0, 1));
        db_free_result(rs);

        // 统计贷方笔数及金额
        if (db_query(&rs, "SELECT count(*), sum(settlamt) FROM hddb..trnjour WHERE"
                    " %s AND originator='%s' AND curcode='%s' AND dcflag='2'", 
                    condi, db_cell(banklist, i, 0), 
                    db_cell(banklist, i, 1)) != 0)
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印提入汇总单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            free(originame);
            free(acptname);
            goto err_handle;
        }
        iCCount = db_cell_i(rs, 0, 0);
        strcpy(sCSumAmt, db_cell(rs, 0, 1));
        db_free_result(rs);

        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%d;%s;;;",
                sysname, chinesedate, GetTrnCtl("WorkRound"), 
                db_cell(banklist, i, 1), 
                GetMsgHdrRq("Originator"), acptname,
                db_cell(banklist, i, 0), originame,
                iDCount, FormatMoney(sDSumAmt), iCCount, FormatMoney(sCSumAmt));

        WriteRptRowCount(fp, 1);
        WriteRptFooter(fp, "");
        fclose(fp);

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

// 提入清单
int PrintInNoteList( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *banklist=NULL;
    result_set *rs=NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char caOrgName[256];
    char condi[1024];
    char buf[128];
    char PayingAcctOrName[81],BeneAcctOrName[81];
    char *originame = NULL;
    char *chinesedate = NULL;

    FILE *fp=NULL;
    double dSumAmt;
    int iRet = 0;
    int i, j, k;

    memset(caOrgName, 0, sizeof(caOrgName));
    strcpy(caOrgName, org_name(GetMsgHdrRq("Originator")));
    chinesedate = ChineseDate(atol(GetTrnCtl("WorkDate")));

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNoteList.para", getenv("HOME") );
    GetTmpFileName(caOutFile);

    sprintf(condi, "inoutflag = '%c'"
            "  AND workdate = '%s'"
            "  AND workround = %d"
            "  AND classid = %d"
            "  AND clearstate in ('1', 'C')"
            "  AND acceptor = '%s'"
            "  AND dcflag = '%s'"
            "  AND truncflag LIKE '%s%%'", 
            RECONREC_ACPT,
            GetTrnCtl("WorkDate"),
            atoi(GetTrnCtl("WorkRound")), 
            atoi(GetTrnCtl("SvcClass")),
            GetMsgHdrRq("Originator"),
            GetTrnCtl("DCFlag"),
            GetTrnCtl("TruncFlag"));

    if (db_query(&banklist, "SELECT DISTINCT curcode FROM hddb..trnjour WHERE %s", 
                condi) != 0)
    {
        return -1;
    }

    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;%s;", GetSysPara("SYSNAME"),
                (atoi(GetTrnCtl("DCFlag")) == PART_DEBIT ? "借方" : "贷方"),
                chinesedate, GetTrnCtl("WorkRound"), 
                ChsName(curcode_list, db_cell(banklist, i, 0)),
                GetMsgHdrRq("Originator"), caOrgName);

        iRet = db_DeclareCur("inlistcur", CURSOR_NORMAL, 
                "SELECT originator, refid, notetype, noteno, payingacct, "
                "payer, beneacct, benename, settlamt, acctoper "
                "FROM hddb..trnjour "
                "WHERE %s and curcode='%s' ORDER BY seqno", 
                condi, db_cell(banklist, i, 0));
        if ( iRet != 0 )
        {
            SDKerrlog(ERRLOG, "%s|%d, 打印提入凭证清单失败",__FILE__, __LINE__);
            db_free_result(banklist);
            goto err_handle;
        }

        // 打开游标
        j = 0;
        dSumAmt = (double)0;
        db_OpenCur("inlistcur");
        for (; ; j++)
        {
            iRet = db_FetchCur("inlistcur", &rs);
            if (iRet == SQLNOTFOUND)
                break;
            if (iRet < 0)
            {
                fclose(fp);
                db_free_result(banklist);
                goto err_handle;
            }

            strcpy(BeneAcctOrName, db_cell(rs, 0, 6));
            all_trim(BeneAcctOrName);
            if(strlen(BeneAcctOrName) == 0)
            {
                strcpy(BeneAcctOrName, db_cell(rs, 0, 7));
                all_trim(BeneAcctOrName);
                BeneAcctOrName[32] = 0;
            }
            strcpy(PayingAcctOrName, db_cell(rs, 0, 4));
            all_trim(PayingAcctOrName);
            if(strlen(PayingAcctOrName) == 0)
            {
                strcpy(PayingAcctOrName, db_cell(rs, 0, 5));
                all_trim(PayingAcctOrName);
                PayingAcctOrName[32] = 0;
            }

            dSumAmt += db_cell_d(rs, 0, 8);
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                    db_cell(rs, 0, 0),
                    db_cell(rs, 0, 1),
                    db_cell(rs, 0, 2),
                    db_cell(rs, 0, 3),
                    PayingAcctOrName,
                    BeneAcctOrName,
                    FormatMoney(db_cell(rs, 0, 8)),
                    db_cell(rs, 0, 9));
            /*
               for (k = 0; k < rs->col_count; k++)
               {
               if ( k == 6)
               {
               strcpy(buf, db_cell(rs, 0, k));
               fprintf(fp, "%s;", FormatMoney(buf));
               }
               else
               {
               fprintf(fp, "%s;", db_cell(rs, 0, k));
               }
               }
               fprintf(fp, "\n");
             */
            db_free_result(rs);
        }
        db_CloseCur("inlistcur");

        WriteRptRowCount(fp, j);
        sprintf(buf, "%.2lf", dSumAmt);
        WriteRptFooter(fp, "%d;%s;", j, FormatMoney(buf));
        fclose(fp);

        if ( j == 0)
            continue;

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(originame);
    ifree(chinesedate);
    return iRet;
}

int PrintOperSettle( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set *rs=NULL;
    result_set *banklist=NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char buf[256];
    char tmp[256];
    char presdate[11];
    double dDSumAmt, dCSumAmt;
    int iDCount, iCCount;
    char *originame = NULL;
    char *chinesedate = NULL;
    char *sysname = NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j, k;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OperSettle.para", getenv("HOME") );

    sysname = GetSysPara("SYSNAME"); 

    originame = org_name(GetMsgHdrRq("Originator"));
    memset(presdate, 0, sizeof(presdate));
    strcpy(presdate, GetTrnCtl("StartDate"));
    if (presdate[0] == 0)
        sprintf(presdate, "%08ld", current_date());

    chinesedate = ChineseDate(atol(presdate));

    GetTmpFileName(caOutFile);

    sprintf(condi, "presdate = '%s'"
            " AND classid = %d"
            " AND originator = '%s'"
            " AND clearstate in ('1', 'C')"
            " AND (acctoper = '%s' OR auditor = '%s')",
            presdate,
            atoi(GetTrnCtl("SvcClass")),
            GetMsgHdrRq("Originator"),
            GetMsgHdrRq("AcctOper"),
            GetMsgHdrRq("AcctOper"));

    if (db_query(&banklist, "SELECT DISTINCT curcode FROM hddb..trnjour WHERE %s"
                " order by curcode", condi) != 0)
    {
        ifree(sysname);
        ifree(originame);
        ifree(chinesedate);
        return -1;
    }

    for (i = 0; i < db_row_count(banklist); i++)
    {
        fp = fopen(GetTmpFileName(caDataFile), "w");

        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", sysname, chinesedate, 
                ChsName(curcode_list,db_cell(banklist, i, 0)), 
                GetMsgHdrRq("Originator"), originame, GetMsgHdrRq("AcctOper"));

        iRet = db_DeclareCur("opersettcur", CURSOR_NORMAL, 
                "SELECT refid, acceptor, notetype, dcflag, noteno, "
                "payingacct, beneacct, settlamt FROM hddb..trnjour "
                "WHERE %s AND curcode='%s' ORDER BY seqno", 
                condi, db_cell(banklist, i, 0));
        if ( iRet != 0 )
        {
            SDKerrlog( ERRLOG, "%s|%d, 打印柜员对账单失败", __FILE__, __LINE__);
            db_free_result(banklist);
            goto err_handle;
        }

        // 打开游标
        db_OpenCur("opersettcur");

        j = 0;
        iDCount = iCCount = 0;
        dDSumAmt = dCSumAmt = (double)0;
        for (;;j++)
        {
            iRet = db_FetchCur("opersettcur", &rs);
            if (iRet == SQLNOTFOUND)
                break;
            if (iRet < 0)
            {
                fclose(fp);
                db_free_result(banklist);
                goto err_handle;
            }

            for (k = 0; k < rs->col_count; k++)
            {
                if ( k == 3)
                {
                    if (db_cell_i(rs, 0, k) == PART_DEBIT)
                        fprintf(fp, "借;");
                    else if (db_cell_i(rs, 0, k) == PART_CREDIT)
                        fprintf(fp, "贷;");
                    else
                        fprintf(fp, ";");
                }
                else if ( k == 7)
                {
                    if (db_cell_i(rs, 0, 3) == PART_DEBIT)
                    {
                        dDSumAmt += db_cell_d(rs, 0, k);
                        iDCount++;
                    }
                    else
                    {
                        dCSumAmt += db_cell_d(rs, 0, k);
                        iCCount++;
                    }
                    strcpy(buf, db_cell(rs, 0, k));
                    fprintf(fp, "%s;", FormatMoney(buf));
                }
                else
                {
                    fprintf(fp, "%s;", db_cell(rs, 0, k));
                }
            }
            fprintf(fp, "\n");
            db_free_result(rs);
        }
        db_CloseCur("opersettcur");

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
            db_free_result(banklist);
            goto err_handle;
        }
    }
    db_free_result(banklist);

    SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(sysname);
    ifree(originame);
    ifree(chinesedate);
    return iRet;
}

func_list uftp_call[ ] =
{
    { "PrintInNoteAdd", PrintInNoteAdd },
    { "PrintOutNoteAdd", PrintOutNoteAdd },
    { "PrintInQuery", PrintInQuery },
    { "PrintDiffNote", PrintDiffNote },
    { "PrintOutNoteJSD", PrintOutNoteJSD },
    { "PrintOutNoteTotal", PrintOutNoteTotal },
    { "PrintOutNoteList", PrintOutNoteList },
    { "PrintInNoteTotal", PrintInNoteTotal },
    { "PrintInNoteList", PrintInNoteList },
    { "PrintOperSettle", PrintOperSettle },
    { "null_func", NULL }
};
