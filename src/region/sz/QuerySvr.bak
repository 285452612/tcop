/*********************************************
  QuerySvr.c
  查询服务
  Chen Jie, May 2007
 *********************************************/

#include "comm.h"
#include "interface.h"

#define org_name(orgid) \
get_tb_value("select organname from organinfo where exchno='%s'", orgid)

/*
 * 对流水号前面去0
 */
int DelZero(char *pSerial)
{
    int i;
    char sSerial[16+1]={0};

    strcpy(sSerial, pSerial);
    for(i=0;i<strlen(sSerial);i++)
    {
        if(sSerial[i] != '0')
            break;
    }
    sprintf(pSerial, "%s", sSerial+i);
    return 0;
}

int QryOutNote( xmlDocPtr xmlReq, char *filename)
{
    char tbname[128];
    result_set *rs=NULL;
    char caParaFile[256], caDataFile[256], caOutFile[256];
    char startdate[9], enddate[9];
    char PayingAcctOrName[81],BeneAcctOrName[81];
    FILE *fp=NULL;
    int iRet = 0, i;
    char tmp[60], sAcctType[40+1], sSerial[8+1], sResult[8+1], sSqlStr[1024];
    char sMemo[64];

    strcpy(startdate, XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, GetWorkdate());
    strcpy(enddate, XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, GetWorkdate());

    if (DiffDate(startdate, GetArchivedate()) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    // 查询交换凭证
    iRet = db_query(&rs, 
            "SELECT distinct a.refid, a.prestime, a.clearstate, a.acceptor, a.notetype, a.dcflag, "
            "a.noteno, a.issuedate, a.curcode,a.settlamt, a.beneacct, a.payingacct, "
            "a.benename, a.payer, a.result, b.errinfo, a.workdate FROM %s a, errinfo b "
            "WHERE a.originator LIKE '%s%%'"
            "  AND a.acceptor LIKE '%s%%'"
            "  AND a.classid = %d"
            "  AND a.curcode LIKE '%s%%'"
            "  AND a.notetype LIKE '%s%%'"
            "  AND a.noteno LIKE '%s%%'"
            "  AND a.dcflag LIKE '%s%%'"
            "  AND a.clearstate LIKE '%s%%'"
            "  AND a.settlamt BETWEEN %.2lf AND %.2lf"
            "  AND a.refid BETWEEN '%s' AND '%s'"
            "  AND a.workdate BETWEEN '%s' AND '%s'"
            "  AND a.inoutflag = '%c' and a.result = convert(int, b.errcode) and a.nodeid = %d",
            tbname,
            XMLGetNodeVal( xmlReq, "//MsgHdrRq/Originator" ), XMLGetNodeVal( xmlReq, "//TrnCtl/Acceptor" ),
            atoi(XMLGetNodeVal( xmlReq, "//TrnCtl/SvcClass" )), XMLGetNodeVal( xmlReq, "//CurCode" ),
            XMLGetNodeVal( xmlReq, "//NoteType" ), XMLGetNodeVal( xmlReq, "//NoteNo" ),
            XMLGetNodeVal( xmlReq, "//DCFlag" ), XMLGetNodeVal( xmlReq, "//ClearState" ),
            atof(XMLGetNodeVal( xmlReq, "//MinAmount")), atof(XMLGetNodeVal( xmlReq, "//MaxAmount")),
            XMLGetNodeVal( xmlReq, "//StartRefId" ), XMLGetNodeVal( xmlReq, "//EndRefId" ),
            startdate, enddate, '1', OP_REGIONID);
    if ( iRet != 0 )
    {
        return E_DB_NORECORD;
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/OutNote.para", getenv("HOME") );

    fp = fopen((char *)GetTmpFileName(caDataFile), "w");
    
    WriteRptHeader(fp, "%08ld;%06ld;%s;%s;%s;", current_date(), current_time(),
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator"), org_name(XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator")), 
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/AcctOper"));

    for (i = 0; i < db_row_count(rs); i++)
    {
        strcpy(BeneAcctOrName, db_cell_by_name(rs, i, "beneacct"));
        all_trim(BeneAcctOrName);
        if(strlen(BeneAcctOrName) == 0)
        {
            strcpy(BeneAcctOrName, db_cell_by_name(rs, i, "benename"));
            all_trim(BeneAcctOrName);
            BeneAcctOrName[32] = 0;
        }
        strcpy(PayingAcctOrName, db_cell_by_name(rs, i, "payingacct"));
        all_trim(PayingAcctOrName);
        if(strlen(PayingAcctOrName) == 0)
        {
            strcpy(PayingAcctOrName, db_cell_by_name(rs, i, "payer"));
            all_trim(PayingAcctOrName);
            PayingAcctOrName[32] = 0;
        }

        memset(sAcctType, 0, sizeof sAcctType);
        memset(sSqlStr, 0, sizeof sSqlStr);
        sprintf(sSqlStr, "select acctserial, result from acctjour where nodeid = %d and workdate = '%s' \
                and originator = '%s' and refid = '%s' and inoutflag = '%s'",
                OP_REGIONID,
                db_cell_by_name(rs, i, "workdate"),
                XMLGetNodeVal( xmlReq, "//MsgHdrRq/Originator" ),
                db_cell_by_name(rs, i, "refid"),
                "1");
        memset(sResult, 0, sizeof sResult);
        memset(sSerial, 0, sizeof sSerial);
        iRet = DBQueryStrings(sSqlStr, 2, sSerial, sResult);
        if(iRet && iRet != E_DB_NORECORD)
        {
            BKINFO("查询记帐流水失败");
            return iRet;
        }
        else if(iRet == E_DB_NORECORD)
        {
            //BKINFO("记帐流水不存在");
            strcpy(sAcctType, "未记帐");
        }
        else if(atoi(sResult) == 0)
            strcpy(sAcctType, "未记帐");
        else if(atoi(sResult) == 1)
            strcpy(sAcctType, "已记帐");
        else if(atoi(sResult) == 2)
            strcpy(sAcctType, "已冲正");
        else if(atoi(sResult) == 3)
            strcpy(sAcctType, "已取消");
        else
            strcpy(sAcctType, "未知");

        memset(tmp, 0, sizeof tmp);
        switch(*db_cell_by_name(rs, i, "clearstate"))
        {
            case CLRSTAT_UNSETTLED: strcpy(tmp, "未清算"); break;
            case CLRSTAT_SETTLED: strcpy(tmp, "清算成功"); break;
            case CLRSTAT_FAILED: strcpy(tmp, "清算失败"); break;
            case CLRSTAT_UNKNOW: strcpy(tmp, "状态未知"); break;
            case CLRSTAT_CHECKED: strcpy(tmp, "已对账"); break;
        }
        memset(sMemo, 0, sizeof sMemo);
        memcpy(sMemo, db_cell_by_name(rs, i, "errinfo"), 16);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(rs, i, "refid"),
                //db_cell_by_name(rs, i, "prestime"),
                tmp,//GetChineseName(clrstat_list, *db_cell_by_name(rs, i, "clearstate")),
                db_cell_by_name(rs, i, "acceptor"),
                db_cell_by_name(rs, i, "notetype"),
                db_cell_by_name(rs, i, "dcflag"),
                db_cell_by_name(rs, i, "noteno"),
                db_cell_by_name(rs, i, "issuedate"),
                db_cell_by_name(rs, i, "curcode"),
                FormatMoney(db_cell_by_name(rs, i, "settlamt")),
                BeneAcctOrName,
                PayingAcctOrName,
              //  db_cell_by_name(rs, i, "beneacct"),
              //  db_cell_by_name(rs, i, "payingacct"),
                //db_cell_by_name(rs, i, "result"));
                sMemo, //db_cell_by_name(rs, i, "errinfo"),
                sSerial, sAcctType);
    }
    db_free_result(rs);

    WriteRptRowCount(fp, db_row_count(rs));
    WriteRptFooter(fp, "");
    fclose(fp);

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        //SetError( E_TR_PRINT );
        iRet = E_SYS_CALL;
        goto err_handle;
    }

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));

err_handle:
    return iRet;
}

int QryInNote( xmlDocPtr xmlReq, char *filename)
{
    char tbname[128];
    char caParaFile[256], caDataFile[256], caOutFile[256];
    char startdate[9], enddate[9];
    char PayingAcctOrName[81],BeneAcctOrName[81];
    result_set *rs=NULL;
    FILE *fp=NULL;
    int iRet = 0, j;
    char tmp[60], sAcctType[40+1], sSerial[16+1], sResult[8+1], sSqlStr[1024];
    char sMemo[64];

    strcpy(startdate, XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, getDate(0));
    strcpy(enddate, XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, getDate(0));

    if (DiffDate(startdate, GetArchivedate()) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");
    
    // 查询交换凭证
    iRet = db_DeclareCur("innotecur", CURSOR_NORMAL, 
            "SELECT distinct a.originator, a.refid, a.prestime, a.clearstate, a.notetype, a.dcflag,"
            "a.noteno, a.issuedate, a.curcode,a.settlamt, a.beneacct, a.payingacct, "
            "a.result, a.benename, a.payer, b.errinfo, a.workdate  FROM %s a, errinfo b "
            "WHERE a.originator LIKE '%s%%'"
            "  AND a.acceptor LIKE '%s%%'"
            "  AND a.classid = %d"
            "  AND a.curcode LIKE '%s%%'"
            "  AND a.notetype LIKE '%s%%'"
            "  AND a.noteno LIKE '%s%%'"
            "  AND a.dcflag LIKE '%s%%'"
            "  AND a.settlamt BETWEEN %.2lf AND %.2lf"
            "  AND a.refid BETWEEN '%s' AND '%s'"
            "  AND a.workdate BETWEEN '%s' AND '%s'"
            "  AND a.inoutflag = '%c' and a.result = convert(int, b.errcode) and a.nodeid = %d",
            tbname,
            XMLGetNodeVal( xmlReq, "//TrnCtl/Originator" ),
            XMLGetNodeVal( xmlReq, "//MsgHdrRq/Originator" ),
            atoi(XMLGetNodeVal( xmlReq, "//TrnCtl/SvcClass" )),
            XMLGetNodeVal( xmlReq, "//CurCode" ),
            XMLGetNodeVal( xmlReq, "//NoteType" ),
            XMLGetNodeVal( xmlReq, "//NoteNo" ),
            XMLGetNodeVal( xmlReq, "//DCFlag" ),
            atof(XMLGetNodeVal(xmlReq, "//MinAmount")),
            atof(XMLGetNodeVal(xmlReq, "//MaxAmount")),
            XMLGetNodeVal( xmlReq, "//StartRefId" ),
            XMLGetNodeVal( xmlReq, "//EndRefId" ),
            startdate,
            enddate,
            '2',
            OP_REGIONID
           );
    if ( iRet != 0 )
    {
        INFO( "查询交换凭证失败");
        goto err_handle;
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/InNote.para", getenv("HOME") );

    fp = fopen((char *)GetTmpFileName(caDataFile), "w");
    
    WriteRptHeader(fp, "%08ld;%06ld;%s;%s;%s;", current_date(), current_time(),
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator"), org_name(XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator")), 
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/AcctOper"));

    // 打开游标
    db_OpenCur("innotecur");

    j = 0;
    for (;;j++)
    {
        iRet = db_FetchCur("innotecur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        if (iRet < 0)
        {
            fclose(fp);
            goto err_handle;
        }

        strcpy(BeneAcctOrName, db_cell(rs, 0, 10));
        all_trim(BeneAcctOrName);
        if(strlen(BeneAcctOrName) == 0)
        {
            strcpy(BeneAcctOrName, db_cell(rs, 0, 13));
            all_trim(BeneAcctOrName);
            BeneAcctOrName[32] = 0;
        }
        strcpy(PayingAcctOrName, db_cell(rs, 0, 11));
        all_trim(PayingAcctOrName);
        if(strlen(PayingAcctOrName) == 0)
        {
            strcpy(PayingAcctOrName, db_cell(rs, 0, 14));
            all_trim(PayingAcctOrName);
            PayingAcctOrName[32] = 0;
        }

        memset(sSerial, 0, sizeof sSerial);
        strcpy(sSerial, db_cell(rs, 0, 1));
        //流水号前去0 来帐对方可能没有在流水号前补0 但网点查询时的流水号补0了
        DelZero(sSerial);
        memset(sAcctType, 0, sizeof sAcctType);
        memset(sSqlStr, 0, sizeof sSqlStr);
        sprintf(sSqlStr, "select acctserial, result from acctjour where nodeid = %d and workdate = '%s' \
                and originator = '%s' and convert(decimal, refid) = %s and inoutflag = '%s'",
                OP_REGIONID,
                db_cell(rs, 0, 16),
                db_cell(rs, 0, 0),
                sSerial, //db_cell(rs, 0, 1),
                "2");
        memset(sResult, 0, sizeof sResult);
        memset(sSerial, 0, sizeof sSerial);
        iRet = DBQueryStrings(sSqlStr, 2, sSerial, sResult);
        if(iRet && iRet != E_DB_NORECORD)
        {
            BKINFO("查询记帐流水失败");
            return iRet;
        }
        else if(iRet == E_DB_NORECORD)
        {
            BKINFO("记帐流水不存在");
            strcpy(sAcctType, "未记帐");
        }
        else if(atoi(sResult) == 0)
            strcpy(sAcctType, "未记帐");
        else if(atoi(sResult) == 1)
            strcpy(sAcctType, "已记帐");
        else if(atoi(sResult) == 2)
            strcpy(sAcctType, "已冲正");
        else if(atoi(sResult) == 3)
            strcpy(sAcctType, "已取消");
        else
            strcpy(sAcctType, "未知");

        memset(tmp, 0, sizeof tmp);
        switch(*db_cell(rs, 0, 3))
        {
            case CLRSTAT_UNSETTLED: strcpy(tmp, "未清算"); break;
            case CLRSTAT_SETTLED: strcpy(tmp, "清算成功"); break;
            case CLRSTAT_FAILED: strcpy(tmp, "清算失败"); break;
            case CLRSTAT_UNKNOW: strcpy(tmp, "状态未知"); break;
            case CLRSTAT_CHECKED: strcpy(tmp, "已对账"); break;
        }
        memset(sMemo, 0, sizeof sMemo);
        memcpy(sMemo, db_cell(rs, 0, 15), 16);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(rs, 0, 0),
                db_cell(rs, 0, 1),
                //db_cell(rs, 0, 2),
                tmp,//GetChineseName(clrstat_list, *db_cell(rs, i, 3)),
                db_cell(rs, 0, 4),
                db_cell(rs, 0, 5),
                db_cell(rs, 0, 6),
                db_cell(rs, 0, 7),
                db_cell(rs, 0, 8),
                FormatMoney(db_cell(rs, 0, 9)),
                BeneAcctOrName,
                PayingAcctOrName,
             //   db_cell(rs, 0, 10),
             //   db_cell(rs, 0, 11),
                //db_cell(rs, 0, 12));
                sMemo, //db_cell(rs, 0, 15),
                sSerial,
                sAcctType);
        db_free_result(rs);
    }
    db_CloseCur("innotecur");

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ( j == 0)
    {
        //SetError( E_GNR_RECNOTFOUND );
        iRet = E_DB_NORECORD;
        goto err_handle;
    }

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        //SetError( E_TR_PRINT );
        iRet = E_SYS_CALL;
        goto err_handle;
    }

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf(filename, "%s", basename(caOutFile));

err_handle:

    return iRet;
}

int QryOutQuery(xmlDocPtr xmlReq, char *filename)
{
    result_set *rs = NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char sysname[41]={0},tmp[60]={0};
    long startdate;
    long enddate;

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/OutQuery.para", getenv("HOME") );

    strcpy(sysname,GetSysName()); 
    startdate = atol(XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate == 0L)
        startdate = atol(GetWorkdate());
    enddate = atol(XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate == 0L)
        enddate = atol(GetWorkdate());

    switch(atoi(XMLGetNodeVal(xmlReq, "//MailType")))
    {
        case 1:strcpy(tmp, "查询书");break;
        case 2:strcpy(tmp, "查复书");break;
        case 3:strcpy(tmp, "通知");break;
        default:strcpy(tmp, "未知");break;
    }
    fp = fopen((char *)GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", 
            sysname, tmp,//GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator"), org_name(XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator")));

    iRet = db_DeclareCur("outquerycur", CURSOR_NORMAL, 
            "SELECT mailid, mailtype, recver, title, senddate, sendtime, "
            "sended FROM sendbox "
            "WHERE mailtype LIKE '%s%%'"
            "  AND sended LIKE '%s%%'"
            "  AND senddate BETWEEN '%08ld' AND '%08ld' "
            "  and nodeid = %d ",
            XMLGetNodeVal(xmlReq, "//MailType"), 
            XMLGetNodeVal(xmlReq, "//Sended"),
            startdate, enddate, OP_REGIONID);
    if (iRet != 0)
        return -1;

    // 打开游标
    db_OpenCur("outquerycur");

    j = 0;
    for (;;j++)
    {
        iRet = db_FetchCur("outquerycur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        if (iRet < 0)
        {
            fclose(fp);
            goto err_handle;
        }

        for (i = 0; i < rs->col_count; i++)
        {
            fprintf(fp, "%s;", db_cell(rs, 0, i));
        }
        fprintf(fp, "\n");
        db_free_result(rs);
    }
    db_CloseCur("outquerycur");

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ( j == 0)
    {
        //SetError( E_GNR_RECNOTFOUND );
        iRet = E_DB_NORECORD;
        goto err_handle;
    }

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        //SetError( E_TR_PRINT );
        iRet = E_SYS_CALL;
        goto err_handle;
    }

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    //ifree(sysname);
    return iRet;
}

int QryInQuery(xmlDocPtr xmlReq, char *filename)
{
    result_set *rs = NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char sysname[41]={0},tmp[60]={0};
    long startdate;
    long enddate;

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/InQuery.para", getenv("HOME") );

    strcpy(sysname, GetSysName()); 
    startdate = atol(XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate == 0L)
        startdate = atol(GetWorkdate());
    enddate = atol(XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate == 0L)
        enddate = atol(GetWorkdate());

    switch(atoi(XMLGetNodeVal(xmlReq, "//MailType")))
    {
        case 1:strcpy(tmp, "查询书");break;
        case 2:strcpy(tmp, "查复书");break;
        case 3:strcpy(tmp, "通知");break;
        default:strcpy(tmp, "未知");break;
    }
    fp = fopen((char *)GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", 
            sysname, tmp,//GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator"), org_name(XMLGetNodeVal(xmlReq, "//MsgHdrRq/Originator")));

    iRet = db_DeclareCur("inquerycur", CURSOR_NORMAL, 
            "SELECT sender, mailid, mailtype, title, recvdate, recvtime, "
            "readed FROM recvbox "
            "WHERE mailtype LIKE '%s%%'"
            "  AND readed LIKE '%s%%'"
            "  AND recvdate BETWEEN '%08ld' AND '%08ld' "
            "  and nodeid = %d ",
            XMLGetNodeVal(xmlReq, "//MailType"), 
            XMLGetNodeVal(xmlReq, "//Readed"),
            startdate, enddate, OP_REGIONID);
    if (iRet != 0)
        return -1;

    // 打开游标
    db_OpenCur("inquerycur");

    j = 0;
    for (;;j++)
    {
        iRet = db_FetchCur("inquerycur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        if (iRet < 0)
        {
            fclose(fp);
            goto err_handle;
        }

        for (i = 0; i < rs->col_count; i++)
        {
            fprintf(fp, "%s;", db_cell(rs, 0, i));
        }
        fprintf(fp, "\n");
        db_free_result(rs);
    }
    db_CloseCur("inquerycur");

    WriteRptRowCount(fp, j);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ( j == 0)
    {
        //SetError( E_GNR_RECNOTFOUND );
        iRet = E_DB_NORECORD;
        goto err_handle;
    }

    iRet = PrintReportList(caParaFile, caDataFile, GetTmpFileName(caOutFile));
    if (iRet != 0)
    {
        //SetError( E_TR_PRINT );
        iRet = E_SYS_CALL;
        goto err_handle;
    }

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));
    /*
    DBExec("UPDATE recvbox SET readed='1' WHERE mailtype LIKE '%s%%' AND readed LIKE '%s%%'  AND recvdate BETWEEN '%08ld' AND '%08ld' ",
            XMLGetNodeVal(xmlReq, "//MailType"), 
            XMLGetNodeVal(xmlReq, "//Readed"),
            startdate, enddate);
            */

err_handle:

    //ifree(sysname);
    return iRet;
}

