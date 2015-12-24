#include "comm.h"
#include "interface.h"

// 邮件查询
int MailQuery(xmlDocPtr xmlReq, char *filename)
{
    result_set *rs = NULL;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char condi[1024];
    char sysname[41]={0};
    long startdate;
    long enddate;

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/MailQuery.para", getenv("HOME") );

    strcpy(sysname,GetSysName());
    startdate = atol((char *)XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate == 0L)
        startdate = atol((char *)GetWorkdate());
    enddate = atol((char *)XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate == 0L)
        enddate = atol((char *)GetWorkdate());

    fp = fopen((char *)GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;",
            sysname, ChineseDate(startdate), ChineseDate(enddate));

    sprintf(condi, "mailtype='%c' AND recver='%s' AND readed LIKE '%s%%'"
            " AND recvdate BETWEEN '%08ld' AND '%08ld' and nodeid = %d", '0',
            XMLGetNodeVal(xmlReq, "//Originator"), XMLGetNodeVal(xmlReq, "//Readed"),
            startdate, enddate, OP_REGIONID);
    INFO("SQL:%s", condi);

    iRet = db_DeclareCur("mailqrycur", CURSOR_NORMAL,
            "SELECT sender, writer, svcclass, "
            "title, content, recvdate, recvtime, mailid"
            " FROM recvbox WHERE %s", condi);
    if (iRet != 0)
        return -1;

    // 打开游标
    db_OpenCur("mailqrycur");

    j = 0;
    for (;;j++)
    {
        iRet = db_FetchCur("mailqrycur", &rs);
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
    db_CloseCur("mailqrycur");

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

    DBExec("UPDATE recvbox SET readed='1' WHERE %s", condi);

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    //ifree(sysname);
    return iRet;
}
