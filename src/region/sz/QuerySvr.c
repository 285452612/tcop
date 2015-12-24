#include "comm.h"
#include "interface.h"
#include "chinese.h"
#include "udb.h"
#include "errcode.h"
#include "Public.h"

#define TMP_T_RS "#tmp_t_rs"
extern char gs_originator[13];
extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq);

extern int ChkPresAgent(char *orgid, char *presproxy);
extern int ChkAcptAgent(char *orgid, char *acptproxy);
extern char *GetTmpFileName(char *);

#define GetTrnCtl(a) XmlGetStringDup(xmlReq, "/UFTP/TrnCtl/"a)

int equal(char *s1,char *s2)
{
    return strcmp(s1,s2)?0:1;
}

/* �����ӡƾ֤��ѯ 
 * �ڱ���������ʾ�����ڡ�ʱ�䡢Ʊ�ֵ�������Ϣ
 */
int QryOutNote( xmlDocPtr xmlReq, char *filename)
{
    char tbname[128];
    result_set rs,result;
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
    char tmp[128];
    char bankcode[8],notetype[8];
    char sSerial[16+1], sAcctType[40+1], sResult[8+1], sSqlStr[1024];
    char sMemo[64];
    char *p;
    int  suc_num = 0;
    double suc_amt = (double)0;
    FILE *fp=NULL;
    int iRet = 0;
    int i,l=0;

    if (InitRptVar(xmlReq) != 0)
        return E_SYS_CALL;

    xmlGetVal(xmlReq, "//TrnCtl/StartDate", startdate);
    if (startdate[0] == 0)
        strcpy(startdate, GetSysPara("CURWORKDATE"));
    xmlGetVal(xmlReq, "//TrnCtl/EndDate", enddate);
    if (enddate[0] == 0)
        strcpy(enddate, GetSysPara("CURWORKDATE"));

    if (DiffDate(startdate, GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    // ��ѯ����ƾ֤
    iRet = db_query(&rs, 
            "SELECT refid, workdate, prestime, clearstate, acceptor, notetype,"
            "dcflag,noteno,issuedate, curcode,settlamt, beneacct, payingacct, "
            "benename, payer, trncode,truncflag,reserved3 FROM %s "
            "WHERE originator LIKE '%s%%' AND acceptor LIKE '%s%%'"
            "  AND classid = %d AND curcode LIKE '%s%%'"
            "  AND notetype LIKE '%s%%' AND noteno LIKE '%s%%'"
            "  AND dcflag LIKE '%s%%' AND clearstate LIKE '%s%%'"
            "  AND settlamt BETWEEN %.2lf AND %.2lf"
            "  AND refid BETWEEN '%s' AND '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            "  AND workround LIKE '%s%%'"
            "  AND inoutflag = '1'"
            " ORDER BY workdate, prestime", tbname,
            GetTrnCtl( "Originator" ), GetTrnCtl( "Acceptor" ),
            atoi(GetTrnCtl( "SvcClass" )), GetTrnCtl( "CurCode" ),
            GetTrnCtl( "NoteType" ), GetTrnCtl( "NoteNo" ),
            GetTrnCtl( "DCFlag" ), GetTrnCtl( "ClearState" ), 
            atof(GetTrnCtl("MinAmount")), atof(GetTrnCtl("MaxAmount")),
            GetTrnCtl( "StartRefId" ), GetTrnCtl( "EndRefId" ),
            startdate, enddate, GetTrnCtl("WorkRound"));
    if ( iRet != 0 )
        return iRet;
    if (db_row_count(&rs) == 0)
    {
        iRet = E_DB_NORECORD;
        goto err_handle;
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutNote.para", getenv("HOME"), TCOP_BANKID );

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
            gs_originator, gs_bankname, gs_oper, tmp);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        int workdate, prestime;
        /* ��ȡ�������ں�ʱ����д��� */
        //������¡��գ����һ���´�
        workdate = db_cell_i(&rs, i, 1);
        prestime = db_cell_i(&rs, i, 2);
        sprintf(newtime, "%04d %02d:%02d",workdate%10000, prestime/10000, 
                prestime%10000/100);

        //��Ʊ������д���
        //��'noteinfo'����ȥ��ѯ����ӦƱ�ݵ�������Ϣ
        db_query_str(notename, sizeof(notename), "SELECT distinct name "
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

        if(*db_cell_by_name(&rs,i,"reserved3")=='1')//reserved3==1
        {
            strcpy(sAcctType, "�Ѽ���");
            strcpy(sSerial,"");
        }
        else
        {
            memset(sSqlStr, 0, sizeof sSqlStr);
            sprintf(sSqlStr, "select result,trncode,acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and refid = '%s' and inoutflag = '1'",
                    OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                    gs_originator, db_cell_by_name(&rs, i, "refid"));
            memset(sResult, 0, sizeof sResult);
            memset(sSerial, 0, sizeof sSerial);
            //iRet = db_query_strs(sSqlStr, sSerial, sResult);
            iRet = db_query(&result,sSqlStr);
            if(iRet!=0)
            {
                if(iRet==E_DB_NORECORD)
                {
                    strcpy(sAcctType,"δ����");
                }
                else
                {
                    db_free_result(&rs);
                    return iRet;
                }
            }
            else
            {
                do
                {
                    l=0;
                    strcpy(bankcode,db_cell(&result, 0, 1));
                    strcpy(notetype,db_cell_by_name(&rs, i, "notetype"));
                    if(db_row_count(&result)>1)
                    {
                        if(strcmp(notetype,"71")==0)
                        {
                            if(equal(bankcode, "8963"))
                                l=0;
                            else
                                l=1;
                        }
                        else if(equal(notetype,"72"))
                        {
                            if(equal(bankcode, "4202"))
                                l=0;
                            else
                                l=1;
                        }
                        else
                        {
                            if(equal(bankcode, "8956"))
                                l=0;
                            else
                                l=1;
                        }
                    }
                    else
                    {
                        if(equal(bankcode, "9579"))
                        {
                            strcpy(sAcctType,"����ʧ��");
                            break;
                        }
                    }
                    switch(*db_cell(&result, l, 0))
                    {
                        case '0':strcpy(sAcctType,"״̬δ֪");break;
                        case '1':strcpy(sAcctType,"���˳ɹ�");break;
                        case '2':strcpy(sAcctType,"�ѳ���");break;
                        case '5':strcpy(sAcctType,"�ѹ���");break;
                        case '9':strcpy(sAcctType,"����ʧ��");break;
                        default:break;
                    }
                    strcpy(sSerial,db_cell(&result, l, 2));
                }while(0);
                db_free_result(&result);
            }
            /*
            if(iRet && iRet != E_DB_NORECORD)
            {
                //BKINFO("��ѯ������ˮʧ��");
                //return iRet;
                strcpy(sAcctType, "δ����");
            }
            else if(iRet == E_DB_NORECORD)
            {
                //BKINFO("������ˮ������");
                strcpy(sAcctType, "δ����");
            }
            else
                strcpy(sAcctType, GetChineseName(acc_stat_list, atoi(sResult)));
            */
            /*
            else if(atoi(sResult) == 0)
                strcpy(sAcctType, "δ����");
            else if(atoi(sResult) == 1)
                strcpy(sAcctType, "�Ѽ���");
            else if(atoi(sResult) == 2)
                strcpy(sAcctType, "�ѳ���");
            else if(atoi(sResult) == 3)
                strcpy(sAcctType, "��ȡ��");
            else
                strcpy(sAcctType, "δ֪");
            */
        }

        if (*db_cell(&rs, i, 3) == CLRSTAT_SETTLED ||
                *db_cell(&rs, i, 3) == CLRSTAT_CHECKED)
        {
            suc_num ++;
            suc_amt += atof(db_cell_by_name(&rs, i, "settlamt"));
        }
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "refid"),
                newtime,
                GetChineseName(clrstat_list, *db_cell(&rs, i, 3)), //clearstate
                db_cell_by_name(&rs, i, "acceptor"),
                (atoi(db_cell_by_name(&rs, i, "trncode"))==7 \
                 ? "��Ʊ" : notename),
                GetChineseName(dcflag_list, db_cell_i(&rs, i, 6)), //dcflag
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "curcode"),
                FormatMoney(db_cell_by_name(&rs, i, "settlamt")),
                BeneAcctOrName, PayingAcctOrName,
                (*db_cell_by_name(&rs, i, "truncflag")=='0' ? "��" : "��"),
                sAcctType,sSerial);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    sprintf(charmoney, "%lf", suc_amt);
    //WriteRptFooter(fp, "%d;%.2lf;", suc_num, suc_amt);
    WriteRptFooter(fp, "%d;%s;", suc_num, FormatMoney(charmoney));
    fclose(fp);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.out",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        iRet = E_SYS_CALL;
        goto err_handle;
    }

    sprintf( filename, "%s", basename(caOutFile));

err_handle:
    return iRet;
}

//�����ӡƾ֤��ѯ,������ӦҪ�ص�������ʾ 
int QryInNote( xmlDocPtr xmlReq, char *filename)
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
    char tmp[60], sAcctType[40+1], sSerial[16+1], sResult[8+1], sSqlStr[1024];
    result_set orglist, rs, result;
    FILE *fp=NULL;
    int iRet = 0;
    int i, j, l;

    if (InitRptVar(xmlReq) != 0)
        return E_SYS_CALL;
    strcpy(startdate, GetTrnCtl("StartDate"));
    if (startdate[0] == 0)
        strcpy(startdate, GetSysPara("CURWORKDATE"));
    strcpy(enddate, GetTrnCtl("EndDate"));
    if (enddate[0] == 0)
        strcpy(enddate, GetSysPara("CURWORKDATE"));

    if (DiffDate(startdate, GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    if (*GetTrnCtl("Acceptor") == 0x00)
    {
        sprintf(condi, "acceptor in (select orgid from organinfo "
                "where acptproxy='%s' and orglevel='2')", gs_originator);
    }
    else
    {
        sprintf(condi, "acceptor='%s'", GetTrnCtl("Acceptor"));
        /*
        if (strcmp(GetTrnCtl("Acceptor"), gs_originator))
        {
            if (ChkAcptAgent(GetTrnCtl("Acceptor"), gs_originator))
            {
                return E_ORG_PERMIT;
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
        return iRet;
    if (db_begin() != 0)
        return iRet;
    iRet = db_exec("insert into " TMP_T_RS " select * from %s "
            "WHERE originator LIKE '%s%%' AND %s AND classid = %d"
            "  AND curcode LIKE '%s%%' AND notetype LIKE '%s%%'"
            "  AND noteno LIKE '%s%%' AND dcflag LIKE '%s%%'"
            "  %s AND settlamt BETWEEN %.2lf AND %.2lf"
            "  AND refid BETWEEN '%s' AND '%s'"
            "  AND workdate BETWEEN '%s' AND '%s'"
            "  AND workround LIKE '%s%%'"
            "  AND inoutflag = '2' ORDER BY workdate,prestime", tbname,
            GetTrnCtl( "Originator" ), condi, atoi(GetTrnCtl( "SvcClass" )),
            GetTrnCtl( "CurCode" ), GetTrnCtl( "NoteType" ),
            GetTrnCtl( "NoteNo" ), GetTrnCtl( "DCFlag" ), exchground_cond,
            atof(GetTrnCtl("MinAmount")), atof(GetTrnCtl("MaxAmount")),
            GetTrnCtl( "StartRefId" ), GetTrnCtl( "EndRefId" ),
            startdate, enddate, GetTrnCtl("WorkRound"));
    if (iRet != 0)
        goto err_handle;

    if ((iRet = db_query(&orglist, "SELECT DISTINCT acceptor FROM " TMP_T_RS
                " order by acceptor")) != 0)
        goto err_handle;
    if (db_row_count(&orglist) == 0)
    {
        iRet = E_DB_NORECORD;
        goto err_handle;
    }

    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InNote.para", getenv("HOME"), TCOP_BANKID );
    GetTmpFileName(caDataFile);
    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.in",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M'");
    for (i = 0; i < db_row_count(&orglist); i++)
    {
        int  suc_num = 0;
        double suc_amt = (double)0;
        // ��ѯ����ƾ֤
        iRet = db_query(&rs,
                "SELECT originator, refid, workdate, prestime, clearstate, "
                "notetype, dcflag, noteno, issuedate, curcode,settlamt, "
                "beneacct, payingacct, benename, payer, "
                "acceptor,trncode,truncflag FROM " TMP_T_RS
                " WHERE acceptor='%s' ORDER BY workdate,prestime", 
                db_cell(&orglist, i, 0));
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
                org_name(db_cell(&orglist, i, 0), bankname), gs_oper, tmp);
        for (j = 0; j < db_row_count(&rs); j++)
        {
            int workdate, prestime;
            /* ��ȡ�������ں�ʱ����д��� */
            //������¡��գ����һ���´�
            workdate = db_cell_i(&rs, j, 2);
            prestime = db_cell_i(&rs, j, 3);
            sprintf(newtime, "%04d %02d:%02d",workdate%10000, prestime/10000, 
                    prestime%10000/100);

            //��Ʊ��������д���
            //��Ʊ������д���
            //��'noteinfo'����ȥ��ѯ����ӦƱ�ݵ�������Ϣ 
            db_query_str(notename, sizeof(notename), "SELECT distinct name FROM "
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
            memset(sSerial, 0, sizeof sSerial);
            strcpy(sSerial, db_cell(&rs, j, 1));
            //��ˮ��ǰȥ0 ���ʶԷ�����û������ˮ��ǰ��0 �������ѯʱ����ˮ�Ų�0��
            //DelZero(sSerial);
            memset(sAcctType, 0, sizeof sAcctType);
            memset(sSqlStr, 0, sizeof sSqlStr);
            sprintf(sSqlStr, "select result,trncode,acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and convert(decimal, refid)=%s and inoutflag = '2'",
                    OP_REGIONID, db_cell_by_name(&rs, 0, "workdate"),
                    db_cell(&rs, j, 0), sdpStringTrimHeadChar(sSerial, '0'));
            memset(sResult, 0, sizeof sResult);
            memset(sSerial, 0, sizeof sSerial);
            //iRet = db_query_strs(sSqlStr, sSerial, sResult);
            iRet = db_query(&result, sSqlStr);
            if(iRet!=0)
            {
                if(iRet==E_DB_NORECORD)
                {
                    strcpy(sAcctType,"δ����");
                }
                else
                {
                    db_free_result(&rs);
                    return iRet;
                }
            }
            else
            {
                l=0;
                if(db_row_count(&result)>1)
                {
                    if(*db_cell_by_name(&rs,j,"dcflag")=='2')
                    {
                        if(strcmp(db_cell(&result, 0, 1),"8249")==0)
                            l=0;
                        else
                            l=1;
                    }
                }
                switch(*db_cell(&result, l, 0))
                {
                    case '0':strcpy(sAcctType,"״̬δ֪");break;
                    case '1':strcpy(sAcctType,"���˳ɹ�");break;
                    case '2':strcpy(sAcctType,"�ѳ���");break;
                    case '5':strcpy(sAcctType,"�ѹ���");break;
                    case '9':strcpy(sAcctType,"����ʧ��");break;
                    default:break;
                }
                strcpy(sSerial,db_cell(&result, l, 2));
                db_free_result(&result);
            }
            /*
            if(iRet && iRet != E_DB_NORECORD)
            {
                BKINFO("��ѯ������ˮʧ��");
                strcpy(sAcctType, "δ����");
                //return iRet;
            }
            else if(iRet == E_DB_NORECORD)
            {
                BKINFO("������ˮ������");
                strcpy(sAcctType, "δ����");
            }
            else
                strcpy(sAcctType, GetChineseName(acc_stat_list, atoi(sResult)));
            */
            /*
            else if(atoi(sResult) == 0)
                strcpy(sAcctType, "δ����");
            else if(atoi(sResult) == 1)
                strcpy(sAcctType, "�Ѽ���");
            else if(atoi(sResult) == 2)
                strcpy(sAcctType, "�ѳ���");
            else if(atoi(sResult) == 3)
                strcpy(sAcctType, "��ȡ��");
            else
                strcpy(sAcctType, "δ֪");
            */

            if (*db_cell(&rs, j, 4) == CLRSTAT_SETTLED ||
                    *db_cell(&rs, j, 4) == CLRSTAT_CHECKED)
            {
                suc_num ++;
                suc_amt += atof(db_cell_by_name(&rs, j, "settlamt"));
            }
            fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                    db_cell(&rs, j, 0),
                    db_cell(&rs, j, 1),
                    newtime,
                    GetChineseName(clrstat_list, *db_cell(&rs, j, 4)),
                    (atoi(db_cell_by_name(&rs, j, "trncode"))==7 \
                     ? "��Ʊ" : notename),
                    GetChineseName(dcflag_list, db_cell_i(&rs, j, 6)), //dcflag
                    db_cell(&rs, j, 7),
                    db_cell(&rs, j, 9),
                    FormatMoney(db_cell(&rs, j, 10)),
                    BeneAcctOrName, PayingAcctOrName,
                    (*db_cell_by_name(&rs, j, "truncflag")=='0' ? "��" : "��"),
                    sAcctType, sSerial);
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
            iRet = E_TR_PRINT;
            db_free_result(&orglist);
            goto err_handle;
        }
    }
    db_free_result(&orglist);

    sprintf( filename, "%s", basename(caOutFile));

err_handle:
    db_commit();
    return iRet;
}

int QryOutQuery(xmlDocPtr xmlReq, char *filename)
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

    if (InitRptVar(xmlReq) != 0)
        return E_SYS_CALL;
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/OutQuery.para", getenv("HOME"), TCOP_BANKID );

    startdate = atol(GetTrnCtl("StartDate"));
    if (startdate == 0L)
        startdate = atol(GetSysPara("CURWORKDATE"));
    enddate = atol(GetTrnCtl("EndDate"));
    if (enddate == 0L)
        enddate = atol(GetSysPara("CURWORKDATE"));

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname,
            GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            gs_originator, org_name(gs_originator, gs_bankname));

    iRet = db_query(&rs,
            "SELECT refid,acceptor,content,workdate,transflag FROM queryinfo "
            "WHERE inoutflag='1' and originator='%s'"
            "  AND workdate BETWEEN '%08ld' AND '%08ld' ", 
            gs_originator, startdate, enddate);
    if (iRet != 0)
        return iRet;
    if (db_row_count(&rs) == 0)
    {
        iRet = E_DB_NORECORD;
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

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.oqry",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        iRet = E_TR_PRINT;
        goto err_handle;
    }

    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    return iRet;
}

int QryInQuery(xmlDocPtr xmlReq, char *filename)
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

    if (InitRptVar(xmlReq) != 0)
        return E_SYS_CALL;
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/InQuery.para", getenv("HOME"), TCOP_BANKID );

    startdate = atol(GetTrnCtl("StartDate"));
    if (startdate == 0L)
        startdate = atol(GetSysPara("CURWORKDATE"));
    enddate = atol(GetTrnCtl("EndDate"));
    if (enddate == 0L)
        enddate = atol(GetSysPara("CURWORKDATE"));

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;%s;", gs_sysname,
            GetChineseName(mailtype_list, *GetTrnCtl("MailType")),
            ChineseDate(startdate), ChineseDate(enddate), 
            gs_originator, org_name(gs_originator, gs_bankname));

    iRet = db_query(&rs,
            "SELECT originator, refid, content, workdate, readflag FROM queryinfo "
            "WHERE inoutflag='2' and readflag LIKE '%s%%'"
            "  AND acceptor='%s'"
            "  AND workdate BETWEEN '%08ld' AND '%08ld' ", 
            GetTrnCtl("Readed"),
            gs_originator,
            startdate, enddate);
    if (iRet != 0)
    {
        fclose(fp);
        return iRet;
    }
    if (db_row_count(&rs) == 0)
    {
        iRet = E_DB_NORECORD;
        fclose(fp);
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

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.iqry",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        iRet = E_TR_PRINT;
        goto err_handle;
    }

    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    return iRet;
}

int QryAgreementList(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char beneid[16]={0};
    char bankid[16]={0};
    char printtime[32]={0};

    FILE *fp=NULL;
    int iRet = 0;
    int i, j;

    if (InitRptVar(xmlReq) != 0)
        return E_SYS_CALL;
    snprintf( caParaFile, sizeof(caParaFile),
            "%s/dat/%d/AgreementList.para", getenv("HOME"), TCOP_BANKID );

    strcpy(beneid, GetTrnCtl("BeneAcct"));
    strcpy(bankid, GetTrnCtl("PayingAcct"));

    fp = fopen(GetTmpFileName(caDataFile), "w");

    WriteRptHeader(fp, "");

    iRet = db_query(&rs,
            "SELECT state, beneid, payingacct, payer, userid, agreementid FROM agreement "
            "WHERE beneid like '%%%s%%' and bankid like '%%%s%%' "
            " group by state "
            " ORDER BY state",
            beneid, bankid);
    if (iRet != 0)
    {
        fclose(fp);
        return iRet;
    }
    if (db_row_count(&rs) == 0)
    {
        iRet = E_DB_NORECORD;
        fclose(fp);
        goto err_handle;
    }

    for (j = 0;j < db_row_count(&rs); j++)
    {
        for (i = 0; i < db_col_count(&rs); i++)
        {
            if(i == 0)
                fprintf(fp, "%s;", *db_cell(&rs, j, i)=='1'?"����":"ע��");
            else
                fprintf(fp, "%s;", db_cell(&rs, j, i));
        }
        fprintf(fp, "\n");
    }
    db_free_result(&rs);

    WriteRptRowCount(fp, j);
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    WriteRptFooter(fp, "%d;%s;%s;", j, printtime, gs_oper);
    fclose(fp);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.iqry",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());
    iRet = PrintReportList(caParaFile, caDataFile, caOutFile);
    if (iRet != 0)
    {
        iRet = E_TR_PRINT;
        goto err_handle;
    }

    sprintf( filename, "%s", basename(caOutFile));

err_handle:

    return iRet;
}
