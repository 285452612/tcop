#include "comm.h"
#include "interface.h"

#define PS_MAXROWS 4          // 附言最大行数
#define PS_MAXCOLS 90        // 附言最大列数


int PrintInQuery( xmlDocPtr xmlReq, char *filename)
{
    xmlDocPtr doc = NULL;
    char caParaFile[256], caDataFile[256], caOutFile[256];
    char startrefid[17], endrefid[17], startdate[9], enddate[9], printdate[9], operno[7];
    result_set *rs=NULL;
    FILE *fp=NULL;
    char *content = NULL;
    char Originator[13], Acceptor[13], WorkDate[9], RefId[17], NoteType[5], NoteNo[31];
    char DCFlag[3], IssueDate[9], SettlAmt[21], IssueAmt[21], PayingAcct[33], Payer[81];
    char BeneAcct[33], BeneName[81], PayKey[30], Agreement[128], ClearState[5];
    char buf[128], condi[2048], purpose_str[1025];
    char purpose[6][101];
    int iRet = 0, updflag = 1;
    int j, i,k,pos;

    sprintf(printdate, "%08ld", current_date());
    strcpy(operno, XMLGetNodeVal(xmlReq, "//AcctOper"));

    strcpy(startdate, XMLGetNodeVal(xmlReq, "//StartDate"));
    if (startdate[0] == 0)
        sprintf(startdate, "%08ld", current_date());
    strcpy(enddate, XMLGetNodeVal(xmlReq, "//EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, startdate);

    strcpy(startrefid, XMLGetNodeVal(xmlReq, "//StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0");
    strcpy(endrefid, XMLGetNodeVal(xmlReq, "//EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "999999999");

    if (*XMLGetNodeVal(xmlReq, "//MailType") == '2')
    {
        snprintf( caParaFile, sizeof(caParaFile),
                "%s/dat/PrintInReQuery.para", getenv("HOME") );
    } else {
        snprintf( caParaFile, sizeof(caParaFile),
                "%s/dat/PrintInQuery.para", getenv("HOME") );
    }

    memset(buf, 0, sizeof(buf));
    switch(*XMLGetNodeVal(xmlReq, "//PrintState"))
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
            "  and nodeid = %d "
            "  AND mailtype LIKE '%s%%' %s",
            XMLGetNodeVal(xmlReq, "//TrnCtl/Acceptor"), startdate, enddate, 
            atol(startrefid), atoi(endrefid), OP_REGIONID,
            XMLGetNodeVal(xmlReq, "//MailType"), buf);
    /*
    iRet = db_DeclareCur("prtinqrycur", CURSOR_NORMAL, 
            "SELECT (case when mailtype='1' then '查询书' "
            "when mailtype='2' then '查复书' "
            "when mailtype='3' then '通知书' end) as typename, sender, writer,"
            " content, title, recvdate, recvtime, mailid FROM recvbox "
            "WHERE %s order by recvdate, mailid", condi);
     */
    DBUG( "condi=[%s]",condi);

    iRet = db_DeclareCur("prtinqrycur", CURSOR_NORMAL, 
            "SELECT mailid, recver, sender, recvdate, content FROM recvbox "
            "WHERE %s order by recvdate, mailid", condi);
    if ( iRet != 0 )
    {
        INFO( "查询提入查询查复书失败");
        goto err_handle;
    }

    // 打开游标
    db_OpenCur("prtinqrycur");
        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

    fp = fopen((char *)GetTmpFileName(caDataFile), "w");
    WriteRptHeader(fp, "");
    j = 0;
    for (;;j++)
    {
        char *orgname = NULL;
        iRet = db_FetchCur("prtinqrycur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        if (iRet < 0)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            goto err_handle;
        }
        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        fprintf(fp, "%s;%s;%s;%s;", 
                db_cell(rs, 0, 0),
                db_cell(rs, 0, 1),
                db_cell(rs, 0, 2),
                db_cell(rs, 0, 3));

        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

        content = encoding_conv(db_cell(rs, 0, 4), "GB18030", "UTF-8");
        if (content == NULL)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            db_free_result(rs);
            //SetError( E_TR_PRINT );
            return -1;
        }
        doc = xmlParseMemory(content, strlen(content));
        if (doc == NULL)
        {
            fclose(fp);
            db_CloseCur("prtinqrycur");
            db_free_result(rs);
            ifree(content);
            //SetError( E_TR_PRINT );
            return -1;
        }

        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
        memset(purpose , 0, sizeof(purpose));
        memset(purpose_str , 0, sizeof(purpose_str));
        XMLGetVal(doc, "//Purpose", purpose_str);
        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );

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

        //SDKerrlog( ERRLOG, "%s|%d ", __FILE__, __LINE__ );
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

    if (updflag == 1)
        DBExec("update recvbox set readed='1' where %s", condi);

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    return iRet;
}

/*
 * 打印对账单
 */
int PrintAcctList(xmlDocPtr doc, char *filename)
{
    char tbname[128];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char startdate[9];
    char enddate[9];
    char PayingAcctOrName[81],BeneAcctOrName[81];
    result_set *rs=NULL;
    FILE *fp=NULL;
    int iRet = 0;
    int j;
    char tmp[128]={0}, sResult[8+1]={0}, sSerial[16+1]={0};
    char sAcctType[20+1], sSqlStr[1024]={0};
    char settledate[16] = {0};

    strcpy(tbname, "trnjour");
    
    //当隔天第一场未对账时 对账场次是前一天最后场次
    //当天晚上归档时 将对账场次置0
    strcpy(settledate, GetSettledDateround());
    BKINFO("已取对账日期场次:%s 系统日期:%ld 机器日期:%ld", settledate, atol(GetWorkdate()), current_date());
    settledate[8] = 0;
    if((atoi(XMLGetNodeVal(doc, "//ClearRound")) > atoi(settledate+9)) || atol(settledate) != current_date())
    {
        BKINFO("未对账，不允许打印对账文件");
        return E_DB_NORECORD;
    }
    if(atoi(XMLGetNodeVal(doc, "//ClearRound")) != 0)
        sprintf(tmp, " and workround = '%s'", XMLGetNodeVal(doc, "//ClearRound"));
    else        //如果打印全部场次 要控制打印已对账场次的对账单
        sprintf(tmp, " and workround <= '%s'", GetSettledDateround()+9);
    sprintf(sSqlStr, "SELECT originator, refid, prestime, clearstate, notetype, dcflag,"
            "noteno, issuedate, curcode,settlamt, beneacct, payingacct, "
            "result, benename, payer, workround, acceptor, inoutflag FROM %s "
            "WHERE classid = %d AND nodeid = %d"
            "  and ((originator = '%s' and inoutflag = '1') or (acceptor = '%s' and inoutflag = '2'))"
            "  AND workdate = '%ld' and clearstate != '0' %s order by workround, clearstate, refid",
            tbname,
            atoi(XMLGetNodeVal( doc, "//SvcClass" )),
            //"CNY", //XMLGetNodeVal( xmlReq, "//CurCode" ),
            OP_REGIONID,
            XMLGetNodeVal(doc, "//Originator"),
            XMLGetNodeVal(doc, "//Originator"),
            current_date(),
            tmp
           );
    BKINFO("SQL:%s", sSqlStr);
    // 查询交换凭证
    iRet = db_DeclareCur("acctlistcur", CURSOR_NORMAL, sSqlStr);
    if ( iRet != 0 )
    {
        BKINFO( "查询交换凭证失败");
        goto err_handle;
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/BankAcctList.para", getenv("HOME") );

    fp = fopen((char *)GetTmpFileName(caDataFile), "w");
    
    WriteRptHeader(fp, "%08ld;%06ld;%s;", current_date(), current_time(),
            XMLGetNodeVal(doc, "//MsgHdrRq/AcctOper"));

    // 打开游标
    db_OpenCur("acctlistcur");

    j = 0;
    for (;;j++)
    {
        iRet = db_FetchCur("acctlistcur", &rs);
        if (iRet == SQLNOTFOUND)
            break;
        if (iRet < 0)
        {
            fclose(fp);
            goto err_handle;
        }
        memset(sSerial, 0, sizeof sSerial);
        strcpy(sSerial, db_cell(rs, 0, 1));
        DelZero(sSerial);
        memset(sAcctType, 0, sizeof sAcctType);
        memset(sSqlStr, 0, sizeof sSqlStr);
        sprintf(sSqlStr, "select acctserial, result from acctjour where nodeid = %d and workdate = '%ld' \
                and originator = '%s' and convert(decimal, refid) = %s and inoutflag = '%s'",
                OP_REGIONID,
                current_date(),
                db_cell(rs, 0, 0),
                sSerial, //db_cell(rs, 0, 1),
                db_cell(rs, 0, 17));
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
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                //current_date(),
                db_cell(rs, 0, 15), //工作场次
                db_cell(rs, 0, 1),
                db_cell(rs, 0, 4),
                atoi(db_cell(rs, 0, 5)) == 1 ? "借" : "贷",
                //db_cell(rs, 0, 6),
                db_cell(rs, 0, 0),
                db_cell(rs, 0, 16), //提入行
                db_cell(rs, 0, 10),
                db_cell(rs, 0, 11),
                FormatMoney(db_cell(rs, 0, 9)),
                tmp,//GetChineseName(clrstat_list, *db_cell(rs, i, 3)),
                sSerial,
                sAcctType
               );
        db_free_result(rs);
    }
    db_CloseCur("acctlistcur");

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
        "NTSenderProxy",
        "NTRecverProxy",
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
        if (sdpXmlSelectNode(doc, path) == NULL)
            continue;
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
int PrintInNoteAdd( xmlDocPtr xmlReq, char *filename)
{
#if 1
    char tbname[128], tbname_ex[128], condi[4096];
    char caParaFile[256], caDataFile[256], caOutFile[256];
    char startrefid[17], endrefid[17], amount[128];
    char buf[256], datetime[30], ps[PS_MAXROWS][256], key[200];
    char sCurName[10+1]={0};
    char *p;
    result_set *rs=NULL;
    result_set *ex=NULL;
    FILE *fp=NULL;
    int iRet = 0, iErrCode, i;

    strcpy(startrefid, XMLGetNodeVal(xmlReq, "//StartRefId"));
    if (startrefid[0] == 0)
        strcpy(startrefid, "0000000000");
    strcpy(endrefid, XMLGetNodeVal(xmlReq, "//EndRefId"));
    if (endrefid[0] == 0)
        strcpy(endrefid, "9999999999");

    if (DiffDate(XMLGetNodeVal(xmlReq, "//WorkDate"), GetArchivedate()) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/InNoteAdd.para", getenv("HOME") );


    memset(buf, 0, sizeof(buf));
    switch(*XMLGetNodeVal(xmlReq, "//PrintState"))
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
    if (*XMLGetNodeVal(xmlReq, "//MaxAmount") != 0x00)
        sprintf(amount, "AND settlamt = %s", XMLGetNodeVal(xmlReq, "//MaxAmount"));
    snprintf(condi, sizeof(condi), 
            "inoutflag='%c' AND workdate = '%s' AND classid = %d"
            "  AND refid BETWEEN '%s' AND '%s' AND acceptor LIKE '%s%%'"
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%' "
            "  AND beneacct LIKE '%s%%' %s"
            "  AND dcflag LIKE '%s%%' AND truncflag LIKE '%s%%' %s"
            "  AND errcode in('%04d', '%04d', '%04d')",
            '2', XMLGetNodeVal(xmlReq, "//WorkDate"), atoi(XMLGetNodeVal(xmlReq, "//SvcClass")),
            startrefid, endrefid, XMLGetNodeVal(xmlReq, "//Acceptor"), 
            XMLGetNodeVal(xmlReq, "//NoteType"), XMLGetNodeVal(xmlReq, "//NoteNo"),
            XMLGetNodeVal(xmlReq, "//BeneAcct"), amount,
            XMLGetNodeVal(xmlReq, "//DCFlag"), XMLGetNodeVal(xmlReq, "//TruncFlag"), buf,
            0, 8048, 308);

    iRet = db_query(&rs, "SELECT * FROM %s WHERE %s "
            "ORDER BY classid, curcode, seqno", tbname, condi);
    if ( iRet != 0 )
    {
        DBUG( "查询提入补充凭证失败, condi=[%s][%s]", tbname, condi );
        return E_DB;
    }

    // 打印时间
    GetDateTime(datetime);

    if ((fp = fopen((char *)GetTmpFileName(caDataFile), "w")) == NULL)
    {
        db_free_result(rs);
        //SetError(E_GNR_FILE_OPEN);
        return E_SYS_CALL;
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
            strcat(key, (char *)errmsg(atoi(db_cell_by_name(rs, i, "errcode"))));
        }

        // 大写金额
        MoneyToChinese(db_cell_by_name(rs, i, "settlamt"), buf);

        memset(ps, 0, sizeof(ps));
        if (atof(db_cell_by_name(rs, i, "issueamt")) > 0)
           sprintf(ps[0], "签发金额: %s ",db_cell_by_name(rs, i, "issueamt"));

/*
        if (atoi(db_cell_by_name(rs, i, "extradataflag")) == 1)
        {
            iRet = db_query(&ex, "select extradata from %s "
                    "where workdate='%s' and seqno=%s", tbname_ex,
                    XMLGetNodeVal(xmlReq, "//WorkDate"), db_cell_by_name(rs, i, "seqno"));
            if (iRet == 0)
            {
*/
                GetExtraData(db_cell(rs, i, 44), ps);
/*
                db_free_result(ex);
            }
        }
*/

        memset(sCurName, 0, sizeof sCurName);
        if(strcmp("CNY", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "人民币");
        else if(strcmp("HKD", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "港币");
        else if(strcmp("USD", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "美元");
        else if(strcmp("GBP", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "英镑");
        else if(strcmp("EUR", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "欧元");
        else if(strcmp("JPY", db_cell_by_name(rs, i, "curcode")) == 0)
            strcpy(sCurName, "日元");
        //fprintf(fp, "%s (%s);%s;%s;第%s场;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;%s %s;%s;"
        fprintf(fp, "%s;%s;%s;第%s场;%s;%s;%s;%s;%s;%s;%s %s;%s;%s;%s %s;%s;"
                "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;\n",
                db_cell_by_name(rs, i, "originator"),
                //db_cell_by_name(rs, i, "termid"),
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
                sCurName, //ChsName(curcode_list, db_cell_by_name(rs, i, "curcode")),
                buf, 
                db_cell_by_name(rs, i, "curcode"),
                FormatMoney(db_cell_by_name(rs, i, "settlamt")), 
                db_cell_by_name(rs, i, "purpose"),
                key,
                ps[0], ps[1], ps[2], ps[3],
                datetime,
                atoi(db_cell_by_name(rs, i, "printnum"))+1,
                XMLGetNodeVal(xmlReq, "//AcctOper"));
    }
    db_free_result(rs);

    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    GetTmpFileName(caOutFile);
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        //SetError( E_TR_PRINT );
        return E_SYS_CALL;
    }

    DBExec("update %s set printnum=printnum+1 where %s", tbname, condi);
    //SetError(0);
    iRet = 0;

    //SetUFTPField( "Reserve", basename(caOutFile), xmlRsp );
    sprintf( filename, "%s", basename(caOutFile));

    return iRet;
#endif
}
