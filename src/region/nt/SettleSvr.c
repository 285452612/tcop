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
        SDKerrlog( ERRLOG, "%s|%d| �շ��ļ� %s MAC����� !",
                __FILE__, __LINE__, path );
        return -1;
    }

    if (ProcessDBFile(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| �����շ��ļ� %s ʧ��!", __FILE__, __LINE__, path);
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

// ������ˮ����
int RecTrnjourByNode(xmlNodePtr node)
{
    xmlDoc *doc = NULL;
    char nodeid[3] = {0};

    doc = XMLDumpNodeAsDoc(node);
    if (doc == NULL)
    {
        SDKerrlog(ERRLOG, "%s|%d| ������ˮʱת���ڵ㵽�ĵ�����!", __FILE__, __LINE__);
        return -1;
    }

    sprintf(nodeid, "%d", OP_REGIONID);
    XMLSetNodeVal(doc, "//SeqNo", nodeid);

    if (InsertTableByID(doc, "trnjour", 900001) != 0)
        return -2;

    DBUG("������ˮ[%s|%s->%s]�ɹ�!", XMLGetNodeVal(doc, "//InOutFlag"), 
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
        // CHAR�������Ͳ�������
        if (stTrnjour[i].type == DATATYPE_CHAR)
            strcat(expr, "'");

        // ǰ����ˮ��ǰ���Զ�����
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


        // ȡ xmlNode ��Ӧֵ
        XMLGetField(node, stTrnjour[i].name, tmp);
        if (!strcmp(stTrnjour[i].name, "ClearState") && tmp[0] == CLRSTAT_SETTLED )
            tmp[0] = CLRSTAT_CHECKED;

        // ���� WORKDATE, ��¼������Ϣ��
        if (!strcmp(stTrnjour[i].name, "WorkDate"))
            strcpy(workdate, tmp);

        // ��ֹĳЩINT��DOUBLE�����ֶ�Ϊ��ֵ
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

    // ȥ�����һ������
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

// ��ϸ���˴���
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
        SDKerrlog(ERRLOG, "%s|%d, ���˻�ִ�ļ�[%s]�޷�����!",
                __FILE__, __LINE__, g_respfile);
        return -1;
    }

    doc = xmlParseFile(filename);
    if (doc == NULL)
    {
        SDKerrlog(ERRLOG, "%s|%d, ��ϸ�����ļ�[%s]�޷���!",
                __FILE__, __LINE__, filename);
        fclose(fp);
        return -1;
    }

    rs = getnodeset(doc, "/UFTP/TrnDetail");
    if (rs == NULL || rs->nodesetval->nodeNr <= 0)
    {
        SDKerrlog(ERRLOG, "%s|%d, ��ϸ�����ļ�[%s]�޼�¼!", 
                __FILE__, __LINE__, filename);
        xmlXPathFreeObject(rs);
        XMLFreeDoc( doc );
        fclose(fp);
        return 0;
    }

    for (i = 0; i < rs->nodesetval->nodeNr; i++)
    {
        node = rs->nodesetval->nodeTab[i];

        // ��������
        sprintf(condi, "nodeid=%d AND inoutflag='%s'", OP_REGIONID, XMLGetField(node, "InOutFlag", buf));
        sprintf(condi, "%s AND originator='%s'", condi, XMLGetField(node, "Originator", buf));
        sprintf(condi, "%s AND workdate='%s'", condi, XMLGetField(node, "WorkDate", buf));
        sprintf(condi, "%s AND refid='%s'", condi, XMLGetField(node, "RefId", buf));

        exist = (IsRecExist("trnjour", condi) == TRUE);

        DBUG("����ϸ��ѯ����:[%s]->[%d]", condi, exist);

        if (exist)
        {
            // �������޸Ľ�������ε�
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
            SDKerrlog(ERRLOG, "%s|%d, �˶���ϸʧ��!", __FILE__, __LINE__);
            fclose(fp);
            return -1;
        }
    }

    sprintf(line, "ǰ�ù����� %d ��.\n", count);
    fputs(line, fp);

    xmlXPathFreeObject(rs);
    XMLFreeDoc( doc );

    if(workround !=0 )
    {
        if (IsRoundEnd(workround)) //����������������δ���˵�Ϊ����ʧ��
        {
            iRc = DBExec("UPDATE trnjour SET clearstate='%c' "
                    "WHERE nodeid=%d and workdate='%s' and workround='%d'"
                    "  and classid=%s and clearstate != '%c'", 
                    CLRSTAT_FAILED, OP_REGIONID, workdate, workround, svcclass, CLRSTAT_CHECKED);
        }
        else  //����δ����ֻ���´��ǵ�δ���˵ĺͽ������ɹ���Ϊ����ʧ��
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
        SDKerrlog(ERRLOG, "%s|%d, �޸�ǰ�ö�����ϸʧ��",__FILE__, __LINE__);
        fclose(fp);
        return -1;
    }
    sprintf(line, "ǰ�ö� %d ��.", ProcessedRows());

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

// ��¼�����Ϣ
int RecReconInfo(char *svcclass, char *workdate, int workround)
{
    char path[256];
    char condi[500];
    int  iRc = 0;

    snprintf(path, sizeof(path), "%s/diff_%s_%s_%d_%s.dat", getenv("FILES_DIR"), 
            GetCBankno(), workdate, workround, svcclass);

    if (VerifyFileAndMove(path) != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| ����ļ� %s MAC����� !",
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
        SDKerrlog( ERRLOG, "%s|%d| �������ļ� %s ʧ��!", __FILE__, __LINE__, path);
        return -1;
    }

    DBExec("delete from ebanksumm where %s ", condi);

    if (DBImportFile("ebanksumm", path, '|') != 0)
    {
        SDKerrlog( ERRLOG, "%s|%d| ����ļ� %s �������ݿ�ʧ��!",
                __FILE__, __LINE__, path );
        return -1;
    }

    // ���� ebanksumm ͳ�ƺ����������в��
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


// ��ϸ����ܺ˶�
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
        SDKerrlog(ERRLOG, "��ˮ�ܱ���:%d, ����ܱ���:%d",
                db_cell_i(rs1, 0, 0), db_cell_i(rs2, 0, 0));
        ret = -1;
    }

    db_free_result(rs1);
    db_free_result(rs2);

    return ret;
}

//��¼���ڶ�������
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
 * ƽ̨�����Ķ���������
 *
 * doc: ȡ����Ӧ��Doc
 */
int SettleSvr(char *dzRspFile, xmlDoc *doc, char *workdate, char *workround, char *svcclass)
{
    int iRc = -1;

    INFO("��¼��������...");
    if ((iRc = RecBankJour(atoi(svcclass), workdate, atoi(workround))) != 0)
        return -1;

    // ����ļ��������
    INFO("��¼���...");
    iRc = RecReconInfo(svcclass, workdate, atoi(workround));
    if (iRc != 0)
        return -2;

    // ����
    INFO("��ϸ����...");
    iRc = ProcSettle(svcclass, workdate, atoi(workround));
    if (iRc != 0)
        return -3;

    // �����������ϸ
    INFO("�����������ϸ...");
    iRc = CheckDiffAndList(svcclass, workdate, atoi(workround));
    if (iRc != 0)
    {
        INFO("�����������ϸʧ�� ret=[%d]", iRc);
        return -4;
    }

    // �����շѻ����ļ�
    INFO("�����շѻ����ļ�...");
    iRc = RecFeeSum(svcclass, workdate, atoi(workround));
    if (iRc != 0)
    {
        INFO("�����շѻ����ļ�ʧ�� ret=[%d]", iRc);
        return -5;
    }

    /*
    INFO("���ɶ��˻�ִ�ļ�...");
    // �������˻�ִ�ļ�
    if ((iRc = CenterAndBankSettle(dzRspFile, atoi(svcclass), workdate, atoi(workround))) != 0)
    {
        INFO("���˻����ɶ��˻�ִ���� ret=[%d]", iRc);
        return -6;
    }
    */

    return 0;
}
