#include "interface.h"
#include "dbstru.h"
#include "errcode.h"
#include "Public.h"

static char g_respfile[256];

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

int RecFeeSum(char *svcclass, char *workdate, int workround)
{
    char path[256];
    char condi[500];

    snprintf(path, sizeof(path), "%s/fee_%s_%s_%d_%s.dat", 
            getenv("FILES_DIR"), GetCBankno(), workdate, workround,
            svcclass);
    if (!IsExistFile(path))
        return 0;
    /*
    if (VerifyFileAndMove(path) != 0)
    {
        INFO("差额文件 %s MAC检验错 !", path);
        return -1;
    }
    */
    if (ProcessDBFile(path) != 0)
    {
        INFO("处理差额文件 %s 失败!", path);
        return -1;
    }

    if (workround != 0) 
        sprintf(condi, "nodeid=%d and classid=%s and workdate='%s' and workround=%d",
                OP_REGIONID, svcclass, workdate, workround);
    else
        sprintf(condi, "nodeid=%d and classid=%s and workdate='%s' ", 
                OP_REGIONID, svcclass, workdate);
    db_exec("delete from feesum where %s ", condi);

    return db_load_data("feesum", path, '|');
}

// 补记流水函数
int RecTrnjourByNode(xmlNodePtr node)
{
    xmlDoc *doc = NULL;
    char nodeid[3] = {0};

    doc = XMLDumpNodeAsDoc(node);
    if (doc == NULL)
    {
        INFO("补记流水时转换节点到文档出错!");
        return -1;
    }

    sprintf(nodeid, "%d", OP_REGIONID);
    XMLSetNodeVal(doc, "//SeqNo", nodeid);

    if (InsertTableByID(doc, "trnjour", 900001) != 0)
        return -2;

    DBUG("补记流水[%s|%s->%s]成功!", XMLGetNodeVal(doc, "//InOutFlag"),
            XMLGetNodeVal(doc, "//Originator"), XMLGetNodeVal(doc, "//RefId"));
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
    int i, count = 0;

    snprintf(buf, sizeof(buf), "%s/%s_%s_%d_%s", getenv("FILES_DIR"), 
            GetCBankno(), workdate, workround, svcclass);
    snprintf(filename, sizeof(filename), "%s.dat", buf);
    snprintf(g_respfile, sizeof(g_respfile), "%s.result", buf);

    if ((fp = fopen(g_respfile, "w")) == NULL)
    {
        INFO("对账回执文件[%s]无法创建!", g_respfile);
        return E_DAYEND_SETTFILE;
    }

    doc = xmlParseFile(filename);
    if (doc == NULL)
    {
        INFO("明细对账文件[%s]无法打开!", filename);
        fclose(fp);
        return E_DAYEND_SETTFILE;
    }

    rs = getnodeset(doc, "/UFTP/TrnDetail");
    if (rs == NULL || rs->nodesetval->nodeNr <= 0)
    {
        INFO("明细对账文件[%s]无记录!", filename);
        xmlXPathFreeObject(rs);
        xmlFreeDoc( doc );
        fclose(fp);
        return 0;
    }

    for (i = 0; i < rs->nodesetval->nodeNr; i++)
    {
        node = rs->nodesetval->nodeTab[i];

        // 检索条件
        XmlGetNodeString(node, "InOutFlag", buf, sizeof(buf));
        sprintf(condi, "inoutflag='%s'", buf);
        XmlGetNodeString(node, "Originator", buf, sizeof(buf));
        sprintf(condi, "%s AND originator='%s'", condi, buf);
        XmlGetNodeString(node, "WorkDate", buf, sizeof(buf));
        sprintf(condi, "%s AND workdate='%s'", condi, buf);
        XmlGetNodeString(node, "RefId", buf, sizeof(buf));
        sprintf(condi, "%s AND refid='%s'", condi, buf);

        if (db_hasrecord("trnjour", condi) == TRUE)
        {
            // 存在则修改结果及场次等
            sprintf(expr, "result=0,");
            sprintf(expr, "%s clearround='%d',", expr, 
                    XmlGetNodeInteger(node, "ClearRound"));
            XmlGetNodeString(node, "ClearDate", buf, sizeof(buf));
            sprintf(expr, "%s cleardate='%s',", expr, buf);
            sprintf(expr, "%s workround='%d',", expr, 
                    XmlGetNodeInteger(node, "WorkRound"));
            XmlGetNodeString(node, "ExchgDate", buf, sizeof(buf));
            sprintf(expr, "%s exchgdate='%s',", expr, buf);
            sprintf(expr, "%s exchground='%d',", expr, 
                    XmlGetNodeInteger(node, "ExchgRound"));
            sprintf(expr, "%s clearstate='%c',", expr, CLRSTAT_CHECKED);
            XmlGetNodeString(node, "Fee", buf, sizeof(buf));
            sprintf(expr, "%s fee=%s, ", expr, buf);
            XmlGetNodeString(node, "FeePayer", buf, sizeof(buf));
            sprintf(expr, "%s feepayer='%s', ", expr, buf);
            XmlGetNodeString(node, "TruncFlag", buf, sizeof(buf));
            sprintf(expr, "%s truncflag='%s' ", expr, buf);
            /*
            sprintf(expr, "%s truncflag='%s', ", expr, buf);
            XmlGetNodeString(node, "ExchArea", buf, sizeof(buf));
            sprintf(expr, "%s excharea='%s', ", expr, buf);
            XmlGetNodeString(node, "ClearArea", buf, sizeof(buf));
            sprintf(expr, "%s cleararea='%s' ", expr, buf);
            */

            iRc = db_exec("UPDATE trnjour SET %s WHERE %s", expr, condi);
        }
        else
        {
            iRc = RecTrnjourByNode(node);
            memset(line, 0, sizeof(line));
            XmlGetNodeString(node, "WorkDate", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "RefId", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "Originator", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "Acceptor", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "NoteType", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "NoteNo", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, " ");
            XmlGetNodeString(node, "SettlAmt", buf, sizeof(buf));
            strcat(line, buf);
            strcat(line, "\n");
            fputs(line, fp);
            count++;
        }
        if (iRc != 0)
        {
            INFO("核对明细失败!");
            break;
        }
    }
    sprintf(line, "前置共补记 %d 笔.\n", count);
    fputs(line, fp);

    xmlXPathFreeObject(rs);
    xmlFreeDoc( doc );

    if (iRc == 0)
    {
        iRc = db_exec("UPDATE trnjour SET clearstate='%c' "
                "WHERE workdate='%s' and workround='%d'"
                "  and classid=%s and clearstate != '%c'",
                CLRSTAT_FAILED, workdate, workround, svcclass, CLRSTAT_CHECKED);
        if (iRc != 0)
            INFO("修改前置多余明细失败");
        //sprintf(line, "前置多 %d 笔.", ProcessedRows());
        fputs(line, fp);
    }
    fclose(fp);

    return iRc;
}

// 记录差额信息
int RecReconInfo(char *svcclass, char *workdate, int workround)
{
    char path[256];
    char condi[512];
    int  iRc;

    snprintf(path, sizeof(path), "%s/diff_%s_%s_%d_%s.dat", 
            getenv("FILES_DIR"), GetCBankno(), workdate, workround,
            svcclass);
    /*
    if (VerifyFileAndMove(path) != 0)
    {
        INFO("差额文件 %s MAC检验错 !", path);
        return -1;
    }
    */
    if (ProcessDBFile(path) != 0)
    {
        INFO("处理差额文件 %s 失败!", path);
        return -1;
    }

    if( workround != 0) 
        sprintf(condi, "nodeid=%d and svcclass=%s and workdate='%s' and workround =%d",
                OP_REGIONID, svcclass, workdate, workround);
    else
        sprintf(condi, "nodeid=%d and svcclass=%s and workdate='%s' ", 
                OP_REGIONID, svcclass, workdate);

    db_exec("delete from ebanksumm where %s ", condi);
    if (db_load_data("ebanksumm", path, '|') != 0)
    {
        INFO("差额文件 %s 导入数据库失败!", path);
        return -1;
    }

    // 根据 ebanksumm 统计后生成清算行差额
    //iRc = GenReconInfo(atoi(svcclass), workdate, workround);
    //iRc = db_exec("exec gen_reconinfo(%d, '%s', %d, 0)", 
    //        atoi(svcclass), workdate, workround);
    if ((iRc = db_exec("delete from reconinfo where %s", condi)) != 0)
    {
        INFO("清除reconinfo数据失败");
        return -1;
    }

    /*
    if(workround == 0)
        iRc = db_exec("insert into reconinfo "
                "select nodeid, bankid, workdate, workround, "
                "cleardate, clearround, svcclass, curcode, curtype, "
                "sum(pres_debit_num), sum(pres_debit_total),"
                "sum(pres_credit_num), sum(pres_credit_total),"
                "sum(acpt_debit_num), sum(acpt_debit_total),"
                "sum(acpt_credit_num), sum(acpt_credit_total),"
                "sum(balance), '0' from ebanksumm"
                " where %s group by nodeid, bankid, workdate, workround, cleardate, "
                "clearround, svcclass, curcode, curtype", condi);
    else
    */
    iRc = db_exec("insert into reconinfo "
            "select nodeid, bankid, workdate, workround, "
            "cleardate, clearround, svcclass, curcode, curtype, "
            "sum(pres_debit_num), sum(pres_debit_total),"
            "sum(pres_credit_num), sum(pres_credit_total),"
            "sum(acpt_debit_num), sum(acpt_debit_total),"
            "sum(acpt_credit_num), sum(acpt_credit_total),"
            "sum(balance), '0' from ebanksumm"
            " where %s group by nodeid, bankid, workdate, workround, cleardate, "
            "clearround, svcclass, curcode, curtype", condi);
    return iRc;
}

// 明细与汇总核对
int CheckDiffAndList(char *svcclass, char *workdate, int workround)
{
    result_set rs1;
    result_set rs2;
    int ret = 0;

    if(workround !=0)
        ret = db_query(&rs1, "select count(*) as num1 from trnjour "
                "where nodeid=%d and classid=%d and dcflag in('1','2') "
                "and workdate='%s' and workround='%d' and clearstate='%c' "
                "and result=0", OP_REGIONID, atoi(svcclass), 
                workdate, workround, CLRSTAT_CHECKED);
    else
        ret = db_query(&rs1, "select count(*) as num1 from trnjour "
                "where nodeid=%d and classid=%d and dcflag in('1','2') "
                "and workdate='%s' and clearstate='%c' and result=0",
                OP_REGIONID, atoi(svcclass), workdate, CLRSTAT_CHECKED);
    if (ret != 0)
        return -1;

    if(workround !=0)
        ret = db_query(&rs2, "select "IS_NULL "(sum(pres_debit_num+"
                "pres_credit_num+acpt_debit_num+acpt_credit_num), 0) as num2 "
                "from ebanksumm where nodeid=%d and svcclass=%d "
                "and workdate='%s' and workround=%d",
                OP_REGIONID, atoi(svcclass), workdate, workround);
    else
        ret = db_query(&rs2, "select "IS_NULL "(sum(pres_debit_num+"
                "pres_credit_num+acpt_debit_num+acpt_credit_num), 0) as num2 "
                "from ebanksumm where nodeid=%d and svcclass=%d "
                "and workdate='%s'", OP_REGIONID, atoi(svcclass), workdate);
    if (ret != 0)
    {
        db_free_result(&rs1);
        return -1;
    }

    if (db_cell_i(&rs1, 0, 0) != db_cell_i(&rs2, 0, 0))
    {
        INFO("流水总笔数:%d, 差额总笔数:%d",
                db_cell_i(&rs1, 0, 0), db_cell_i(&rs2, 0, 0));
        ret = -1;
    }

    db_free_result(&rs1);
    db_free_result(&rs2);

    return ret;
}

//记录行内对账数据
int RecBankJour(int svcclass, char *workdate, int workround)
{
    int result = 0;

    result = db_exec("delete from bankjour where nodeid=%d", OP_REGIONID);

    if ( workround == 0)
        result = db_exec("insert into bankjour select nodeid, workdate, workround, inoutflag, refid, originator, acceptor, classid, dcflag, notetype, noteno, curcode, curtype, settlamt, payingacct, beneacct, payer, benename, '0' from trnjour where nodeid=%d and workdate='%s' and classid=%d and ( clearstate='1' or clearstate='C') ", OP_REGIONID, workdate, svcclass);
    else
        result = db_exec("insert into bankjour select nodeid, workdate, workround, inoutflag, refid, originator, acceptor, classid, dcflag, notetype, noteno, curcode, curtype, settlamt, payingacct, beneacct, payer, benename, '0' from trnjour where nodeid=%d and workdate = '%s' and classid = %d and workround = '%d' and ( clearstate='1' or clearstate='C') ", OP_REGIONID, workdate, svcclass, workround);
    return result;
}

// 记录交换包信息
int RecExchgFile(char *svcclass, char *workdate, int workround)
{
    char path[256];

    snprintf(path, sizeof(path), "%s/exchg_%s_%s_%d_%s.dat",
            getenv("FILES_DIR"), GetCBankno(), workdate, workround,
            svcclass);

    if (!IsExistFile(path))
    {
        INFO("File[%s] not exist, return.", path);
        return 0;
    }

    if (ProcessDBFile(path) != 0)
    {
        INFO("处理差额文件 %s 失败!", path);
        return -1;
    }

    if ( workround == 0 )
        db_exec("delete from baginfo where workdate='%s' ", workdate );
    else
        db_exec("delete from baginfo where workdate='%s' and workround=%d",
                workdate, workround);

    return db_load_data("baginfo", path, '|');
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

    // 导入交换包汇总
    INFO("导入交换包汇总...");
    iRc = RecExchgFile(svcclass, workdate, atoi(workround));
    if (iRc != 0)
    {
        INFO("导入交换包汇总失败!");
        return -6;
    }

    INFO("对账完成.");

    return 0;
}
