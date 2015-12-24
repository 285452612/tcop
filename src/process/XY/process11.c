#include "interface.h"
#include "nt_const.h"

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
   ��ѯ������ˮ
   */
int QueryAcctSerial(xmlDocPtr opDoc, char *sSerial, char *sState)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "select acctserial, result from acctjour where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Originator"),
            XMLGetNodeVal(opDoc, "//Refid"),
            XMLGetNodeVal(opDoc, "//Inoutflag"));
    ret = DBQueryStrings(sSqlStr, 2, sSerial, sState);
    return ret;
}

/*
 * ���¼�����ˮ���¼Ϊ�ѳ���
 */
int UpAcctSerial(xmlDocPtr opDoc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "update acctjour set revserial = '%s', result = '%s' where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            XMLGetNodeVal(opDoc, "//opHostSerial"),
            "2",    //�ѳ���
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Originator"),
            XMLGetNodeVal(opDoc, "//Refid"),
            pInOutFlag
           );
    ret = OPDBExec(sSqlStr);
    return ret;
}

/*
 * ��ȡ���������˻�
 */
int GetClearAcct(char *pExchgNo, char *pAcct)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "select clearacct from organinfo where exchno = '%s'",
            pExchgNo);
    ret = DBQueryString(pAcct, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("��ѯ[%s]�����ʺ�ʧ��", pExchgNo);
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("����ͬ���к�[%s]δ�ҵ����������ʺ�", pExchgNo);
        return ret;
    }
    return 0;
}

/*
 * ����ҵ���ȡ���ڻ�����
 */
int GetBankOrgInfo(char *pExchgNo, char *pOrgId)
{
    int ret;
    char sSqlStr[1024];

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select organid from organinfo where exchno = '%s'",
            pExchgNo);
    //memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = DBQueryString(pOrgId, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("��ѯ���ڻ�����ʧ��");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("����ͬ���к�[%s]δ�ҵ����ڻ�����", pExchgNo);
        return ret;
    }
    return 0;
}

/*
 * ������ʲ�ѯ�����Ա��
 */
int GetInputOper(char *pSerial, char *pWorkDate, int iNode, char *pOper, char *pExchgNo)
{
    int ret;
    char sSqlStr[1024];

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select acctoper from trnjour where inoutflag = '1' and refid = '%s' \
            and workdate = '%s' and nodeid = %d and originator = '%s'",
            pSerial, pWorkDate, iNode, pExchgNo);
    BKINFO("QUERY INPUTOPER SQL:%s", sSqlStr);
    ret = DBQueryString(pOper, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("��ѯ���׾����Աʧ��");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("���ײ�����");
        return ret;
    }
    return ret;
}

/*
 * ��֯1102ת������
 * ����� (ת��֧Ʊ,���ڽ��,3ʡ1�л�Ʊ)
 */
int PACKCHG_1102(xmlDoc *doc, char *p)
{
    int ret;
    char sSerial[20+1], sDate[8+1], sOper[6+1];
    char sExchgNo[12+1]={0}, sClearAcct[32+1]={0};

    /*���˼���ʱ�����ʹ�Ա���;���Ա�ţ��������ѯ�����Ա*/
    BKINFO("��ȡ�����Ա��...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //���
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    ret = GetClearAcct(sExchgNo, sClearAcct);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//Payacct", sClearAcct);

    return 0;
}

/*
 * ��֯1106ת������
 * �����ڳ���
 * pInOutFlag 1-���ҵ�� ���г��� 2-����ҵ�� ���г���
 */
int PACKCHG_1106(xmlDoc *doc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};
    char sExchgNo[12+1]={0}, sHnOrgId[3+1]={0};
    char sSerial[20+1]={0}, sOper[6+1]={0};

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    if(pInOutFlag[0] == '2')        //���뽻�׳���
        strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    else                                    //������׳���
        strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    sprintf(sSqlStr, "select acctserial from acctjour where workdate = '%s' and refid = '%s' \
            and originator = '%s' and inoutflag = '%s'",
            GetWorkdate(), XMLGetNodeVal(doc, "//Refid"), XMLGetNodeVal(doc, "//Originator"), pInOutFlag);
    ret = DBQueryString(sSerial, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("��ѯԭ���׼�����ˮʧ��");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("ԭ���׼�����ˮ������");
        return ret;
    }
    XMLSetNodeVal(doc, "//opHreserved1", sSerial);
    memcpy(sOper, sSerial, 4);
    XMLSetNodeVal(doc, "//opOperid", sOper);
    return 0;
}

/*
 * ��֯1202ת������
 * ��֤֧������
 */
int PACKCHG_1202(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];
    char sNoteNo[20+1], sNoteType[2+1], sDCFlag[1+1], sHnType[2+1], sHnNo[20+1];
    char sPasswd[20+1]={0};

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, "1");
    //strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //Ʊ�ݺ��� Ʊ������ת��
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);

    strcpy(sPasswd, XMLGetNodeVal(doc, "//Agreement")); //����֧�������Žڵ�
    if(strlen(sPasswd) != 0)
        XMLSetNodeVal(doc, "//Paykey", sPasswd);        //ǰ̨����֧�������Žڵ�(����ת���ڵ�)

    return 0;
}

/*
 * ��֯1621ת������ ��ͨ�ݲ�ʹ��
 * �����(�ǽ���Ʊ��)
 */
int PACKCHG_1621(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * ��֯1622ת������
 * ����� (֧Ʊ,��ת,���ʴ�ת)
 */
int PACKCHG_1622(xmlDoc *doc, char *p)
{
    int ret;
    char sSqlStr[1024];
    char sSerial[20+1], sDate[8+1], sOper[6+1], sDCFlag[1+1]={0};
    char sNoteType[2+1]={0}, sNoteNo[20+1]={0}, sHnType[2+1]={0}, sHnNo[20+1]={0};
    char sExchgNo[12+1]={0};

    /*���˼���ʱ�����ʹ�Ա���;���Ա�ţ��������ѯ�����Ա*/
    BKINFO("��ȡ�����Ա��...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //���
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    //ƾ֤����Ϊ������ƾ֤����Ϊ99������ƾ֤)
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    //ƾ֤���벻Ϊ���Ҳ�Ϊ0ʱ��ƾ֤������ת��
    ret = trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    if(ret)
    {
        BKINFO("ƾ֤����,ƾ֤����ת��ʧ��");
        return ret;
    }
    BKINFO("ƾ֤����:%s", sHnType);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    //ƾ֤����Ϊ2��ת��֧Ʊ����Ϊ����ҵ��ʱ ��Ʊ���൱�ڽ��ʵ�
    if((atoi(sNoteType) == NOTE_CHECK || atoi(sNoteType) == NOTE_REMIT) && atoi(sDCFlag) == 2)
    {
        XMLSetNodeVal(doc, "//Issueamt", "0");
        //XMLSetNodeVal(doc, "//Notetype", sHnType);
        XMLSetNodeVal(doc, "//Noteno", sHnNo);
    }
    else
    {
        XMLSetNodeVal(doc, "//Notetype", "99");
        XMLSetNodeVal(doc, "//opEXBKDrawType", "3");   //3 ƾӡǩȡ��
        XMLSetNodeVal(doc, "//Noteno", sNoteNo+strlen(sNoteNo)-7);
    }
    return 0;
}

/*
 * ��֯1623ת������
 * ����� ����(֧Ʊ,˰��ɿ���,���ڽ�/��)
 */
int PACKCHG_1623(xmlDoc *doc, char *p)
{
    int ret;
    char sNoteType[2+1], sNoteNo[20+1], sHnType[2+1], sHnNo[20+1];
    char sDCFlag[1+1], sExchgNo[12+1], sHnOrgId[3+1];

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    XMLSetNodeVal(doc, "//Issueamt", "0");  //�޶�

    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //Ʊ�ݺ��� Ʊ������ת��
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);
    if(atoi(sNoteType) == 2 && atoi(sDCFlag) == 1)            //����֧Ʊ��ƾ��  1-��
        XMLSetNodeVal(doc, "//opEXBKDrawType", "1");   //ƾ��
    else
        XMLSetNodeVal(doc, "//opEXBKDrawType", "3");   //ƾǩ
    //XMLSetNodeVal(doc, "//Memo", "103");

    return 0;
}

/*
 * ��֯1624ת������
 * ����� (֧Ʊ,���ʴ�ת,��ת,����)
 */
int PACKCHG_1624(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);
    return 0;
}

/*
 * ��֯1625ת������ ��ͨ�ݲ���
 * ����������Ʊ(�������Ʊ)
 */
int PACKCHG_1625(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * ��֯1626ת������ ��ͨ�ݲ���
 * ����������Ʊ ����ֱ���1623��1624���ʹ��
 */
int PACKCHG_1626(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * ��֯1631ת������ ��ͨ�ݲ�ʹ��
 * ���ڷǱ������ʺŽ�����������׼���
 */
int PACKCHG_1631(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * ��֯1632ת������
 * ����� (�ŵ��)
 */
int PACKCHG_1632(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    XMLSetNodeVal(doc, "//Noteno", "");

    return 0;
}

/*
 * ��֯1637ת������
 * ����� (���ƾ֤,����)
 */
int PACKCHG_1637(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * ��֯1643ת������
 * ����� (�������б�Ʊ)
 */
int PACKCHG_1643(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1], sDCFlag[1+1];
    char sNoteType[2+1], sNoteNo[20+1], sHnType[2+1], sHnNo[20+1], sHnHpNo[20+1];

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //Ʊ�ݺ��� Ʊ������ת��
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);
    memset(sHnHpNo, 0, sizeof sHnHpNo);
    sprintf(sHnHpNo, "%s%s", sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Reserved", sHnHpNo);

    return 0;
}

/*
 * ��֯1648ת������
 * ����� (�������б�Ʊ)
 */
int PACKCHG_1648(xmlDoc *doc, char *p)
{
    int ret;
    char sSerial[20+1], sDate[8+1], sOper[6+1];
    char sExchgNo[12+1]={0};

    BKINFO("��ȡ�����Ա��...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //���
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    return 0;
}

/*
   ��ѯ������2380����һ��
   */
int PACKCHG_3380(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1]={0}, sExchgNo[12+1]={0};

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    return 0;
}

/*
 * ��֯6101ת������ 
 * �����(���ڽ�/��,˰��ɿ���)����Э��ż��
 */
int PACKCHG_6101(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1]={0}, sExchgNo[12+1]={0};

    BKINFO("���뽻�׻�ȡ���ڻ���...");
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);
    return 0;
}

/*
 * ¼�����
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF11_100001(void *doc, char *p)
{
    return 0;
}

/*
 * ���˼���
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF11_100002(void *doc, char *p)
{
    int ret;
    int iNoteType,iTrnCode;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sNoteType[2+1]={0}, sDCFlag[1+1]={0}, sCurCode[5+1]={0};
    char sResult[8+1]={0}, sTrnCode[6+1]={0}, sNoteNo[20+1]={0};

    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//Curcode"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//Notetype"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    iNoteType = atoi(sNoteType);

    BKINFO("ƾ֤����:%d �����־:%s ������ͨѶ��־:%s", iNoteType, sDCFlag, p);
    //���¸�ֵƾ֤���룬�����͹���������0��12λƾ֤����
    //������Ҫ����8λƾ֤����
    strcpy(sNoteNo, XMLGetNodeVal(opDoc, "//Noteno"));
    if(strlen(sNoteNo) > 8)
        XMLSetNodeVal(opDoc, "//Noteno", sNoteNo+strlen(sNoteNo)-8);
    //������ͨѶ�� �ж����ķ��صĽ�������� ����������ʧ�� �����
    if(p[0] == COMMTOPH_AFTER[0])
    {
        //���ķ��ؽ��
        strcpy(sResult, XMLGetNodeVal(opDoc, "//opTCRetcode")); 
        if(atoi(sResult) != 0)
        {
            ret = IsAcctCode(sResult);
            if(ret == 7)            //����״̬��ȷ��
                return E_SYS_COMM;
            if(sDCFlag[0] == FL_CREDIT[1] && memcmp(sCurCode, "CNY", 3) == 0)
            {
                ret = PACKCHG_1106(opDoc, "1");                   //����
                iTrnCode = 1106;
                goto ACCT;
            }
            return E_APP_NONEEDACCOUNT;           //���ڲ����ʷ��� ����ʧ��
        }
    }
    //��Ҳ��Զ�����
    if(memcmp(sCurCode, "CNY", 3) != 0)
        return 0;

    if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_DEBIT[1] 
            && p[0] == COMMTOPH_AFTER[0])                    //�����֧Ʊ���ķ���
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    else if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_CREDIT[1]
            && p[0] == COMMTOPH_BEFORE[0])                   //�����֧Ʊ��������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_PROM && p[0] == COMMTOPH_AFTER[0])    //����豾Ʊ���ķ���
    {
        ret = PACKCHG_1648(opDoc, p);
        iTrnCode = 1648;
    }
    else if(iNoteType == NOTE_ZONE_DRAFT && p[0] == COMMTOPH_AFTER[0])  //�����3ʡ1�л�Ʊ���ķ���
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    else if(iNoteType == NOTE_CORRXFER && p[0] == COMMTOPH_BEFORE[0])   //��������ʴ�ת��������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_SPCXFER && p[0] == COMMTOPH_BEFORE[0])    //�������ת��������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_NETBANK && p[0] == COMMTOPH_BEFORE[0])    //�����������������
    {
        ret = PACKCHG_1637(opDoc, p);
        iTrnCode = 1637;
    }
    else if(iNoteType == NOTE_REMIT && p[0] == COMMTOPH_BEFORE[0])      //��������ƾ֤��������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_DRAFT && p[0] == COMMTOPH_BEFORE[0])      //�����ȫ�����л�Ʊ��������
    {
        ret = PACKCHG_1643(opDoc, p);
        iTrnCode = 1643;
    }
    else if(iNoteType == NOTE_CONPAY && p[0] == COMMTOPH_BEFORE[0])     //�����������֧����������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_COLLECTION && p[0] == COMMTOPH_BEFORE[0])   //���������ƾ֤��������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_PDC && p[0] == COMMTOPH_BEFORE[0] && sDCFlag[0] == FL_CREDIT[1])   //��������ڽ�/����������
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    //˰�ѽɿ����������������
    else if(iNoteType == NOTE_TAXPAY && p[0] == COMMTOPH_BEFORE[0] && sDCFlag[0] == FL_CREDIT[1])
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    //����趨�ڽ�/��(��)��������
    else if(iNoteType == NOTE_PDC && p[0] == COMMTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    /*˰�ѽɿ����������������
    else if(iNoteType == NOTE_TAXPAY && p[0] == COMMTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }*/
    else
    {
        if(p[0] == COMMTOPH_BEFORE[0])
            BKINFO("��������ǰ������");
        else if(p[0] == COMMTOPH_AFTER[0])
        {
            BKINFO("�������ĺ󲻼���");
            return E_APP_ACCOUNTSUCC;
        }
        return 0;
    }

ACCT:
    if(ret)
    {
        BKINFO("���ڱ�����֯ʧ��");
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "���ڳ���ʧ��" : "���ڼ���ʧ��");
        return iTrnCode == 1106 ? E_APP_CZFAIL : E_APP_ACCOUNTFAIL;
    }

    //�����ڼ���
    ret = callInterface(iTrnCode, opDoc);
    if(ret)
        return ret;
    memset(sResult, 0, sizeof sResult);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
    {
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "���ڳ���ʧ��" : "���ڼ���ʧ��");
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }
    //else
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "���ڳ����ɹ�" : "���ڼ��ʳɹ�");

    //����ǳ������ײ���¼������ˮ
    if(iTrnCode == 1106)
    {
        ret = UpAcctSerial(opDoc, "1");
        return E_APP_ACCOUNTANDCZ;
    }
    sprintf(sTrnCode, "%d", iTrnCode);
    XMLSetNodeVal(opDoc, "//opOreserved2", sTrnCode);
    XMLSetNodeVal(opDoc, "//opRetinfo", vstrcat("��Ա��ˮ%s", XMLGetNodeVal(opDoc, "//opHostSerial")));
    ret = InsertAcctjour(opDoc);
    BKINFO("��¼������ˮ��%s", ret == 0 ? "�ɹ�" : "ʧ��");

    return E_APP_ACCOUNTSUCC;
}

/*
   ͬ������
   */
int PF11_100010(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sSerial[8+1]={0}, sState[1+1]={0};

    ret = QueryAcctSerial(opDoc, sSerial, sState);
    if(ret == 0)
    {
        if(atoi(sState) == 1)
            return E_APP_ACCOUNTSUCC;
        else if(atoi(sState) == 2)
            return E_APP_CZSUCC;
        else
            return E_APP_ACCOUNTFAIL;
    }
    else if(ret == E_DB_NORECORD)
    {
        ret = PF11_100002(opDoc, p);
        return ret;
    }
    else
        return E_APP_ACCOUNTFAIL;
    //return PF11_100002(doc, p);
}

/*
 * �������
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF11_100005(void *doc, char *p)
{
    int ret;
    int iNoteType, iTrnCode;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sResult[8+1]={0}, sCurCode[5+1]={0}, sNoteNo[20+1]={0};
    char sNoteType[2+1]={0}, sTrnCode[6+1]={0}, sDCFlag[1+1]={0};
    char sAcctName[50+1]={0};
    char *AcctCheck;

    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//Curcode"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//Notetype"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    iNoteType = atoi(sNoteType);

    BKINFO("ƾ֤����:%d �����־:%s ������ͨѶ��־:%s ����:%s", iNoteType, sDCFlag, p, sCurCode);
    //��Ҳ��Զ�����
    if(memcmp(sCurCode, "CNY", 3) != 0)
    {
        if(iNoteType == NOTE_FORCHECK && sDCFlag[0] == FL_DEBIT[1]) //��������ת��֧Ʊ������
        {
            ret = PACKCHG_1202(opDoc, p);
            iTrnCode = 1202;
            goto ACCT;
        }
        return 0;
    }
    //������뽻�׵�ƾ֤���ȴ���8λ��������ʱ���ȡ
    strcpy(sNoteNo, XMLGetNodeVal(opDoc, "//Noteno"));
    if(strlen(sNoteNo) > 8)
        XMLSetNodeVal(opDoc, "//Noteno", sNoteNo+strlen(sNoteNo)-8);
    //�����Ľ���ʧ�� ���������� ���������
    //if(p[0] == COMMRSPTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    if(p != NULL && sDCFlag[0] == FL_DEBIT[1])
    {
        if(p[0] == COMMRSPTOPH_AFTER[0])
        {
            ret = PACKCHG_1106(opDoc, "2");
            iTrnCode = 1106;
            goto ACCT;
        }
    }

    /*���������ʺŻ��� ������û�м������������ɹ�*/
    if(sDCFlag[0] == FL_CREDIT[1])
    {
        //��ȡ�˻�����־
        AcctCheck = XMLGetNodeVal(doc, "//AcctCheck");
        if(AcctCheck != NULL)
        {
            if(atoi(AcctCheck) == 1)
            {
                ret = PACKCHG_3380(doc, p);
                ret = callInterface(3380, doc);
                if(ret)
                    return E_APP_ACCOUNTSUCC;
                memset(sResult, 0, sizeof sResult);
                strcpy(sResult, XMLGetNodeVal(doc, "//opBKRetcode"));
                if(atoi(sResult) != 0)
                {
                    BKINFO("��ѯ����ʧ��");
                    return E_APP_ACCOUNTSUCC;
                }
                strcpy(sAcctName, XMLGetNodeVal(doc, "//opOreserved2"));
                if(strcmp(sAcctName, XMLGetNodeVal(doc, "//Benename")) != 0)
                {
                    BKINFO("��������;���ջ���:%s ���ڻ���:%s", XMLGetNodeVal(doc, "//Benename"), sAcctName);
                    return E_APP_ACCOUNTSUCC;
                }
            }
        }
    }

    //BKINFO("ƾ֤����:%d �����־:%s ������ͨѶ��־:%s", iNoteType, sDCFlag, p);
    if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_DEBIT[1])    //ת��֧Ʊ�����
    {
        ret = PACKCHG_1623(opDoc, p);
        iTrnCode = 1623;
    }
    else if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_CREDIT[1])  //ת��֧Ʊ�����
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_CORRXFER)  //���˴�ת����ƾ֤����
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_SPCXFER)  //����ת��ƾ֤����
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_NETBANK)  //����
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_DRAWBACK) //˰���˻���
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_FP)       //��������֧��ƾ֤
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_COLLECTION)   //����ƾ֤
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_CONPAY)       //������֧��
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_PDC && sDCFlag[0] == FL_CREDIT[1])    //���ڽ�/��(��)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    //���ڽ�/��(��) ˰�ѽɿ���
    else if((iNoteType == NOTE_PDC || iNoteType == NOTE_TAXPAY) && sDCFlag[0] == FL_DEBIT[1])
    {
        //�ȼ��Э���
        BKINFO("�����ڼ��Э���...");
        ret = PACKCHG_6101(opDoc, p);
        if(ret)
            E_APP_ACCOUNTFAIL;
        ret = callInterface(6101, opDoc);
        if(ret)
            return ret;
        if(atoi(XMLGetNodeVal(opDoc, "//opBKRetcode")) != 0)
        {
            BKINFO("�����ʺ���Э��Ų���");
            return E_APP_ACCOUNTFAIL;
        }
        ret = PACKCHG_1623(opDoc, p);
        iTrnCode = 1623;
    }
    else if(iNoteType == NOTE_DRAFT)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_REMIT)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else
    {
        BKINFO("��ƾ֤���ʷ�ʽδ��");
        if(sDCFlag[0] == FL_DEBIT[1])
            return E_APP_ACCOUNTFAIL;
        else
            return 0;
    }

ACCT:
    if(ret)
    {
        BKINFO("���ڱ�����֯ʧ��");
        XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "���ڳ���ʧ��" : "���ڼ���ʧ��");
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }

    ret = callInterface(iTrnCode, opDoc);
    if(ret)
        return ret;

    memset(sResult, 0, sizeof sResult);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
    {
        BKINFO("����%sʧ��", iTrnCode == 1106 ? "����" : "����");
        if(iTrnCode != 1106 && sDCFlag[0] == FL_CREDIT[1])
            return 0;
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }
    else
        BKINFO("����%s�ɹ�", iTrnCode == 1106 ? "����" : "����");

    sprintf(sTrnCode, "%d", iTrnCode);
    XMLSetNodeVal(opDoc, "//opOreserved2", sTrnCode);
    ret = InsertAcctjour(opDoc);
    BKINFO("��¼������ˮ��%s", ret == 0 ? "�ɹ�" : "ʧ��");

    return E_APP_ACCOUNTSUCC;
}

/*
   ���ײ�ѯ����������
   */
int PF11_100008(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sTcResult[4+1]={0}, sDCFlag[1+1]={0};
    char sOpState[1+1]={0}, sHnOrgId[3+1]={0};
    char sSerial[8+1]={0}, sResult[8+1]={0};

    strcpy(sTcResult, XMLGetNodeVal(opDoc, "//opTCRetcode"));
    ret = IsAcctCode(sTcResult);
    if(ret == 7)
        return CLRSTAT_UNKNOW;
    ret = GetBankOrgInfo(atoi(XMLGetNodeVal(opDoc, "//Inoutflag")) == 1 ? XMLGetNodeVal(opDoc, "//Originator") : XMLGetNodeVal(opDoc, "//Acceptor"), sHnOrgId);
    if(ret)
        return atoi(sTcResult) == 0 ? CLRSTAT_SETTLED : CLRSTAT_FAILED;
    XMLSetNodeVal(opDoc, "//opBankno", sHnOrgId);

    sprintf(sSqlStr, "select clearstate from trnjour where nodeid = %d and workdate = '%s' and refid = '%s' \
            and inoutflag = '%s' and originator = '%s'",
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Refid"),
            XMLGetNodeVal(opDoc, "//Inoutflag"),
            XMLGetNodeVal(opDoc, "//Originator"));
    ret = DBQueryString(sOpState, sSqlStr);
    if(ret)
    {
        BKINFO("��ѯԭ��¼����״̬ʧ��");
        return atoi(sTcResult) == 0 ? CLRSTAT_SETTLED : CLRSTAT_FAILED;
    }
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    BKINFO("��������״̬:%s ����������:%s �����:%s", sOpState, sTcResult, sDCFlag);
    //��������ɹ� ��������ɹ� ��������ɹ�
    if(sOpState[0] == CLRSTAT_SETTLED && atoi(sTcResult) == 0)
    {
        /*
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        //û�м�����ˮ �� �м�����ˮ�����Ǽ��ʳɹ�״̬�貹��
        if(ret == E_DB_NORECORD || (ret == 0 && atoi(sResult) != 1))
        {
            BKINFO("δ����,���ڽ����貹��");
            ret = PF11_100002(opDoc, atoi(sDCFlag) == 2 ? COMMTOPH_BEFORE : COMMTOPH_AFTER);
            return CLRSTAT_SETTLED;
        }*/
        return CLRSTAT_SETTLED;
    }
    //��������ʧ�� ��������ʧ�� ��������ʧ��
    else if(sOpState[0] == CLRSTAT_FAILED && atoi(sTcResult) != 0)
    {
        /*
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        //���ʳɹ� �� ����
        if(ret == 0 && atoi(sResult) == 1)
        {
            BKINFO("�Ѽ���,���ڽ��������");
            ret = PACKCHG_1106(opDoc, "1");     //��ѯ���𷽳���
            if(ret)
            {
                BKINFO("���������ʧ�ܣ�����ʧ��");
                return CLRSTAT_FAILED;
            }
            ret = callInterface(1106, opDoc);
            if(ret)
                BKINFO("����ʧ��");
            return CLRSTAT_FAILED;
        }*/
        return CLRSTAT_FAILED;
    }
    //��������ʧ�� ��������ɹ� �貹��
    else if(sOpState[0] == CLRSTAT_FAILED && atoi(sTcResult) == 0)
    {
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        if(ret == E_DB_NORECORD || (ret == 0 && atoi(sResult) != 1))
        {
            BKINFO("δ���ʻ����ʧ��,���ڽ����貹��");
            ret = PF11_100002(opDoc, atoi(sDCFlag) == 2 ? COMMTOPH_BEFORE : COMMTOPH_AFTER);
            return CLRSTAT_SETTLED;
        }
        return CLRSTAT_SETTLED;
    }
    //��������ɹ� ��������ʧ�� �����
    else if(sOpState[0] == CLRSTAT_SETTLED && atoi(sTcResult) != 0)
    {
        //������׳���
        BKINFO("���ڽ��������");
        ret = PACKCHG_1106(opDoc, "1");
        if(ret)
        {
            BKINFO("���������ʧ�ܣ�����ʧ��");
            return CLRSTAT_FAILED;
        }
        ret = callInterface(1106, opDoc);
        if(ret)
            BKINFO("����ʧ��");
        return CLRSTAT_FAILED;
    }
    //��������δ֪ ��������ɹ� �����������
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) == 0 && atoi(sDCFlag) == 2)
        return CLRSTAT_SETTLED;
    //��������δ֪ ��������ɹ� ������貹��
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) == 0 && atoi(sDCFlag) == 1)
    {
        BKINFO("��������ڽ����貹��");
        ret = PF11_100002(opDoc, COMMTOPH_AFTER);
        return CLRSTAT_SETTLED;
    }
    //��������δ֪ ��������ʧ�� ����費����
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) != 0 && atoi(sDCFlag) == 1)
        return CLRSTAT_FAILED;
    //��������δ֪ ��������ʧ�� ����������
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) != 0 && atoi(sDCFlag) == 2)
    {
        BKINFO("��������ڽ��������");
        ret = PACKCHG_1106(opDoc, "1");
        if(ret)
        {
            BKINFO("���������ʧ�ܣ�����ʧ��");
            return CLRSTAT_FAILED;
        }
        ret = callInterface(1106, opDoc);
        if(ret)
            BKINFO("����ʧ��");
        return CLRSTAT_FAILED;
    }
    return 0;
}

/*
   ���������ڲ�ѯ���׽��
   */
int PF11_100009(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sSerial[8+1]={0}, sState[1+1]={0};

    ret = QueryAcctSerial(opDoc, sSerial, sState);
    if(ret == 0)
    {
        if(atoi(sState) == 1)
            return E_APP_ACCOUNTSUCC;
        else if(atoi(sState) == 2)
            return E_APP_CZSUCC;
        else
            return E_APP_ACCOUNTFAIL;
    }
    else if(ret == E_DB_NORECORD)
    {
        ret = PF11_100005(opDoc, "");
        return ret;
    }
    else
        return E_APP_ACCOUNTFAIL;
}

/*
 * Ʊ�ݺϷ��Լ��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF11_100007(void *doc, char *p)
{
    int ret;
    ret = PACKCHG_1202(doc, p);
    return callInterface(1202, (xmlDoc *)doc);
}

/*
 * ������ѯ
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF11_400001(void *doc, char *p)
{
    return callInterface(2380, (xmlDoc *)doc);
}

/*
 * ��ȡ��������
 */
int HFGetRegionIdInfo(void *src, char *dst)
{
    strcpy(dst, "40");
    return 0;
}

/*
 * ��ȡ�ն˴���
 */
int HFGetTermNoInfo(void *src, char *dst)
{
    switch(OP_REGIONID)
    {
        case 10:        //����  
            strcpy(dst, "ttysztc ");
            break;
        case 11:        //��ͨ  
            strcpy(dst, "ttynttc ");
            break;
        case 12:        //����  
            strcpy(dst, "ttywxtc ");
            break;
        case 13:        //����  
            strcpy(dst, "ttycztc ");
            break;
        default:
            strcpy(dst, "ttyjstc ");
            break;  
    }
    return 0;
}

/* 
   ��ǰ̨8λƾ֤�ű��7λ,��ʽƾֻ֤������λ,���Ҳ���ʮ�����ƴ��
   ��ʽƾ֤������8λ,������ʮ�����ƴ�� (�ο�conpz8to7.src�ļ�)
 */
void NoteNo2PZXH(char *eight, char *seven)
{
    int val = atoi(eight), i = 0;
    char ch;
    char sevenBak[8];
    BKINFO("eight:%s", eight);
    while(val > 0)
    {
        ch = val % 16;
        if(ch < 10)
            ch += '0';
        else 
            ch += 'a'-10;
        sevenBak[i++] = ch;
        val /= 16;
    }
    sevenBak[i] = 0;
    BKINFO("servnBak:%s i:%d", sevenBak, i);
    for (i=0; i<7; i++)
        seven[i] = sevenBak[6-i];
    BKINFO("pzxh:%s", seven);
}
/* ����ͬ��ƾ֤���ͺ�ƾ֤����ת�����ڼ���ƾ֤���ͺ�ƾ֤��� */
int trans_pzxh(char *pnoteno, char *notetype, char *dcflag, char *hntype, char *pzxh)
{
    int ret;
    char sSqlStr[1024];
    char Snotetype[3];
    char Sdsbs[3];
    char Spzdjbz[2];
    char Spzzl[3];
    int pzzl;
    char tmp[30];
    char *noteno = NULL;
    
    noteno = pnoteno + strlen(pnoteno) - 8;
    BKINFO("NOTENO:[%s]", noteno);

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select banktype, reserved1, dcflag from notetypemap where tctype = '%s' \
            and nodeid = %d and dcflag = '%s'",
            notetype, OP_REGIONID, dcflag);
    memset(Spzzl, 0, sizeof Spzzl);
    memset(Sdsbs, 0, sizeof Sdsbs);
    memset(Spzdjbz, 0, sizeof Spzdjbz);
    ret = DBQueryStrings(sSqlStr, 3, Spzzl, Sdsbs, Spzdjbz);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("��ѯ����ƾ֤����ʧ��");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("û�ж�Ӧ������ƾ֤����");
        return ret;
    }
    pzzl = atoi(Spzzl);
    memcpy(hntype, Spzzl, 2); //ת������ƾ֤����

    //memcpy(pzxh, noteno+strlen(noteno)-7, 7); Ĭ��ȡ��7λ(����Ĭ��ƾ֤�����Ҫ�پ���)
    if(strlen(noteno) >7)
        memcpy(tmp, noteno+strlen(noteno)-7, 7);
    else
        memcpy(tmp, noteno, strlen(noteno));
    sprintf(pzxh, "%07d", atoi(tmp));

    BKINFO("Spzdjbz:%s pzzl:%d reserved1:%s", Spzdjbz, pzzl, Sdsbs);
    if (Spzdjbz[0] == '2' || pzzl == 60 || pzzl == 61 || pzzl == 62 || pzzl == 83 || Spzdjbz[0] == '1')
    {
        BKINFO("NOTENO LEN:%d NOTENO:%s", strlen(noteno), noteno);
        if (Sdsbs[0] == '0') //��ʽƾ֤
        {
            memset(tmp, 0, sizeof tmp);
            if(strlen(noteno) > 7)
                memcpy(tmp, noteno+strlen(noteno)-7, 7);
            else
                memcpy(tmp, noteno, strlen(noteno));
            //���и����ж�(�ο�postPZXH_ZG.src)
            //memcpy(pzxh, noteno+strlen(noteno)-7, 7);
            sprintf(pzxh, "%07d", atoi(tmp));
        }
        else //��ʽƾ֤
        {
            if (strlen(noteno) != 8) //��ʼ��ű���Ϊ��λ
                return -1;
            else
                NoteNo2PZXH(noteno, pzxh);
        }
    }

    return 0;
}
