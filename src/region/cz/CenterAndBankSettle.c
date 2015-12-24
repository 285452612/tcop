#include "interface.h"
#include "chinese.h"

#define E_GNR_RECNOTFOUND 913

int CenterAndBankSettle(char *dzRspFile, int classid, char *workdate, int workround)
{
    result_set rs1;
    result_set rs2;
    char outdebitamt[30], outcreditamt[30];
    char indebitamt[30], increditamt[30];
    char c_outdebitamt[30], c_outcreditamt[30];
    char c_indebitamt[30], c_increditamt[30];
    char diffamt[30], c_diffamt[30];
    int outdebitnum, outcreditnum, indebitnum, increditnum;
    int c_outdebitnum, c_outcreditnum, c_indebitnum, c_increditnum;
    char syscondi[128];
    char bkcondi[128];
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char *bankid = NULL;
    char bankname[128];
    char curcode[4];
    char buf[2048];
    char tmp[128];
    int  settle_result = 0;
    int  curtype;
    char *p;
    FILE *fp=NULL;
    int rc = -1;
    int i;

    if ((bankid = GetCBankno()) == NULL)
        return -1;

    memset(bankname, 0, sizeof(bankname));
    org_name(bankid, bankname);
    snprintf(caParaFile, sizeof(caParaFile), "%s/dat/SettleRpt.para", getenv("HOME"));
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%s_%d_%d.result",
            getenv("FILES_DIR"), bankid, workdate, workround, classid);
    unlink(caOutFile);

    if (workround == 0)
    {
        sprintf(syscondi, "nodeid = %d and workdate='%s'", OP_REGIONID, workdate);
        sprintf(bkcondi, "nodeid = %d and workdate='%s'", OP_REGIONID, workdate);
    }
    else
    {
        sprintf(syscondi, "nodeid = %d and workdate='%s' and workround=%d", OP_REGIONID, workdate, workround);
        sprintf(bkcondi, "nodeid = %d and workdate='%s' and workround='%d'", OP_REGIONID, workdate, workround);
    }

    rc = db_query(&rs1, "select curcode, curtype, "
            "sum(pres_debit_num), sum(pres_debit_total),"
            "sum(pres_credit_num), sum(pres_credit_total),"
            "sum(acpt_debit_num), sum(acpt_debit_total),"
            "sum(acpt_credit_num), sum(acpt_credit_total),"
            "sum(balance) from reconinfo "
            "where bankid='%s' and %s and svcclass=%d "
            "group by curcode, curtype order by curcode, curtype",
            bankid, syscondi, classid);
    DBUG("rc=%d", rc);
    if (rc != 0)
    {
        if (rc != E_DB_NORECORD)
        {
            return -1;
        }
        else
            return 0;
    }

    for (i = 0; i < db_row_count(&rs1); i++)
    {
        if ((fp = fopen((char *)GetTmpFileName(caDataFile), "w")) == NULL)
        {
            return -3;
        }
        strcpy(outdebitamt, "0.00");
        strcpy(outcreditamt, "0.00");
        strcpy(indebitamt, "0.00");
        strcpy(increditamt, "0.00");
        outdebitnum = outcreditnum = indebitnum = increditnum = 0;
        strncpy(curcode, db_cell(&rs1, i, 0), sizeof(curcode)-1);
        curtype = db_cell_i(&rs1, i, 1);
        // 提出借
        rc = db_query(&rs2, "select count(*), sum(settlamt) from bankjour "
                "where %s and curcode='%s' and curtype='%d' and classid=%d "
                "and inoutflag='1' and dcflag='1'",
                bkcondi, curcode, curtype, classid);
        if (rc != 0)
        {
            if (rc != E_DB_NORECORD)
            {
                fclose(fp);
                return -4;
            }
            else
                rc = 0;
        }
        else
        {
            outdebitnum = db_cell_i(&rs2, 0, 0);
            strcpy(outdebitamt, db_cell(&rs2, 0, 1));
            db_free_result(&rs2);
        }
        // 提出贷
        rc = db_query(&rs2, "select count(*), sum(settlamt) from bankjour "
                "where %s and curcode='%s' and curtype='%d' and classid=%d "
                "and inoutflag='1' and dcflag='2'",
                bkcondi, curcode, curtype, classid);
        if (rc != 0)
        {
            if (rc != E_DB_NORECORD)
            {
                fclose(fp);
                return -5;
            }
            rc = 0;
        }
        else
        {
            outcreditnum = db_cell_i(&rs2, 0, 0);
            strcpy(outcreditamt, db_cell(&rs2, 0, 1));
            db_free_result(&rs2);
        }
        // 提入借
        rc = db_query(&rs2, "select count(*), sum(settlamt) from bankjour "
                "where %s and curcode='%s' and curtype='%d' and classid=%d "
                "and inoutflag='2' and dcflag='1'",
                bkcondi, curcode, curtype, classid);
        if (rc != 0)
        {
            if (rc != E_DB_NORECORD)
            {
                fclose(fp);
                return -6;
            }
            rc = 0;
        }
        else
        {
            indebitnum = db_cell_i(&rs2, 0, 0);
            strcpy(indebitamt, db_cell(&rs2, 0, 1));
            db_free_result(&rs2);
        }
        // 提入贷
        rc = db_query(&rs2, "select count(*), sum(settlamt) from bankjour "
                "where %s and curcode='%s' and curtype='%d' and classid=%d "
                "and inoutflag='2' and dcflag='2'",
                bkcondi, curcode, curtype, classid);
        if (rc != 0)
        {
            if (rc != E_DB_NORECORD)
            {
                fclose(fp);
                return -7;
            }
            rc = 0;
        }
        else
        {
            increditnum = db_cell_i(&rs2, 0, 0);
            strcpy(increditamt, db_cell(&rs2, 0, 1));
            db_free_result(&rs2);
        }

        c_outdebitnum = db_cell_i(&rs1, i, 2);
        strcpy(c_outdebitamt, db_cell(&rs1, i, 3));
        c_outcreditnum = db_cell_i(&rs1, i, 4);
        strcpy(c_outcreditamt, db_cell(&rs1, i, 5));
        c_indebitnum = db_cell_i(&rs1, i, 6);
        strcpy(c_indebitamt, db_cell(&rs1, i, 7));
        c_increditnum = db_cell_i(&rs1, i, 8);
        strcpy(c_increditamt, db_cell(&rs1, i, 9));
        strcpy(c_diffamt, db_cell(&rs1, i, 10));

        DBUG("reconinfo info:");
        DBUG(" c_outdebitnum=[%d|%s]", c_outdebitnum, c_outdebitamt);
        DBUG("c_outcreditnum=[%d|%s]", c_outcreditnum, c_outcreditamt);
        DBUG("  c_indebitnum=[%d|%s]", c_indebitnum, c_indebitamt);
        DBUG(" c_increditnum=[%d|%s]", c_increditnum, c_increditamt);
        DBUG(" c_diffamt=[%s]", c_diffamt);

        DBUG("bankjour info:");
        DBUG(" outdebitnum=[%d|%s]", outdebitnum, outdebitamt);
        DBUG("outcreditnum=[%d|%s]", outcreditnum, outcreditamt);
        DBUG("  indebitnum=[%d|%s]", indebitnum, indebitamt);
        DBUG(" increditnum=[%d|%s]", increditnum, increditamt);

        if (c_outdebitnum != outdebitnum || c_outcreditnum != outcreditnum
                || c_indebitnum != indebitnum || c_increditnum != increditnum)
        {
            settle_result = 1;
        }
        if (strcmp(c_outdebitamt, outdebitamt)
                || strcmp(c_outcreditamt, outcreditamt)
                || strcmp(c_indebitamt, indebitamt)
                || strcmp(c_increditamt, increditamt))
        {
            settle_result = 1;
        }
        snprintf(buf, sizeof(buf), "%s;%s", bankid, bankname);
        if (classid == 3)
        {
            snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "  外币:%s %s;",
                    ChsName(curcode_list, curcode),
                    GetChineseName(curtype_list, curtype));
        }
        else
        {
            snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "  人民币;");
        }
        if (workround == 0)
            strcpy(tmp, "所有场次");
        else
            sprintf(tmp, "第%d场", workround);
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
                "%s;%s;", ChineseDate(atoi(workdate)), tmp);
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%d;%s;%d;%s;%.2lf;",
                c_outdebitnum, c_outdebitamt, outdebitnum, outdebitamt, 
                atof(outdebitamt) - atof(c_outdebitamt));
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%d;%s;%d;%s;%.2lf;",
                c_outcreditnum, c_outcreditamt, outcreditnum, outcreditamt, 
                atof(outcreditamt) - atof(c_outcreditamt));
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%d;%s;%d;%s;%.2lf;",
                c_indebitnum, c_indebitamt, indebitnum, indebitamt, 
                atof(indebitamt) - atof(c_indebitamt));
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%d;%s;%d;%s;%.2lf;",
                c_increditnum, c_increditamt, increditnum, increditamt, 
                atof(increditamt) - atof(c_increditamt));
        sprintf(diffamt, "%.2lf", 
                atof(outdebitamt)+atof(increditamt)-atof(outcreditamt)-atof(indebitamt));
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s;%s;%.2lf;", 
                c_diffamt, diffamt, atof(diffamt)-atof(c_diffamt));
        if (settle_result == 1)
        {
            snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),
                    "不平;**对帐不平,下面进行明细对帐**;");
        }
        else
            snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "一致;;");

        WriteRptHeader(fp, buf);

        rc = 0; // rc 临时用作不平记录数
        if (settle_result == 1)
        {
            if ((rc=CenterAndBankDiffer(fp, classid, workdate, workround)) < 0)
            {
                err_log( "生成明细对账结果出错.");
                fclose(fp);
                db_free_result(&rs1);
                return -8;
            }
        }
        if(rc == 0) //平
        {
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
                    "", "", "","", "", "", "", "", "", "");
            rc =1;
        }

        WriteRptRowCount(fp, rc);
        WriteRptFooter(fp, "");
        fclose(fp);
        rc = PrintReportList(caParaFile, caDataFile, caOutFile);
        if (rc != 0)
        {
            fclose(fp);
            db_free_result(&rs1);
            return -9;
        }
    }
    //strncpy(gcaResponseFile, basename(caOutFile), sizeof(gcaResponseFile));
    strcpy(dzRspFile, basename(caOutFile));
    db_free_result(&rs1);
    return 0;
}

int CenterAndBankDiffer(FILE *fp, int classid, char *workdate, int workround)
{
    result_set rs;
    char syscondi[128];
    char condi[1024];
    int count;
    int rc, i;
    char dbname[10];
    char dctype[10];

    if (workround == 0)
    {
        sprintf(syscondi, "nodeid=%d and workdate='%s'", OP_REGIONID, workdate);
    }
    else
    {
        sprintf(syscondi, "nodeid=%d and workdate='%s' and workround='%d'", OP_REGIONID, workdate, workround);
    }
    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
            "数据来源", "提出行", "提出流水", "提入行", "借贷", "票据种类",
            "票据号码", "收款人帐号", "付款人帐号", "金额");
    fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
            "────", "──────", "────────", "──────", 
            "────", "────", "──────",
            "────────────────", 
            "────────────────", "────────");
    count = 2;
    /*
    if(atol(workdate) < atol(GetSysPara("WORKDATE")))
        strcpy(dbname,"hddb");
    else
        strcpy(dbname,"cddb");
        */
    rc = db_query(&rs, "select originator, refid, acceptor, dcflag, notetype,"
            "noteno, beneacct, payingacct, settlamt, inoutflag "
            "from trnjour where %s and classid=%d and clearstate='C'", syscondi, classid);
    if (rc != 0)
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        for (i = 0; i < db_row_count(&rs); i++)
        {
            snprintf(condi, sizeof(condi), 
                    "nodeid=%d and inoutflag='%s' and refid='%s' and originator='%s'", 
                    OP_REGIONID, db_cell(&rs, i, 9), db_cell(&rs, i, 1), db_cell(&rs, i, 0));
            if (db_hasrecord("bankjour", condi) == FALSE)
            {
                if(strcmp(db_cell(&rs,i,3),"1") == 0)
                    strcpy(dctype,"借记");
                else if(strcmp(db_cell(&rs,i,3),"2") == 0)
                    strcpy(dctype,"贷记");
                else
                    strcpy(dctype,db_cell(&rs,i,3));

                // 结算中心多
                fprintf(fp, "结算中心;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
                        db_cell(&rs,i, 0), db_cell(&rs, i, 1), db_cell(&rs, i, 2),
                        dctype, db_cell(&rs, i, 4), db_cell(&rs, i, 5),
                        db_cell(&rs,i, 6), db_cell(&rs, i, 7), db_cell(&rs, i, 8));
                count++;
            }
            else
            {
                // 结算中心与综合业务系统一致
                rc = DBExec("update bankjour set flag='1' where %s", condi);
                if (rc != 0)
                {
                    db_free_result(&rs);
                    return -1;
                }
            }
        }
        db_free_result(&rs);
    }

    rc = db_query(&rs, "select originator, refid, acceptor, dcflag, notetype,"
            "noteno, beneacct, payingacct, settlamt from bankjour "
            "where %s and classid=%d and flag = '0'", syscondi, classid);
    if (rc != 0)
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        for (i = 0; i < db_row_count(&rs); i++)
        {
            // 综合业务系统多
            if(strcmp(db_cell(&rs,i,3),"1") == 0)
                strcpy(dctype,"借记");
            else if(strcmp(db_cell(&rs,i,3),"2") == 0)
                strcpy(dctype,"贷记");
            else
                strcpy(dctype,db_cell(&rs,i,3));

            fprintf(fp, "行内系统;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n",
                    db_cell(&rs, i, 0), db_cell(&rs, i, 1), db_cell(&rs, i, 2),
                    dctype, db_cell(&rs, i, 4), db_cell(&rs, i, 5),
                    db_cell(&rs, i, 6), db_cell(&rs, i, 7), db_cell(&rs, i, 8));
            count++;
        }
        db_free_result(&rs);
    }

    return count;
}
