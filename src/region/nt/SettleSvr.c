#include "interface.h"

static char g_respfile[256];

//int GenReconInfo(int nodeid, int svcclass, char *workdate, int workround);

int CenterAndBankSettle(char *dzRspFile, int classid, char *workdate, int workround);

int RecFeeSum(char *svcclass, char *workdate, int workround)
{
    char path[256];
    char condi[500];

    snprintf(path, sizeof(path), "%s/fee_%s_%s_%d_%s.dat", 
            getenv("FILES_DIR"), GetCBankno(), workdate, workround,
            svcclass);
    if (!IsExistFile(path))
        return 0;

    if (VerifyFileAndMove(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| 收费文件 %s MAC检验错 !",
                __FILE__, __LINE__, path );
        return -1;
    }

    if (ProcessDBFile(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| 处理收费文件 %s 失败!", __FILE__, __LINE__, path);
        return -1;
    }

    memset(condi,0,sizeof(condi));

    if( workround != 0) 
        sprintf(condi, "nodeid=%d and classid=%s and workdate='%s' and workround = %d", 
                OP_REGIONID, svcclass, workdate, workround);
    else
        sprintf(condi, "nodeid=%d and classid=%s and workdate='%s' ", 
                OP_REGIONID, svcclass, workdate);
    DBExec("delete from feesum where %s ", condi);

    return DBImportFile("feesum", path, '|');
}

// 补记流水函数
int RecTrnjourByNode(xmlNodePtr node)
{
    xmlDoc *doc = NULL;
    char nodeid[3] = {0};

    doc = XMLDumpNodeAsDoc(node);
    if (doc == NULL)
    {
        SDKerrlog(ERRLOG, "%s|%d| 补记流水时转换节点到文档出错!", __FILE__, __LINE__);
        return -1;
    }

    sprintf(nodeid, "%d", OP_REGIONID);
    XMLSetNodeVal(doc, "//SeqNo", nodeid);

    if (InsertTableByID(doc, "trnjour", 900001) != 0)
        return -2;

    DBUG("补记流水[%s|%s->%s]成功!", XMLGetNodeVal(doc, "//InOutFlag"), 
            XMLGetNodeVal(doc, "//Originator"), XMLGetNodeVal(doc, "//RefId"));

    /*
    int fieldnum;
    char expr[8192];
    char tmp[512];
    char workdate[9];
    char extradata[2049];
    long serialno;
    int extradataflag = 0;
    int i;

    fieldnum = sizeof(stTrnjour) / sizeof(stTrnjour[0]);

    memset(expr, 0, sizeof expr);

    for (i = 0; i < fieldnum; i++)
    {
        // CHAR数据类型补单引号
        if (stTrnjour[i].type == DATATYPE_CHAR)
            strcat(expr, "'");

        // 前置流水由前置自动产生
        if (!strcmp(stTrnjour[i].name, "SeqNo"))
        {
            serialno = GenSerial();
            sprintf(tmp, "%ld,", serialno);
            strcat(expr, tmp);
            continue;
        }

        if (!strcmp(stTrnjour[i].name, "ExtraDataFlag"))
        {
            XMLGetField(node, "ExtraData", extradata);
            if (extradata[0] == 0)
                strcat(expr, "0',");
            else
            {
                extradataflag = 1;
                strcat(expr, "1',");
            }
            continue;
        }


        // 取 xmlNode 对应值
        XMLGetField(node, stTrnjour[i].name, tmp);
        if (!strcmp(stTrnjour[i].name, "ClearState") && tmp[0] == CLRSTAT_SETTLED )
            tmp[0] = CLRSTAT_CHECKED;

        // 保存 WORKDATE, 记录附加信息用
        if (!strcmp(stTrnjour[i].name, "WorkDate"))
            strcpy(workdate, tmp);

        // 防止某些INT和DOUBLE类型字段为空值
        if (tmp[0] == 0)
        { 
            switch(stTrnjour[i].type)
            {
                case DATATYPE_INT:
                    strcat(expr, "0,");
                    break;
                case DATATYPE_DBL:
                    strcat(expr, "0.0,");
                    break;
                default:
                    strcat(expr, "',");
                    break;
            }
            continue;
        }

        strsrpl(tmp, "'", "''");
        strcat(expr, tmp);

        if (stTrnjour[i].type == DATATYPE_CHAR)
            strcat(expr, "'");
        strcat(expr, ",");
    }

    // 去掉最后一个逗号
    expr[strlen(expr)-1] = 0;

    if (DBExec("INSERT INTO trnjour VALUES(%s)", expr) != 0)
        return -1;

    if (extradataflag == 1)
    {
        if (DBExec("INSERT INTO extradata VALUES(%ld, '%s', '%s')", 
                    serialno, workdate, extradata) != 0)
            return -1;
    }
    */

    return 0;
}

// 明细对账处理
int ProcSettle(char *svcclass, char *workdate, int workround)
{
    xmlDocPtr doc = NULL;
    xmlXPathObjectPtr rs = NULL;
    xmlNodePtr node;
    FILE *fp;
    char filename[256];
    char buf[512];
    char expr[1024];
    char condi[1024];
    char line[1024];
    int  iRc = -1;
    int i, count = 0, exist = 0;

    snprintf(buf, sizeof(buf), "%s/%s_%s_%d_%s", getenv("FILES_DIR"), 
            GetCBankno(), workdate, workround, svcclass);
    snprintf(filename, sizeof(filename), "%s.dat", buf);
    snprintf(g_respfile, sizeof(g_respfile), "%s.recon_result", buf);
    
    if ((fp = fopen(g_respfile, "w")) == NULL)
    {
        SDKerrlog(ERRLOG, "%s|%d, 对账回执文件[%s]无法创建!",
                __FILE__, __LINE__, g_respfile);
        return -1;
    }

    doc = xmlParseFile(filename);
    if (doc == NULL)
    {
        SDKerrlog(ERRLOG, "%s|%d, 明细对账文件[%s]无法打开!",
                __FILE__, __LINE__, filename);
        fclose(fp);
        return -1;
    }

    rs = getnodeset(doc, "/UFTP/TrnDetail");
    if (rs == NULL || rs->nodesetval->nodeNr <= 0)
    {
        SDKerrlog(ERRLOG, "%s|%d, 明细对账文件[%s]无记录!", 
                __FILE__, __LINE__, filename);
        xmlXPathFreeObject(rs);
        XMLFreeDoc( doc );
        fclose(fp);
        return 0;
    }

    for (i = 0; i < rs->nodesetval->nodeNr; i++)
    {
        node = rs->nodesetval->nodeTab[i];

        // 检索条件
        sprintf(condi, "nodeid=%d AND inoutflag='%s'", OP_REGIONID, XMLGetField(node, "InOutFlag", buf));
        sprintf(condi, "%s AND originator='%s'", condi, XMLGetField(node, "Originator", buf));
        sprintf(condi, "%s AND workdate='%s'", condi, XMLGetField(node, "WorkDate", buf));
        sprintf(condi, "%s AND refid='%s'", condi, XMLGetField(node, "RefId", buf));

        exist = (IsRecExist("trnjour", condi) == TRUE);

        DBUG("对明细查询条件:[%s]->[%d]", condi, exist);

        if (exist)
        {
            // 存在则修改结果及场次等
            sprintf(expr, "result=0,");
            sprintf(expr, "%s clearround='%s',", expr, XMLGetField(node, "ClearRound", buf));
            sprintf(expr, "%s cleardate='%s',", expr, XMLGetField(node, "ClearDate", buf));
            sprintf(expr, "%s workround='%s',", expr, XMLGetField(node, "WorkRound", buf));
            sprintf(expr, "%s clearstate='%c',", expr, CLRSTAT_CHECKED);
            sprintf(expr, "%s fee=%s, ", expr, XMLGetField(node, "Fee", buf));
            sprintf(expr, "%s feepayer='%s' ", expr, XMLGetField(node, "FeePayer", buf));

            iRc = DBExec("UPDATE trnjour SET %s WHERE %s", expr, condi);
        }
        else
        {
            iRc = RecTrnjourByNode(node);
            memset(line, 0, sizeof(line));
            strcat(line, XMLGetField(node, "WorkDate", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "RefId", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "Originator", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "Acceptor", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "NoteType", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "NoteNo", buf));
            strcat(line, " ");
            strcat(line, XMLGetField(node, "SettlAmt", buf));
            strcat(line, "\n");
            fputs(line, fp);
            count++;
        }
        if (iRc != 0)
        {
            SDKerrlog(ERRLOG, "%s|%d, 核对明细失败!", __FILE__, __LINE__);
            fclose(fp);
            return -1;
        }
    }

    sprintf(line, "前置共补记 %d 笔.\n", count);
    fputs(line, fp);

    xmlXPathFreeObject(rs);
    XMLFreeDoc( doc );

    if(workround !=0 )
    {
        if (IsRoundEnd(workround)) //当场结束更新所有未对账的为清算失败
        {
            iRc = DBExec("UPDATE trnjour SET clearstate='%c' "
                    "WHERE nodeid=%d and workdate='%s' and workround='%d'"
                    "  and classid=%s and clearstate != '%c'", 
                    CLRSTAT_FAILED, OP_REGIONID, workdate, workround, svcclass, CLRSTAT_CHECKED);
        }
        else  //当场未结束只更新贷记的未对账的和借记清算成功的为清算失败
        {
            iRc = DBExec("UPDATE trnjour SET clearstate='%c' "
                    "WHERE nodeid=%d and workdate='%s' and workround='%d'"
                    "  and classid=%s and ((clearstate != '%c' and dcflag='2') or (clearstate='1' and dcflag='1'))", 
                    CLRSTAT_FAILED, OP_REGIONID, workdate, workround, svcclass, CLRSTAT_CHECKED);
        }
    }
    else
    {
        iRc = DBExec("UPDATE trnjour SET clearstate='%c' "
                "WHERE nodeid=%d and workdate='%s' "
                "  and classid=%s and clearstate != '%c'", 
                CLRSTAT_FAILED, OP_REGIONID, workdate, svcclass, CLRSTAT_CHECKED);
    }

    if (iRc != 0)
    {
        SDKerrlog(ERRLOG, "%s|%d, 修改前置多余明细失败",__FILE__, __LINE__);
        fclose(fp);
        return -1;
    }
    sprintf(line, "前置多 %d 笔.", ProcessedRows());

    fputs(line, fp);

    fclose(fp);

    return 0;
}

int ProcessDBFile(char *file)
{
    FILE *fp, *fp2 = NULL;
    char filebak[256] = {0};
    char line[2048] = {0};
    char regionid[4] = {0};
    int ret = 0;

    if ((fp = fopen(file, "r")) == NULL)
    {
        ret = -2; 
        goto EXIT;
    }

    strcpy(filebak, file);
    if ((fp2 = fopen(strcat(filebak, ".bak"), "w")) == NULL)
    {
        ret = -3; 
        goto EXIT;
    }

    sprintf(regionid, "%d|", OP_REGIONID);

    memset(line, 0, sizeof(line));
    while (fgets(line+3, sizeof(line), fp) != NULL)
    {
        memcpy(line, regionid, 3);
        fprintf(fp2, "%s", line);
        memset(line, 0, sizeof(line));
    }

EXIT:
    if (fp != NULL)
        fclose(fp);
    if (fp2 != NULL)
        fclose(fp2);
    if (ret == 0)
        strcpy(file, filebak);

    return ret;
}

// 记录差额信息
int RecReconInfo(char *svcclass, char *workdate, int workround)
{
    char path[256];
    char condi[500];
    int  iRc = 0;

    snprintf(path, sizeof(path), "%s/diff_%s_%s_%d_%s.dat", getenv("FILES_DIR"), 
            GetCBankno(), workdate, workround, svcclass);

    if (VerifyFileAndMove(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| 差额文件 %s MAC检验错 !",
                __FILE__, __LINE__, path );
        return -1;
    }

    memset(condi,0,sizeof(condi));

    if( workround != 0) 
        sprintf(condi, "nodeid=%d and svcclass=%s and workdate='%s' and workround = %d", 
                OP_REGIONID, svcclass, workdate, workround);
    else
        sprintf(condi, "nodeid=%d and svcclass=%s and workdate='%s' ", 
                OP_REGIONID, svcclass, workdate);

    if (ProcessDBFile(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| 处理差额文件 %s 失败!", __FILE__, __LINE__, path);
        return -1;
    }

    DBExec("delete from ebanksumm where %s ", condi);

    if (DBImportFile("ebanksumm", path, '|') != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| 差额文件 %s 导入数据库失败!",
                __FILE__, __LINE__, path );
        return -1;
    }

    // 根据 ebanksumm 统计后生成清算行差额
    //iRc = GenReconInfo(OP_REGIONID, atoi(svcclass), workdate, workround);

    if (iRc = DBExec("delete from reconinfo where %s", condi))
        return -2;

    if (workround == 0)
    {
        iRc = DBExec("insert into reconinfo select nodeid, bankid, workdate, workround, cleardate, clearround, svcclass, curcode, curtype, sum(pres_debit_num), sum(pres_debit_total), sum(pres_credit_num), sum(pres_credit_total), sum(acpt_debit_num), sum(acpt_debit_total), sum(acpt_credit_num), sum(acpt_credit_total), sum(balance), '0' from ebanksumm where %s group by nodeid, bankid, workdate, workround, cleardate, clearround, svcclass, curcode, curtype", condi);
    } else {
        iRc = DBExec("insert into reconinfo select nodeid, bankid, workdate, workround, cleardate, clearround, svcclass, curcode, curtype, sum(pres_debit_num), sum(pres_debit_total), sum(pres_credit_num), sum(pres_credit_total), sum(acpt_debit_num), sum(acpt_debit_total), sum(acpt_credit_num), sum(acpt_credit_total), sum(balance), '0' from ebanksumm where %s group by nodeid, bankid, workdate, workround, cleardate, clearround, svcclass, curcode, curtype", condi); }

    return iRc;
}


// 明细与汇总核对
int CheckDiffAndList(char *svcclass, char *workdate, int workround)
{
    result_set *rs1 = NULL;
    result_set *rs2 = NULL;
    int ret = 0;

    if(workround !=0)
        ret = db_query(&rs1, "select count(*) as num1 from trnjour "
                "where nodeid=%d and classid=%d and dcflag in('1','2') and workdate='%s' "
                "and workround='%d' and clearstate='%c' and result=0",
                OP_REGIONID, atoi(svcclass), workdate, workround, CLRSTAT_CHECKED);
    else
        ret = db_query(&rs1, "select count(*) as num1 from trnjour "
                "where nodeid=%d and classid=%d and dcflag in('1','2') and workdate='%s' "
                "and clearstate='%c' and result=0",
                OP_REGIONID, atoi(svcclass), workdate, CLRSTAT_CHECKED);

    if (ret != 0)
        return -1;

    if(workround != 0)
        ret = db_query(&rs2, "select isnull(sum(pres_debit_num+pres_credit_num+"
                "acpt_debit_num+acpt_credit_num), 0) as num2 from ebanksumm "
                "where nodeid=%d and svcclass=%d and workdate='%s' and workround=%d",
                OP_REGIONID, atoi(svcclass), workdate, workround);
    else
        ret = db_query(&rs2, "select isnull(sum(pres_debit_num+pres_credit_num+"
                "acpt_debit_num+acpt_credit_num), 0) as num2 from ebanksumm "
                "where nodeid=%d and svcclass=%d and workdate='%s' ",
                OP_REGIONID, atoi(svcclass), workdate);
    if (ret != 0)
    {
        db_free_result(rs1);
        return -1;
    }

    if (db_cell_i(rs1, 0, 0) != db_cell_i(rs2, 0, 0))
    {
        SDKerrlog(ERRLOG, "流水总笔数:%d, 差额总笔数:%d",
                db_cell_i(rs1, 0, 0), db_cell_i(rs2, 0, 0));
        ret = -1;
    }

    db_free_result(rs1);
    db_free_result(rs2);

    return ret;
}

//记录行内对账数据
int RecBankJour(int svcclass, char *workdate, int workround)
{
    int result = 0;

    result = DBExec("delete from bankjour where nodeid=%d", OP_REGIONID);

    if ( workround == 0)
        result = DBExec("insert into bankjour select nodeid, workdate, workround, inoutflag, refid, originator, acceptor, classid, dcflag, notetype, noteno, curcode, curtype, settlamt, payingacct, beneacct, payer, benename, '0' from trnjour where nodeid=%d and workdate='%s' and classid=%d and ( clearstate='1' or clearstate='C') ", OP_REGIONID, workdate, svcclass);
    else
        result = DBExec("insert into bankjour select nodeid, workdate, workround, inoutflag, refid, originator, acceptor, classid, dcflag, notetype, noteno, curcode, curtype, settlamt, payingacct, beneacct, payer, benename, '0' from trnjour where nodeid=%d and workdate = '%s' and classid = %d and workround = '%d' and ( clearstate='1' or clearstate='C') ", OP_REGIONID, workdate, svcclass, workround);
    return result;
}

/*
 * 平台与中心对账主程序
 *
 * doc: 取对账应答Doc
 */
int SettleSvr(char *dzRspFile, xmlDoc *doc, char *workdate, char *workround, char *svcclass)
{
    int iRc = -1;

    INFO("记录行内数据...");
    if ((iRc = RecBankJour(atoi(svcclass), workdate, atoi(workround))) != 0)
        return -1;

    // 差额文件插入差额表
    INFO("记录差额...");
    iRc = RecReconInfo(svcclass, workdate, atoi(workround));
    if (iRc != 0)
        return -2;

    // 对账
    INFO("明细对账...");
    iRc = ProcSettle(svcclass, workdate, atoi(workround));
    if (iRc != 0)
        return -3;

    // 检查总账与明细
    INFO("检查总账与明细...");
    iRc = CheckDiffAndList(svcclass, workdate, atoi(workround));
    if (iRc != 0)
    {
        INFO("检查总账与明细失败 ret=[%d]", iRc);
        return -4;
    }

    // 导入收费汇总文件
    INFO("导入收费汇总文件...");
    iRc = RecFeeSum(svcclass, workdate, atoi(workround));
    if (iRc != 0)
    {
        INFO("导入收费汇总文件失败 ret=[%d]", iRc);
        return -5;
    }

    /*
    INFO("生成对账回执文件...");
    // 产生对账回执文件
    if ((iRc = CenterAndBankSettle(dzRspFile, atoi(svcclass), workdate, atoi(workround))) != 0)
    {
        INFO("对账或生成对账回执出错 ret=[%d]", iRc);
        return -6;
    }
    */

    return 0;
}
