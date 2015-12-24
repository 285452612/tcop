/*********************************************
  QuerySvr.c
  ǰ�û���ѯ����
  Chen Jie, May 2007
 *********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq, xmlDocPtr xmlRsp);

extern int ChkPresAgent(char *orgid, char *presproxy);
extern int ChkAcptAgent(char *orgid, char *acptproxy);

/* �����ӡƾ֤��ѯ 
 * �ڱ���������ʾ�����ڡ�ʱ�䡢Ʊ�ֵ�������Ϣ
 */
int QryOutNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    char tbname[128];
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startdate[9];
    char enddate[9];
    char PayingAcctOrName[81];
    char BeneAcctOrName[81];
    char notename[32 + 1]; //Ʊ�������������Ϣ
    char newtime[20];       //��ʽ��������ڸ�ʽ
    char charmoney[16];     //�ַ��ͽ��
    char printtime[31];
    int  exchground = -1;
    char exchground_cond[64];
    char tmp[128];
    int  suc_num = 0;
    double suc_amt = (double)0;
    FILE *fp=NULL;
    int iRet = 0;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, GetSysPara("WORKDATE"));
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, GetSysPara("WORKDATE"));

    if (DiffDate(startdate, GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");

    memset(exchground_cond, 0, sizeof(exchground_cond));
    if (*GetTrnCtl("ExchgRound") != 0x00)
    {
        exchground = atoi(GetTrnCtl("ExchgRound"));
        sprintf(exchground_cond, "and exchground=%d", exchground);
    }
    // ��ѯ����ƾ֤
    iRet = db_query(&rs, 
            "SELECT refid, workdate, prestime, clearstate, acceptor, notetype,"
            "dcflag,noteno,issuedate, curcode,settlamt, beneacct, payingacct, "
            "benename, payer, errcode, seqno,trncode,truncflag FROM %s "
            "WHERE originator LIKE '%s%%' AND acceptor LIKE '%s%%'"
            "  AND classid = %d AND curcode LIKE '%s%%'"
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%'"
            "  AND dcflag LIKE '%s%%' AND clearstate LIKE '%s%%' %s"
            "  AND settlamt BETWEEN %.2lf AND %.2lf"
            "  AND refid BETWEEN '%s' AND '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            "  AND inoutflag = '%c'"
            " ORDER BY workdate, seqno", tbname,
            GetTrnCtl( "Originator" ), GetTrnCtl( "Acceptor" ),
            atoi(GetTrnCtl( "SvcClass" )), GetTrnCtl( "CurCode" ),
            GetTrnCtl( "NoteType" ), GetTrnCtl( "NoteNo" ),
            GetTrnCtl( "DCFlag" ), GetTrnCtl( "ClearState" ), exchground_cond,
            atof(GetTrnCtl("MinAmount")), atof(GetTrnCtl("MaxAmount")),
            GetTrnCtl( "StartRefId" ), GetTrnCtl( "EndRefId" ),
            startdate, enddate, RECONREC_PRES);
    if ( iRet != 0 )
        return -1;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutNote.para", getenv("HOME") );

    fp = fopen(GetTmpFileName(caDataFile), "w");

    memset(tmp, 0, sizeof(tmp));
    if (exchground > 0)
        sprintf(tmp, "ƱͶ����:%d", exchground);
    else if (exchground == 0)
        sprintf(tmp, "δ����ƾ֤");
    else
        sprintf(tmp, "����ƱͶ����");

    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname, printtime,
            GetMsgHdrRq("Originator"), gs_bankname,
            GetMsgHdrRq("AcctOper"), tmp);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        int workdate, prestime;
        //������¡��գ����һ���´�
        workdate = db_cell_i(&rs, i, 1);
        prestime = db_cell_i(&rs, i, 2);
        sprintf(newtime, "%04d %02d:%02d",workdate%10000, prestime/10000, 
                prestime%10000/100);

        //��Ʊ������д���
        //��'noteinfo'����ȥ��ѯ����ӦƱ�ݵ�������Ϣ
        sql_result(notename, sizeof(notename), "SELECT distinct name "
                "FROM noteinfo WHERE notetype='%s' AND dcflag='%s'", 
                db_cell_by_name(&rs, i, "notetype"),
                db_cell_by_name(&rs, i, "dcflag"));
        strcpy(BeneAcctOrName, db_cell_by_name(&rs, i, "beneacct"));
        all_trim(BeneAcctOrName);
        if(strlen(BeneAcctOrName) == 0)
        {
            strcpy(BeneAcctOrName, db_cell_by_name(&rs, i, "benename"));
            all_trim(BeneAcctOrName);
            BeneAcctOrName[32] = 0;
        }
        strcpy(PayingAcctOrName, db_cell_by_name(&rs, i, "payingacct"));
        all_trim(PayingAcctOrName);
        if(strlen(PayingAcctOrName) == 0)
        {
            strcpy(PayingAcctOrName, db_cell_by_name(&rs, i, "payer"));
            all_trim(PayingAcctOrName);
            PayingAcctOrName[32] = 0;
        }

        if (*db_cell(&rs, i, 3) == CLRSTAT_SETTLED ||
                *db_cell(&rs, i, 3) == CLRSTAT_CHECKED)
        {
            suc_num ++;
            suc_amt += atof(db_cell_by_name(&rs, i, "settlamt"));
        }
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "refid"),
                newtime,
                GetChineseName(clrstat_list, *db_cell(&rs, i, 3)), //clearstate
                db_cell_by_name(&rs, i, "acceptor"),
                (atoi(db_cell_by_name(&rs, i, "trncode"))==T_TRAN_REFUND \
                 ? "��Ʊ" : notename),
                GetChineseName(dcflag_list, db_cell_i(&rs, i, 6)), //dcflag
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "curcode"),
                FormatMoney(db_cell_by_name(&rs, i, "settlamt")),
                BeneAcctOrName, PayingAcctOrName,
                (*db_cell_by_name(&rs, i, "truncflag")=='0' ? "��" : "��"));
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    sprintf(charmoney, "%lf", suc_amt);
    //WriteRptFooter(fp, "%d;%.2lf;", suc_num, suc_amt);
    WriteRptFooter(fp, "%d;%s;", suc_num, FormatMoney(charmoney));
    fclose(fp);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.oqry",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    return iRet;
}

//�����ӡƾ֤��ѯ,������ӦҪ�ص�������ʾ 
int QryInNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    char tbname[128];
    char condi[512];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startdate[9];
    char enddate[9];
    char printtime[31];
    char bankname[81];
    char PayingAcctOrName[81];
    char BeneAcctOrName[81];
    char newtime[15 + 1];  //����µĸ�ʽ�����ں�ʱ�� 
    char charmoney[16];    //�ܼƽ����ַ�����ʾ
    int  exchground = -1;
    char exchground_cond[64];
    char notename[32 + 1]; //Ʊ������(������ʾ)
    char tmp[128];
    result_set orglist, rs;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, GetSysPara("WORKDATE"));
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, GetSysPara("WORKDATE"));

    if (DiffDate(startdate, GetSysPara("ARCHIVE_DATE")) <= 0)
        strcpy(tbname, HDDB_TRNJOUR);
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
    {
        sprintf(condi, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel=2)", 
                GetMsgHdrRq("Originator"));
    }
    else
    {
        sprintf(condi, "acceptor='%s'", GetTrnCtl("Acceptor"));
        /*
        if (strcmp(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), GetMsgHdrRq("Originator")))
            {
                SetError(E_ORG_PERMIT);
                return -1;
            }
        }
        */
    }

    memset(exchground_cond, 0, sizeof(exchground_cond));
    if (*GetTrnCtl("ExchgRound") != 0x00)
    {
        exchground = atoi(GetTrnCtl("ExchgRound"));
        sprintf(exchground_cond, "and exchground=%d", exchground);
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
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE originator LIKE '%s%%' AND %s AND classid = %d"
            "  AND curcode LIKE '%s%%' AND notetype LIKE '%s%%'"
            "  AND noteno LIKE '%s%%' AND dcflag LIKE '%s%%'"
            "  %s AND settlamt BETWEEN %.2lf AND %.2lf"
            "  AND refid BETWEEN '%s' AND '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            "  AND inoutflag = '%c' ORDER BY workdate,seqno", tbname,
            GetTrnCtl( "Originator" ), condi, atoi(GetTrnCtl( "SvcClass" )),
            GetTrnCtl( "CurCode" ), GetTrnCtl( "NoteType" ),
            GetTrnCtl( "NoteNo" ), GetTrnCtl( "DCFlag" ), exchground_cond,
            atof(GetTrnCtl("MinAmount")), atof(GetTrnCtl("MaxAmount")),
            GetTrnCtl( "StartRefId" ), GetTrnCtl( "EndRefId" ),
            startdate, enddate, RECONREC_ACPT);
    if (iRet != 0)
        goto err_handle;

    if (db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                " order by acceptor") != 0)
        goto err_handle;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InNote.para", getenv("HOME") );
    GetTmpFileName(caDataFile);
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.iqry",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
    for (i = 0; i < db_row_count(&orglist); i++)
    {
        int  suc_num = 0;
        double suc_amt = (double)0;
        // ��ѯ����ƾ֤
        iRet = db_query(&rs,
                "SELECT originator, refid, workdate, prestime, clearstate, "
                "notetype, dcflag, noteno, issuedate, curcode,settlamt, "
                "beneacct, payingacct, errcode, seqno, benename, payer, "
                "acceptor,trncode,truncflag FROM " TMP_T_RS
                " WHERE acceptor='%s' ORDER BY seqno", db_cell(&orglist, i, 0));
        if ( iRet != 0 )
        {
            err_log( "��ѯ����ƾ֤ʧ��" );
            db_free_result(&orglist);
            goto err_handle;
        }

        if ((fp = fopen(caDataFile, "w")) == NULL)
        {
            err_log( "���ļ�ʧ��" );
            db_free_result(&orglist);
            goto err_handle;
        }

        memset(tmp, 0, sizeof(tmp));
        if (exchground > 0)
            sprintf(tmp, "ƱͶ����:%d", exchground);
        else if (exchground == 0)
            sprintf(tmp, "δ����ƾ֤");
        else
            sprintf(tmp, "����ƱͶ����");
        WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname, printtime, 
                db_cell(&orglist, i, 0), 
                org_name(db_cell(&orglist, i, 0), bankname),
                GetMsgHdrRq("AcctOper"), tmp);
        for (j = 0; j < db_row_count(&rs); j++)
        {
            int workdate, prestime;
            //������¡��գ����һ���´�
            workdate = db_cell_i(&rs, j, 2);
            prestime = db_cell_i(&rs, j, 3);
            sprintf(newtime, "%04d %02d:%02d",workdate%10000, prestime/10000, 
                    prestime%10000/100);

            //��Ʊ��������д���
            //��Ʊ������д���
            sql_result(notename, sizeof(notename), "SELECT distinct name FROM "
                    "noteinfo WHERE notetype='%s' and dcflag='%s'",
                    db_cell_by_name(&rs, j, "notetype"),
                    db_cell_by_name(&rs, j, "dcflag"));

            strcpy(BeneAcctOrName, db_cell(&rs, j, 11));
            all_trim(BeneAcctOrName);
            if(strlen(BeneAcctOrName) == 0)
            {
                strcpy(BeneAcctOrName, db_cell(&rs, j, 15));
                all_trim(BeneAcctOrName);
                BeneAcctOrName[32] = 0;
            }
            strcpy(PayingAcctOrName, db_cell(&rs, j, 12));
            all_trim(PayingAcctOrName);
            if(strlen(PayingAcctOrName) == 0)
            {
                strcpy(PayingAcctOrName, db_cell(&rs, j, 16));
                all_trim(PayingAcctOrName);
                PayingAcctOrName[32] = 0;
            }

            if (*db_cell(&rs, j, 4) == CLRSTAT_SETTLED ||
                    *db_cell(&rs, j, 4) == CLRSTAT_CHECKED)
            {
                suc_num ++;
                suc_amt += atof(db_cell_by_name(&rs, j, "settlamt"));
            }
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                    db_cell_by_name(&rs, j, "seqno"),
                    db_cell(&rs, j, 0),
                    db_cell(&rs, j, 1),
                    newtime,
                    GetChineseName(clrstat_list, *db_cell(&rs, j, 4)),
                    (atoi(db_cell_by_name(&rs, j, "trncode"))==T_TRAN_REFUND \
                     ? "��Ʊ" : notename),
                    GetChineseName(dcflag_list, db_cell_i(&rs, j, 6)), //dcflag
                    db_cell(&rs, j, 7),
                    db_cell(&rs, j, 9),
                    FormatMoney(db_cell(&rs, j, 10)),
                    BeneAcctOrName, PayingAcctOrName,
                    (*db_cell_by_name(&rs, j, "truncflag")=='0' ? "��" : "��"));
        }
        db_free_result(&rs);
        WriteRptRowCount(fp, j);
        sprintf(charmoney, "%lf", suc_amt);
        //WriteRptFooter(fp, "%d;%.2lf;", suc_num, suc_amt);
        WriteRptFooter(fp, "%d;%s;", suc_num, FormatMoney(charmoney));
        fclose(fp);

        iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (iRet != 0)
        {
            SetError( E_TR_PRINT );
            db_free_result(&orglist);
            goto err_handle;
        }
    }
    db_free_result(&orglist);

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof(gcaResponseFile));

err_handle:
    db_commit();
    return iRet;
}

#if 0
int QryCurDiffNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char caAttechOrg[13];
    char buf[256];
    FILE *fp=NULL;

    int iRet = 0;
    int i, j;

    memset( caAttechOrg, 0, sizeof( caAttechOrg ) );
    iRet = GetAttechOrg( GetMsgHdrRq( "Originator" ), caAttechOrg );
    if ( iRet != 0 )
        goto err_handle;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/curdiffnote.para", getenv("HOME") );

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%08ld;%06ld;%s;", 
            current_date(), current_time(), GetMsgHdrRq("AcctOper"));

    // ��ѯ�����е�����ƾ֤
    iRet = db_query(&rs,
            "SELECT * FROM ebanksumm "
            "WHERE bankid = '%s'"
            "  AND cleardate = '%s'"
            "  AND clearround = %s"
            "  AND svcclass = %s"
            "  AND curcode = '%s'"
            "  AND curtype = '%s'"
            " ORDER BY branchid", 
            caAttechOrg,
            GetSysPara( "CLEARDATE" ),
            GetSysPara( "CLEARROUND" ),
            GetTrnCtl( "SvcClass" ),
            GetTrnCtl( "CurCode" ),
            GetTrnCtl( "CurType" )
            );
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯ�����е�����ƾ֤ʧ��",
                __FILE__, __LINE__ );
        goto err_handle;
    }

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
        {
            if ( i == 10 || i == 12 || i == 14 || i == 16 || i == 18) 
            {
                strcpy(buf, db_cell(&rs, j, i));
                fprintf(fp, "%s;", FormatMoney(buf));
            }
            else
            {
                fprintf(fp, "%s;", db_cell(&rs, j, i));
            }
        }
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);

    /*
    // ��ѯ�����е�����ƾ֤ 
    iRet = db_query(&rs, "SELECT * FROM reconinfo "
    "WHERE bankid = '%s'"
    "  AND cleardate = '%s'"
    "  AND clearround = %s"
    "  AND svcclass = %s"
    "  AND curcode = '%s'"
    "  AND curtype = '%s'",
    caAttechOrg,
    GetSysPara( "CLEARDATE" ),
    GetSysPara( "CLEARROUND" ),
    GetTrnCtl( "SvcClass" ),
    GetTrnCtl( "CurCode" ),
    GetTrnCtl( "CurType" )
    );
    if ( iRet != 0 )
    {
    SDKerrlog( ERRLOG, "%s|%d, ��ѯ�����е�����ƾ֤ʧ��",
    __FILE__, __LINE__ );
    goto err_handle;
    }
    WriteRptFooter(fp, "%s;%ld;%.2lf;");
    */
    WriteRptFooter(fp, ";;;;;;;;;;;;;;;;;;;;");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    return iRet;
}

int QryHisDiffNote( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    char caAttechOrg[13];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startdate[9];
    char enddate[9];
    char caTmp[256];
    xmlDocPtr datadoc=NULL;
    xmlDocPtr rsdoc=NULL;
    xmlDocPtr rsdoc2=NULL;
    int iRet = 0;

    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, GetSysPara("WORKDATE"));
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, GetSysPara("WORKDATE"));

    memset( caAttechOrg, 0, sizeof( caAttechOrg ) );
    iRet = GetAttechOrg( GetMsgHdrRq( "Originator" ), caAttechOrg );
    if ( iRet != 0 )
        goto err_handle;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/hisdiffnote.para", getenv("HOME") );

    datadoc = GetReportDataDoc();

    sprintf( caTmp, "%08ld", current_date() );
    XMLAppendNodeVal( datadoc, "//ReportHeader", "QryDate", NULL, caTmp );
    sprintf( caTmp, "%06ld", current_time() );
    XMLAppendNodeVal( datadoc, "//ReportHeader", "QryTime", NULL, caTmp );
    XMLAppendNodeVal( datadoc, "//ReportHeader", "AcctOper", NULL, 
            GetMsgHdrRq( "AcctOper" ) );

    // ��ѯ�����в��ƾ֤
    iRet = DBQuery( &rsdoc, "SELECT * FROM ebanksumm "
            "WHERE bankid = '%s'"
            "  AND svcclass = %s"
            "  AND curcode = '%s'"
            "  AND curtype = '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            " ORDER BY workdate, branchid", 
            caAttechOrg,
            GetTrnCtl( "SvcClass" ),
            GetTrnCtl( "CurCode" ),
            GetTrnCtl( "CurType" ),
            startdate, 
            enddate
            );
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯ��������ʷ���ƾ֤ʧ��",
                __FILE__, __LINE__ );
        goto err_handle;
    }
    iRet = XMLAppendNode( datadoc, "/Report/ReportBody", DOCROOT(rsdoc) );
    if ( iRet != 0 )
    {
        SDKerrlog(ERRLOG, "%s|%d, XMLAppendNode() Fail !", __FILE__, __LINE__);
        goto err_handle;
    }

    // ��ѯ�����е�����ƾ֤ 
    iRet = DBQuery( &rsdoc2, "SELECT * FROM reconinfo "
            "WHERE bankid = '%s'"
            "  AND svcclass = %s"
            "  AND curcode = '%s'"
            "  AND curtype = '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            " ORDER BY workdate", 
            caAttechOrg,
            GetTrnCtl( "SvcClass" ),
            GetTrnCtl( "CurCode" ),
            GetTrnCtl( "CurType" ),
            GetTrnCtl( "StartDate" ),
            GetTrnCtl( "EndDate" )
            );
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯ��������ʷ���ƾ֤ʧ��",
                __FILE__, __LINE__ );
        goto err_handle;
    }

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.qdf",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    iRet = PrintReportList( caParaFile, caDataFile, caOutFile);
    if ( iRet != 0 )
        SetError( E_TR_PRINT );

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:
    /*
       if ( datadoc != NULL )
       xmlFreeDoc( datadoc );
       */
    if ( rsdoc != NULL )
        xmlFreeDoc( rsdoc );
    if ( rsdoc2 != NULL )
        xmlFreeDoc( rsdoc2 );

    return iRet;
    return 0;
}
#endif

// ������Ϣ��ѯ
int QryBankInfo( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/BankInfo.para", getenv("HOME") );

    chinesedate = ChineseDate(current_date());

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "%s;%s;", GetSysPara("SYSNAME"), chinesedate);

    iRet = db_query(&rs,
            "SELECT orgid, attechorg, exchflag, presproxy, acptproxy, "
            "orgabbr, linkman, phone, region, exchroute FROM organinfo "
            "WHERE orgid like '%s%%' ORDER BY orgid", GetTrnCtl("BankId"));
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯ������Ϣʧ��", __FILE__, __LINE__ );
        goto err_handle;
    }

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
            fprintf(fp, "%s;", db_cell(&rs, j, i));
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.org",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

int QryNoteInfo( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/NoteInfo.para", getenv("HOME") );

    chinesedate = ChineseDate(current_date());

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "%s;%s;", GetSysPara("SYSNAME"), chinesedate);

    iRet = db_query(&rs,
            "SELECT notetype, name, (case when svcclass=1 then '���ҶԹ�' "
            "when svcclass=2 then '���Ҹ���' when svcclass=3 then '���' when "
            "svcclass=4 then '����' end) as classname, (case when dupcheck='1'"
            " then '���' when dupcheck!='1' then '�����' end) as chkname, "
            "(case when dcflag='%d' then '���' when dcflag='%d' then '����' "
            "end) as dcname, (case when truncflag='1' then '����' when "
            "truncflag!='1' then '������' end) as truncname, (case when "
            "enableflag='1' then '����' when enableflag!='1' then '������' "
            "end) as enablename, notedesc FROM noteinfo "
            "WHERE notetype like '%s%%' "
            "AND dcflag LIKE '%s%%' "
            "AND truncflag LIKE '%s%%' "
            "ORDER BY notetype", PART_DEBIT, PART_CREDIT, 
            GetTrnCtl("NoteType"), GetTrnCtl("DCFlag"), 
            GetTrnCtl("TruncFlag"));
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯƾ֤��Ϣʧ��", __FILE__, __LINE__ );
        goto err_handle;
    }

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
        {
            fprintf(fp, "%s;", db_cell(&rs, j, i));
        }
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

//�������ѯ
int QryErrInfo( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *chinesedate = NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/ErrInfo.para", getenv("HOME") );

    chinesedate = ChineseDate(current_date());

    fp = fopen(GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "%s;%s;", GetSysPara("SYSNAME"), chinesedate);

    iRet = db_query(&rs,
            "SELECT errcode, errdesc FROM errmsg order by errcode");
    if ( iRet != 0 )
    {
        SDKerrlog( ERRLOG, "%s|%d, ��ѯ������Ϣʧ��", __FILE__, __LINE__ );
        goto err_handle;
    }

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
        {
            fprintf(fp, "%s;", db_cell(&rs, j, i));
        }
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    ifree(chinesedate);
    return iRet;
}

int QryOutQuery(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    long startdate;
    long enddate;

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/OutQuery.para", getenv("HOME") );

    startdate = atol(GetTrnCtl("StartDate"));
    if (startdate == 0L)
        startdate = atol(GetSysPara("WORKDATE"));
    enddate = atol(GetTrnCtl("EndDate"));
    if (enddate == 0L)
        enddate = atol(GetSysPara("WORKDATE"));

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname,
            GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            GetMsgHdrRq("Originator"), org_name(GetMsgHdrRq("Originator"), gs_bankname));

    iRet = db_query(&rs,
            "SELECT mailid, mailtype, recver, title, senddate, sendtime, "
            "sended FROM sendbox "
            "WHERE mailtype LIKE '%s%%'"
            "  AND sended LIKE '%s%%'"
            "  AND sender='%s'"
            "  AND senddate BETWEEN '%08ld' AND '%08ld' ", 
            GetTrnCtl("MailType"), 
            GetTrnCtl("Sended"),
            GetMsgHdrRq("Originator"),
            startdate, enddate);
    if (iRet != 0)
        return -1;

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
        {
            fprintf(fp, "%s;", db_cell(&rs, j, i));
        }
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    return iRet;
}

int QryInQuery(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    long startdate;
    long enddate;

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/report/InQuery.para", getenv("HOME") );

    startdate = atol(GetTrnCtl("StartDate"));
    if (startdate == 0L)
        startdate = atol(GetSysPara("WORKDATE"));
    enddate = atol(GetTrnCtl("EndDate"));
    if (enddate == 0L)
        enddate = atol(GetSysPara("WORKDATE"));

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname,
            GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            GetMsgHdrRq("Originator"), org_name(GetMsgHdrRq("Originator"), gs_bankname));

    iRet = db_query(&rs,
            "SELECT sender, mailid, mailtype, title, recvdate, recvtime, "
            "readed FROM recvbox "
            "WHERE mailtype LIKE '%s%%'"
            "  AND readed LIKE '%s%%'"
            "  AND recver='%s'"
            "  AND recvdate BETWEEN '%08ld' AND '%08ld' ", 
            GetTrnCtl("MailType"), 
            GetTrnCtl("Readed"),
            GetMsgHdrRq("Originator"),
            startdate, enddate);
    if (iRet != 0)
        return -1;

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
            fprintf(fp, "%s;", db_cell(&rs, j, i));
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        SetError( E_TR_PRINT );
        goto err_handle;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy( gcaResponseFile, basename(caOutFile), sizeof( gcaResponseFile ) );

err_handle:

    return iRet;
}

int WebQryFeeList(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
{
    xmlDocPtr rs = NULL;
    char date1[16], date2[16];
    char filename[256];
    char *p;

    // ���������ؼƷ���Ϣ
    if ( CommByNode( 0, xmlReq, &rs ) != 0)
        return -1;
    XMLFreeDoc(rs);

    p = GetMsgHdrRq("Reserve");
    if (sscanf(p, "%[^,],%s", date1, date2) == 0)
    {
        SDKerrlog(ERRLOG, "���������ʽ����.");
        SetError(E_GNR_DATAFMT);
        return -1;
    }

    // ���ԭ�м�¼
    if (db_exec("DELETE FROM feesum WHERE workdate BETWEEN '%s' AND '%s'",
                date1, date2) != 0)
    {
        SDKerrlog(ERRLOG, "����Ʒѱ�ɼ�¼ʧ��.");
        return -1;
    }

    sprintf(filename, "%s/%s", getenv("FILES_DIR"), gcaRecvFile);
    if (db_load_data("feesum", filename) != 0)
    {
        SDKerrlog(ERRLOG, "����Ʒѱ��¼ʧ��.");
        return -1;
    }

    return 0;
}

int QryFeeList(xmlDocPtr xmlReq, xmlDocPtr xmlRsp)
{
    int iRc;

    if (!strcmp(GetMsgHdrRq("Sender"), "999"))
        iRc = FeeSumQry(xmlReq, xmlRsp);
    else
        iRc = WebQryFeeList(xmlReq, xmlRsp);

    return iRc;
}

// ��ѯ��Ʊ������
int QryTPExcuseBook( xmlDocPtr xmlReq, xmlDocPtr xmlRsp )
{
    xmlDocPtr doc;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char inoutflag[1 + 1];   //���ձ�־
    char inoutname[5];
    char startdate[8 + 1];   //��ʼ����
    char enddate[8 + 1];     //��������
    char originator[13], acceptor[13];
    char refid[17], dcflag[2], workdate[9];
    char notetype[3], notetype_name[41], noteno[21];
    char settlamt[31], reason[61], flagname[10];
    char printtime[31];
    char buf[2048];
    result_set rs;
    FILE *fp=NULL;
    int rc = 0;
    int i;

    if (InitRptVar(xmlReq, xmlRsp) != 0)
        return -1;

    //�Խ��ա����͵���Ʊ��������д���
    XmlGetString(xmlReq, "/UFTP/TrnCtl/InOutFlag", 
            inoutflag, sizeof(inoutflag));
    if (inoutflag[0] == '0')
        strcpy(inoutname, "����");
    else
        strcpy(inoutname, "����");
    //ȡ��ʼ����
    XmlGetString(xmlReq, "/UFTP/TrnCtl/StartDate", 
            startdate, sizeof(startdate));
    //ȡ��������
    XmlGetString(xmlReq, "/UFTP/TrnCtl/EndDate", 
            enddate, sizeof(enddate));

    //���ǿ�ʼ���ڻ��߽�������Ϊ�յ����
    //��ʼ����Ϊ��(Ϊ��������ǰ���ڴ���)
    if (startdate[0] == 0)
        sprintf(startdate, "%08ld", current_date());
    //��������Ϊ��(Ҳͬ��������ǰ���ڴ���)
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    //�ж��ǽ��ջ��ǽ��ջ��Ƿ�����Ʊ������ 0 - ����,1 - ����
    if (!strcmp(inoutflag,"0"))
    {	
        //����
        rc = db_query(&rs, "SELECT * FROM recvbox WHERE mailtype='5' "
                "AND recver='%s' AND recvdate BETWEEN '%s' AND '%s'", 
                GetMsgHdrRq("Originator"), startdate, enddate);
    }
    else if (!strcmp(inoutflag,"1"))
    {	
        //����	
        rc = db_query(&rs, "SELECT * FROM sendbox WHERE mailtype='5' "
                "AND sender='%s' AND senddate BETWEEN '%s' AND '%s'", 
                GetMsgHdrRq("Originator"), startdate, enddate);
    }
    else
    {
        SetError(E_GNR_DATAFMT);
        return -1;
    }
    //���û�в鵽���߷����쳣���
    if ( rc != 0 )
    {
        err_log("û������Ҫ�����Ʊ������.");
        return -1;
    }

    snprintf(caParaFile, sizeof(caParaFile),
            "%s/report/TPQryExcuseBook.para", getenv("HOME"));
    if ((fp = fopen(GetTmpFileName(caDataFile), "w")) == NULL)
    {
        db_free_result(&rs);
        SetError(E_GNR_FILE_OPEN);
        return -1;
    }
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");

    //ѭ��ȡ����
    WriteRptHeader(fp, "%s;%s;%s;%s;%sʱ��;", gs_sysname, inoutname,
            startdate, enddate, inoutname);
    for (i = 0; i < db_row_count(&rs); i++)
    {
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
        sql_result(notetype_name, sizeof(notetype_name),
                "select name from noteinfo where notetype='%s' and dcflag='%s'",
                notetype, dcflag);
        if (inoutflag[0] == '0')
            strcpy(flagname, (*db_cell_by_name(&rs, i, "readed") == '0' 
                        ? "δ��ӡ" : "�Ѵ�ӡ"));
        else
            strcpy(flagname, (*db_cell_by_name(&rs, i, "sended") == '0' 
                        ? "δ����" : "�ѷ���"));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;\n", 
                originator, workdate, refid, notetype_name, noteno, settlamt, 
                reason, db_cell(&rs, i, 8)+4, db_cell(&rs, i, 9), flagname,
                acceptor);

        xmlFreeDoc(doc);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "%s;%s;%s;%s;", GetMsgHdrRq("Originator"), gs_bankname,
            printtime, GetMsgHdrRq("AcctOper"));
    fclose(fp); fp = NULL;

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.qtpr",
            getenv("FILES_DIR"),GetMsgHdrRq("Originator"),
            current_date()%100,current_time());
    rc = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (rc != 0)
    {
        SetError( E_TR_PRINT );
        return -1;
    }

    XmlSetString(xmlRsp, "/UFTP/MsgHdrRs/Reserve", basename(caOutFile));
    strncpy(gcaResponseFile, basename(caOutFile), sizeof(gcaResponseFile));

    return rc;
}

func_list uftp_call[ ] =
{
    { T_QRY_OUTNOTE, NULL, QryOutNote, NULL },
    { T_QRY_INNOTE,  NULL, QryInNote, NULL },
//    { T_QRY_CUR_DIFFNOTE, NULL, QryCurDiffNote, NULL },
//    { T_QRY_HIS_DIFFNOTE, NULL, QryHisDiffNote, NULL },
    { T_QRY_BANKINFO, NULL, QryBankInfo, NULL },
    { T_QRY_NOTETYPE, NULL, QryNoteInfo, NULL },
    { T_QRY_ERRINFO, NULL, QryErrInfo, NULL },
    { T_QRY_OUTQUERY, NULL, QryOutQuery, NULL },
    { T_QRY_INQUERY, NULL, QryInQuery, NULL },
    { T_QRY_EXCUSEBOOK, NULL, QryTPExcuseBook, NULL },
    { T_MNG_FEESUMQRY, NULL, QryFeeList, NULL },
    { -1, NULL, NULL, NULL }
};
