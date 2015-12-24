#include "interface.h"
#include "chinese.h"
char *_sSysCode = "3214001";
static char _RecvSerial[8+1];
//static char _AcctSerial[8+1];
static char _FeeSerial[8+1];
int _iFeeAcctFlag=0;

static char gs_originator[13] = {0x00};
static char gs_bankid[13] = {0x00};
static char gs_bankname[81] = {0x00};
static char gs_sysname[61] = {0x00};
static char gs_oper[30] = {0x00};

static char *ChineseDate(long curr_date)
{
    char buf[20];

    sprintf(buf, "%04ld��%02ld��%02ld��", 
            curr_date/10000, curr_date%10000/100, curr_date%100);

    return strdup(buf);
}

static char *GetTmpFileName( char *pFileName )
{
    char buf[256];

    sprintf( buf, "%s/tmpXXXXXX", getenv("FILES_DIR") );
    mkstemp( buf );
    strcpy( pFileName, buf );

    return pFileName;
}

static void ifree(char *p)
{
    if (p != NULL)
    {
        free(p); p = NULL;
    }
}

extern int Acct_Bj(xmlDoc *doc, char *pDate, char *pBank, char *pSerial, char *pInoutflag, int iFlag);

int InitRptVar(xmlDocPtr xmlReq)
{
    char opername[20];
    strcpy(gs_bankid, XMLGetNodeVal(xmlReq, "//opInnerBank"));
    strcpy(gs_originator, XMLGetNodeVal(xmlReq, "//opOriginator"));
    strcpy(gs_oper, XMLGetNodeVal(xmlReq, "//opOperid"));
    // ����Ա����
    db_query_str(opername, sizeof(opername), 
            "select name from operinfo where nodeid=%d and operid='%s' "
            "and bankno = '%s'", OP_REGIONID, gs_oper, gs_bankid);
    strcat(gs_oper, " "); strcat(gs_oper, opername);
    // ��������
    org_name(gs_originator, gs_bankname);
    // ϵͳ����->����ͷ
    /*
    memset(gs_sysname, 0, sizeof(gs_sysname));
    strcpy(gs_sysname, GetSysPara("SYSNAME"));
    if (*gs_sysname == 0x00)
    */
    strcpy(gs_sysname, "�й����������������ݷ���");

    return 0;
}

/*
 * �ж��˻�����(�Թ� ���� ��)
 * ����:
 *      �Թ� 0
 *      ���� 1
 *      ��   2
 */
int IsAcctType(char *pAcctNo)
{
    int ret=0;
    char sAcctNo[32+1]={0};
    char *p = NULL;

    memset(sAcctNo, 0, sizeof(sAcctNo));
    if(pAcctNo[0] == '*')
        strcpy(sAcctNo, pAcctNo+1);
    else
        strcpy(sAcctNo, pAcctNo);
    if (strlen(sAcctNo) == 7)
    {
        // 7λת28λ
        if ((p = GetSysPara(sAcctNo)) != NULL)
            strcpy(pAcctNo, p);
        else
            return -1;
    }
    else
        strcpy(pAcctNo, sAcctNo);

    //�Թ��˻�����18λ ���� ��10��ͷ
    /*
    //if(memcmp(sAcctNo, "10", 2) == 0 && strlen(sAcctNo) == 18) 
    if(strlen(sAcctNo) == 18) 
	    return 0;
    //�Թ��˻�����28λ����7λ,Ϊ�ڲ��ʻ� 
    if( strlen(sAcctNo) == 28 )
	    return 3;
    if( strlen(sAcctNo) == 7 )
	    return 4;
    if(strlen(pAcctNo) == 19 || strlen(pAcctNo) == 16)
	    return 2;
    */
    switch(strlen(sAcctNo))
    {
        case 18:
            //if(memcmp(sAcctNo, "10", 2) == 0)
            if(sAcctNo[0] == '1')
                ret = 0;
            else
                ret = 1;
            break;
        case 16:
        case 19:
            ret = 2;
            break;
        case 7:
        case 28:
            ret = 3;
            break;
        default:
            ret = 1;
            break;
    }

    return ret;
}

/*�ʻ�ת��
  1-�����˻�,2-�ڲ�5�ֻ�*/
int GetBankAcct( char *sAcctNo, char *sBankNo, int Flag ) 
{
    char sql[1024]={0};
    char sTmp[37]={0};
    int ret;

    switch( Flag )
    {
        case 1: //�����˻�
            sprintf(sql, "select clearacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 2: //��������
            sprintf(sql, "select returnacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 3: //�ʽ���������
            sprintf(sql, "select debitacct from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        case 4: //��ƻ���
            sprintf(sql, "select bankid from bankinfo where nodeid=%d "
                    "and exchno='%s' and banklevel='2'", OP_REGIONID, sBankNo);
            break;
        default:
            return -1;
    }
    ret = db_query_str(sTmp, sizeof(sTmp), sql );
    if( ret )
    {
        BKINFO("��ѯ������Ϣʧ��.");
        return ret;
    }
    memset( sAcctNo, 0, sizeof(sAcctNo) );
    strcpy( sAcctNo, sTmp );
    return 0;
}

/*
 * ����������
 * ���� 1 �ۿ�������
 *      0 ʵʱ��
 */
int Fee_Cast(char *pAcctNo, char *pFeeAmt)
{
    int ret=0;
    char sBaseAmt[15+1]={0}, sFeeRate[15+1]={0}, sType[2+1]={0};
    char sFlag[1+1]={0};
    char sSqlStr[1024]={0};

    BKINFO("����������...");
    sprintf(sSqlStr, "select value from feetype where nodeid = %d and typeid = %s", OP_REGIONID, "1");
    ret = db_query_str(sBaseAmt, sizeof(sBaseAmt), sSqlStr);
    if(ret) {
        BKINFO("��ѯ����������ʧ��");
        return ret;
    }
    sprintf(sSqlStr, "select typeid from feeset where nodeid = %d and acctno = '%s'", OP_REGIONID, pAcctNo);
    ret = db_query_str(sType, sizeof(sType), sSqlStr);
    if(ret && ret != E_DB_NORECORD) {
        BKINFO("��ѯ����������ʧ��");
        return ret;
    }
    else if(ret == E_DB_NORECORD) {
        strcpy(pFeeAmt, sBaseAmt);
        return 0;
    }
    sprintf(sSqlStr, "select flag, value from feetype where nodeid = %d and typeid = %s", OP_REGIONID, sType);
    ret = db_query_strs(sSqlStr, sFlag, sFeeRate);
    if(ret) {
        BKINFO("��ѯ�����ѷ���ʧ��");
        return ret;
    }
    sprintf(pFeeAmt, "%.2f", atof(sFeeRate) * atof(sBaseAmt));
    BKINFO("����ʵʱ��־:%s ������:%s", sFlag, pFeeAmt);

    return atoi(sFlag);
}

/*
 * ���ݽ��׽���ж���������״̬
 */
int IsAcctCode(char *pRet)
{
    switch(atoi(pRet))
    {
        case 0:
            return 0;
            //���׽������ȷ
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
        case 207:
        case 3002:
        case 3003:
        case 8043:
        case 8048:
        case 8202:
        case 8203:
        case 8204:
        case 8205:
            return 7;       //����״̬��ȷ��
        default:
            return 2;
    }
}

/*
 * �����ʺŻ�ȡЭ���
 */
int Qry_Agreement(char *pKhAcct, char *pXyh)
{
    int ret=0;
    char sXyh[60+1]={0};

    // �ڲ���û��Э���
    if (strlen(pKhAcct) == 28)
    {
        pXyh[0] = 0x00;
        return 0;
    }

    ret = db_query_str(sXyh, sizeof(sXyh), "select agreement from acctinfo where "
            " nodeid = %d and acctid = '%s'",
            OP_REGIONID, pKhAcct);
    if(ret == E_DB_NORECORD) {
        BKINFO("%s�ʺ���Ϣ������", pKhAcct);
        return ret;
    }
    else if(ret) {
        BKINFO("%s��ѯ�ʺ���Ϣʧ��", pKhAcct);
        return ret;
    }
    if(strlen(sXyh) == 0) {
        BKINFO("%s�ʺ�������Э����ϢΪ��", pKhAcct);
        return E_DB_NORECORD;
    }
    strcpy(pXyh, sXyh);
    return 0;
}

/*
 * ���¼�����ˮ���¼Ϊ�ѳ���
 */
int UpAcctSerial(xmlDocPtr opDoc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "update acctjour set result = '%s' \
            where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' \
            and inoutflag = '%s'",
            //XMLGetNodeVal(opDoc, "//opHostSerial"),
            "2",    //�ѳ���
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opOriginator"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            pInOutFlag
           );
    ret = db_exec(sSqlStr);
    return ret;
}

/*
 * ��Կ����
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_333(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sMAC[16+1]={0};
    char sResult[6+1]={0};

    BKINFO("��ʼ��Կ����...");
    ret = callInterface(8002, opDoc);
    if(ret)
    {
        BKINFO("����8002ʧ��");
        return ret;
    }

    strcpy(sMAC, XMLGetNodeVal(opDoc, "//opHreserved1"));
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0) {
        BKINFO("��Կ����ʧ��");
        return atoi(sResult);
    }

    if (db_exec("update syspara set paraval = '%s' where paraname = 'MACKEY' and nodeid = %d", sMAC, OP_REGIONID) != 0)
        return E_DB;

    BKINFO("��Կ�������");

    return ret;
}

/*
 * ���¼��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_101(void *doc, char *p)
{
    int ret=0,iAcctFlag;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *pDCFlag=NULL, *pAcctNo=NULL;

    /* �ж��Ƿ�����ʺ� �öԹ���˽�ʺű�־ */
    pDCFlag = XMLGetNodeVal(opDoc, "//opDcflag");
    if(atoi(pDCFlag) == 1)
        pAcctNo = XMLGetNodeVal(opDoc, "//opBeneacct");
    else if(atoi(pDCFlag) == 2)
        pAcctNo = XMLGetNodeVal(opDoc, "//opPayacct");

    ret = IsAcctType(pAcctNo);
    if (ret < 0)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "�˻����ͼ�����");
        return 8999;
    }

    /* resflag1������Ƕ�˽�ʺ� �Թ��ʺŵı�־ 0�Թ� 1��˽ */
    ret = db_exec("update trnjour set resflag1 = '%d' where nodeid = %d and "
            " workdate = '%s' and refid = '%s' and inoutflag = '%s' and originator = '%s'",
            (ret == 1 || ret == 2) ? 1 : 0,
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            XMLGetNodeVal(opDoc, "//opInoutflag"),
            XMLGetNodeVal(opDoc, "//opOriginator"));
    if(ret)
        BKINFO("�����ʺű�־ʧ��");

    return ret;
}

void SetOperInfo(xmlDocPtr doc)
{
    char sAcctOper[64], sChkOper[64], sTmp[64];
    int ret;

    db_query_str(sTmp, sizeof(sTmp), 
            "select name from operinfo where nodeid=%d and operid='%s' "
            "and bankno='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opOperid"),
            XMLGetNodeVal(doc, "//opInnerBank"));
    sprintf(sChkOper, "%s %s", XMLGetNodeVal(doc, "//opOperid"), sTmp);
    XMLSetNodeVal(doc, "//opChecker", sChkOper);

    ret = db_query_str(sAcctOper, sizeof(sAcctOper), 
            "select acctoper from trnjour where nodeid=%d and workdate='%s' "
            "and originator='%s' and refid='%s' and inoutflag='1'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), 
            XMLGetNodeVal(doc, "//opOriginator"), 
            XMLGetNodeVal(doc, "//opRefid"));
    if(ret == 0)
    {
        ret = db_query_str(sTmp, sizeof(sTmp), 
                "select name from operinfo where nodeid=%d and operid='%s' "
                "and bankno = '%s'", OP_REGIONID, sAcctOper, 
                XMLGetNodeVal(doc, "//opInnerBank"));
        sprintf(sAcctOper+strlen(sAcctOper), " %s", sTmp);
        XMLSetNodeVal(doc, "//opInputer", sAcctOper);
    }
    BKINFO("inputer=[%s]", sAcctOper);
    BKINFO("checker=[%s]", sChkOper);

    return;
}

/*
 * ���˼���
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_102(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sCurCode[3+1]={0};
    char sResult[6+1]={0};
    char sTmp[40]={0}, sStr[40]={0};

    BKINFO("������ͨѶǰ���־:%s", p);

    // ���ز���Ա����
    SetOperInfo(opDoc);

    /* ��Ʊ���ײ����� */
    if(OP_TCTCODE == 7) {
        BKINFO("��Ʊ�������ڲ�����");
        return 0;
    }
    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//opCurcode"));
    /* ��Ҳ��Զ����� */
    if(memcmp(sCurCode, "CNY", 3) != 0)
        return 0;

    XMLSetNodeVal(opDoc, "//opInoutflag", "1");
    if(p[0] == COMMTOPH_AFTER[0])           //������ͨѶ��
        ret = CommToExbAfter(opDoc);
    else if(p[0] == COMMTOPH_BEFORE[0])     //������ͨѶǰ
        ret = CommToExbBefore(opDoc);

    return ret;
}

int AcctToBank(xmlDoc *opDoc, char *pAcctNo, char *pTxType, char *pTrnCode)
{
    int ret=0;
    char sOpSerial[20+1]={0}, sNoteType[2+1]={0}, sDCFlag[1+1]={0};
    char sHnType[6+1]={0}, sAmt[15+1]={0}, sStr[89+1]={0};
    char sWorkDate[8+1]={0}, sRefId[16+1]={0}, sOutBank[12+1]={0};
    char sOrgId[12+1]={0}, sAcctOper[6+1]={0}, sResult[6+1]={0};
    char sFeeAmt[15+1]={0}, sInoutflag[1+1]={0}, retstr[1+1]={0};
    char sSqlStr[1024]={0};
    long lTmp = 0L;

    /* pTxType=2�������Ѽ��� ������Ʊ������ת�� */
    if(atoi(pTxType) != 2)
    {
        /* ����ͬ��Ʊ�����Ͳ�ѯ����Ʊ������ */
        BKINFO("ת��ƾ֤����...");
        strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
        strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
        sprintf(sSqlStr, "select banktype from notetypemap \
                where nodeid = %d and tctype = '%s' and dcflag = '%s'",
                OP_REGIONID, sNoteType, sDCFlag);
        ret = db_query_str(sHnType, sizeof(sHnType), sSqlStr);
        if(ret) {
            BKINFO("��ѯ����Ʊ������ʧ��");
            return 0; // modi by chenjie ret->0
        }
        XMLSetNodeVal(opDoc, "//opNotetype", sHnType);
    }

    /* ���������ڼ�����ˮ */
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(opDoc, "//opOreserved1", sOpSerial);
    BKINFO("���ɼ�����ˮ:%s", sOpSerial);

    /* ����MAC */
    /*
       strcpy(sAmt, XMLGetNodeVal(opDoc, "//opSettlamt"));
       sprintf(sStr, "%s %s %s %s %s %015.0f %s", 
       _sSysCode, 
       pTrnCode,
       XMLGetNodeVal(opDoc, "//opWorkdate"), 
       sOpSerial, 
       pAcctNo,
       atof(sAmt)*100, pTxType);
     */

    XMLSetNodeVal(opDoc, "//opTermno", pTxType);
    //������
    if(atoi(pTxType) == 2) {
        BKINFO("�����Ѽ���...");
        strcpy(sInoutflag, "3");
    }
    else if (atoi(pTxType) == 1) {
        BKINFO("���������...");
        strcpy(sInoutflag, "4");
    }else{
        BKINFO("���׼���...");
        strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    }
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sRefId, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sOrgId, XMLGetNodeVal(opDoc, "//opInnerorganid"));
    strcpy(sAcctOper, XMLGetNodeVal(opDoc, "//opOperid"));
    /* �жϼ�����ˮ�Ƿ���� */
    ret = db_query_str(retstr, sizeof(retstr), "select result from acctjour "
            "where nodeid = %d and workdate = '%s' and originator = '%s' and "
            "refid = '%s' and inoutflag = '%s'", 
            OP_REGIONID, sWorkDate, sOutBank, 
            sRefId, sInoutflag);

    if(ret == E_DB_NORECORD) {
        ret = db_exec("insert into acctjour values(%d, '%s', '%s', '%s',"
                "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
                "'%s', '%s', '%s')",
                OP_REGIONID, sWorkDate, sOutBank, sRefId, sInoutflag, 
                pTrnCode, sOpSerial, _RecvSerial, sOrgId, sAcctOper, 
                "", "0", "0", "", "", "", "");
        if(ret) {
            BKINFO("���������Ϣʧ��");
            return E_APP_ACCOUNTFAIL;
        }
    }
    else if(ret)
        return E_APP_ACCOUNTFAIL;
    if(atoi(retstr) == 1)   //�Ѽ���
        return E_APP_ACCOUNTSUCC;
#if 0
    /* �ж��������շ���Ϣ�Ƿ���� */
    if(atoi(pTxType) == 2) {
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist "
                "where nodeid = %d and workdate = '%s' and originator = '%s' and "
                "refid = '%s' and inoutflag = '%s'", 
                OP_REGIONID, sWorkDate, sOutBank, 
                sRefId, sInoutflag);
        if(ret == E_DB_NORECORD) {
            ret = db_exec("insert into feelist values(%d, '%s', '%s', '%s', "
                    "'%s', '%s', '%s', '%s', %.2f, '%s')",
                    OP_REGIONID, sWorkDate, "1", sOutBank, sRefId, 
                    sWorkDate, pAcctNo, "", atof(sAmt)/100, "0");
            if(ret) {
                BKINFO("�����������嵥ʧ��");
                return E_APP_ACCOUNTFAIL;
            }
        }
        else if(ret)
            return E_APP_ACCOUNTFAIL;
    }
    if(atoi(retstr) == 1)   //���շ�
        return E_APP_ACCOUNTSUCC;
#endif

    /* �����ڼ���,�������Ӧ����Ϣ */
    XMLSetNodeVal(opDoc, "//opBKRetcode","");
    XMLSetNodeVal(opDoc, "//opBKRetinfo","");
    ret = callInterface(atoi(pTrnCode), opDoc);
    if(ret)
        return E_APP_ACCOUNTFAIL;
    memset(sResult, 0, sizeof(sResult));
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if( strlen(sResult) != 0 && atoi(sResult) != 0)
        return E_APP_ACCOUNTFAIL;
    if( strlen(sResult) == 0 )
    {
        ret = db_exec("update acctjour set acctserial = '%s', revserial = '%s', result = '%s', reserved1 = '%s' "
                "where nodeid = %d and workdate = '%s' and originator = '%s' "
                "and refid = '%s' and inoutflag = '%s' and trncode = '%s'",
                sOpSerial, _RecvSerial, "2", XMLGetNodeVal(opDoc, "//opOreserved1"),
                OP_REGIONID, sWorkDate,
                sOutBank, sRefId, sInoutflag, pTrnCode);
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "���׽������ȷ,���ѯ���ڽ��.");
        return E_SYS_COMM;
    }

    /* ���������ˮ */
    memset(_RecvSerial, 0, sizeof _RecvSerial);
    strcpy(_RecvSerial, XMLGetNodeVal(opDoc, "//opHostSerial"));
    XMLSetNodeVal(opDoc, "//opNotetype", sNoteType);
    BKINFO("����������ˮ:%s", _RecvSerial);

    /* reserved1�ֶα������ڷ��صļ������� */
    ret = db_exec("update acctjour set acctserial = '%s', revserial = '%s', result = '%s', reserved1 = '%s' "
            "where nodeid = %d and workdate = '%s' and originator = '%s' "
            "and refid='%s' and inoutflag='%s' and trncode='%s'",
            sOpSerial, _RecvSerial, "1", XMLGetNodeVal(opDoc, "//opOreserved1"),
            OP_REGIONID, sWorkDate, 
            sOutBank, sRefId, sInoutflag, pTrnCode);

    return E_APP_ACCOUNTSUCC;
}

int CommToExbAfter(void *doc)
{
    int ret=0, iAcctFlag=0,iFeeType;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sNoteType[2+1]={0}, sOpSerial[20+1]={0};
    char sResult[6+1]={0}, sTruncFlag[1+1]={0}, sBankSerial[8+1]={0};
    char sFlCode[5+1]={0}, sAcctNo[32+1]={0}, sFee[15+1]={0};
    char sAcctDate[8+1]={0};
    char sSqlStr[1024]={0};
    char sOriginator[12+1]={0}, sPayAcct[36+1]={0}, sClearAcct[36+1]={0};
    int trncode;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));

    /* ʵʱ��ȡ������ȡ�� */
#if 0
    /* ���������� */
    ret = Fee_Cast(sAcctNo, sFee);
    if(ret == 0)
        XMLSetNodeVal(opDoc, "//opFee", sFee);
#endif

    strcpy(sResult, XMLGetNodeVal(opDoc, "//opTCRetcode"));

    if(sDCFlag[0] == '2' && atoi(sResult) == 0) {
        BKINFO("������ύ���ĺ����Ĵ���ɹ������ڲ���������");
        return 0;
    }
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));

    ret = IsAcctCode(sResult);
    if(ret == 7) {
        BKINFO("���ķ��ؽ��׽������ȷ");
        return E_SYS_COMM;
    }
    if(sDCFlag[0] == '1' && atoi(sResult) != 0) {
        BKINFO("��������Ĵ���ʧ�ܣ����ڲ�����");
        return 0;
    }
    /* �ж��˻����� */
    iAcctFlag = IsAcctType(sAcctNo);
    /* ��˽�˻���������� */
    if ((iAcctFlag == 1 || iAcctFlag == 2) && atoi(sResult) == 0)
        return E_APP_ACCOUNTSUCC;
    /*
       else if(iAcctFlag != 0 && atoi(sResult) != 0)
       return E_APP_ACCOUNTANDCZ;
     */
    /* ��ȡ���Ľ��� */
    if(atoi(sResult) != 0) {
        BKINFO("��ʼȡ������...");
        ret = AcctCancelQry(XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opOriginator"),
                XMLGetNodeVal(opDoc, "//opRefid"), XMLGetNodeVal(opDoc, "//opInoutflag"), sBankSerial, sAcctDate);
        if(ret)
            return E_APP_ACCOUNTNOCZ;
        /*�ڲ��˻���Ҫ�����ύ�׽���*/
        if( iAcctFlag ==3 || iAcctFlag ==4) //�ڲ���
        {
            BKINFO("�ڲ�������ȡ��.");
            strcpy( sOriginator, XMLGetNodeVal(opDoc, "//opOriginator") );

            if( iAcctFlag == 4 )//7->28
                GetBankAcct( sAcctNo, sOriginator, 2 );
            BKINFO("�ڲ�5�ֻ�[%s].",sAcctNo);
            XMLSetNodeVal(opDoc, "//opBeneacct", sAcctNo);

            GetBankAcct( sClearAcct, sOriginator, 1 );
            BKINFO("�����˻�[%s].",sClearAcct);
            XMLSetNodeVal(opDoc, "//opPayacct", sClearAcct);

            sprintf(sOpSerial, "%ld", GenSerial("account", 1, 999999999, 1));
            XMLSetNodeVal(opDoc, "//opOreserved1", sOpSerial);
            XMLSetNodeVal(opDoc, "//opTermno", "0000");
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "2");
            /*�ڲ��ʺŸ���,���������6130ͬ8130*/
            trncode = 6130;
        }
        else
        {
            sprintf(sOpSerial, "%ld", GenSerial("account", 1, 999999999, 1));
            XMLSetNodeVal(opDoc, "//opOreserved2", sOpSerial);
            XMLSetNodeVal(opDoc, "//opOreserved3", sAcctDate);
            XMLSetNodeVal(opDoc, "//opHostSerial", sBankSerial);
            XMLSetNodeVal(opDoc, "//opAccountType", "0");
            trncode = 9121;
        }
        /* �ڴ�ֻ�������ҵ���Ѽ��� �������ʧ����ȡ�� */
        ret = callInterface(trncode, opDoc);   //9121ͬ8121����
        if(ret)
            return E_APP_ACCOUNTNOCZ;
        memset(sResult, 0, sizeof sResult);
        strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
        if(atoi(sResult) != 0)
            return E_APP_ACCOUNTNOCZ;
        ret = UpAcctSerial(opDoc, "1");
        return E_APP_ACCOUNTANDCZ;
    }
    sprintf(sSqlStr, "select truncflag from noteinfo \
            where nodeid = %d and notetype = '%s' and dcflag = '%s'", 
            OP_REGIONID, sNoteType, sDCFlag);
    ret = db_query_str(sTruncFlag, sizeof(sTruncFlag), sSqlStr);
    if(ret) {
        BKINFO("��ѯƱ�ݽ�������ʧ��");
        return ret;
    }

    /* ���� */
    if(atoi(sTruncFlag) == 0 && atoi(sNoteType) != 3 && atoi(sNoteType) != 4)
        ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8130");
    else {
        strcpy(sFlCode, "0000");
        ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
    }

    return ret;
}

int CommToExbBefore(void *doc)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sNoteType[2+1]={0};
    char sResult[6+1]={0};
    char sFlCode[5+1]={0}, sAcctNo[32+1]={0};
    char sXyh[60+1]={0};
    char sTrnCode[5];
    int iAcctFlag;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
    if(sDCFlag[0] == '1') {
        BKINFO("������ύ����ǰ���ڲ���������");
        return 0;
    }

    strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    BKINFO("����˻�����...");
    iAcctFlag = IsAcctType(sAcctNo);
    /* ��˽�˻������� */
    if(iAcctFlag == 1 || iAcctFlag == 2)
        return E_APP_ACCOUNTSUCC;

    /* ����˺�Ϊ�ڲ���, ���6130. acctlen=28 */
    if (iAcctFlag == 3)
    {
        sprintf(sTrnCode, "6130");
        XMLSetNodeVal(opDoc, "//opEXBKTxflag", "0");
    }
    else
    {
        sprintf(sTrnCode, "8120");
        /* ����ҵ��ǽ��˵�ͨ��Э�鷽ʽ�ǿͻ��� */
        if(atoi(sNoteType) != 1)
        {
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "4");
            /* �����ʺŻ�ȡЭ��� */
            ret = Qry_Agreement(sAcctNo, sXyh);
            if(ret)
                return ret;
            XMLSetNodeVal(opDoc, "//opPaykey", sXyh);
        }
    }

    /* ���� */
    strcpy(sFlCode, "0000");
    ret = AcctToBank(opDoc, sAcctNo, sFlCode, sTrnCode);
    if(ret != E_APP_ACCOUNTSUCC)
        return ret;
    //BKINFO("RECVSERIAL:%s", _RecvSerial);
    //strcpy(gpBankSerial, _RecvSerial);
    //BKINFO("ACCTSERIAL:%s", _AcctSerial);

    return ret;
}

/* * ������
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_110(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[2+1]={0}, sAcctNo[32+1]={0};

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));

    ret = AcctToBank(opDoc, sAcctNo, "0002", "8120");

    return ret;
}

/*
 * ����ȷ��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 111 -> 123
 */
int PF10_123(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char *pDCFlag=NULL, *pNoteType=NULL;
    char sAcctNo[32+1]={0}, sXyh[60+1]={0};
    char where[1024] = {0};

    pDCFlag = XMLGetNodeVal(opDoc, "//opDcflag");
    pNoteType = XMLGetNodeVal(opDoc, "//opNotetype");

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='2'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'));

    BKINFO("����ȷ��:���[%s]", pDCFlag);
    XMLSetNodeVal(doc, "//opInoutflag", "2");

    if(atoi(pDCFlag) == 1)
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    else if(atoi(pDCFlag) == 2)
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));

    /* ����� ת��֧Ʊ��ӡ ����Э�鷽ʽ */
    if(atoi(pDCFlag) == 1) {
        if(atoi(pNoteType) != 2) {
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "4");
            /* �����ʺŻ�ȡЭ��� */
            ret = Qry_Agreement(sAcctNo, sXyh);
            if(ret)
                return ret;
            XMLSetNodeVal(opDoc, "//opPaykey", sXyh);
        }
        else
            XMLSetNodeVal(opDoc, "//opEXBKTxflag", "1");
    }

    ret = AcctToBank(opDoc, sAcctNo, "0000", atoi(pDCFlag) == 1 ? "8120" : "8110");

    if (ret == E_APP_ACCOUNTSUCC)
        db_exec("UPDATE %s SET chkflag='1' %s", 
                strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);

    return ret;
}

/*
 * ����޸�
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_103(void *doc, char *p)
{
    int ret=0;

    ret = PF10_101(doc, p);

    return ret;
}

/*
 * ���ȡ��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_301(void *doc, char *p)
{
    return 0;
}

/* 
   ���Э���, ����ƽ̨������
 */
int ChkAgreement(char *id)
{
    result_set rs;
    int rc;

    rc = db_query(&rs, "select state from agreement where nodeid=%d "
            "and agreementid='%s'", OP_REGIONID, id);
    if (rc != 0)
    {
        if (rc == E_DB_NORECORD)
            rc = E_MNG_XY_NOTEXIST;
        return rc;
    }
    if (*db_cell(&rs, 0, 0) != '1')
    {
        rc = E_MNG_XY_CANCEL;
        return rc;
    }
    db_free_result(&rs);

    return 0;
}

/*
 * �������
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_104(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sDCFlag[1+1]={0}, sResult[6+1]={0};
    char sCurCode[3+1]={0}, sNoteType[2+1]={0};
    char sSqlStr[1024]={0}, sTruncFlag[1+1]={0};
    char sAcctNo[32+1]={0}, sFlCode[4+1]={0};
    char sOrgId[12+1]={0}, sAcctCheck[1+1]={0};
    char sAcctState[1+1]={0}, sBAcctName[60+1]={0}, sTAcctName[60+1]={0};
    int iAcctFlag;

    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    /* ��Ҳ��Զ����� */
    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//opCurcode"));
    if(memcmp(sCurCode, "CNY", 3) != 0) {
        BKINFO("��������Զ�����");
        return 0;
    }
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//opNotetype"));
    sprintf(sSqlStr, "select truncflag from noteinfo \
            where nodeid = %d and notetype = '%s' and dcflag = '%s'", 
            OP_REGIONID, sNoteType, sDCFlag);
    ret = db_query_str(sTruncFlag, sizeof(sTruncFlag), sSqlStr);
    if(ret) {
        BKINFO("��ѯƱ�ݽ�������ʧ��");
        return ret;
    }

    //�ǽ���Ʊ�ݲ�ʵʱ����
    if(atoi(sTruncFlag) == 0)
        return 0;

    if(atoi(sNoteType) == 41)
    {
        // ��˰���͹����Ľ�˰����
        if ((ret = ChkAgreement(XMLGetNodeVal(opDoc, "//opAgreement"))) != 0)
            return ret;
    }

#if 1
    if(atoi(sNoteType) == 31)
    {
        XMLSetNodeVal(opDoc, "//opEXBKTxflag", "3");
        //return E_SYS_COMM;
    }
#endif

    if(sDCFlag[0] == '1')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opPayacct"));
    else if(sDCFlag[0] == '2')
        strcpy(sAcctNo, XMLGetNodeVal(opDoc, "//opBeneacct"));
    else {
        BKINFO("�����־����ȷ");
        return E_APP_ACCOUNTFAIL;
    }
    if(sAcctNo[0] == '*' && sDCFlag[0] == '1') {
        sprintf(sAcctNo, "%s", sAcctNo+1);
        XMLSetNodeVal(opDoc, "//opPayacct", sAcctNo);
    }
    else if(sAcctNo[0] == '*' && sDCFlag[0] == '2') {
        sprintf(sAcctNo, "%s", sAcctNo+1);
        XMLSetNodeVal(opDoc, "//opBeneacct", sAcctNo);
    }

    /* �����ڽӿڽ��м�� */
#if 0
    strcpy(sAcctCheck, XMLGetNodeVal(opDoc, "//opAcctCheck"));
    /* ����� �ж��Ƿ����黧�� */
    if(atoi(sAcctCheck) == 1 && sDCFlag[0] == '2')
    {
        strcpy(sTAcctName, XMLGetNodeVal(opDoc, "//opBenename"));
        ret = callInterface(9202, opDoc);   //9202ͬ8202
        if(ret) {
            BKINFO("��ѯ�˻���Ϣʧ��");
            return E_APP_ACCOUNTFAIL;
        }
        strcpy(sAcctState, XMLGetNodeVal(opDoc, "//opOreserved5"));
        if(atoi(sAcctState) == 1) {
            BKINFO("�˻�״̬������");
            return E_APP_ACCOUNTFAIL;
        }
        strcpy(sBAcctName, XMLGetNodeVal(opDoc, "//opOreserved3"));
        if(strcmp(sTAcctName, sBAcctName) != 0) {
            BKINFO("������һ��:����[%s] ����[%s]", sBAcctName, sTAcctName);
            return E_APP_ACCOUNTFAIL;
        }
    }
#endif
    ret = db_query_str(sOrgId, sizeof(sOrgId), "select bankid from bankinfo "
            "where exchno = '%s'", XMLGetNodeVal(opDoc, "//opAcceptor"));
    if (ret != 0)
    {
        BKINFO("�к�[%s]ת�������ڻ���ʧ��", XMLGetNodeVal(opDoc, "//opAcceptor"));
        return E_DB;
    }

    XMLSetNodeVal(opDoc, "//opInnerBank", sOrgId);
    iAcctFlag = IsAcctType(sAcctNo);
    ret = db_exec("update trnjour set resflag1 = '%d',innerorganid='%s' "
            "where nodeid = %d and workdate = '%s' "
            " and refid = '%s' and inoutflag = '%s' and originator = '%s'",
            iAcctFlag, sOrgId, OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//opRefid"),
            XMLGetNodeVal(opDoc, "//opInoutflag"),
            XMLGetNodeVal(opDoc, "//opOriginator"));
    if(ret) {
        BKINFO("�����ʺű�־ʧ��");
        return ret;
    }
    // �����˻�
    if (iAcctFlag == 1 || iAcctFlag == 2)
    {
        if (atoi(sDCFlag) == 1)
        {
            BKINFO("��˽�ʺ�û�������ҵ��");
            ret = E_APP_ACCOUNTFAIL;
        }
        else
        {
            // ��ѯ���������ͱȶԻ���
            XMLSetNodeVal(opDoc, "//opAcctNo", sAcctNo);
            ret = callInterface(8202, opDoc);
            if(ret)
                return ret;
            strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
            if(atoi(sResult) != 0)
                return atoi(sResult);
            strcpy(sOrgId, XMLGetNodeVal(opDoc, "//opAcctOrgid"));
            XMLSetNodeVal(opDoc, "//opInnerBank", sOrgId);

            BKINFO("�����˻����˵�8110�ӿ�, orgid=[%s]...", sOrgId);
            strcpy(sFlCode, "0000"); //??
            ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
        }
    }
    else
    {
        XMLSetNodeVal(opDoc, "//opInoutflag", "2");
        strcpy(sFlCode, "0000");
        if(sDCFlag[0] == '1')
            ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8120");
        else if(sDCFlag[0] == '2')
        {
            /* ����տ��˺�Ϊ�ڲ���, ���5130. acctlen=28 */
            if (strlen(sAcctNo) == 28) {
                XMLSetNodeVal(opDoc, "//opEXBKTxflag", "0");
                ret = AcctToBank(opDoc, sAcctNo, sFlCode, "5130");
            }
            else
                ret = AcctToBank(opDoc, sAcctNo, sFlCode, "8110");
        }
        else
            ret = E_APP_ACCOUNTFAIL;
    }

    return ret;
}

/*
 * �˻���ѯ
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_401(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sResult[6+1]={0};
    char sOpenUnit[12+1]={0}, sOpenBank[12+1]={0};
    char sSqlStr[1024]={0};
    int iAcctFlag;

    ret = callInterface(8202, opDoc);
    if(ret)
        return ret;

    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
        return atoi(sResult);

    strcpy(sOpenUnit, XMLGetNodeVal(opDoc, "//opAcctOrgid"));
    iAcctFlag = IsAcctType(XMLGetNodeVal(doc, "//opAcctNo"));
    if (iAcctFlag < 0)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "�˻����ͼ�����");
        return 8999;
    }
    if (iAcctFlag != 1 && iAcctFlag != 2)
    {
        ret = db_query_str(sOpenBank, sizeof(sOpenBank), 
                "select exchno from bankinfo where bankid = '%s'", sOpenUnit);
        if (ret != 0)
            return E_DB;
    }
    XMLSetNodeVal(opDoc, "//opBankid", sOpenBank);

    return 0;
}

/*
 * ����
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_106(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sOutBank[12+1]={0}, sWorkDate[8+1]={0};
    char sInoutflag[1+1]={0}, sRefid[32+1]={0};

    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));

    ret = Acct_Bj(opDoc, sWorkDate, sOutBank, sRefid, sInoutflag, 0);

    // ������׷��ز���Ա����
    if (sInoutflag[0] == '1')
        SetOperInfo(opDoc);

    return ret;
}

/*
 * ������ȡ��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_107(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sRefid[16+1]={0}, sInoutflag[1+1]={0}, sOutBank[12+1]={0}, sWorkDate[8+1]={0};
    char sBankSerial[8+1]={0}, sOpSerial[20+1]={0};
    char sSqlStr[1024]={0}, sResult[6+1]={0};
    char sTab[20+1]={0}, sAcctFlag[1+1]={0}, sAcctResult[1+1]={0};
    char sDCFlag[1+1]={0};
    long lTmp = 0L;
    int trncode;

    // ���ز���Ա����
    SetOperInfo(opDoc);

    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));

    if(OP_TCTCODE == 10) {
        strcpy(sTab, "hacctjour");
        strcpy(sAcctFlag, "1");
        strcpy(sAcctResult, "2");
    }
    else if(OP_TCTCODE == 12) {
        strcpy(sTab, "acctjour");
        strcpy(sAcctFlag, "0");
        strcpy(sAcctResult, "3");
    }
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(opDoc, "//opOreserved2", sOpSerial);

    sprintf(sSqlStr, "select acctserial from %s \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and inoutflag = '%s' and refid = '%s'",
            sTab, OP_REGIONID, sWorkDate, sOutBank, sInoutflag, sRefid);
    ret = db_query_str(sBankSerial, sizeof(sBankSerial), sSqlStr);
    if(ret)
        return ret;
    XMLSetNodeVal(opDoc, "//opSeqno", sBankSerial);
    XMLSetNodeVal(opDoc, "//opReserved", sAcctFlag);

    BKINFO("�����־:%s ������־:%s", sDCFlag, sInoutflag);
    if((atoi(sDCFlag) == 1 && atoi(sInoutflag) == 1) ||
            (atoi(sDCFlag) == 2 && atoi(sInoutflag) == 2))
        trncode = 8111;
    else if((atoi(sDCFlag) == 2 && atoi(sInoutflag) == 1) ||
            (atoi(sDCFlag) == 1 && atoi(sInoutflag) == 2))
        trncode = 8121;
    ret = callInterface(trncode, opDoc);
    if(ret)
        return ret;
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
        return atoi(sResult);
    ret = db_exec("update %s set result = '%s' \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and inoutflag = '%s' and refid = '%s'",
            sTab, sAcctResult,
            OP_REGIONID, sWorkDate, sOutBank, sInoutflag, sRefid);
    return 0;
}

/*
 * ���ڽ��ײ�ѯ
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_402(void *doc, char *p)
{
    int ret=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sResult[6+1]={0}, sRet[1+1]={0};
    char sRefid[16+1]={0}, sOutBank[12+1]={0}, sInoutflag[1+1]={0};
    char sWorkDate[8+1]={0}, sDCFlag[1+1]={0};
    char sSqlStr[1024]={0};

    strcpy(sRefid, XMLGetNodeVal(opDoc, "//opRefid"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//opDcflag"));
    strcpy(sInoutflag, XMLGetNodeVal(opDoc, "//opInoutflag"));
#if 0
    /* ����������� �����Ϊ�տ��� */
    if((sDCFlag[0] == '1' && sInoutflag[0] == '1') 
            || (sDCFlag[0] == '2' && sInoutflag[0] == '2'))
        strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opBenebank"));
    /* ����������� �����Ϊ������ */
    else if((sDCFlag[0] == '1' && sInoutflag[0] == '2') 
            || (sDCFlag[0] == '2' && sInoutflag[0] == '1'))
        strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opPaybank"));
#endif
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));
    strcpy(sWorkDate, XMLGetNodeVal(opDoc, "//opWorkdate"));

    ret = CallInBank8201(opDoc, sWorkDate, sOutBank, sRefid, sInoutflag);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(strlen(sResult) == 0 || atoi(sResult) != 0)
    {
        BKINFO("�����ڲ�ѯ����״̬ʧ��");
        return ret;
    }
    ret = atoi(XMLGetNodeVal(opDoc, "//opTreserved1"));
    switch(ret)
    {
        case 1:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "���׳ɹ�" );
            break;
        case 0:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "����ʧ��" );
            break;
        case 2:
        default:
            XMLSetNodeVal( opDoc, "//opBKRetinfo", "���׽������ȷ" );
            break;
    }

    db_exec("update acctjour set result = '%d' where nodeid = %d and workdate = '%s' \
            and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            ret, OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);

    return ret;
}

int PF10_702(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    int ret;

    /* ģ�ⷵ��
       XMLSetNodeVal(opDoc, "//opOreserved1", "1"); // ��¼��
       XMLSetNodeVal(opDoc, "//opOreserved2", "20100622|320501061|401|100500604890010001|�½�|123|�տ��|290020|1234.00|û����;|");//����
       return 0;
     */
    ret = callInterface(8204, doc);

    return ret;
}

int CallInBank8201(xmlDoc *doc, char *pDate, char *pBankNo, char *pSerial, char *pInoutflag)
{
    int ret=0;
    char sAcctSerial[12+1]={0}, sOpSerial[20+1]={0};
    char sSqlStr[1024]={0};
    long lTmp=0L;

    BKINFO("�����ڲ�ѯ����״̬...");
    sprintf(sSqlStr, "select acctserial from acctjour \
            where nodeid = %d and workdate = '%s' and originator = '%s' \
            and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBankNo, pSerial, pInoutflag);

    ret = db_query_str(sAcctSerial, sizeof(sAcctSerial), sSqlStr);
    if (ret) {
        BKINFO("��ѯ������ˮʧ��");
        return ret;
    }
    XMLSetNodeVal(doc, "//opTermno", sAcctSerial);
    /*
       else if (ret == E_DB_NORECORD)
       {
    // δ�ҵ����ڼ�����ˮ��,�򷵻�δ����
    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    ret = db_exec("insert into acctjour values(%d, '%08ld', '%s', '%s',"
    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
    "'%s', '%s', '%s')",
    OP_REGIONID, current_date(), XMLGetNodeVal(doc, "//opOriginator"), 
    XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opInoutflag"), 
    "8201", "", "", XMLGetNodeVal(doc, "//opInnerBank"), 
    XMLGetNodeVal(doc, "//opOperid"), "", "0", "0", sOpSerial, "", "", "");
    if (ret != 0)
    return ret;
    XMLSetNodeVal(doc, "//opBKRetcode", "0");
    XMLSetNodeVal(doc, "//opTreserved1", "999");
    XMLSetNodeVal(doc, "//opTreserved2", "��" );
    XMLSetNodeVal(doc, "//opBKRetinfo", "δ����");
    return 0;
    }
     */

    lTmp = GenSerial("account", 1, 999999999, 1);
    sprintf(sOpSerial, "%ld", lTmp);
    XMLSetNodeVal(doc, "//opOreserved2", sOpSerial);

    ret = callInterface(8201, doc);
    if(ret)
        BKINFO("�����ڲ�ѯ����״̬ʧ��");

    return ret;
}

/*
 * ҵ����ˮ������ˮ�ȶ�
 */
int PF10_901(void *doc)
{
    int ret=0,i;
    xmlDoc *opDoc = (xmlDoc *)doc;
    xmlDoc *jzDoc;
    result_set yw_rs, fee_rs, jz_rs;
    char settledate[16] = {0};
    char sOriginator[13], sWorkDate[8+1]={0}, sClassid[1+1]={0};
    char sOutBank[12+1]={0}, sRefid[20+1]={0}, sInoutflag[1+1]={0};
    char sInnerBank[12+1]={0}, sAcctOper[12+1]={0};
    char filename[256];
    char condi[512];
    int clearround = 0;

    XmlGetString(opDoc, "//opCleardate", sWorkDate, sizeof(sWorkDate));
    XmlGetString(opDoc, "//opClassid", sClassid, sizeof(sClassid));
    XmlGetString(opDoc, "//opInnerBank", sInnerBank, sizeof(sInnerBank));
    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opOperid", sAcctOper, sizeof(sAcctOper));
    clearround = XmlGetInteger(doc, "//opClearround");
    strcpy(settledate, GetSettledDateround());

    BKINFO("����:%d ����:%d ��ȡ�������ڳ���:%s ϵͳ����:%s",
            sClassid, clearround, settledate, GetWorkdate());
    settledate[8] = 0;

    //�������һ��δ����ʱ���˳�����ǰһ����󳡴�,�������Ϲ鵵ʱ�����˳�����0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, sWorkDate) != 0)
    {
        BKINFO("[%d]>[%d] ? , settldate[%s]!=currdate[%s] ?",
                clearround, atoi(settledate+9), settledate, sWorkDate);
        XMLSetNodeVal(doc, "//opTCRetinfo", "δ����,�������ӡ�����嵥");
        return 0;
    }

    /* �����Ϊ�����з���, �����ɶ��˱��� */
    if (strcmp(GetCBankno(), sOriginator))
        goto gen_report;

    /* �ȶ�ҵ����ˮ */
    ret = db_query(&yw_rs, "select refid, inoutflag, originator, dcflag, "
            "clearstate from trnjour where nodeid=%d and workdate='%s' and"
            " classid=%s and clearstate='C'", OP_REGIONID, sWorkDate, sClassid);
    for (i = 0; i < db_row_count(&yw_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&yw_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&yw_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&yw_rs, i, "originator"));
        ret = db_query(&jz_rs, "select result, acctserial from acctjour \
                where nodeid = %d and workdate = '%s' and originator = '%s' \
                and refid = '%s' and inoutflag = '%s'",
                OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
        /* reserved3Ϊ1�ѱȶ� */
        if(db_row_count(&jz_rs) == 0) {
            ret = db_exec("insert into acctjour values(%d, '%s', '%s', '%s',"
                    "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
                    "'%s', '%s', '%s', '%s')",
                    OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag, "",
                    "", "", sInnerBank, sAcctOper, "", "0", "0", "", "", "1", "");
        }
        else {
            ret = db_exec("update acctjour set reserved3 = '1' "
                    "where nodeid = %d and workdate = '%s' and originator='%s'"
                    " and refid = '%s' and inoutflag = '%s'",
                    OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
        }
        if(ret)
            BKINFO("���¼�¼[�����:%s �����ˮ:%s]ʧ��", sOutBank, sRefid);
        db_free_result(&jz_rs);
    }
    db_free_result(&yw_rs);

    /* �Լ��ʲ��ɹ���δ���ʵĽ��׽��в��� */
    jzDoc = getOPTemplateDoc(100);
    ret = db_query(&fee_rs, "select * from acctjour where nodeid = %d and "
            "workdate='%s' and (reserved3='1' or inoutflag in('3', '4')) "
            "and result!='1'", OP_REGIONID, sWorkDate);
    for (i = 0; i < db_row_count(&fee_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&fee_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&fee_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&fee_rs, i, "originator"));
        ret = Acct_Bj(jzDoc, sWorkDate, sOutBank, sRefid, sInoutflag, 0);
        /* ���ǳɹ�������Ϊ�������ѱȶ�(reserved3 ���� 2) 
           if(ret == E_APP_ACCOUNTSUCC)
           db_exec("update acctjour set reserved3 = '2' where nodeid = %d and "
           "workdate = '%s' and originator = '%s' and refid = '%s' and "
           "inoutflag = '%s'",
           OP_REGIONID, sWorkDate, sOutBank, sRefid, sInoutflag);
         */
    }
    db_free_result(&fee_rs);
    /* �Լ��ʳɹ��Ľ��׽���ȡ�� */
    ret = db_query(&fee_rs, "select * from acctjour where nodeid = %d and "
            "workdate = '%s' and reserved3 not in('1', '2') and inoutflag not in('3', '4') "
            " and result = '1'",
            OP_REGIONID, sWorkDate);
    for (i = 0; i < db_row_count(&fee_rs); i++)
    {
        memset(sRefid, 0, sizeof sRefid);
        memset(sInoutflag, 0, sizeof sInoutflag);
        memset(sOutBank, 0, sizeof sOutBank);
        strcpy(sRefid, db_cell_by_name(&fee_rs, i, "refid"));
        strcpy(sInoutflag, db_cell_by_name(&fee_rs, i, "inoutflag"));
        strcpy(sOutBank, db_cell_by_name(&fee_rs, i, "originator"));
        ret = Acct_Qx(jzDoc, sWorkDate, sOutBank, sRefid, sInoutflag);
    }
    db_free_result(&fee_rs);

gen_report:
    memset(filename, 0, sizeof(filename));
    BKINFO("Ready PrintSettleList()...");
    if ((ret = PrintSettleList(opDoc, filename)) == 0)
        XMLSetNodeVal(opDoc, "//opFilenames", filename);
    else
        BKINFO("PrintSettlList() ret=[%d]", ret);

    return 0;
}

//����ȡ��
int Acct_Qx(xmlDoc *opDoc, char *pDate, char *pBank, char *pSerial, char *pInoutflag)
{
    int ret=0;
    char sResult[6+1]={0};
    char sBankSerial[8+1]={0}, sAmount[15+1]={0};
    char sSqlStr[1024]={0};

    BKINFO("��ȡ����¼����[%s] �����[%s] ��ˮ��[%s] �����־[%s]", 
            pDate, pBank, pSerial, pInoutflag);

    sprintf(sSqlStr, "select acctserial from acctjour "
            "where nodeid = %d and workdate = '%s' and "
            "originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_str(sBankSerial, sizeof(sBankSerial), sSqlStr);
    if(ret) {
        BKINFO("ȡ�����ʲ�ѯԭ������ˮʧ��");
        return ret;
    }

    sprintf(sSqlStr, "select settlamt from trnjour "
            "where nodeid = %d and workdate = '%s' and refid = '%s' "
            "and inoutflag = '%s' and originator = '%s'",
            OP_REGIONID, pDate, pSerial, pInoutflag, pBank);
    ret = db_query_str(sAmount, sizeof(sAmount), sSqlStr);
    if(ret) {
        BKINFO("ȡ�����ʲ�ѯԭ����ʧ��");
        return ret;
    }
    XMLSetNodeVal(opDoc, "//opHreserved2", sBankSerial);
    XMLSetNodeVal(opDoc, "//opHreserved3", sAmount);
    XMLSetNodeVal(opDoc, "//opHreserved4", "0");

    ret = callInterface(7111, opDoc);
    if(ret) {
        BKINFO("ȡ������ʧ��");
        return ret;
    }
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0) {
        BKINFO("ȡ���������ڷ���ʧ��");
        return atoi(sResult);
    }
    db_exec("update acctjour set result = '%s' "
            "where where nodeid = %d and workdate = '%s' and "
            "originator = '%s' and refid = '%s' and inoutflag = '%s'",
            "3", OP_REGIONID, pDate, pBank, pSerial, pInoutflag);

    return ret;
}

//����   iFlag:0���� 1��ǰ������ 2��ʷ������
int Acct_Bj(xmlDoc *doc, char *pDate, char *pBank, char *pSerial, char *pInoutflag, int iFlag)
{
    int ret=0;
    //xmlDoc *doc = NULL;
    char sInnerBank[13];
    char sNoteType[2+1]={0}, sTruncFlag[1+1]={0}, sDCFlag[1+1]={0};
    char sTab[20+1]={0}, sBKTrnCode[6+1]={0}, sTxType[4+1]={0};
    char sAcctNo[32+1]={0};
    char sStr[1024]={0};

    BKINFO("����[%s] �к�[%s] ��ˮ[%s] �����־[%s] �������ױ�־[%d]", pDate, pBank, pSerial, pInoutflag, iFlag);
    if(iFlag == 0 || iFlag == 1)
        strcpy(sTab, "trnjour");
    else                            //����
        strcpy(sTab, "htrnjour");
    sprintf(sStr, "nodeid = %d and workdate = '%s' and refid = '%s' and originator = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pSerial, pBank, pInoutflag);
    ret = QueryTable(doc, sTab, sStr);
    if (ret)
        return ret;
    XmlGetString(doc, "//opInnerorganid", sInnerBank, sizeof(sInnerBank));
    if (sInnerBank[0] == 0x00)
    {
        BKINFO("ȡ���ڼ��˻����ų���");
        return E_OTHER;
    }
    XMLSetNodeVal(doc, "//opInnerBank", sInnerBank);

    strcpy(sTruncFlag, XMLGetNodeVal(doc, "//opTruncflag"));
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//opDcflag"));
    //����� �ǽ���
    if(sDCFlag[0] == '1' && sTruncFlag[0] == '0' && pInoutflag[0] == '1') {
        if(iFlag == 0)
            strcpy(sBKTrnCode, "7130");
        //����
        else {
            strcpy(sBKTrnCode, "7110");
            strcpy(sTxType, "0001");
        }
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //����� ����
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '1' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //����� �ǽ���
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '0' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //����� ����
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '1' && pInoutflag[0] == '1') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //����� �ǽ���
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '0' && pInoutflag[0] == '2')
        return 0;
    //����� ����
    else if(sDCFlag[0] == '1' && sTruncFlag[0] == '1' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7120");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opPayacct"));
    }
    //����� �ǽ���
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '0' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    //����� ����
    else if(sDCFlag[0] == '2' && sTruncFlag[0] == '1' && pInoutflag[0] == '2') {
        strcpy(sBKTrnCode, "7110");
        strcpy(sTxType, "0000");
        strcpy(sAcctNo, XMLGetNodeVal(doc, "//opBeneacct"));
    }
    else {
        BKINFO("�����־[%s] ������־[%s] ������־[%s] �������", sDCFlag, sTruncFlag, pInoutflag);
        return 0;
    }

    // ��˽�˻�������
    if (IsAcctType(sAcctNo) != 0)
        return 0;

    //���ײ���Ҫȥ���ڼ��״̬
    if(strcmp(sTxType, "0001") == 0)
        goto ACCT;
    /* ��ѯ���ڼ���״̬ */
    ret = CallInBank8201(doc, pDate, pBank, pSerial, pInoutflag);
    if(ret)
        return ret;

    if(XmlGetInteger(doc, "//opBKRetcode") == 0 
            && XmlGetInteger(doc, "//opTreserved1") == 0) {
        db_exec("update acctjour set result='1', acctserial='%s', trncode='%s'"
                " where nodeid = %d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='%s'",
                XMLGetNodeVal(doc, "//opTreserved2"), sBKTrnCode,
                OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
        return 0;
    }

ACCT:
    ret = AcctToBank(doc, sAcctNo, sTxType, sBKTrnCode);

    return ret;
}

/* 
 * ����ȡ�� ��ѯ���ʽ�����ˮ 
 */
int AcctCancelQry(char *pDate, char *pBank, char *pSerial, char *pInoutflag, char *pBankSerial, char *pAcctDate)
{
    int ret=0;
    char sSqlStr[1024]={0};
    char sResult[1+1]={0}, sBankSerial[8+1]={0}, sAcctDate[8+1]={0};

    sprintf(sSqlStr, "select result from acctjour where nodeid = %d and workdate = '%s'"
            " and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_str(sResult, sizeof(sResult), sSqlStr);
    if(ret)
    {
        BKINFO("��ѯ���ʽ��ʧ��");
        return E_DB;
    }
    if(atoi(sResult) != 1)
    {
        BKINFO("�˽����������");
        return 0;
    }
    /* reserved2 ����Ϊ���ڼ������� */
    sprintf(sSqlStr, "select acctserial, reserved2 from acctjour where nodeid = %d and workdate = '%s'"
            " and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, pDate, pBank, pSerial, pInoutflag);
    ret = db_query_strs(sSqlStr, sBankSerial, sAcctDate);
    if(ret)
    {
        BKINFO("��ѯ���ڼ�����ˮʧ��");
        return E_DB;
    }
    strcpy(pBankSerial, sBankSerial);
    strcpy(pAcctDate, sAcctDate);

    return 0;
}

/* ��������� */
int PF10_122(void *doc, char *p)
{
    int ret=0,i,iFlag;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char filename[256];
    result_set yw_rs, tp_rs;
    char sClassid[1+1]={0}, sOutBank[12+1]={0};
    char sSTDate[8+1]={0}, sSTRound[1+1]={0}, sDQRound[1+1]={0};
    char sDQ_JHDate[8+1]={0}, sDQ_JHRound[1+1]={0};
    char sSysDate[8+1]={0}, sSysState[1+1]={0};
    char sGDDate[8+1]={0}, sTab[20+1]={0}, sTab1[20+1]={0};
    char sORefid[20+1]={0}, sOWorkdate[8+1]={0}, sTmp[64]={0};
    char sOOutbank[12+1];

    strcpy(sSTDate, XMLGetNodeVal(opDoc, "//opCleardate"));     //��������
    strcpy(sSTRound, XMLGetNodeVal(opDoc, "//opClearround"));
    //strcpy(sClassid, XMLGetNodeVal(opDoc, "//opClassid"));
    strcpy(sOutBank, XMLGetNodeVal(opDoc, "//opOriginator"));

    strcpy(sDQ_JHDate, GetExchgdate());
    strcpy(sDQ_JHRound, GetExchground());
    strcpy(sSysDate, GetWorkdate());
    strcpy(sSysState, GetSysStat());
    strcpy(sGDDate, GetArchivedate());
    strcpy(sDQRound, GetRound());

    /* �������ڴ���ϵͳ���ڲ������� */
    if(atol(sSTDate) > atol(sSysDate)) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "�������ڻ򳡴β���ȷ");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* �������ڵ��ڵ�ǰ�������� �������׳��δ��ڵ��ڽ������β������� */
    else if(atol(sSTDate) == atol(sDQ_JHDate) 
            && atoi(sSTRound) >= atoi(sDQ_JHRound)) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "�������ڻ򳡴β���ȷ");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* �������ڵ��ڵ�ǰ�������� ���� �������δ������׳���1�� ���� ϵͳΪ����״̬�������� */
    else if(atol(sSTDate) == atol(sDQ_JHDate) && (atoi(sDQ_JHRound) - atoi(sSTRound) == 1) 
            && atoi(sSysState) == 1) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "�������ڻ򳡴β���ȷ");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    /* ��������С�ڵ�ǰ�������� ���� ��ǰ��������Ϊ1 ���� ϵͳΪ����״̬�������� */
    /* chenjie �ĳ��������ڣ���ǰ�������� ??? */
    else if(atol(sSTDate) == atol(sDQ_JHDate) && atoi(sDQ_JHRound) == 1
            && atoi(sSysState) == 1) {
        XMLSetNodeVal(opDoc, "//opTCRetinfo", "�������ڻ򳡴β���ȷ");
        XMLSetNodeVal(opDoc, "//opTCRetcode", "999");
        BKINFO("fail");
        return 999;
    }
    if(atol(sSTDate) <= atol(sGDDate)) {
        strcpy(sTab, "htrnjour");
        iFlag = 2;
    }
    else{
        strcpy(sTab, "trnjour");
        iFlag = 1;
    }

    /* ���ݵ�����Ʊ��Ϣ��ԭ���������Ʊ��־ */
    ret = db_query(&tp_rs, "select acceptor, agreement from trnjour where "
            "nodeid = %d and classid = %s and workdate = '%s' and "
            "inoutflag = '%s' and trncode = '%s'",
            OP_REGIONID, "1", sSTDate, "2", "7");
    if (db_row_count(&tp_rs) == 0)
        goto NEXT;
    for (i = 0; i < db_row_count(&tp_rs); i++)
    {
        memset(sTmp, 0, sizeof sTmp);
        memset(sOOutbank, 0, sizeof sOOutbank);
        memset(sOWorkdate, 0, sizeof sOWorkdate);
        memset(sORefid, 0, sizeof sORefid);
        strcpy(sTmp, db_cell_by_name(&tp_rs, i, "agreement"));
        strcpy(sOOutbank, db_cell_by_name(&tp_rs, i, "acceptor"));
        memcpy(sOWorkdate, sTmp, 8);
        strcpy(sORefid, sTmp+8+1);
        /* tpflag=1��Ʊ */
        db_exec("update %s set tpflag = '%s' where nodeid = %d and classid = %s "
                "and originator = '%s' and refid = '%s' and workdate = '%s' and inoutflag = '%s'",
                sTab, "1", OP_REGIONID, "1", sOOutbank, sORefid, sOWorkdate, "1");
    }
    db_free_result(&tp_rs);

NEXT:
    ret = db_query(&yw_rs, "select * from %s where "
            "nodeid = %d and classid = %s and clearstate = '%s' and truncflag = '%s' "
            "and dcflag = '%s' and inoutflag = '%s' and exchgdate = '%s' and exchground = '%s' "
            "and originator = '%s' and notetype not in('%s', '%s') and tpflag = '0' and stflag='0'", //"and stflag='0'", add by chenjie
            sTab, OP_REGIONID, "1", "C", "0", "1", "1", 
            sSTDate, sSTRound, sOutBank, "03", "04");
    if (db_row_count(&yw_rs) == 0) {
        BKINFO("ҵ����ˮû���ҵ�");
        return E_DB_NORECORD;
    }
    for (i = 0; i < db_row_count(&yw_rs); i++)
    {
        ret = Acct_Bj(opDoc, db_cell_by_name(&yw_rs, i, "workdate"), sOutBank, 
                db_cell_by_name(&yw_rs, i, "refid"), "1", iFlag);
        /* stflag=1���� */
        if(ret == E_APP_ACCOUNTSUCC)
            db_exec("update %s set stflag = '%s' where nodeid = %d and classid = %s and inoutflag = '%s'"
                    " and originator = '%s' and refid = '%s' and workdate = '%s'",
                    sTab, "1", OP_REGIONID, "1", "1", sOutBank,
                    db_cell_by_name(&yw_rs, i, "refid"), db_cell_by_name(&yw_rs, i, "workdate"));
    }
    db_free_result(&yw_rs);

    memset(filename, 0, sizeof(filename));
    BKINFO("Ready PrintAcceptNoteList()...");
    if ((ret = PrintAcceptNoteList(opDoc, filename)) == 0)
        XMLSetNodeVal(opDoc, "//opFilenames", filename);
    else
        BKINFO("PrintAcceptNoteList() ret=[%d]", ret);
    return ret;
}

/*
 * ������ȡ������
 */
int PF10_153(void *doc, char *p)
{
    /* iFlag=1�������Ѵ���ʧ�� iFlag=0��������ȫ���ɹ� */
    int ret=0,i=0,iFlag=0;
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char filename[256];
    char *pJzDate=NULL, *pJgh=NULL, sSxfAcct[32+1]={0};
    char *pKhAcct=NULL, sQsDate[8+1]={0}, *pJdbz=NULL;
    char *pWorkDate=NULL, *pOutBank=NULL, *pRefid=NULL;
    char *pSysDate=NULL, *pInoutflag=NULL, *pJhh=NULL;
    char sSqlStr[1024]={0}, sSxf[15+1]={0}, retstr[1+1]={0};
    char sXyh[60+1]={0}, sRemark[60+1]={0};
    char sTmp[64]={0}, sAcctName[60+1]={0};

    pJzDate = XMLGetNodeVal(opDoc, "//opWorkdate");
    pJgh = XMLGetNodeVal(opDoc, "//opInnerBank");
    pJhh = XMLGetNodeVal(opDoc, "//opOriginator");
    pSysDate = GetWorkdate();

    strcpy(sRemark, "���׳ɹ�");
    /* reserved �ֶδ�Ŵ˻����ϴ���ȡ�����ѵ�������� */
    /* reserved2�ֶδ�Ŵ˻����������ʺ� */
    sprintf(sSqlStr, "select reserved, reserved2 from bankinfo where bankid = '%s'", pJgh);
    ret = db_query_strs(sSqlStr, sQsDate, sSxfAcct);
    if(ret) {
        strcpy(sRemark, "��ѯ�����������ʺ�ʧ��");
        goto END;
    }
    XMLSetNodeVal(opDoc, "//opBeneacct", sSxfAcct);
    /* feeflag=0û����ȡ������ resflag1=0�ǶԹ��ʺ� */
    sprintf(sSqlStr, "select * from htrnjour where nodeid = %d and workdate > '%s' and workdate <= '%s'"
            " and clearstate = 'C' and inoutflag = '1' and ((notetype in(select notetype from noteinfo "
            " where nodeid = %d and feepayer = '1') and dcflag = '2') or (notetype in(select notetype "
            " from noteinfo where nodeid = %d and feepayer = '2') and dcflag = '1')) and feeflag = '0' "
            " and originator = '%s' and resflag1 = '0'",
            OP_REGIONID, sQsDate, pJzDate, OP_REGIONID, OP_REGIONID, pJhh);
    ret = db_query(&rs, sSqlStr);
    if(ret == E_DB_NORECORD) {
        //strcpy(sRemark, "���������ѵ�ҵ����ˮ��Ϣ������");
        ret = 0;
        goto END;
    }
    else if(ret){
        strcpy(sRemark, "��ѯ���������ѵ�ҵ����ˮ��Ϣʧ��");
        goto END;
    }
    for (i = 0; i < db_row_count(&rs); i++)
    {
        /* ���ݽ����־��ȡ����ȡ�����ѵĿͻ��ʺ� */
        pJdbz = db_cell_by_name(&rs, i, "dcflag");
        if(atoi(pJdbz) == 1)
            pKhAcct = db_cell_by_name(&rs, i, "beneacct");
        else if(atoi(pJdbz) == 2)
            pKhAcct = db_cell_by_name(&rs, i, "payingacct");

        pWorkDate = db_cell_by_name(&rs, i, "workdate");
        pOutBank = db_cell_by_name(&rs, i, "originator");
        pRefid = db_cell_by_name(&rs, i, "refid");
        pInoutflag = db_cell_by_name(&rs, i, "inoutflag");
        /* ������ȡ�������� */
        ret = Fee_Cast(pKhAcct, sSxf);
        if(ret && ret != 1) {
            BKINFO("%s %s %s����������ʧ��", pWorkDate, pOutBank, pRefid);
            continue;
        }
        /* ����շ���Ϣ�Ƿ��Ѵ��� */
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist "
                " where nodeid = %d and workdate = '%s' and inoutflag = '%s' "
                " and originator = '%s' and refid = '%s'",
                OP_REGIONID, pWorkDate, pInoutflag, pOutBank, pRefid);
        if(ret == E_DB_NORECORD) {
            /* �����շ���Ϣ�������ѱ� */
            sprintf(sSqlStr, "insert into feelist values(%d, '%s', '%s', '%s', '%s',"
                    "'%s', '%s', '%s', %.2f, %d, '%s', '%s', '', '', '', '', '')",
                    OP_REGIONID, pWorkDate, pInoutflag,
                    pOutBank, pRefid, pSysDate, pKhAcct, "", atof(sSxf), 1, "0", "0");
            ret = db_exec(sSqlStr);
            if(ret) {
                BKINFO("%s %s %s�����շ���Ϣʧ��", pWorkDate, pOutBank, pRefid);
                continue;
            }
        }
        else if(ret) {
            BKINFO("%s %s %s��ѯ�շ���Ϣʧ��", pWorkDate, pOutBank, pRefid);
            continue;
        }
    }
    db_free_result(&rs);
    /* resflag1=0Ϊ��ϸ��������Ϣ resflag1=1Ϊ���ܺ���������Ϣ */
    ret = db_query(&rs, "select acctno, count(*) as bs, sum(amount) as je from feelist "
            " where nodeid = %d and originator = '%s' and feedate = '%s' and result = '0' "
            " and resflag1 = '0' group by acctno",
            OP_REGIONID, pJhh, pSysDate);
    /* ���������ѻ�����Ϣ������ */
    for (i = 0; i < db_row_count(&rs); i++)
    {
        /* ������Ϣ��ˮ����� f+��������+˳��� */
        sprintf(sTmp, "F%s%d", pJzDate, i);
        pKhAcct = db_cell_by_name(&rs, i, "acctno");
        memset(sSxf, 0, sizeof sSxf);
        strcpy(sSxf, db_cell_by_name(&rs, i, "je"));
        ret = db_query_str(retstr, sizeof(retstr), "select result from feelist where nodeid = %d "
                " and acctno = '%s' and workdate = '%s' and resflag1 = '1' and originator = '%s' ",
                OP_REGIONID, pKhAcct, pJzDate, pJhh);
        if(ret == E_DB_NORECORD) {
            /* ������Ϣ��������Ϊ�������� */
            sprintf(sSqlStr, "insert into feelist values(%d, '%s', '%s', '%s', '%s',"
                    "'%s', '%s', '%s', %.2f, %s, '%s', '%s', '', '', '', '', '')",
                    OP_REGIONID, pJzDate, "1",
                    pJhh, sTmp, pSysDate, pKhAcct, "", atof(sSxf),
                    db_cell_by_name(&rs, i, "bs"), "0", "1");
            ret = db_exec(sSqlStr);
            if(ret) {
                BKINFO("%s �����շѻ�����Ϣʧ��", pKhAcct);
                continue;
            }
        }
        else if(ret) {
            BKINFO("%s ��ѯ�շѻ�����Ϣʧ��", pKhAcct);
            continue;
        }

        if(atoi(retstr) == 1)   //���������շѳɹ��Ĳ����շ�
            continue;
        /* ��ѯ�ʺſ������ѵ�Э��� */
        ret = Qry_Agreement(pKhAcct, sXyh);
        if(ret) {
            iFlag = 1;  
            continue;
        }

        /* �����ʺŲ�ѯ���� */
        ret = db_query_str(sAcctName, sizeof(sAcctName), "select name from acctinfo where acctid = '%s'", pKhAcct);
        if(ret) {
            BKINFO("%s ��ѯ����ʧ��", pKhAcct);
            continue;
        }

        XMLSetNodeVal(opDoc, "//opTrdate", pSysDate); //ϵͳ��������
        XMLSetNodeVal(opDoc, "//opWorkdate", pJzDate);
        XMLSetNodeVal(opDoc, "//opRefid", sTmp);
        XMLSetNodeVal(opDoc, "//opOriginator", pJhh);
        XMLSetNodeVal(opDoc, "//opInnerBank", pJgh);
        XMLSetNodeVal(opDoc, "//opAgreement", sXyh);
        XMLSetNodeVal(opDoc, "//opPayacct", pKhAcct);
        XMLSetNodeVal(opDoc, "//opSettlamt", sSxf);
        XMLSetNodeVal(opDoc, "//opPayname", sAcctName);

        /* �����ڼ��� */
        ret = AcctToBank(opDoc, pKhAcct, "0002", "9130");
        if(ret != E_APP_ACCOUNTSUCC) 
            continue;
        /* ������������Ϣ���շѽ��result=1�շѳɹ� */
        ret = db_exec("update feelist set result = '1' where nodeid = %d and "
                " originator = '%s' and feedate = '%s' and acctno = '%s'",
                OP_REGIONID, pJhh, pSysDate, pKhAcct);
        if(ret)
            BKINFO("%s ���ڼ��ʳɹ�,���ظ���ʧ��", pKhAcct);

        /* ����ҵ����е��շѱ�־feeflag=1���������� */
        sprintf(sSqlStr, "update htrnjour set feeflag = '1' where nodeid = %d and workdate > '%s' and "
                " workdate <= '%s' and originator = '%s' and inoutflag = '1' and clearstate = 'C' and "
                " ((notetype in(select notetype from noteinfo where nodeid = %d and feepayer = '1') and dcflag = '2') "
                " or (notetype in(select notetype from noteinfo where nodeid = %d and feepayer = '2') and dcflag = '1')) "
                " and feeflag = '0' and resflag1 = '0' and ((dcflag = '1' and beneacct = '%s') or "
                " (dcflag = '2' and payingacct = '%s'))",
                OP_REGIONID, sQsDate, pJzDate, pJhh, OP_REGIONID, OP_REGIONID, pKhAcct, pKhAcct);
        ret = db_exec(sSqlStr);
        if(ret)
            BKINFO("%s ���ڼ��ʳɹ�,���½�����ˮ��ʧ��", pKhAcct);
    }
    db_free_result(&rs);
    /* ���´˻����շѵ��������Ҳ������һ���շѵ���ʼ���� */
    db_exec("update bankinfo set reserved = '%s' where bankid = '%s'", pJzDate, pJgh);
END:

    // ������������ȡ�������
    if (ret == 0)
    {
        memset(filename, 0, sizeof(filename));
        BKINFO("Ready PrintFeeTransInfo()...");
        if ((ret = PrintFeeTransInfo(opDoc, filename)) == 0)
            XMLSetNodeVal(opDoc, "//opFilenames", filename);
        else
        {
            sprintf(sRemark, "������������ȡ�������ʧ��!");
            BKINFO("PrintFeeTransInfo() ret=[%d]", ret);
        }
    }

    BKINFO(sRemark);
    /*
       sprintf(sTmp, "%04d", ret);
       XMLSetNodeVal(opDoc, "//opTCRetcode", sTmp);
       XMLSetNodeVal(opDoc, "//opTCRetinfo", sRemark);
     */

    return ret;
}

/* ��������ҵ���ļ� 123->154 */
int PF10_154(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char filename[256], cond[1024];
    char *opBankno, *pWorkDate, *pAcceptor;
    long lSerialno;
    double totamt;
    int totnum;
    FILE *fp;
    int rc, i;

    pWorkDate = XMLGetNodeVal(opDoc, "//opWorkdate");
    pAcceptor = XMLGetNodeVal(opDoc, "//opOriginator");
    opBankno = XMLGetNodeVal(opDoc, "//opInnerBank");

    sprintf(cond, "inoutflag='2' and acceptor='%s' and workdate='%s' and "
            "clearstate in('1','C') and dcflag='2' and resflag1 != '0'",
            pAcceptor, pWorkDate);

    rc = db_query(&rs, "select count(*), sum(settlamt) from trnjour"
            "where %s", cond);
    if (rc != 0)
        return rc;
    totnum = db_cell_i(&rs, 0, 0);
    totamt = db_cell_d(&rs, 0, 1);
    db_free_result(&rs);

    rc = db_query(&rs, "select beneacct,settlamt,fee,benename from trnjour "
            "where %s", cond);
    if (rc != 0)
        return rc;
    lSerialno = GenSerial("batch", 1, 999, 1);
    sprintf(filename, "%s/TC%s%06ld.TXT", getenv("FILES_DIR"), opBankno, 
            current_time());
    if ((fp = fopen(filename, "w")) == NULL)
    {
        db_free_result(&rs);
        return 999;
    }
    fprintf(fp, "%9s%06ld%03ld|%d|%.0lf|||||ͬ��ת��|\n", 
            opBankno, current_date()%1000000, lSerialno, totnum, totamt*100);
    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%d|%d|%s|%.0lf||%s|%s|||\n", 
                i+1, IsAcctType(db_cell(&rs, i, 0)), db_cell(&rs, i, 0), 
                db_cell_d(&rs, i, 1)*100, "10", db_cell(&rs, i, 3));
    }
    db_free_result(&rs);
    fclose(fp);

    return 0;
}

/* ��ӡ������ƾ֤ */
int PF10_762(void *doc, char *p)
{
    xmlDoc *opDoc = (xmlDoc *)doc;
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char sFeeDate[9];
    char amount[128], littl_amt[30];
    char acctserial[20], name[81];
    char printtime[30];
    char *chinesedate = NULL;
    char *acctno;
    FILE *fp;
    int i, rc;

    if (InitRptVar(opDoc) != 0)
        return E_OTHER;

    memset(sFeeDate, 0, sizeof(sFeeDate));
    strcpy(sFeeDate, XMLGetNodeVal(opDoc, "//opWorkdate"));

    chinesedate = ChineseDate(atol(sFeeDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/FeeNote.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.fn",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100, current_time());

    rc = db_query(&rs, "select * from feelist where nodeid=%d "
            "and inoutflag='1' and workdate='%s' and originator='%s' "
            "and result='1' and resflag1='1'",
            OP_REGIONID, sFeeDate, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "");

    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    for (i = 0; i < db_row_count(&rs); i++)
    {
        // ��ѯ������ˮ
        db_query_str(acctserial, sizeof(acctserial),
                "select acctserial from acctjour "
                "where nodeid=%d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='3'", 
                OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                gs_originator, db_cell_by_name(&rs, i, "refid"));

        // ��ѯ����
        acctno = db_cell_by_name(&rs, i, "acctno");
        db_query_str(name, sizeof(name),
                "select name from acctinfo where acctid='%s' and nodeid=%d",
                acctno, OP_REGIONID);

        memset(littl_amt, 0, sizeof(littl_amt));
        strcpy(littl_amt, FormatMoney(db_cell_by_name(&rs, i, "amount")));
        MoneyToChinese(db_cell_by_name(&rs, i, "amount"), amount);
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                sFeeDate,
                acctno,
                name, 
                db_cell_by_name(&rs, i, "number"),
                littl_amt, 
                amount,
                printtime,
                gs_oper,
                acctserial);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);
    WriteRptFooter(fp, "");
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }

    XMLSetNodeVal(opDoc, "//opFilenames", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

/*
   �����嵥
 */
int PrintAcceptNoteList(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char tbname[81];
    char printtime[30];
    char notetype_name[61];
    char *chinesedate = NULL;
    char sSTDate[9], sSTRound[3];
    char acctserial[20], sucbuf[40], failbuf[40];
    FILE *fp=NULL;
    int iSucSum, iFailSum;
    double dSucAmt, dFailAmt;
    int i, rc = 0;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    memset(sSTDate, 0, sizeof(sSTDate));
    strcpy(sSTDate, XMLGetNodeVal(xmlReq, "//opCleardate"));
    memset(sSTRound, 0, sizeof(sSTRound));
    strcpy(sSTRound, XMLGetNodeVal(xmlReq, "//opClearround"));
    if (DiffDate(sSTDate, GetSysPara("ARCHIVEDATE")) <= 0)
        strcpy(tbname, "htrnjour");
    else
        strcpy(tbname, "trnjour");

    chinesedate = ChineseDate(atol(sSTDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/STList.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.st",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

    rc = db_query(&rs, "select * from %s where nodeid=%d and classid=1 "
            "and clearstate='C' and truncflag='0' and dcflag='1' and "
            "inoutflag='1' and exchgdate='%s' and exchground='%s' and "
            "originator='%s' and notetype not in('03','04') and tpflag='0'",
            tbname, OP_REGIONID, sSTDate, sSTRound, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, sSTRound,
            gs_bankid, gs_bankname);

    iSucSum = iFailSum = 0L;
    dSucAmt = dFailAmt = 0.0;
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if (atoi(db_cell_by_name(&rs, i, "stflag")) == 1)
        {
            // ����
            iSucSum++;
            dSucAmt += atof(db_cell_by_name(&rs, i, "settlamt"));
            db_query_str(acctserial, sizeof(acctserial),
                    "select acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and refid='%s' and inoutflag='1'", 
                    OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                    gs_originator, db_cell_by_name(&rs, i, "refid"));
        }
        else
        {
            iFailSum++;
            dFailAmt += atof(db_cell_by_name(&rs, i, "settlamt"));
            sprintf(acctserial, "δ����");
        }
        db_query_str(notetype_name, sizeof(notetype_name),
                "select distinct name from noteinfo "
                "where notetype='%s'", db_cell_by_name(&rs, i, "notetype"));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "acceptor"), notetype_name, 
                db_cell_by_name(&rs, i, "noteno"),
                db_cell_by_name(&rs, i, "payingacct"),
                db_cell_by_name(&rs, i, "payer"),
                db_cell_by_name(&rs, i, "beneacct"),
                db_cell_by_name(&rs, i, "benename"),
                FormatMoney(db_cell_by_name(&rs, i, "settlamt")),
                acctserial);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, i);

    sprintf(sucbuf, "%.2lf", dSucAmt);
    sprintf(failbuf, "%.2lf", dFailAmt);
    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    WriteRptFooter(fp, "%d;%s;%d;%s;%s;%s;\n", iSucSum, FormatMoney(sucbuf),
            iFailSum, FormatMoney(failbuf), printtime, gs_oper);
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }
    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

int PrintFeeTransInfo(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caParaFile[256];
    char caDataFile[256];
    char caOutFile[256];
    char tbname[81];
    char printtime[30];
    char *chinesedate = NULL;
    char sFeeDate[9];
    char acctserial[20], sucbuf[40], failbuf[40];
    FILE *fp=NULL;
    int iSucSum, iFailSum;
    double dSucAmt, dFailAmt;
    int i, rc = 0;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    memset(sFeeDate, 0, sizeof(sFeeDate));
    strcpy(sFeeDate, XMLGetNodeVal(xmlReq, "//opWorkdate"));
    /*
       memset(sSTDate, 0, sizeof(sSTDate));
       strcpy(sSTDate, GetTrnCtl("ExchgDate"));
       if (DiffDate(sSTDate, GetSysPara("ARCHIVEDATE")) <= 0)
       strcpy(tbname, "htrnjour");
       else
       strcpy(tbname, "trnjour");
     */

    chinesedate = ChineseDate(atol(sFeeDate));
    snprintf(caParaFile, sizeof(caParaFile),
            "%s/dat/%d/FeeInfo.para", getenv("HOME"), TCOP_BANKID);

    snprintf(caOutFile, sizeof(caOutFile), "%s/%s_%02ld%06ld.fee",
            getenv("FILES_DIR"), gs_originator,
            current_date()%100,current_time());

    rc = db_query(&rs, "select * from feelist where nodeid=%d "
            "and inoutflag='1' and workdate='%s' and originator='%s' "
            "and resflag1='1'",
            OP_REGIONID, sFeeDate, gs_originator);
    if (rc != 0)
        return rc;

    GetTmpFileName(caDataFile);
    if ((fp = fopen(caDataFile, "w")) == NULL)
    {
        db_free_result(&rs);
        return E_OTHER;
    }

    WriteRptHeader(fp, "%s;%s;%s;%s;%s;", gs_sysname, chinesedate, 
            GetWorkdate(), gs_bankid, gs_bankname);

    iSucSum = iFailSum = 0L;
    dSucAmt = dFailAmt = 0.0;
    for (i = 0; i < db_row_count(&rs); i++)
    {
        if (atoi(db_cell_by_name(&rs, i, "result")) == 1)
        {
            // ��ȡ�ɹ�
            iSucSum++;
            dSucAmt += atof(db_cell_by_name(&rs, i, "amount"));
            db_query_str(acctserial, sizeof(acctserial),
                    "select acctserial from acctjour "
                    "where nodeid=%d and workdate='%s' and originator='%s' "
                    "and refid='%s' and inoutflag='3'", 
                    OP_REGIONID, db_cell_by_name(&rs, i, "workdate"),
                    gs_originator, db_cell_by_name(&rs, i, "refid"));
        }
        else
        {
            iFailSum++;
            dFailAmt += atof(db_cell_by_name(&rs, i, "amount"));
            sprintf(acctserial, "δ��ȡ");
        }

        fprintf(fp, "%s;%s;%s;%s;%s;\n", 
                db_cell_by_name(&rs, i, "refid"), 
                db_cell_by_name(&rs, i, "acctno"),
                db_cell_by_name(&rs, i, "number"),
                FormatMoney(db_cell_by_name(&rs, i, "amount")),
                acctserial);
    }
    db_free_result(&rs);
    sprintf(sucbuf, "%.2lf", dSucAmt);
    sprintf(failbuf, "%.2lf", dFailAmt);
    fprintf(fp, "�ϼ�;�ɹ�����:%d ���: %s;;ʧ�ܱ���: %d;���: %s;\n", 
            iSucSum, FormatMoney(sucbuf), iFailSum, FormatMoney(failbuf));
    WriteRptRowCount(fp, i+1);

    gettime(printtime, sizeof(printtime), "%Y/%m/%d %H:%M:%S");
    WriteRptFooter(fp, "%s;%s;\n", printtime, gs_oper);
    fclose(fp);

    if ((rc = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0)
    {
        rc = ERR_APP;
        goto err_handle;
    }
    sprintf(filename, "%s", basename(caOutFile));

err_handle:
    ifree(chinesedate);
    return rc;
}

char *getClearState(char clearstate)
{
    switch(clearstate)
    {
        case CLRSTAT_SETTLED:   return "����ɹ�";
        case CLRSTAT_FAILED:    return "����ʧ��";
        case CLRSTAT_UNKNOW:    return "״̬δ֪";
        case CLRSTAT_CHECKED:   return "�Ѷ���";
        case CLRSTAT_UNSETTLED: return "δ����";
        default: return "δ֪";
    }
}

/*
 * ��ӡ�����嵥
 * ����:opDoc ƽ̨���� p ����
 */
int PrintSettleList(void *opDoc, char *filename)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs, rs2;
    FILE *fp = NULL;
    char tbname[10] = "trnjour", tmp[1024] = {0};
    char caParaFile[256] = {0}, caDataFile[256] = {0}, caOutFile[256] = {0};
    char sOriginator[13], sInnerBank[13], sWorkDate[9];
    char acctstat[20];
    int classid, workround;
    int recordCount = 0, recordCount2 = 0;
    int i = 0, j = 0;
    int ret = 0;

    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opInnerBank", sInnerBank, sizeof(sInnerBank));
    XmlGetString(opDoc, "//opCleardate", sWorkDate, sizeof(sWorkDate));
    workround = XmlGetInteger(opDoc, "//opClearround");
    classid = XmlGetInteger(opDoc, "//opClassid");
    /*
       strcpy(settledate, GetSettledDateround());
       if ((cleardate = XMLGetNodeVal(doc, "//opCleardate")) == NULL)
       strcpy(currdate, getDate(0));
       else
       strcpy(currdate, cleardate);
       classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
       clearround = XmlGetInteger(doc, "//opClearround");

       BKINFO("����:%d ����:%d ��ȡ�������ڳ���:%s ϵͳ����:%s ��������:%s", 
       classid, clearround, settledate, GetWorkdate(), currdate);
       settledate[8] = 0;

    //�������һ��δ����ʱ���˳�����ǰһ����󳡴�,�������Ϲ鵵ʱ�����˳�����0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, currdate) != 0) {
    BKINFO("[%d]>[%d] ? , settldate[%s]!=currdate[%s] ?",
    clearround, atoi(settledate+9),
    settledate, currdate);
    XMLSetNodeVal(doc, "//opTCRetinfo", "δ����,�������ӡ�����嵥");
    return 0;
    }
     */

    memset(tmp, 0, sizeof(tmp));
    if (strcmp(GetCBankno(), sOriginator))
        sprintf(tmp, "AND innerorganid='%s'", sInnerBank);
    ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
            "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
            "WHERE classid=%d %s AND workdate='%s' AND workround='%d' AND clearstate!='0' order by workround, clearstate, refid",
            tbname, classid, tmp, sWorkDate, workround);
    /*
       ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
       "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
       "WHERE classid=%d AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
       tbname, classid, currdate, clearround);
       ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
       "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
       "WHERE classid=%d AND ((originator='%s' and inoutflag='1') OR (acceptor='%s' and inoutflag='2'))"
       "  AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
       tbname, classid,
       XMLGetNodeVal(doc, "//opOriginator"), XMLGetNodeVal(doc, "//opOriginator"),
       currdate, clearround);
     */
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s;%s;%s;%s;%d;%d;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"),
            sWorkDate, classid, "���г���", XMLGetNodeVal(doc, "//opOriginator"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++)
    {
        sprintf(tmp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                *db_cell(&rs, i, 4) == '1' ? "��" : "��",
                db_cell(&rs, i, 5),
                db_cell(&rs, i, 6),
                db_cell(&rs, i, 9),
                db_cell(&rs, i, 11),
                FormatMoney(db_cell(&rs, i, 7)),
                getClearState(*db_cell(&rs, i, 13)));

        ret = db_query(&rs2, "SELECT acctserial, result FROM acctjour WHERE " 
                "nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%c'", 
                OP_REGIONID, sWorkDate, db_cell(&rs, i, 1), 
                sdpStringTrimHeadChar(db_cell(&rs, i, 0), '0'), *db_cell(&rs, i, 3));
        if (ret) {
            if (ret == E_DB_NORECORD) {
                fprintf(fp, "%s;%s;\n", tmp, "δ����");
                continue;
            } else {
                BKINFO("��ѯ������ˮʧ��,ret=%d", ret);
                db_free_result(&rs2);
                return ret;
            }
        }
        else { 
            if (db_cell_i(&rs2, j, 1) != 1)
                strcpy(acctstat, GetChineseName(acc_stat_list, db_cell_i(&rs2, j, 1)));
            else
                strcpy(acctstat, db_cell(&rs2, j, 0));
            fprintf(fp, "%s;%s;\n", tmp, acctstat);
        }
        db_free_result(&rs2);
    }
    db_free_result(&rs);
    WriteRptRowCount(fp, recordCount);
    WriteRptFooter(fp, "");
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/BankAcctList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("������ʱ�ļ����򱨱����ļ�ʧ��!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    sprintf(filename, "%s", basename(caOutFile));
    return 0;
}

/*
   ���ײ�ѯ����������
 */
int PF10_151(void *doc, char *p)
{
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sFlag[2], sInOutFlag[2];
    char sWorkDate[9], sOriginator[13], sRefId[17];
    int ret;

    XmlGetString(opDoc, "//opOriginator", sOriginator, sizeof(sOriginator));
    XmlGetString(opDoc, "//opWorkdate", sWorkDate, sizeof(sWorkDate));
    XmlGetString(opDoc, "//opRefid", sRefId, sizeof(sRefId));
    XmlGetString(opDoc, "//opInoutflag", sInOutFlag, sizeof(sInOutFlag));

    BKINFO("��ѯ���ڽ���: workdate=[%s] originator=[%s] refid=[%s] io=[%s]...", 
            sWorkDate, sOriginator, sRefId, sInOutFlag);

    // �ж��˻�����, ��˽�������ڲ�ѯ
    ret = db_query_str(sFlag, sizeof(sFlag), 
            "select resflag1 from trnjour where nodeid=%d "
            "AND workdate='%s' AND refid='%s' AND originator='%s' "
            "AND inoutflag='%s'", OP_REGIONID,
            sWorkDate, sRefId, sOriginator, sInOutFlag);
    if (ret != 0)
        return ret;
    /*
       if (*sFlag == '1') // ��˽
       {
       XMLSetNodeVal(opDoc, "//opBKRetcode", "0");
       XMLSetNodeVal(opDoc, "//opTreserved1", "999");
       XMLSetNodeVal(opDoc, "//opTreserved2", "��" );
       XMLSetNodeVal(opDoc, "//opBKRetinfo", "��˽ҵ�񲻼���");
       return 0;
       }
     */

    /* ��ѯ���ڼ���״̬ */
    ret = CallInBank8201(opDoc, sWorkDate, sOriginator, sRefId, sInOutFlag);

    if (ret == 0 && XmlGetInteger(opDoc, "//opBKRetcode") == 0 
            && XmlGetInteger(opDoc, "//opTreserved1") == 0) {
        db_exec("update acctjour set result='1', acctserial='%s' where "
                "nodeid = %d and workdate='%s' and originator='%s' "
                "and refid='%s' and inoutflag='%s'",
                XMLGetNodeVal(opDoc, "//opTreserved2"), 
                OP_REGIONID, sWorkDate, sOriginator, sRefId, sInOutFlag);
    }

    return ret;
}
