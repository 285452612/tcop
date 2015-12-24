#include "interface.h" 
#include "errcode.h" 
#include "sz_const.h" 
#define GenAccountSerial() GenSerial("account", 1, 9999999, 1)
//#define TEST_PF
//#define TEST_PF_PT
#define DEBUG_PF 

static char GSQL[SQLBUFF_MAX] = {0};
static int ret = 0;
//����ҵ��
#define ACCOUNT_OUTZZ_PERSON            8227    //����ҵ��(ת��)7708
#define ACCOUNT_OUTCASH_PERSON          8227    //����ҵ��(�ֽ�)7710
#define ACCOUNT_TRAN_PERSON             8227    //����ҵ��
#define ACCOUNT_TRAN_PERSON_KS          8227    //Э���˰�����˻�
#define ACCOUNT_OUTCASH_SAVE            4201    //����ֽ�ͨ�� 
#define ACCOUNT_OUTCASH_WITHDRAW        4202    //����ֽ�ͨ��
//����ҵ��
#define ACCOUNT_CHECK_ZFMM              8299    //У��֧������
#define ACCOUNT_GET_PH                  7130    //��ȡ֧����������
#define ACCOUNT_CHECK_BPMY              9989    //У�鱾Ʊ��Ѻ(sz61)
#define ACCOUNT_INCREDIT_FS             9960    //���c(sz60)
#define ACCOUNT_CREDITCZ                7704    //���ǣ����ĳ���  OK
#define ACCOUNT_DEBITCZ                 7704    //��ǣ��ۿ�ĳ���  OK
#define ACCOUNT_OPERCZ                  9038    //����Ա��ˮ���� 
#define ACCOUNT_EFBPCZ                  9928    //EF14,EF16���׳���
#define ACCOUNT_ACCTINFO_PERSON         8982    //�����˻���Ϣ��ѯ
#define ACCOUNT_ACCTINFO_PUB            9982    //�Թ��˻���Ϣ��ѯ
#define ACCOUNT_ACCTINFO_PERSON_KS      3982    //�����˻���Ϣ��ѯ(Э���˰ʹ��)
#define ACCOUNT_ACCTINFO_PUB1           4302    //�Թ��˻���Ϣ��ѯ(�˻�����ʹ��)
#define ACCOUNT_QUERY_BPINFO            8307    //��Ʊ(��Ʊ)�ǼǱ���ѯ(ȷ��)
#define ACCOUNT_QUERY_BPINFO_IN         9307    //��Ʊ(��Ʊ)�ǼǱ���ѯ(����)
#define ACCOUNT_QUERY_ACCOUNTTYPE       8992    //��ѯ���׵����ʷ�ʽ
#define ACCOUNT_QUERY_ORG               9328    //��ѯӪҵ����
#define ACCOUNT_CATCH_FEE               9579    //�ۿͻ�������
#define ACCOUNT_GET_PERSONMX            8263    //��ȡ����ҵ����ļ�����ϸ
#define ACCOUNT_GET_TERMNO              8978    //��ȡ��Ա�ն˺�
#define ACCOUNT_GET_DATE                1650    //��ȡ����ϵͳ����(e650)
#define ACCOUNT_QUERY_BPMY              8359    //��ѯ���б�Ʊ��Ѻ
//�Թ�ҵ��
#define ACCOUNT_OUTCREDIT               8963    //�����  
#define ACCOUNT_OUTCREDIT_PERSON        9963    //�����(����ҵ��,ͬ8963)
#define ACCOUNT_OUTTRUNCDEBIT           8953    //��������
#define ACCOUNT_OUTNOTRUNCDEBIT         8951    //�����ǽ���  OK
#define ACCOUNT_OUTNOTRUNCDEBIT_PERSON  9951    //�����ǽ���(����ҵ��,ͬ8951) 
#define ACCOUNT_INCREDIT                8248    //������Զ������������Ʊ 
#define ACCOUNT_INDEBITRUNC_BP          9984    //���뱾Ʊ(����)

/*****************����ȷ��ҵ��******************
  1��   ��Ʊ���͵���8984����
  2��   ���л�Ʊ���͵���8983����
  3��   �жһ�Ʊ���͵���EF14����
  4��   ��ҵ��Ʊ���͵���EF16����
  5��   ������ǵ���9953==8953����
  6��   ����ȷ����8249����
*/
#define ACCOUNT_INDEBITCHK_BP           8984    //���뱾Ʊȷ��
#define ACCOUNT_INDEBITCHK_HP           8983    //�����Ʊȷ��
#define ACCOUNT_INDEBITCHK_OTHER        9953    //�����������ȷ��
#define ACCOUNT_INDEBITCHK_CDHP         9914    //����жһ�Ʊȷ��EF14
#define ACCOUNT_INDEBITCHK_SYHP         9916    //������ҵ��Ʊȷ��EF16
#define ACCOUNT_INCREDIT_CHK            8249    //�������ȷ������ 

//��Ʊҵ��
#define ACCOUNT_OUTCREDIT_TPA           8247    //���������ƱA OK
#define ACCOUNT_OUTCREDIT_TPB           8956    //���������ƱB OK
#define ACCOUNT_OUTDEBIT_TPA            8955    //���������ƱA 
#define ACCOUNT_OUTDEBIT_TPA_BP         1955    //��Ʊ����Ʊ
#define ACCOUNT_OUTDEBIT_TPB            8956    //���������ƱB 
#define ACCOUNT_INCREDIT_TP             8248    //���������Ʊ 

//����(�ֹ�����,ƽ̨������,�ݲ�����)
#define ACCOUNT_OUTDEBIT_ST             9402    //���������

//����δʹ��
#define ACCOUNT_INTRUNCDEBIT            9101    //��������
#define ACCOUNT_INNOTRUNCDEBIT          9201    //�����ǽ���
#define ACCOUNT_INNOTRUNCDEBIT_CHECK    9000    //�����ǽ���ȷ�� �������н��� 
#define ACCOUNT_INCREDIT_TPCHK          9801    //���������Ʊȷ�ϴ���
#define ACCOUNT_CREDIT_REOUT            9901    //���������Ʊ�����
#define ACCOUNT_INDEBIT_TP              9602    //����豻��Ʊ�Զ�����
#define ACCOUNT_INDEBIT_TPCHK           9702    //����豻��Ʊȷ�ϴ���

#define ACCOUNT_OUTDEBIT_TP             9401    //�����ǽ�������Ʊ
#define ACCOUNT_ZZFEE                   9501    //ת����������
#define ACCOUNT_CASHFEE                 9601    //�ֽ���������

#define ACCOUNT_IN_SAVE                 8002    //����ͨ��
#define ACCOUNT_OUTZZ_WITHDRAW          8202    //���ת��ͨ��
//����

int equal(char *s1, char *s2)
{
    return strcmp(s1,s2)?0:1;
}

//��ȡ����ҵ����ļ�������
int GetBankPersonMX( void *opDoc )
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char *p=NULL;

    BKINFO("ȡ���˽��׺��ļ�����ϸ[%d]...",ACCOUNT_GET_PERSONMX);

    XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate() );
    if (ret = callInterface(ACCOUNT_GET_PERSONMX, doc))
        return ret;
    p = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", p);
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        BKINFO("ȡ���˽��׺��ļ�����ϸʧ��...");
        return -1;
    }

    p = XMLGetNodeVal(doc, "//opBAOBLJ");
    BKINFO("ȡ���˽��׺��ļ�����ϸ�ļ�[%s]...", p);
    if( strlen(p) )
    {
        BKINFO("ȡ���˽��׺��ļ�����ϸʧ��[%s]...", p);
        return -1;
    }

    return ret;
}
//����˰Ʊƾ֤�����ڽ�����Ϣ
void SaveTaxNote(void *opDoc)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs,rs_serial;
    char file[256] = {0};
    FILE *fp = NULL;
    char PrintBuf[20480] = {0};
    char where[256] = {0};
    int rc = 0,i = 0,len = 0;
    char tmp[256]={0};

    sprintf(where, "workdate='%s' and refid='%s' and originator='%s' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opOriginator"));

    rc = db_query(&rs,"select payingacct,payer,payingbank,benebank,noteno,beneacct, benename from trnjour where %s", where);
    if(rc != 0)
    {
        BKINFO("����˰Ʊƾ֤�ͻ���Ϣʧ��[%s]", where);
        return;
    }

    rc = db_query(&rs_serial, "select acctserial from acctjour where %s", where);
    if(rc != 0)
    {
        BKINFO("����˰Ʊƾ֤�ͻ���Ϣʧ��[%s]", where);
        db_free_result(&rs);
        return;
    }

    sprintf(file,"%s/consoleprint/%s.tax",getenv("HOME"), db_cell(&rs,0,4));
    //if((fp=fopen(file, "a"))==NULL)
    if((fp=fopen(file, "r+"))==NULL)
    {
        BKINFO("�޷����ļ�[%s]", file);
        db_free_result(&rs);
        db_free_result(&rs_serial);
        return;
    }

    //fseek(fp, 0L, SEEK_SET);
    len=0;
    len+=sprintf(PrintBuf+len,"\n");
    len+=sprintf(PrintBuf+len,"  �����ʺ�:%-32s   �����:%-60s\n", db_cell(&rs,0,0), db_cell(&rs,0, 1));
    len+=sprintf(PrintBuf+len,"  �����к�:%-20s   ��������:%-60s\n", 
            XMLGetNodeVal(doc, "//opOriginator"), org_name(XMLGetNodeVal(doc, "//opOriginator"), tmp));
    len+=sprintf(PrintBuf+len,"  �տ��ʺ�:%-32s   �տ��:%-60s\n", db_cell(&rs,0,5), db_cell(&rs,0, 6));
    len+=sprintf(PrintBuf+len,"  �տ��к�:%-20s   �տ�����:%-60s\n", 
            XMLGetNodeVal(doc, "//opAcceptor"), org_name(XMLGetNodeVal(doc, "//opAcceptor"), tmp));
    len+=sprintf(PrintBuf+len,"  ������ˮ:%-20s\n", db_cell(&rs_serial,0, 0));
    fwrite(PrintBuf, len,1, fp);
    //fprintf(fp, "%s", PrintBuf);

    db_free_result(&rs);
    db_free_result(&rs_serial);
    fclose(fp);

    BKINFO("����˰Ʊ��ӡ�ļ�[%s]�ͻ���Ϣ�ɹ�", file);
}
//�������ƾ֤
void SavePersonNote(void *opDoc)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    result_set rs,rs_serial;
    FILE *fp = NULL;
    int rc = 0,i = -1,len = 0;
    char file[256] = {0};
    char where[256] = {0};
    char wheretest[256] = {0};
    char notetype[4] = {0};
    char PrintBuf[20480] = {0};

    strcpy(notetype, XMLGetNodeVal(doc, "//opNotetype"));
    sprintf(where, "workdate='%s' and refid='%s' and originator='%s' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opOriginator"));
    sprintf(wheretest, "workdate='%s' and refid='%s' and originator='622' and inoutflag='1' ",
            XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opRefid"));
    rc = db_query(&rs,"select beneacct,benename,payingacct,payer,settlamt,acctoper,payingbank,benebank from trnjour where %s", where);
    if(rc != 0)
    {
        BKINFO("�������ƾ֤ʧ��[%s]", where);
        return;
    }

#ifdef TEST_PF
    if(equal(notetype, "73") || equal(notetype, "74"))
        rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", where);
    else
        rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", wheretest);
#else
    rc = db_query(&rs_serial, "select acctserial,trncode from acctjour where %s order by acctserial", where);
#endif
    if(rc != 0)
    {
        /*
           if(equal(notetype, "73") || equal(notetype, "74"))
           BKINFO("�������ƾ֤ʧ��[%s]", where);
           else
           BKINFO("�������ƾ֤ʧ��[%s]", wheretest);
         */
        BKINFO("�������ƾ֤ʧ��[%s]", where);
        db_free_result(&rs);
        return;
    }

    if(db_row_count(&rs_serial) > 1)
    {
        if(equal(notetype, "71"))
        {
            if(equal(db_cell(&rs_serial,0,1), "9963"))
                i=0;
            else
                i=1;
        }
        else if(equal(notetype, "72"))
        {
            if(equal(db_cell(&rs_serial,0,1), "4202"))
                i=0;
            else
                i=1;
        }
        if( i == 0 )
            sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,0,0));
        else
            sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,1,0));
    }
    else
        sprintf(file,"%s/consoleprint/%s.list", getenv("HOME"), db_cell(&rs_serial,0,0));


    if((fp=fopen(file,"w")) == NULL)
    {
        BKINFO("�������ƾ֤ʧ��:�޷���д�뷽ʽ���ļ�[%s]",file);
        db_free_result(&rs);
        db_free_result(&rs_serial);
        return;
    }

    if(equal(notetype,"72") || equal(notetype,"74"))
        len+=sprintf(PrintBuf+len, "\n\n\n\n\n\n\n\n\n\n\n");
    len+=sprintf(PrintBuf+len, "\n\n\n");
    len+=sprintf(PrintBuf+len, "          �տ��к�:%s\n", db_cell(&rs,0,7));
    len+=sprintf(PrintBuf+len, "          �տ��˺�:%s\n", db_cell(&rs,0,0));
    len+=sprintf(PrintBuf+len, "          �տ�������:%s\n", db_cell(&rs,0,1));
    len+=sprintf(PrintBuf+len, "          �����к�:%s\n", db_cell(&rs,0,6));
    len+=sprintf(PrintBuf+len, "          �����˺�:%s\n", db_cell(&rs,0,2));
    len+=sprintf(PrintBuf+len, "          ����������:%s\n", db_cell(&rs,0,3));
    len+=sprintf(PrintBuf+len, "          ���׽��:%s\n", db_cell(&rs,0,4));
    len+=sprintf(PrintBuf+len, "          �� Ա ��:%s\n", db_cell(&rs,0,5));
    len+=sprintf(PrintBuf+len, "          ��Ա��ˮ:%s\n", db_cell(&rs_serial, 0,0));
    len+=sprintf(PrintBuf+len, "                                                  �ͻ�ǩ��:");

    fwrite(PrintBuf, len, 1, fp);

    fclose(fp);
    db_free_result(&rs);
    db_free_result(&rs_serial);

    BKINFO("�������ƾ֤�ɹ�:[%s]", file);
}

char *getAccountResult(char *acctresult)
{
    switch (atoi(acctresult)) {
        case 0:   return "״̬δ֪";
        case 1:   return "���˳ɹ�";
        case 2:   return "�ѳ���";
        case 5:   return "�ѹ���";
        case 9:   return "����ʧ��";
        default: return "δ֪";
    }
}
char *getAccountDesc(char *accttype)
{
    switch (atoi(accttype)) {
        case ACCOUNT_OUTCREDIT:             return "�����";
        case ACCOUNT_INTRUNCDEBIT:          return "������";
        case ACCOUNT_INNOTRUNCDEBIT:        return "�����ǽ�";
        case ACCOUNT_INNOTRUNCDEBIT_CHECK:  return "�����ȷ��";
        case ACCOUNT_OUTDEBIT_TP:           return "����豻��";
        case ACCOUNT_ZZFEE:                 return "ת���շ�";
        case ACCOUNT_CASHFEE:               return "�ֽ��շ�";
        case ACCOUNT_INCREDIT_TPCHK:        return "������ȷ��";
        case ACCOUNT_CREDIT_REOUT:          return "����������";

        case ACCOUNT_OUTCASH_SAVE:          return "����ִ�";

        case ACCOUNT_INCREDIT_CHK:          return "�����ȷ��";
        case ACCOUNT_OUTTRUNCDEBIT:         return "��������";
        case ACCOUNT_OUTNOTRUNCDEBIT:       return "�����ǽ�";
        case ACCOUNT_OUTCREDIT_TPA:         return "�������ƱA";
        case ACCOUNT_OUTCREDIT_TPB:         return "������Ʊ(������ױ��˻�)";
        case ACCOUNT_OUTDEBIT_ST:           return "���������";
        case ACCOUNT_INDEBIT_TP:            return "�豻���Զ�";
        case ACCOUNT_INDEBIT_TPCHK:         return "�豻��ȷ��";

        case ACCOUNT_IN_SAVE:               return "����ͨ��";
        case ACCOUNT_OUTCASH_WITHDRAW:      return "����ֶ�";
        case ACCOUNT_OUTZZ_WITHDRAW:        return "���ת��";

        case ACCOUNT_CREDITCZ:              return "����";
                                            //case ACCOUNT_CREDITCZ:              return "���ǳ���";
                                            //case ACCOUNT_DEBITCZ:               return "��ǳ���";
        default: return "δ֪";
    }
}

static int getcols( char *line, char *words[], int maxwords, int delim )
{
    char *p = line, *p2 = NULL;
    int nwords = 0;
    int append = 0;

    if (line[strlen(line)-1] == delim)
        append = 1;

    while ( *p != '\0' )
    {
        words[nwords++] = p;
        if ( nwords >= maxwords )
            return nwords;

        while(1)
        {
            p2 = strchr( p, delim );
            if ( p2 == NULL )
                break;

            // ��� delim�ַ�ǰ��б�������
            if (p2-1 == NULL || *(p2-1) != '\\')
                break;

            memmove(p2-1, p2, strlen(p2)+1);
            p = p2;
        }
        if (p2 == NULL)
            break;

        *p2 = '\0';
        p = p2 + 1;
    }

    if (append == 1)
    {
        words[nwords] = strdup(" ");
        *(words[nwords]) = 0;
    }

    return nwords + append;
}

// ����Сдת��
static int  MoneyToChinese ( char *money, char * chinese )
{
    int len, zerotype, i, unit_num , allzero = 1 ;
    char fundstr [ 51 ];
    char *numberchar [ ] =
    {
        "��", "Ҽ", "��", "��", "��", "��", "½", "��", "��", "��"
    };
    char *rmbchar [ ] =
    {
        "��", "��", "", "Ԫ", "ʰ", "��", "Ǫ", "��", "ʰ", "��", "Ǫ", 
        "��", "ʰ", "��", "Ǫ"
    };

    sprintf ( fundstr, "%.2lf", atof( money ) );
    len = strlen ( fundstr );
    unit_num = sizeof ( rmbchar ) / sizeof ( rmbchar [ 0 ] );

    for ( i = zerotype = 0, chinese [ 0 ] = '\0' ; i < len ; i++ )
    {
        switch ( fundstr [ i ] )
        {
            case '-':
                {
                    strcat ( chinese, "��" );
                    break;
                }
            case '.':
                {
                    if ( chinese [ 0 ] == '\0' )
                        strcat ( chinese, numberchar [ 0 ] );
                    if ( zerotype == 1 )
                        strcat ( chinese, rmbchar [ 3 ] );
                    zerotype = 0;
                    break;
                }
            case '0':
                {
                    if ( len - i  == 12 && 11 < unit_num )
                        strcat ( chinese, rmbchar [ 11 ] );
                    if ( len - i  == 8 && 7 < unit_num )
                    {
                        if( !allzero )
                            strcat ( chinese, rmbchar [ 7 ] );
                    }
                    zerotype = 1;
                    break;
                }
            default:
                {
                    if ( len - i  < 12 )
                        allzero = 0 ;
                    if ( zerotype == 1 )
                        strcat ( chinese, numberchar [ 0 ] );
                    strcat ( chinese, numberchar [ fundstr [ i ] - '0' ] );
                    if ( len - i - 1 < unit_num )
                        strcat ( chinese, rmbchar [ len - i - 1 ] );
                    zerotype = 0;
                    break;
                }
        }
    }

    if ( memcmp( fundstr + len -2 , "00", 2 ) == 0 ) strcat ( chinese, "��" );
    return 0;
}

//���ݽ��׽���ж���������״̬
int isClearstateUnknowTran(int tcResult)
{
    switch(tcResult)
    {
        //���׽������ȷ,����״̬��ȷ��
        case 201: case 202: case 203: case 204: case 205: case 207:
        case 3002: case 3003: case 8043: case 8048: case 8202: case 8203:
        case 8204: case 8205: return 1; 
        default: return 0;
    }
}

//�����Ƿ����ƾ֤ ʧ�ܷ���-1
int getTruncflag(int notetype, char dcflag)
{
    char tmp[256] = {0};

    if (ret = db_query_str(tmp, sizeof(tmp),
                "SELECT truncflag FROM noteinfo WHERE nodeid=%d AND notetype='%02d' AND dcflag='%c'", 
                OP_REGIONID, notetype, dcflag)) {
        BKINFO("��ѯ���[%c]Ʊ��[%02d]������־ʧ��!ret=[%d]", dcflag, notetype, ret);
        return -1;
    }

    return atoi(tmp);
}

/* 
   ���Э���, ����ƽ̨������
 */
int ChkAgreement(char *id, char *payacct)
{
    result_set rs;
    int rc;

    rc = db_query(&rs, "select state from agreement where nodeid=%d "
            "and agreementid='%s' and payingacct='%s' ", OP_REGIONID, id, payacct);
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

//����ƾ֤���㵥��������:�ɹ� > 0 ʧ�� < 0 �����շ� 0
double feeCalculate(char dcflag, int notetype)
{
    char tmp[64] = {0};

    BKINFO("���㵥��������,ƾ֤����[%d]���[%c]", notetype, dcflag);

    ret = db_query_str(tmp, sizeof(tmp),
            "SELECT feepayer FROM noteinfo WHERE nodeid=%d AND dcflag='%c' AND notetype='%02d'", 
            OP_REGIONID, dcflag, notetype);
    if (ret) {
        BKINFO("��ѯƾ֤�շѷ�����Ϣʧ��");
        return -1;
    }

    //�����Ҹ��ѷ�Ϊ��������Ҹ��ѷ�Ϊ�տ
    if ((dcflag == '2' && atoi(tmp) == 1) || (dcflag == '1' && atoi(tmp) == 2)) {
        if (ret = db_query_str(tmp, sizeof(tmp), "SELECT value FROM feetype WHERE nodeid=%d AND typeid=1", OP_REGIONID)) {
            BKINFO("�����շ����Ͳ�ѯ�շ�ֵʧ��");
            return -2;
        }
        return atof(tmp);
    } 
    return 0;
}

//���˲���¼������ˮ
int Accounting(int bktcode, xmlDoc *doc)
{
    char *p = NULL;
    char where[1024]={0};
    char *workdate = NULL;
    char *exchgdate= NULL;
    char *chkflag = NULL;
    char tmp[24]={0};
    char sql[512]={0};
    char sAcctSerial[14]={0};
    int result;

    BKINFO("������˳�������...");

    p= XMLGetNodeVal(doc, "//opCurcode");
    if (*p!= 0 && strcmp("CNY", p) && bktcode != ACCOUNT_OUTCREDIT) {
        BKINFO("���������, ���[%s]������", p);
        return 0;
    }

    p= sdpXmlSelectNodeText(doc, "//opNoteno");
    //43���c����Ҫ�ض�
    if (p!= NULL && strlen(p) > 8 && bktcode != ACCOUNT_INCREDIT_FS ) {
        BKINFO("ƾ֤��̫��,�ض�[%s->%s]", p, p+strlen(p)-8);
        XMLSetNodeVal(doc, "//opNoteno", p+strlen(p)-8); //����ƾ֤�����8λ(�����8λ)
    }

    XMLSetNodeVal(doc, "//opTrcode", vstrcat("%d", bktcode));
    /*********************************************
     *�������ȡǰ̨��������ˮ,ƽ̨������������ˮ
     *���뽻��ƽ̨������ˮ
     *********************************************/
    p = XMLGetNodeVal(doc, "//opPDQTLS");
    if( strlen(p) )
        XMLSetNodeVal(doc, "//opHreserved5", XMLGetNodeVal(doc, "//opPDQTLS"));
    else {
        /*����ҵ��ƽ̨����Ľ�����ˮǰ��ӻ�������*/
        p = XMLGetNodeVal(doc, "//opSYSMACSBNO");
        if( strlen(p) )
        {
            sprintf( sAcctSerial, "%4s%08ld", p, GenAccountSerial());
            XMLSetNodeVal(doc, "//opHreserved5", sAcctSerial);
        }
        else
            XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));
    }

#if 1
    chkflag = XMLGetNodeVal(doc, "//opChkflag");
    //ȷ�Ͻ���
    if(  chkflag != NULL && atoi(chkflag) == 1 )
    {
        workdate= XMLGetNodeVal(doc, "//opWorkdate");
        exchgdate= XMLGetNodeVal(doc, "//opExchgdate");
        BKINFO("����ȷ��CHKFLAG[%s], TrnCode[%d], WorkDate[%s], ExchgDate[%s]", chkflag, bktcode, workdate, exchgdate);
        sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opExchgdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
        /*��acctjourΪԭ��������Ϊ��������*/
        XMLSetNodeVal(doc, "//opWorkdate", exchgdate);
    }
    else
    {
        sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
    }
#endif

    /*
    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' AND trncode='%d'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
            */

    //ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s AND result='1' AND trncode='%d'", where, bktcode);
    ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s", where );

    if (ret == E_DB_NORECORD)
    {
        if ((ret = InsertAcctjour(doc)) != 0)
        {
            BKINFO("���������Ϣʧ��");
            return E_APP_ACCOUNTFAIL;
        }
    }
    else if(ret)
        return E_APP_ACCOUNTFAIL;

#if 0 
    /*��ԭ��������*/
    if(  chkflag != NULL && atoi(chkflag) == 1 )
        XMLSetNodeVal(doc, "//opWorkdate", workdate);
#endif
    /*********************************************************
     *1:�Ѿ����ʵ��򷵻ؼ��ʳɹ�
     *0:���׽������ȷ���򷵻�����ͨѶ����
     *2:���ױ�����
     *9:����ʧ��
     *5:������ǹ���(���c,����������,8248�������ڹ��˲���ͻ���)
     *acctserial:��Ա��ˮ,revserial:ͬ�������,reserved1:ƽ̨��ˮ
     ************************************************************/
    if(atoi(tmp) == 1)   
        return E_APP_ACCOUNTSUCC;
    else if( strlen(tmp) && atoi(tmp) == 0)
        return E_SYS_COMM_BANK;

    if (ret = callInterface(bktcode, doc))
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", p);
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        //XMLSetNodeVal(doc, "//opBKRetcode", "");
        /*���ڷ�����ȷ������Ϣ*/
        sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"9", where );
        db_exec( sql );
        return E_APP_ACCOUNTFAIL;
    }

    /*����״̬*/
    if( bktcode == ACCOUNT_INCREDIT )
    {
        p = XMLGetNodeVal(doc, "//opTISHXI");
        if( strlen(p) && !sdpStringIsAllChar(p, '0') )
        {
            BKINFO("����������ʹ���,���ڷ���[%s]", p);
            sprintf( sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                    XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"5", where );
            goto EXIT;
        }
    }

    /*������Ʊ8247,����ȷ��8249����Ҫ����ͬ�������,��ԭ���˵�ͬ�������һ��*/
    if( bktcode == ACCOUNT_OUTCREDIT_TPA || bktcode == ACCOUNT_INCREDIT_CHK )
    {
        sprintf(sql, "update acctjour set acctserial = '%s', result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), "1", where );
    }
    else
    {
        sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
                XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"1", where );
    }
    /*
       sprintf(sql, "update acctjour set acctserial = '%s', revserial = '%s',result = '%s' %s",
       XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opOreserved3"),"1", where );
     */

    /*˰Ʊ����˰Ʊ����*/
    p = XMLGetNodeVal(doc, "//opNotetype");
    if( atoi(p) == 14 )
        XMLSetNodeVal( doc, "//opOreserved3", XMLGetNodeVal(doc, "//opXINX03") );
EXIT:
    ret = db_exec( sql );
    if (ret  == 0)
        return E_APP_ACCOUNTSUCC;
    else 
    {
        BKINFO("���׳ɹ�,���½���״̬ʧ��[%d]", ret);
        return ret;
    }
}

//�յ���������(0:�ɹ� ����:ʧ��)
int fetchFee(int bktcode, xmlDoc *doc, int isHtrnjour)
{
    char where[1024] = {0};
    char feeResult[4] = {0};

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='1'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), 
            XMLGetNodeVal(doc, "//opRefid"), 
            XMLGetNodeVal(doc, "//opOriginator"));

    if ((ret = db_query_str(feeResult, sizeof(feeResult), "SELECT result FROM feelist %s", where)) != 0) {
        if (ret != E_DB_NORECORD)
            return ret;
        //��¼��������ˮ
        if ((ret = InsertTableByID(doc, "feelist", 0)) != 0)
            return ret;
    } else if (feeResult[0] == '1') //���շ�
        goto EXIT;

    XMLSetNodeVal(doc, "//opWorkdate", getDate(0));
    if ((ret = Accounting(bktcode, doc)) != E_APP_ACCOUNTSUCC)
        return ret;

    //�����շ���ˮ
    if ((ret = db_exec("UPDATE feelist SET result='1',reserved1='%s' %s", 
                    XMLGetNodeVal(doc, "//opHostSerial"), where)) != 0)
        return ret;

EXIT:
    //���½�����ˮ
    db_exec("UPDATE %s SET feeflag='1', bankfee=%s %s", 
            isHtrnjour ? "htrnjour" : "trnjour", 
            XMLGetNodeVal(doc, "//opSettlamt"), where);

    return 0;
}

//�жϼ�����ˮ������
int CheckAccounting(int bktcode, xmlDoc *doc)
{
    char tmp[8] = {0};
    char where[1024] = {0};

    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s'",
            OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"));

    ret = db_query_str(tmp, sizeof(tmp), "SELECT count(1) FROM acctjour %s AND result='1' AND trncode='%d'", 
            where, bktcode);

    if (ret == E_DB_NORECORD) 
        ret = Accounting(bktcode, doc);
    return ret;
}

//���������¼�����ˮ
int AccountCancel(int bktcode, int flag, xmlDoc *doc)
{
    char acctResult[8] = {0};
    char acctSerial[20] = {0};
    char acctReserved1[20] = {0};
    char where[1024] = {0};
    char sAcctSerial[14]={0};
    char *p = NULL;

    /*����ն�Ϊ����Ϊ����ҵ�������Ʊ,��Ҫȥ���ڲ�ѯ��Ա���ն˺�*/
    p = XMLGetNodeVal(doc, "//opPDWSNO");
    if( strlen(p) == 0 )
    {
        ret = callInterface(ACCOUNT_GET_TERMNO, doc);
        if( ret )
        {
            BKINFO("��ѯ����ҵ�����Ա���ն˺�ʧ��[%d]...", ret );
            return ret;
        }
    }

    if( bktcode == ACCOUNT_CREDITCZ || bktcode == ACCOUNT_DEBITCZ )
    {
        sprintf(where, " WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' ",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag") );
    }
    else
    {
        sprintf(where, " WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%s' and trncode='%d'",
                OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"), XMLGetNodeVal(doc, "//opOriginator"),
                sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'), XMLGetNodeVal(doc, "//opInoutflag"), bktcode);
    }

    sprintf(GSQL, "SELECT acctserial, result, reserved1 FROM acctjour %s", where);

    if (ret = db_query_strs(GSQL, acctSerial, acctResult, acctReserved1)) {
        BKINFO("��ѯԭ������ˮʧ��");
        return E_DB_SELECT;
    }

    if (atoi(acctResult) != 1) {
        BKINFO("�˽����������");
        return 0;
    }

    if( flag == 3 )
        XMLSetNodeVal(doc, "//opHostSerial", acctSerial); //���ڷ��ع�Ա��ˮ
    else
        XMLSetNodeVal(doc, "//opHostSerial", acctReserved1); //ԭϵͳ�ο���

    /*********************************************
     *1:���淢��Ľ���,ȡǰ̨��ˮ, 7704
     *2:ƽ̨�Զ�����ĳ���,ƽ̨������ˮ, 7704
     *3:������¼����, 9038����Ա��ˮ����
     *���ΪEF14,EF16��������EF28(9928)����
     *********************************************/
    if( flag == 1 )
    {
        p = XMLGetNodeVal(doc, "//opPDQTLS");
        if( strlen(p) )
            XMLSetNodeVal(doc, "//opHreserved5", XMLGetNodeVal(doc, "//opPDQTLS"));
        else {
            /*����ҵ��ƽ̨����Ľ�����ˮǰ��ӻ�������*/
            p = XMLGetNodeVal(doc, "//opSYSMACSBNO");
            if( strlen(p) )
            {
                sprintf( sAcctSerial, "%4s%08ld", p, GenAccountSerial());
                XMLSetNodeVal(doc, "//opHreserved5", sAcctSerial);
            }
            else
                XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));
        }
    }
    else if( flag == 2 || flag == 3 )
        XMLSetNodeVal(doc, "//opHreserved5", vstrcat("%012ld", GenAccountSerial()));

     
#if 0 
    /*���Ϊ�ֹ�����������ԭϵͳ��������*/
    p = XMLGetNodeVal(doc, "//opCZFLAG");
    if( p != NULL && atoi(p) == 1 )
    {
        BKINFO("������־[%s]", p);
        XMLSetNodeVal(doc, "//opWorkdate", XMLGetNodeVal(doc, "//opJIOHRQ"));
    }
#endif

     
#if 0 
    /*���Ϊ�ֹ�����������ԭϵͳ��������*/
    p = XMLGetNodeVal(doc, "//opCZFLAG");
    if( p != NULL && atoi(p) == 1 )
    {
        BKINFO("������־[%s]", p);
        XMLSetNodeVal(doc, "//opWorkdate", XMLGetNodeVal(doc, "//opJIOHRQ"));
    }
#endif

    XMLSetNodeVal(doc, "//opBKRetcode", "");
    if( flag == 3 )
        ret = callInterface(ACCOUNT_OPERCZ, doc);
    else if( bktcode == ACCOUNT_INDEBITCHK_CDHP || bktcode == ACCOUNT_INDEBITCHK_SYHP )
        ret = callInterface(ACCOUNT_EFBPCZ, doc);
    else
        ret = callInterface(ACCOUNT_CREDITCZ, doc);
    if( ret )
        return ret;

    p = XMLGetNodeVal(doc, "//opBKRetcode");
    if (strlen(p) && !sdpStringIsAllChar(p, '0'))
    {
        BKINFO("����ʧ��[%s]...", p);
        XMLSetNodeVal(doc, "//opBKRetcode", "");
        return E_APP_CZFAIL;
    }
    BKINFO("opBKRetcode=[%s]", p);

    /****************************************
     *revserial:��������������ˮ
     *reserved2:��������ƽ̨��ˮ
     *****************************************/
    ret = db_exec("UPDATE acctjour SET revserial='%s',result='2',reserved2='%s' %s",
            XMLGetNodeVal(doc, "//opHostSerial"), XMLGetNodeVal(doc, "//opHreserved5"), where);

    return 0;
}

/*
 * ���˼���(��ʹ��,ֻ��Ϊ�ο�)
 * ����:opDoc ƽ̨���� p ����
 * ����:ƽ̨������
 */
#if 0
int PF10_102(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char tmp[128] = {0};
    char *ptmp = NULL;
    char dcflag = 0;
    int tcResult = 0;
    int notetype = 0;
    int truncflag = 0;

    BKINFO("������ͨѶǰ���־:%s", p);

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 

    if (OP_TCTCODE == 7)
        return tpHandle(dcflag, doc, p);

    if (p[0] == COMMTOPH_BEFORE[0])    //������ͨѶǰ
    {
        if (dcflag == OP_CREDITTRAN)
        {
            ptmp = XMLGetNodeVal(doc, "//opPayacct");
            if (ptmp != NULL && ptmp[0] == '*')
                XMLSetNodeVal(doc, "//opPayacct", ptmp+1);
            ret = Accounting(ACCOUNT_OUTCREDIT, doc);  //�������
            XMLSetNodeVal(doc, "//opPayacct", ptmp);
        }
        else if (dcflag == OP_DEBITTRAN)
            BKINFO("�����Ƿ�������ǰ������");
    } 
    else if (p[0] == COMMTOPH_AFTER[0])  //������ͨѶ��
    {
        tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
        if (!tcResult)
        { 
            double fee = 0;
            ptmp = XMLGetNodeVal(doc, dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct");

            BKINFO("�ж�����Ƿ�����˺�[%s]���[%c]", ptmp, dcflag);
            if (ptmp != NULL && ptmp[0] == '*')
                XMLSetNodeVal(doc, (dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct"), ptmp+1);

            if (ptmp != NULL && strlen(ptmp) > 1 && (*ptmp == '1' || (*ptmp == '*' && ptmp[1] == '1'))) { 
                //��������˺�ʵʱ��������
                if ((fee = feeCalculate(dcflag, atoi(XMLGetNodeVal(doc, "//opNotetype")))) > 0) {
                    sprintf(tmp, "%.2lf", fee);
                    XMLSetNodeVal(doc, "//opBankFee", tmp);
                }
            }
        }
        if (dcflag == OP_CREDITTRAN) {
            if (isClearstateUnknowTran(tcResult))
                return E_SYS_COMM;
            if (tcResult) {
                if ((ret = AccountCancel(ACCOUNT_DEBITCZ, doc)) == 0) //������ǳ���
                    return E_APP_ACCOUNTANDCZ;
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            else
                BKINFO("������������ĺ����Ĵ���ɹ�,���ڲ���������");
        } 
        else if (dcflag == OP_DEBITTRAN) {
            if (tcResult) {
                BKINFO("��������Ĵ���ʧ��,���ڲ�����");
                return E_APP_NONEEDACCOUNT;
            } else {
                notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
                if (notetype == 3 || notetype == 4) { //���б�Ʊ,��ʡһ�����л�Ʊ
                    ret = Accounting(ACCOUNT_OUTTRUNCDEBIT, doc); //����輴��Ʊ��
                } else {
                    if ((truncflag = getTruncflag(notetype, dcflag)) < 0)
                        return E_DB_SELECT;

                    if (truncflag)
                        ret = Accounting(ACCOUNT_OUTTRUNCDEBIT, doc); //����������
                    else
                        ret = Accounting(ACCOUNT_OUTNOTRUNCDEBIT, doc); //����ǽ������
                }
                //should return 0 -- add by zhaoxq
                return 0;
            }
        }
    } else 
    {
        BKINFO("������ͨѶǰ���־���Ϸ�");
        return E_SYS_CALL;
    }
    return ret;
}
#endif

/*************************************
flag:
1-�������ȥ��'*'
2-���뽻�ײ�ȥ��'*'
 **************************************/
void ProcAcctNo(char *newacct, char *oldacct, int flag)
{
    char acct[40];
    char *p = NULL;

    sprintf(acct, oldacct);
    if ((p = strchr(acct, '-')) != NULL) {
        memcpy(p, p+1, strlen(p+1));
        acct[strlen(acct)-1] = 0;
    }
    if (flag == 1)
    {

        if (acct[0] == '*')
            strcpy(newacct, acct+1);
        else
            strcpy(newacct, acct);
    }

    strcpy(newacct, acct);
    return;
}

/**************************************
  �ж��ʻ�����
  1--�Թ��ʻ�
  2--�����˻�
  ************************************/
int IsAcctType( char *acctno )
{
    char sBankPer[6]={0};

    sprintf( sBankPer, "%s", GetBankPerFlag() );
    BKINFO( "�����˻���־[%s]....", sBankPer );

    if( (strncmp( acctno, sBankPer, 3 ) == 0) || (strncmp( acctno, "6225", 4 ) == 0) )
        return 2;
    else
        return 1;
}

/*
 * �������
 * ����:opDoc ƽ̨���� p ����
 * ����:ƽ̨������
 */
int PF10_104(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char dcflag = 0;
    int  truncflag = 0;
    int  notetype = 0, classid = 0;
    int  bktcode, opcode;
    char sql[512] = {0};
    char buf[128] = {0};
    char tmp[128] = {0};
    char tmp1[128] = {0};
    char *pp = NULL, *pAcctPath = NULL;
    char *acctno = NULL, *pwd = NULL, *orgid = NULL, *exchno = NULL;
    char sAutoOrg[12+1]={0}, sAutoOper[12+1]={0};
    char sSqlStr[1024]={0};
    char sLimitAmt[16]={0};
    char *pSettlAmt;
    char noteno1[9]={0};
    char noteno2[9]={0};
    int  iAcctFlag = -1;
    char fs_Acct[36] = {0};

    /*��Ʊ����*/
    opcode = atoi(XMLGetNodeVal(doc, "//opTctcode"));
    if( opcode == 7 )
    {
        BKINFO("������Ʊ����...");
        return  TP_Process(opDoc, p);
    }

    BKINFO("���뽻��...");

    dcflag   = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));
    classid  = atoi(XMLGetNodeVal(doc, "//opClassid"));

    // ���Ҷ�Ӧ����ĺ��Ľ�����, ��������־
    sprintf(sql, "select distinct truncflag, reserved2 from notetypemap "
            "where tctype='%02d' and dcflag='%c' and nodeid=%d",
            notetype, dcflag, OP_REGIONID);
    if (db_query_strs(sql, buf, tmp) != 0)
        return E_DB_SELECT;

    truncflag = atoi(buf);
    bktcode = atoi(tmp);
    BKINFO("***truncflag=[%d]|bktcode=[%d]***",truncflag, bktcode);

    // �����ǽ��׸����˺�Ϊ�����˺�, ������ǽ����տ��˺�Ϊ�����˺�
    pAcctPath = (dcflag == OP_DEBITTRAN ? "//opPayacct" : "//opBeneacct");
    ProcAcctNo(tmp, XMLGetNodeVal(doc, pAcctPath), 2);
    iAcctFlag = IsAcctType(tmp);
    // ���ñ����е��˺�
    XMLSetNodeVal(doc, pAcctPath, tmp);

    /************************************************************
     * ����ҵ���ѯĬ�ϵļ��˻����ͼ��˹�Ա
     ************************************************************/
    if( classid == 2 )
    {
        XMLSetNodeVal(doc, "//opInnerBank", GetDefOrg()); //�ڲ�����
        XMLSetNodeVal(doc, "//opOperid", GetDefOper()); //��Ա
    }

    if (notetype == 71 || notetype == 73)
    {
#if 0 
        if (strcmp(XMLGetNodeVal(doc, "//opMac"), 
                    vstrcat("%d", E_SYS_SYDDECRYPT)) == 0)
            return E_SYS_SYDDECRYPT;
#endif

#ifdef TEST_PF
        XMLSetNodeVal(doc, "//opPayacct", "95010134900000011"); //�����˻�(3490==2881)
#else
        memset( sql, 0, sizeof(sql) );
        memset( tmp, 0, sizeof(tmp) );
        /*��3490�ڲ��ʻ�*/
        sprintf(sql, "select distinct clearacct from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp) != 0)
            return E_DB_SELECT;
        BKINFO("����ת���ڲ��ʻ�[%s]...", tmp);
        XMLSetNodeVal(doc, "//opPayacct", tmp); //�ڲ��ʻ�
#endif
        XMLSetNodeVal(doc, "//opZZCZLX", "9"); //ת���ʺ����� 9-�ڲ��ʻ�
        XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-��У�齻������
        XMLSetNodeVal(doc, "//opHUILUU", "3"); //�̶�3
        XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-�Ǵ���,1-����
        XMLSetNodeVal(doc, "//opJIOHLX", "2"); //2-�������

        XMLSetNodeVal(doc, "//opZJZZLX", "1"); //ת���ʺ����� 1-��
        XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //ת���ʺ�����
        XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //ת���ʺ�����
    }
    //else if(notetype == 41) // ��˰���͹����Ľ�˰����, ���Э���
    /*�޸���2012-9-4�����ӶԹ�����ƾ֤(33)���ж�*/
    else if((notetype == 41) || (notetype == 33 )) // ��˰���͹����Ľ�˰����, ���Э���
    {
        if ((ret = ChkAgreement(XMLGetNodeVal(doc, "//opAgreement"), XMLGetNodeVal(doc, "//opPayacct")))!=0)
        {
            BKINFO("��˰ƾ֤У��Э���[%s]�����ʺ�[%s]ʧ��[%d]", XMLGetNodeVal(doc, "//opAgreement"), 
                    XMLGetNodeVal(doc, "//opPayacct"), ret);
            return ret;
        }
        //�����ʻ�
        if( iAcctFlag == 2 )
        {
            //�����˻��Ȳ�ѯ����
            ret = callInterface(ACCOUNT_ACCTINFO_PERSON_KS, doc);
            if ( XMLGetNodeVal(doc, "//opCardName") != NULL)
                XMLSetNodeVal(doc, "//opPayname", XMLGetNodeVal(doc, "//opCardName") );
            BKINFO("�����˺Ż���[%s]", XMLGetNodeVal(doc, "//opPayname")); 
#ifdef TEST_PF
            XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //�ڲ��˻�
#else
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            sprintf(sql, "select distinct clearacct from bankinfo "
                    "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
            if (db_query_strs(sql, tmp) != 0)
                return E_DB_SELECT;
            BKINFO("����ת���ڲ��ʻ�[%s]...", tmp);
            XMLSetNodeVal(doc, "//opBeneacct", tmp); //�ڲ��ʻ�
#endif

            XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-��У�齻������
            XMLSetNodeVal(doc, "//opZZCZLX", "1"); //ת���ʺ����� 1-��
            XMLSetNodeVal(doc, "//opZJZZLX", "9"); //ת���ʺ����� 9-�ڲ��ʻ�
            XMLSetNodeVal(doc, "//opHUILUU", "3"); //�̶�3
            XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-�Ǵ���,1-����
            XMLSetNodeVal(doc, "//opJIOHLX", "3"); //3-����跽
            XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //ת���ʺ�����
            XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //ת���ʺ�����

            bktcode = ACCOUNT_TRAN_PERSON_KS;
            BKINFO("Э���˰�����˻��ۿ�BKCODE[%d]", bktcode);
        }
    }
    else if( notetype == 43 )//�ȵ�9960(sz60),��ѯ�c����Ϣ,��8248�Զ�����
    {
        if( (ret = Accounting(ACCOUNT_INCREDIT_FS, doc)) != E_APP_ACCOUNTSUCC )
        {
            BKINFO("���cҵ��[%d]ʧ��", ACCOUNT_INCREDIT_FS);
            return ret;
        }
        pp = XMLGetNodeVal(doc, "//opTIOZLX");
        if( pp != NULL )
        {
            if( strlen(pp) == 0 )
            {
                BKINFO("���cҵ��[%d]ʧ��[%s]", ACCOUNT_INCREDIT_FS,pp);
                return E_APP_ACCOUNTFAIL;
            }
            else if( pp[0] != '0' )
            {
                BKINFO("���cҵ��[%d]ʧ��[%s]", ACCOUNT_INCREDIT_FS,pp);
                return E_APP_ACCOUNTFAIL;
            }
        }
        else
            return E_APP_ACCOUNTFAIL;
#if 0
        /*��˰ҵ���ѯ�ڲ��ʻ�����*/
        memset( tmp, 0, sizeof(tmp) );
        memset( tmp1, 0, sizeof(tmp1) );
        memset( sql, 0, sizeof(sql) );
        sprintf(sql, "select creditacct, reserved2 from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp, tmp1) != 0)
            return E_DB_SELECT;
        BKINFO("��˰�ڲ��ʻ�[%s]����[%s]...", tmp, tmp1);
        XMLSetNodeVal(doc, "//opBeneacct", tmp);  //��˰�ڲ��ʻ�
        XMLSetNodeVal(doc, "//opBenename", tmp1); //��˰�ڲ��ʻ�����
#endif
    }
    else if (notetype == 72 || notetype == 74)
    {
        strcpy( sLimitAmt, GetAmtLimit());
        pSettlAmt = XMLGetNodeVal(doc, "//opSettlamt");

        BKINFO("***sLimitAmt=[%s]|pSettlAmt=[%s]***", sLimitAmt, pSettlAmt);
        if( atof(pSettlAmt) > atof(sLimitAmt) )
        {
            BKINFO("����ͨ�ҳ���ȡ���޶�...");
            XMLSetNodeVal(doc, "//opBKRetinfo", "����ȡ���޶�");
            return E_APP_ACCOUNTFAIL;
        }
        // ת���������뵽����
        memset( tmp, 0, sizeof(tmp) );
        acctno = XMLGetNodeVal(doc, "//opPayacct");
        pwd    = XMLGetNodeVal(doc, "//opAgreement");

        BKINFO("***AcctNo=[%s]|BankPwd=[%s]***",acctno, pwd);
        if( ret = ConvertPersonKey( 2, acctno, acctno, pwd, NULL, tmp) )
        {
            BKINFO("��������ת����ʧ��[%d]...", ret);
            return ret;
        }

        XMLSetNodeVal(doc, "//opAgreement", tmp); //��������
        XMLSetNodeVal(doc, "//opSYSPINSEED", acctno); 
#ifdef TEST_PF
        XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //�ڲ��˻�
#else
        memset( sql, 0, sizeof(sql) );
        memset( tmp, 0, sizeof(tmp) );
        sprintf(sql, "select distinct clearacct from bankinfo "
                "where nodeid=%d and bankid='%s'", OP_REGIONID, XMLGetNodeVal(doc, "//opInnerBank"));
        if (db_query_strs(sql, tmp) != 0)
            return E_DB_SELECT;
        BKINFO("����ת���ڲ��ʻ�[%s]...", tmp);
        XMLSetNodeVal(doc, "//opBeneacct", tmp); //�ڲ��ʻ�
#endif
        XMLSetNodeVal(doc, "//opMMJYFS", "2"); //2-У�齻������
        XMLSetNodeVal(doc, "//opZZCZLX", "1"); //ת���ʺ����� 1-��
        XMLSetNodeVal(doc, "//opZJZZLX", "9"); //ת���ʺ����� 9-�ڲ��ʻ�
        XMLSetNodeVal(doc, "//opHUILUU", "3"); //�̶�3
        XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-�Ǵ���,1-����
        XMLSetNodeVal(doc, "//opJIOHLX", "3"); //3-����跽
        XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //ת���ʺ�����
        XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //ת���ʺ�����
    }

    if (dcflag == OP_CREDITTRAN)
    {
        //��ѯ����������
        /*
           memset( sql, 0, sizeof(sql) );
           memset( tmp, 0, sizeof(tmp) );
           sprintf(sql, "select distinct orgname  from organinfo "
           "where nodeid=%d and orgid='%s' and orglevel='3'", OP_REGIONID, XMLGetNodeVal(doc, "//opOriginator"));
           BKINFO("����������[%s]...", tmp);
           XMLSetNodeVal(doc, "//opPJFKHM", tmp); //����������
         */
        org_name(XMLGetNodeVal(doc, "//opOriginator"), tmp);
        XMLSetNodeVal(doc, "//opPJFKHM", tmp); //����������
        /*��˰�ʺŲ��Զ�����*/
        acctno = XMLGetNodeVal(doc, "//opBeneacct");
        sprintf(fs_Acct, "%s", GetFSAcctNo() );
        BKINFO("��˰�˻�[%s].....", fs_Acct);
        //if( !strcmp(acctno, "89010155260000408") )
        if( !strcmp(acctno, fs_Acct) )
        {
            BKINFO("�������(��˰�˻�)���ڹ���");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
        }
        else if( notetype == 43 || !truncflag )  //�Զ��ۿ��־
        {
            /*���c(43)ҵ�����������˴���*/
            BKINFO("�������(���������˰)���ڹ���");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
        }
        else
        {
            BKINFO("������������Զ�����");
            XMLSetNodeVal(doc, "//opZDKZBZ", "1");
        }

        XMLSetNodeVal(doc, "//opJIOHLX", "2");
    }
    else if(dcflag == OP_DEBITTRAN)
    {
        if( notetype == 31 )
        {
            if(strlen(XMLGetNodeVal(doc, "//opNoteno")) > 8)
            {
                /*���˺ͺ���ʱ,ƾ֤���ú�8λ(��16λ), ��ѯ����ʱ�ֱ�ŵ�2���ֶ���
                 */
                if(strlen(XMLGetNodeVal(doc, "//opNoteno")) == 17 )
                {
                    sprintf(noteno1,"%.8s",XMLGetNodeVal(doc, "//opNoteno"));
                    sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+9);
                }
                else
                {
                    sprintf(noteno1,"%.8s",XMLGetNodeVal(doc, "//opNoteno"));
                    sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+8);
                }
                XMLSetNodeVal(doc, "//opTJJXTS", noteno1);
                XMLSetNodeVal(doc, "//opNoteno", noteno2);
            }
        }
        if( notetype == 31 ) 
            ret = CheckNoteInfo(ACCOUNT_CHECK_ZFMM, opDoc); //У��֧������
        else if( notetype == 32 )
        {
            if( ret = callInterface(ACCOUNT_QUERY_BPINFO_IN, doc) )
            {
                BKINFO("��ѯ�ǼǱ�ʧ��[%d]...", ret);
                return ret;
            }

            //��ȡ��Ʊ����
            pp = XMLGetNodeVal(doc, "//opBENPLX");
            if( pp != NULL )
            {
                if( strlen(pp) == 0 )
                {
                    BKINFO("��ѯ��Ʊ����ʧ��[%s]...", pp);
                    return E_APP_ACCOUNTFAIL;
                }
                BKINFO("��Ʊ(��Ʊ)����[%s]��Ա��ˮ[%s]��Ч����[%s]...", 
                        pp, XMLGetNodeVal(doc, "//opGUIYLS"), XMLGetNodeVal(doc, "//opSXIORQ"));
                //��ѯ���б�Ʊ��Ѻ
                ret = callInterface( ACCOUNT_QUERY_BPMY, opDoc );
                if( ret )
                {
                    BKINFO("��ѯ��Ʊ������Ѻ��[%d]...", ret);
                    return ret;
                }
                //pp = XMLGetNodeVal(doc, "//opHUIPMY");

                ret = CheckNoteInfo(ACCOUNT_CHECK_BPMY, opDoc); //У�鱾Ʊ��Ѻ
            }
        }

        if( ret )
        {
            BKINFO("Ʊ��У��ʧ��");
            return ret;
        }
        //����
#ifdef TEST_PF_PT
        if( notetype == 31 )
            XMLSetNodeVal(doc, "//opPNGZPH", "g");
        else if( notetype == 32 )
            XMLSetNodeVal(doc, "//opPNGZPH", "c");
#else 
        if( notetype == 31 || notetype == 32 )
        {
            /*ƾ֤���Ų�ѯ*/
            BKINFO("��ʼ��ȡ֧������֧Ʊ����...");
            if (ret = callInterface(ACCOUNT_GET_PH, opDoc))
            {
                BKINFO("��ȡ����ʧ��");
                return ret;
            }
            pp = XMLGetNodeVal(doc, "//opSHFOBZ");
            if( pp != NULL )
            {
                if( !strlen(pp) || atoi(pp) != 1 )
                {
                    BKINFO("��ѯ֧������֧Ʊ����ʧ��SHFOBZ[%s]", pp);
                    return E_APP_ACCOUNTFAIL; 
                }
            }
            else
                return E_APP_ACCOUNTFAIL; 
            /*��ԭƾ֤����, add by lt@sunyard.com 2011-7-12*/
            if( notetype == 31 )
                XMLSetNodeVal(doc, "//opNotetype", "31");
            else if( notetype == 32 )
                XMLSetNodeVal(doc, "//opNotetype", "32");
        }
#endif
        if (!truncflag) 
        {
            BKINFO("����ǽ�������˹�����");
            return 0;
        }
    }
    ret = Accounting(bktcode, doc); 

    if (dcflag == OP_CREDITTRAN)//���Ǵ��׷��سɹ�
        ret = 0;

    return ret;
}

//���������Ʊ����
int TP_Process(void *opDoc, char *p)
{
    xmlDoc *doc = (xmlDoc *)opDoc;
    char dcflag = 0;
    char tblName[16] = "trnjour";
    char *ptmp = NULL;
    int tcResult;
    char tmp[50];
    char *q= NULL, *q1=NULL, *q2=NULL; //
    char sworkdate[9]={0};
    char sAutoOrg[13]={0},sAutoOper[13]={0};
    char sql[1024]={0};

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    BKINFO("��ʼ���������Ʊ����,���[%c]", dcflag);

    ptmp = XMLGetNodeVal(doc, "//opAgreement");
    memcpy( sworkdate, ptmp, 8 );

    if (strcmp(sworkdate, GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");


    if (p == NULL) { //����(�������)
        /**************������ױ���Ʊ**************************************
         *����ԭ������ˮΪ����Ʊ
         *������Ǳ���Ʊ����8248(��������=5,�Զ����˱�־=0,��Ҫ�˹�ȷ������
         ******************************************************************
         *�����Ǳ���Ʊ���ȷ���8992��ѯ���ʷ�ʽ:
         *1:ֱ������(��Ʊ)��8955���,������8956��Ʊ
         ****************************************************************/
        ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c'",
                tblName, OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '1');

        sprintf( sql, "select autooper, autoorg from bankinfo where nodeid=%d and exchno='%s'", 
                OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor") );
        ret = db_query_strs( sql, sAutoOper, sAutoOrg );
        XMLSetNodeVal(doc, "//opInnerBank", sAutoOrg);
        XMLSetNodeVal(doc, "//opInnerorganid", sAutoOrg);
        XMLSetNodeVal(doc, "//opOperid", sAutoOper);
        /*������ո����ʺŻ�ԭ��ԭ���׵��ʺ�*/
        XMLSetNodeVal(doc, "//opSHKRZH", XMLGetNodeVal(doc, "//opPayacct"));
        XMLSetNodeVal(doc, "//opSHKRXM", XMLGetNodeVal(doc, "//opPayname"));
        XMLSetNodeVal(doc, "//opFUKRZH", XMLGetNodeVal(doc, "//opBeneacct"));
        XMLSetNodeVal(doc, "//opFUKRXM", XMLGetNodeVal(doc, "//opBenename"));

        XMLSetNodeVal(doc, "//opInoutflag", "2");
        if (dcflag == OP_CREDITTRAN) { //���������Ʊ
            BKINFO("������Ǳ���Ʊ...");
            XMLSetNodeVal(doc, "//opJIOHLX", "5");
            XMLSetNodeVal(doc, "//opZDKZBZ", "0");
            XMLSetNodeVal(doc, "//opBEIZXX", XMLGetNodeVal(doc, "//opPurpose") );
            ret = Accounting(ACCOUNT_INCREDIT_TP, doc);
        } else if (dcflag == OP_DEBITTRAN) { //����豻��Ʊ
            BKINFO("�����Ǳ���Ʊ...");
            //������Ʊ�͵��칤�����ڼ���(���ڽ�������)
            XMLSetNodeVal(doc, "//opExchgdate", XMLGetNodeVal(doc, "//opWorkdate"));
            memset(tmp, 0, sizeof(tmp));
            ret = db_query_str(tmp, sizeof(tmp), "SELECT revserial FROM acctjour where "
                    "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c' and trncode !='%d'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '1', ACCOUNT_CATCH_FEE);
            BKINFO("ͬ�������[%s]...",tmp);
            XMLSetNodeVal(doc, "//opTCHTCH", tmp);
            if (ret = callInterface(ACCOUNT_QUERY_ACCOUNTTYPE, doc))
            {
                BKINFO("��ѯ���ʷ�ʽʧ��[%d]...",ret);
                return ret;
            } 
            q = XMLGetNodeVal(doc, "//opTJRZFS");
            if( q != NULL )
            {
                if( strlen(q) )
                {
                    BKINFO("���ʷ�ʽ[%s]...",q);
                    //ֱ������(��Ʊ)��8955���,������8956��Ʊ
                    if( q[0] == '1' ) 
                    {
                        BKINFO("ֱ��������������Ʊ...");
                        XMLSetNodeVal(doc, "//opTUIPLX", "0");
                        //XMLSetNodeVal(doc, "//opOriginator", XMLGetNodeVal(doc, "//opAcceptor"));
                        ret = Accounting(ACCOUNT_OUTDEBIT_TPA_BP, doc);
                    }
                    else
                        ret = Accounting(ACCOUNT_OUTDEBIT_TPB, doc);
                }
                else
                {
                    BKINFO("��ѯ���ʷ�ʽʧ��[%s]...",q);
                    return E_APP_ACCOUNTFAIL;
                }
            }
            else
                return E_APP_ACCOUNTFAIL;
        }
    } else if (p[0] == COMMTOPH_BEFORE[0]) {   //������ͨѶǰ(���뽻��)
        /***************************************
         *�����������8247���,�ɹ���8956��Ʊ,
         * 8956���ɹ�����8247�ĳ���,�ɹ�����������
         *����ʧ��������8956�ĳ���,����8247�ĳ���
         *���гɹ�������Ʊ��־=1,���ڲ�������
         *****************************************
         *����������8955���,�ɹ���8956��Ʊ,
         * 8956���ɹ�����8955�ĳ���,�ɹ�����������
         *����ʧ��������8956�ĳ���,����8955�ĳ���
         *���гɹ�������Ʊ��־=1,���ڲ�������
         ***************************************/

#if 1 
        /*************************************************************
         * ��Ʊ�����������ڼ����׷�¼,��������9038����Ҫ����������Ϊ׼
         * ��������������������ڲ�һ��, �������Ʊ�����������ڼ���
         ***********************************************************/


        ret = callInterface( ACCOUNT_GET_DATE, doc );
        q=XMLGetNodeVal(doc, "//opOreserved1");
        if( q != NULL )
        {
            if( strlen(q) )
            {
                BKINFO("����ϵͳ����[%s]", q );
                XMLSetNodeVal(doc, "//opWorkdate", q);
            }
        }
#endif

        /********************************************************
          ��������(��Ʊ���׵ķ�����)�����ڻ�������
          ���п��Դ�������������Ʊ,�����������תΪ���ڻ�����Ʊ
          ������Ҫ�������ȥ����
         *********************************************************/
        /*
           memset(tmp, 0, sizeof(tmp));
           q=XMLGetNodeVal(doc, "//opOriginator");
           q1=XMLGetNodeVal(doc, "//opInnerBank");
           db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", OP_REGIONID, q);
           XMLSetNodeVal(doc, "//opInnerBank",tmp);
         */

        if (dcflag == OP_CREDITTRAN) { //�������Ʊ
            BKINFO("���������Ʊ...");
            //�����͵��칤�����ڼ���(���ڽ�������)
            XMLSetNodeVal(doc, "//opExchgdate", XMLGetNodeVal(doc, "//opWorkdate"));
            if ((ret = Accounting(ACCOUNT_OUTCREDIT_TPA, doc)) == E_APP_ACCOUNTSUCC)
            {
                XMLSetNodeVal(doc, "//opTCHTCH", XMLGetNodeVal(doc, "//opOreserved3"));
                BKINFO("���������Ʊ,����[%d]�ɹ�,ͬ�������[%s],��ʼ[%d]...", ACCOUNT_OUTCREDIT_TPA, 
                        XMLGetNodeVal(doc, "//opTCHTCH"),ACCOUNT_OUTCREDIT_TPB);

                ret = Accounting(ACCOUNT_OUTCREDIT_TPB, doc);
                /*8956���ɹ�����8247�ĳ���*/
                if( ret != E_APP_ACCOUNTSUCC )
                {
                    //XMLSetNodeVal(doc, "//opInnerBank",q1); //��ԭ����
                    BKINFO("���������Ʊ,����[%d]ʧ��,��ʼ[%d]����...", ACCOUNT_OUTCREDIT_TPB, ACCOUNT_OUTCREDIT_TPA);

                    if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPA, 2, doc)) == 0) //����(8247)
                        return E_APP_ACCOUNTANDCZ;
                    else
                    {
                        BKINFO("[%d]����ʧ��[%d]...", ACCOUNT_OUTCREDIT_TPA, ret);
                        return E_APP_ACCOUNTNOCZ;
                    }
                }
            }
        } else if (dcflag == OP_DEBITTRAN) { //�������Ʊ
            /**********************************************
             * ���ݹ�Ա��ѯ�����ڵ�Ӫҵ����
             **************************************************************
             *��Ʊ���������޸�    2011-04-06
             *����89����,���Ϊ89��ͷ���ͱ���������,������Ϊ������ͬ�ǻ���
             *����ֱ���ͷ���������ϼ���������,������Ϊ�ϼ�����ͬ�ǻ���
             ***************************************************************
             *���Ϲ���region���洦��  OP_DoInit
             *���Ϊ8955����ԭ���׵Ľ��ջ���ȥ���ڼ���
             *8956���������Ļ�������
             **************************************************************/
            BKINFO("��������Ʊ...");
            //��ԭ���׵Ĺ������ں�̨����(��������)
            XMLSetNodeVal(doc, "//opExchgdate", sworkdate);

            q=XMLGetNodeVal(doc, "//opQISHHH");
            q1=XMLGetNodeVal(doc, "//opInnerBank");
            q2=XMLGetNodeVal(doc, "//opOriginator");

            //OP_DoInit �����,��ԭ���������������
            memset(tmp, 0, sizeof(tmp));
            db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", OP_REGIONID, q);

            BKINFO("�����Ǳ���Ʊ,ͬ�ǻ���[%s],�����ڻ���[%s]ȥ����[%d]����...", q, tmp, ACCOUNT_OUTDEBIT_TPA);
            XMLSetNodeVal(doc, "//opInnerBank",tmp);
            XMLSetNodeVal(doc, "//opOriginator",XMLGetNodeVal(doc, "//opQISHHH"));
            if ((ret = Accounting(ACCOUNT_OUTDEBIT_TPA, doc)) == E_APP_ACCOUNTSUCC)
            {
                XMLSetNodeVal(doc, "//opInnerBank",q1);
                XMLSetNodeVal(doc, "//opOriginator",q2);
                XMLSetNodeVal(doc, "//opTCHTCH", XMLGetNodeVal(doc, "//opOreserved3"));
                BKINFO("��������Ʊ,����[%d]�ɹ�,ͬ�������[%s]��ʼ[%d]...", ACCOUNT_OUTDEBIT_TPA,
                        XMLGetNodeVal(doc, "//opTCHTCH"),ACCOUNT_OUTDEBIT_TPB);
#if 0
                if( ret = callInterface(ACCOUNT_QUERY_ORG, doc) )
                {
                    BKINFO("��ѯӪҵ����ʧ��[%d]...", ret);
                    return ret;
                }

                q=XMLGetNodeVal(doc, "//opHostSerial");
                if(strncmp(q,"89",2))
                {
                    BKINFO("����ҵ�������Ա[%s]���ڻ���[%s]...", XMLGetNodeVal(doc, "//opOperid"), q);
                    memset(tmp, 0, sizeof(tmp));
                    ret = db_query_str(tmp, sizeof(tmp), "SELECT parent FROM bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, q1 );
                    BKINFO("[%s]ͬ��֧��,��[%s]��Ʊ...", q, tmp);
                    XMLSetNodeVal(doc, "//opInnerBank", tmp);
                }
                else
                    XMLSetNodeVal(doc, "//opInnerBank",q);
#endif
                ret = Accounting(ACCOUNT_OUTDEBIT_TPB, doc);
                /*8956���ɹ�����8955�ĳ���  ????*/
                if( ret !=  E_APP_ACCOUNTSUCC )
                {
                    XMLSetNodeVal(doc, "//opInnerBank", tmp); //��ԭ����
                    XMLSetNodeVal(doc, "//opOriginator",q);
                    BKINFO("��������Ʊ,����[%d]ʧ��,��ʼ[%d]����...", ACCOUNT_OUTDEBIT_TPB, ACCOUNT_OUTDEBIT_TPA);
                    if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPA, 2, doc)) == 0) //����(8955)
                        return E_APP_ACCOUNTANDCZ;
                    else
                        return E_APP_ACCOUNTNOCZ;
                }
            }
        }
    }
    else if (p[0] == COMMTOPH_AFTER[0])  { //������ͨѶ��
        tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
        BKINFO("�����Ʊ���з���[%d]", tcResult);
        if (!tcResult)
        {
            BKINFO("�����Ʊ���гɹ�,������Ʊ��־,���ڲ�������");
            ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                    "nodeid=%d and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='%c'",
                    tblName, OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"), ptmp+9, ptmp, '2');
        }
        else
        {
            if(dcflag == OP_CREDITTRAN)//������Ʊ����ʧ����������
            {
                BKINFO("���������Ʊ����ʧ��,���ڳ���...");
                if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPB, 3, doc)) == 0) //����(8956)
                {
                    BKINFO("���������Ʊ����ʧ��,����[%d]�����ɹ�,��ʼ[%d]����...", ACCOUNT_OUTCREDIT_TPB, ACCOUNT_OUTCREDIT_TPA);
                    if ((ret = AccountCancel(ACCOUNT_OUTCREDIT_TPA, 3, doc)) == 0) //����(8247)
                        return E_APP_ACCOUNTANDCZ;
                    else 
                        return E_APP_ACCOUNTNOCZ;
                }
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            else if( dcflag == OP_DEBITTRAN ) //�����Ʊ����ʧ��
            {
                BKINFO("��������Ʊ����ʧ��,���ڳ���...");
                if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPB, 3, doc)) == 0) //����(8956)
                {
                    BKINFO("��������Ʊ����ʧ��,����[%d]�����ɹ�,��ʼ[%d]����...", ACCOUNT_OUTDEBIT_TPB, ACCOUNT_OUTDEBIT_TPA);
                    memset(tmp, 0, sizeof(tmp));
                    db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", 
                            OP_REGIONID, XMLGetNodeVal(doc, "//opQISHHH"));

                    XMLSetNodeVal(doc, "//opInnerBank",tmp);
                    XMLSetNodeVal(doc, "//opOriginator",XMLGetNodeVal(doc, "//opQISHHH"));
                    if ((ret = AccountCancel(ACCOUNT_OUTDEBIT_TPA, 3, doc)) == 0) //����(8955)
                        return E_APP_ACCOUNTANDCZ;
                    else 
                        return E_APP_ACCOUNTNOCZ;
                }
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
        }
    }

    return ret;
}


int BeforeTransOut(int bktcode, char dcflag, xmlDocPtr doc)
{
    char pwd[128];
    char tmp[256];
    char *acctno = NULL;
    int  iFeeFlag=-1;
    int  classid=-1;
    char *p =NULL;
    char *acctname = NULL;

    if( strlen(XMLGetNodeVal(doc, "//opSHFEBZ")) )
        iFeeFlag = atoi(XMLGetNodeVal(doc, "//opSHFEBZ"));
    classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
    if (dcflag == OP_DEBITTRAN)
    {
        //�����ǶԹ�ҵ��ҪȥУ�鱾���˻������Ƿ���ȷ
        if( classid == 1 )
        {
            XMLSetNodeVal(doc, "//opZHANGH", XMLGetNodeVal(doc, "//opBeneacct")); //�տ��ʻ�
            if( ret = callInterface(ACCOUNT_ACCTINFO_PUB, doc) )
                return ret;
            acctname = XMLGetNodeVal(doc, "//opKEHZWM");
            /*
               XMLSetNodeVal(doc, "//opAcctNo", XMLGetNodeVal(doc, "//opBeneacct")); //�տ��ʻ�
               if( ret = callInterface(ACCOUNT_ACCTINFO_PERSON, doc) )
               return ret;
               acctname = XMLGetNodeVal(doc, "//opCardName");
             */

            p= XMLGetNodeVal(doc, "//opBenename");
            if( acctname != NULL )
            {
                if( strcmp( acctname, p ) )
                {
                    sprintf( tmp, "���������:�����ύ[%s],����[%s]", p, acctname );
                    BKINFO( "%s...", tmp);
                    XMLSetNodeVal(doc, "//opBKRetinfo", tmp);
                    return E_APP_NONEEDACCOUNT;
                }
            }
            else
            {
                sprintf( tmp, "�ʻ�������ѯʧ��" );
                BKINFO( "%s...", tmp);
                XMLSetNodeVal(doc, "//opBKRetinfo", tmp);
                return E_APP_NONEEDACCOUNT;
            }
        }

        if( iFeeFlag == 1 )
        {
            BKINFO("�����Ƿ�������ǰ�ȿۿͻ�������...");
            if( (ret = Accounting( ACCOUNT_CATCH_FEE, doc ) )!= E_APP_ACCOUNTSUCC )
            {
                BKINFO("�ۿͻ�������ʧ��[%d]...", ret);
                return ret;
            }
        }
        BKINFO("�����Ƿ�������ǰ���ڲ�����...");
        return 0;
    }

    // ������ǽ��׸����˺�Ϊ�����˺�, ȥ��'*'�ŵ�
    acctno = XMLGetNodeVal(doc, "//opPayacct");
    ProcAcctNo(tmp, acctno, 1 );
    XMLSetNodeVal(doc, "//opPayacct", tmp);

    ret = Accounting(bktcode, doc);

    // ���ԭ�˺�
    XMLSetNodeVal(doc, "//opPayacct", acctno);

    return ret;
}

int AfterTransOut(int bktcode, char dcflag, xmlDocPtr doc)
{
    char tmp[40]   = {0};
    char tmp1[40]  = {0};
    char sql[512]  = {0};
    int  tcResult  = 0;
    int  truncflag = 0;
    int  ret       = 0;
    int  iFeeFlag  = -1;
    int  notetype, oldflag=-1;
    char *orgid    = NULL, *acctno = NULL, *acctname = NULL;
    char *p        = NULL;

    tcResult = atoi(XMLGetNodeVal(doc, "//opTCRetcode"));
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    if (dcflag == OP_CREDITTRAN)
    {
        if (isClearstateUnknowTran(tcResult))
            return E_SYS_COMM;
        ret = tcResult;
        p = XMLGetNodeVal(doc, "//opReserved");
        if( p != NULL )
            oldflag= atoi(p);

        if (tcResult) {
            if(oldflag == 1)
            {
                //return 0;
                //modify by lt@sunyard.com 2012-6-19
                return E_APP_ACCOUNTFAIL;
            }
            if ((ret = AccountCancel(ACCOUNT_CREDITCZ, 2, doc)) == 0) //������ǳ���(8963)
                return E_APP_ACCOUNTANDCZ;
            else 
                return E_APP_ACCOUNTNOCZ;
        }

        /*��ͨ��ҵ�������гɹ�����±�־*/
        if( oldflag == 1 )
        {
            BKINFO("�������гɹ�,����Ϊ��ͬ��ҵ��..." );
            memset( sql, 0, sizeof(sql) );
            sprintf(sql, "update trnjour set reserved3='1' WHERE nodeid=%d AND workdate='%s' AND "
                    "originator='%s' AND convert(decimal, refid)=%s AND inoutflag='1'",
                    OP_REGIONID, XMLGetNodeVal(doc, "//opWorkdate"),XMLGetNodeVal(doc, "//opOriginator"),
                    XMLGetNodeVal(doc, "//opRefid"));
            ret = db_exec( sql );
            return ret;
        }
        /*�����ֽ�ͨ��(71)�����гɹ�������8963����*/
        if( notetype == 71 )
        {
#ifdef TEST_PF 
            XMLSetNodeVal(doc, "//opPayacct", "95010133150000098"); //�ڲ��ʻ�
            orgid  = XMLGetNodeVal(doc, "//opOriginator");
            XMLSetNodeVal(doc, "//opOriginator", "622"); //��������
            /*�ڲ���������ѯ*/
            XMLSetNodeVal(doc, "//opZHANGH", "95010133150000098"); //�ڲ��ʻ�
            XMLSetNodeVal(doc, "//opPayname", "����ת���"); //�ڲ��ʻ�
#else
            /*��ѯ����ת���ڲ��ʻ�(3315) returnacct:�ʺ�, reserved1:����*/
            orgid  = XMLGetNodeVal(doc, "//opOriginator");
            sprintf(sql, "select distinct returnacct, reserved1 from bankinfo where nodeid=%d and  exchno='%s' ", OP_REGIONID, orgid );
            if (db_query_strs(sql, tmp, tmp1) != 0)
                return E_DB_SELECT;

            BKINFO("����ת���ڲ��ʻ�[%s]����[%s]...", tmp, tmp1);
            XMLSetNodeVal(doc, "//opPayacct", tmp); //�ڲ��ʻ�
            XMLSetNodeVal(doc, "//opPayname", tmp1); //�ڲ��ʻ�����
            /*
            //�ڲ���������ѯ
            XMLSetNodeVal(doc, "//opZHANGH", tmp); //�ڲ��ʻ�
            if( ret = callInterface(ACCOUNT_ACCTINFO_PUB, doc) )
            return ret;
            acctname = XMLGetNodeVal(doc, "//opKEHZWM");
            if( acctname != NULL )
            {
            if ( acctname[0] != '\0' )
            {
            BKINFO("�ڲ��ʻ�����[%s]...", acctname);
            XMLSetNodeVal(doc, "//opPayname", XMLGetNodeVal(doc, "//opKEHZWM") ); //�ڲ��ʻ�����
            }
            else
            {
            BKINFO("�ڲ��ʻ�����[%s]��ѯʧ��...", acctname);
            return -1;
            }
            }
            else
            {
            BKINFO("�ڲ��ʻ�����[%s]��ѯʧ��...", acctname);
            return -1;
            }
             */
#endif

            XMLSetNodeVal(doc, "//opPNGZZL", "zz"); //ƾ֤����
            BKINFO("����ҵ�����Ĵ���ɹ�,����[%d]����...", ACCOUNT_OUTCREDIT);
            if( (ret = Accounting(ACCOUNT_OUTCREDIT_PERSON, doc)) != E_APP_ACCOUNTSUCC )
            {
                BKINFO("����ת�������������[%d]ʧ��...", ACCOUNT_OUTCREDIT_PERSON);
#ifdef TEST_PF
                //��ԭ�������(����ʹ��)
                XMLSetNodeVal(doc, "//opOriginator", orgid); //��������
#endif
                return ret;
            }
#ifdef TEST_PF
            //��ԭ�������(����ʹ��)
            XMLSetNodeVal(doc, "//opOriginator", orgid); //��������
#endif
        }
        else
            BKINFO("��������Ĵ���ɹ�,���ڲ�����");
    } 
    else if (dcflag == OP_DEBITTRAN) 
    {
        if( strlen(XMLGetNodeVal(doc, "//opSHFEBZ")) )
            iFeeFlag = atoi(XMLGetNodeVal(doc, "//opSHFEBZ"));
        if (tcResult != 0) 
        {
            if( iFeeFlag == 1 )
            {
                BKINFO("��������Ĵ���ʧ��,����[%d]����", ACCOUNT_CATCH_FEE);
                if ((ret = AccountCancel(ACCOUNT_CATCH_FEE, 2, doc)) == 0) //�����ѳ���(9579)
                    return E_APP_ACCOUNTANDCZ;
                else 
                    return E_APP_ACCOUNTNOCZ;
            }
            BKINFO("��������Ĵ���ʧ��,���ڲ�����");
            return E_APP_NONEEDACCOUNT;
        } 

        /******************************
          72(ͨ��)-��8951,��4202
          74(ת��)-8227
         ******************************/
        if(notetype == 74 || notetype == 72)
        {
#ifdef TEST_PF
            if(notetype==74)
                XMLSetNodeVal(doc, "//opPayacct", "95010134900000011"); //�����˻�
            else
                XMLSetNodeVal(doc, "//opBeneacct", "95010133150000098"); //�ڲ��˻�
#else
            /*��ѯ����ת���ڲ��ʻ�*/
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            memset( tmp1, 0, sizeof(tmp1) );
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
            /*72��3315,74��3490(2881)*/
            if(notetype == 72 )
                sprintf(sql, "select distinct returnacct, reserved1 from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            else
                sprintf(sql, "select distinct clearacct, clearacct from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            if (db_query_strs(sql, tmp, tmp1) != 0)
                return E_DB_SELECT;

            BKINFO("����ת���ڲ��ʻ�[%s]����[%s]...", tmp, tmp1);
            if(notetype==74)
                XMLSetNodeVal(doc, "//opPayacct", tmp); //�����˻�
            else
            {
                XMLSetNodeVal(doc, "//opBeneacct", tmp); //�ڲ��˻�
                XMLSetNodeVal(doc, "//opBenename", tmp1); //�ڲ��˻�����
            }
#endif
            /*��8951�ڲ��ʻ��տ�,4202-�ֽ��*/
            if(notetype == 72 )
            {
#ifdef TEST_PF
                orgid  = XMLGetNodeVal(doc, "//opOriginator");
                XMLSetNodeVal(doc, "//opOriginator", "622"); //��������
                XMLSetNodeVal(doc, "//opZHANGH", "95010133150000098"); //�ڲ��ʻ�
                XMLSetNodeVal(doc, "//opBenename", "����ת���"); //�ڲ��ʻ�
#endif
                XMLSetNodeVal(doc, "//opPNGZZL", "zz"); //ƾ֤����
                XMLSetNodeVal(doc, "//opTJRZFS", "1");  //1-ֱ������
                BKINFO("�������������Ĵ���ɹ�,������[%d]����...", ACCOUNT_OUTNOTRUNCDEBIT_PERSON);
                if( (ret = Accounting(ACCOUNT_OUTNOTRUNCDEBIT_PERSON, doc)) != E_APP_ACCOUNTSUCC )
                {
                    BKINFO("����ת������������[%d]ʧ��...", ACCOUNT_OUTNOTRUNCDEBIT_PERSON );
                    return ret;
                }
#ifdef TEST_PF
                //��ԭ�������(����ʹ��)
                XMLSetNodeVal(doc, "//opOriginator", orgid); //��������
#endif

#ifdef TEST_PF
                XMLSetNodeVal(doc, "//opPayacct", "95010133150000098"); //�ڲ��˻�
#else
                /*4202��ɸ����ʻ�Ϊ�ڲ��ʻ�*/
                XMLSetNodeVal(doc, "//opPayacct", tmp); //�ڲ��˻�
#endif
            }
            else
            {

                XMLSetNodeVal(doc, "//opZZCZLX", "9"); //ת���ʺ����� 9-�ڲ�
                XMLSetNodeVal(doc, "//opHUILUU", "3"); //�̶�3
                XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-�Ǵ���,1-����
                XMLSetNodeVal(doc, "//opJIOHLX", "1"); //�̶�1-����跽
                XMLSetNodeVal(doc, "//opZJZZLX", "1"); //ת���ʺ����� 1-��
                XMLSetNodeVal(doc, "//opMMJYFS", "0"); //0-��У�齻������
                XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //ת���ʺ�����
            }
        }
        ret = Accounting(bktcode, doc); 
    }

    return ret;
}

// ��ȡ������
int CollectFee(int notetype, char dcflag, xmlDocPtr doc)
{
    double fee         = 0;
    char   *pAcctPath  = NULL;
    char   acctno[40]  ={0};
    char   tmp[20]     ={0};

    pAcctPath = (dcflag == OP_CREDITTRAN ? "//opPayacct" : "//opBeneacct");
    ProcAcctNo(acctno, XMLGetNodeVal(doc, pAcctPath), 1);

    BKINFO("�ж�����Ƿ�����˺�[%s]���[%c]", acctno, dcflag);
    if (acctno != NULL && strlen(acctno) > 1 
            && (*acctno == '1' || (*acctno == '*' && acctno[1] == '1'))) 
    {
        //��������˺�ʵʱ��������
        if ((fee = feeCalculate(dcflag, notetype)) > 0) {
            sprintf(tmp, "%.2lf", fee);
            XMLSetNodeVal(doc, "//opBankFee", tmp);
        }
    }

    return 0;
}

/*
 * �������
 * ����:opDoc ƽ̨���� p ����
 * ����:ƽ̨������
 */
int PF10_108(void *opDoc, char *p)
{
    xmlDoc *doc      = (xmlDoc *)opDoc;
    char   sql[512]  = {0};
    char   tmp[128]  = {0}; 
    char   tmp1[128] = {0};
    char   dcflag;
    char   *acctno   = NULL, *oacctno = NULL, *dacctno = NULL;
    char   *pwd      = NULL, *orgid = NULL, *noteno = NULL, *pin = NULL;
    int    notetype;
    int    bktcode, oldflag, opcode;
    char   *pzFlag   =NULL;

    /*��Ʊ����*/
    opcode = atoi(XMLGetNodeVal(doc, "//opTctcode"));
    if( opcode == 7 )
    {
        BKINFO("�����Ʊ...");
        return  TP_Process(opDoc, p);
    }

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    // ���Ҷ�Ӧ��������Ľ�����//add distinct @ 2011.01.20
    sprintf(sql, "select distinct reserved1 from notetypemap "
            "where tctype='%02d' and dcflag='%c' and nodeid=%d",
            notetype, dcflag, OP_REGIONID);
    if (db_query_strs(sql, tmp) != 0)
        return E_DB_SELECT;
    bktcode = atoi(tmp);

    if (p[0] == COMMTOPH_BEFORE[0]) 
    { 
        BKINFO("�����������: �����־=[%c] Ʊ������=[%02d] ������ͨѶǰ...",
                dcflag, notetype);
        /*������� ��������ǰ̨����,ƽֱ̨�ӷ������ں�̨����
          ������ ת���ܸ�������(ƽ̨�������ڼ��ܻ�)
         */

        oldflag= atoi(XMLGetNodeVal(doc, "//opReserved"));
        if( oldflag == 1 )
        {
            BKINFO("��ͬ��ҵ��,���ڲ�����..." );
            return 0;
        }

        if (notetype == 72 || notetype == 74)
        {
            /*******************************************************
             *�������ת��(74)��8227���ڼ���
             *******************************************************
             *�������ͨ��(72)�������д���ɹ����ȵ�8951���ڼ���(�ڲ��ʻ�)
             *�ɹ�����4202����(�ڲ��ʻ�)
             *******************************************************/
            memset(tmp, 0, sizeof(tmp));
            dacctno = XMLGetNodeVal(doc, "//opPayacct");
            pwd     = XMLGetNodeVal(doc, "//opAgreement");
            orgid   = XMLGetNodeVal(doc, "//opInnerBank");
            oacctno = XMLGetNodeVal(doc, "//opSYSPINSEED");

            if( ret = ConvertPersonKey( 1, oacctno, dacctno, pwd, orgid, tmp) )
                return ret;
            XMLSetNodeVal(doc, "//opAgreement", tmp);
        }
        else if (notetype == 73 || notetype == 71)//�����������
        {
            /*******************************************************
             *�������ת��(73)��8227���ڼ���
             *******************************************************
             *�������ͨ��(71)�ȵ�4201���ڼ���(�����ʻ�3315)
             *���гɹ�����8963����(�ڲ��ʻ�3315��3490)
             *******************************************************/
            memset(tmp, 0, sizeof(tmp));
            acctno = XMLGetNodeVal(doc, "//opBeneacct");
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
            pwd    = XMLGetNodeVal(doc, "//opAgreement");
            orgid  = XMLGetNodeVal(doc, "//opInnerBank");
#ifdef TEST_PF
            if( notetype == 71 )
                XMLSetNodeVal(doc, "//opBeneacct", "95010133150000098"); //�ڲ��ʻ�(3315)
            else 
                XMLSetNodeVal(doc, "//opBeneacct", "95010134900000011"); //�ڲ��ʻ�(3490)
#else
            /*ѯ����ת���ڲ��ʻ�*/
            memset( sql, 0, sizeof(sql) );
            memset( tmp, 0, sizeof(tmp) );
            /*ͨ��(71)��3315�Ĺ����ʻ�,ת��(73)��3490���ڲ��ʻ�*/
            if( notetype == 71 )
                sprintf(sql, "select distinct returnacct from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            else
                sprintf(sql, "select distinct clearacct  from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
            if (db_query_strs(sql, tmp) != 0)
                return E_DB_SELECT;
            BKINFO("����ת���ڲ��ʻ�[%s]...", tmp);
            XMLSetNodeVal(doc, "//opBeneacct", tmp); //�ڲ��ʻ�
            XMLSetNodeVal(doc, "//opPurpose", acctno); //�����տ��ʻ�����ʱ�ʻ�
#endif
            XMLSetNodeVal(doc, "//opZZCZLX", "1"); //ת���ʺ����� 1-��
            XMLSetNodeVal(doc, "//opZJZZLX", "9"); //ת���ʺ����� 9-�ڲ��ʻ�
            XMLSetNodeVal(doc, "//opMMJYFS", "2"); //2-У�齻������
            XMLSetNodeVal(doc, "//opHUILUU", "3"); //�̶�3
            XMLSetNodeVal(doc, "//opSHFOBZ", "0"); //0-�Ǵ���,1-����
            XMLSetNodeVal(doc, "//opJIOHLX", "0"); //0-�������
            XMLSetNodeVal(doc, "//opZRZHXZ", "0001"); //ת���ʺ�����
            XMLSetNodeVal(doc, "//opZHHUXZ", "0001"); //ת���ʺ�����
        }
        else if( notetype == 43 ) 
        {
            //ƾ֤����
            XMLSetNodeVal(doc, "//opPNGZZL", "zz");
            XMLSetNodeVal(doc, "//opNoteno", XMLGetNodeVal(doc, "//opBEIZXX"));
        }
        else if( notetype == 42 )
        {
            //��Ϊg6��ʱ������ڲ��˻��ۿ�
            pzFlag = XMLGetNodeVal(doc, "//opPNG1ZL");
            if( !strcmp(pzFlag, "g6") )
            {
                //����˰Ʊ���ڲ��ʻ��ۿ�,reserved:����,debitacct:�ڲ��ʻ�
                orgid   = XMLGetNodeVal(doc, "//opInnerBank");
                memset( sql, 0, sizeof(sql) );
                sprintf(sql, "select distinct debitacct, reserved from bankinfo where nodeid=%d and bankid='%s'", OP_REGIONID, orgid);
                if (db_query_strs(sql, tmp, tmp1) != 0)
                    return E_DB_SELECT;
                BKINFO("����˰Ʊ�ڲ��ʻ�[%s]����[%s]...", tmp, tmp1);
                XMLSetNodeVal(doc, "//opPayacct", tmp); //�ڲ��ʻ�
                XMLSetNodeVal(doc, "//opPayname", tmp1); //�ڲ��ʻ�����
            }
        }

        ret = BeforeTransOut(bktcode, dcflag, doc);
    } 
    else if (p[0] == COMMTOPH_AFTER[0])  
    { 
        BKINFO("�����������: �����־=[%c] Ʊ������=[%d] ������ͨѶ��...",
                dcflag, notetype);

        if (notetype == 73 || notetype == 71)//�����������
        {
            acctno = XMLGetNodeVal(doc, "//opPurpose");
            XMLSetNodeVal(doc, "//opBeneacct", acctno); //��ԭ�տ��ʻ�
            BKINFO("����ͬ�������ԭ�տ��ʻ�[%s]", acctno);
        }
        ret = AfterTransOut(bktcode, dcflag, doc);

        if(*XMLGetNodeVal(doc, "//opClassid")=='2')
        {
            BKINFO("��ʼ������˽�����Ϣ���ļ�");
            SavePersonNote(doc);
        }
        if( notetype == 14 )
        {
            BKINFO("��ʼ����˰Ʊ��Ϣ(������Ϣ)���ļ�...");
            SaveTaxNote(doc);
            BKINFO("����˰Ʊ��Ϣ(������Ϣ)���ļ�����...");
        }
        /*
        // ��ȡ������
        if (atoi(XMLGetNodeVal(doc, "//opTCRetcode")) == 0)
        CollectFee(notetype, dcflag, doc);
         */
    }

    return ret;
}

/*
 * ������(��Ʊ)ȷ��
 * ���� opDoc ƽ̨���� p ����
 */
int PF10_123(void *opDoc, char *p)
{
    xmlDoc *doc           = (xmlDoc *)opDoc;
    char   where[1024]    = {0};
    char   where1[1024]   = {0};
    char   workdate[9]    = {0};
    char   dcflag         = 0;
    char   tmp[20]        = {0};
    char   bktype[3]      = {0};
    char   *q             = NULL;
    char   noteno[14]     = {0};
    char   *dfamount      = NULL;
    char   exchgdate[9]   = {0}; 
    char   exchground[2]  = {0};

    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0]; 
    strcpy(bktype,XMLGetNodeVal(doc, "//opPNGZZL"));

    strcpy(workdate, XMLGetNodeVal(doc, "//opWorkdate"));
    sprintf(where, "WHERE nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='2'",
            OP_REGIONID, workdate, XMLGetNodeVal(doc, "//opOriginator"),
            sdpStringTrimHeadChar(XMLGetNodeVal(doc, "//opRefid"), '0'));

    BKINFO("����ȷ��:���[%c]����ƾ֤����[%s]", dcflag, bktype);

    XMLSetNodeVal(doc, "//opInoutflag", "2");
    /*ȷ�Ͻ�����ƽ̨��������,���ڽ�������(ƽ̨ԭ���׵Ĺ�������),��������*/
    XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate());
    //ȷ�ϼ�����ԭ�������ڱ��������ˮ
    XMLSetNodeVal(doc, "//opChkflag", "1");
    sprintf( GSQL, "select exchgdate, exchground from %s %s", 
            strcmp(workdate, GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);
    ret = db_query_strs( GSQL, exchgdate, exchground );
    BKINFO("ԭ���׵Ľ�������[%s],��������[%s]", exchgdate, exchground );
    if( ret == 0 )
    {
        XMLSetNodeVal(doc, "//opExchgdate", workdate);
        XMLSetNodeVal(doc, "//opExchground", exchground);
    }

    /*
       ret = callInterface( ACCOUNT_GET_DATE, doc );
       q=XMLGetNodeVal(doc, "//opOreserved1");
       if( strlen(q) )
       {
       BKINFO("����ϵͳ����[%s]", q );
       XMLSetNodeVal(doc, "//opWorkdate", q);
       }
     */

    if (*XMLGetNodeVal(doc, "//opReserved") == '7') 
    { //������Ʊȷ��
        if (dcflag == OP_DEBITTRAN)
            ret = Accounting(ACCOUNT_INDEBIT_TPCHK, doc);
        else if (dcflag == OP_CREDITTRAN)
            ret = Accounting(ACCOUNT_INCREDIT_TPCHK, doc);
    } else {
        if (dcflag == OP_DEBITTRAN) {
            ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s AND result='1'", where);
            if (ret == E_DB_NORECORD)
            {
                /*****************************************
                 *��Ʊ,��Ʊ����8307��ѯ����,�������ڼ���
                 *�жһ�Ʊ��ef14
                 *��ҵ��Ʊ��ef16
                 *����ͬ�������в������ڻ���
                 ****************************************/
                memset( tmp, 0, sizeof(tmp) );
                ret = db_query_str(tmp, sizeof(tmp), "SELECT bankid FROM bankinfo where nodeid=%d and exchno='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
                BKINFO( "*****EXCHNO[%s]-->BANKID[%s]*****", XMLGetNodeVal(doc, "//opAcceptor"), tmp );
                XMLSetNodeVal( doc, "//opInnerorganid", tmp );
                XMLSetNodeVal( doc, "//opInnerBank", tmp );

                if(strcmp(bktype,"12")==0 )
                {
                    if( ret = callInterface(ACCOUNT_QUERY_BPINFO, doc) )
                    {
                        BKINFO("��ѯ�ǼǱ�ʧ��[%d]...", ret);
                        return ret;
                    }

                    q = XMLGetNodeVal(doc, "//opBENPLX");
                    if( strlen(q) == 0 )
                    {
                        BKINFO("��ѯ��Ʊ����ʧ��[%s]...", q);
                        return E_APP_ACCOUNTFAIL;
                    }
                    BKINFO("��Ʊ(��Ʊ)����[%s]...", q);
                    ret = Accounting(ACCOUNT_INDEBITCHK_BP, doc);
                }
                else if(strcmp(bktype,"07")==0)//���л�Ʊ
                {
                    dfamount = XMLGetNodeVal(doc, "//opDUIFJE");
                    if(strlen(dfamount) && atof(dfamount)>0.00)
                    {
                        XMLSetNodeVal(doc, "//opDUIFJE", XMLGetNodeVal(doc, "//opSettlamt"));
                        XMLSetNodeVal(doc, "//opSettlamt", dfamount);
                    }
                    ret = Accounting(ACCOUNT_INDEBITCHK_HP, doc);
                }
                else if(strcmp(bktype,"08")==0)//���гжһ�Ʊ
                {

                    q = sdpXmlSelectNodeText(doc, "//opNoteno");
                    if (q != NULL && strlen(q) > 8) {
                        BKINFO("ƾ֤��̫��,�ض�[%s->%s]", q, p+strlen(q)-8);
                        XMLSetNodeVal(doc, "//opNoteno", q+strlen(q)-8); //����ƾ֤�����8λ(�����8λ)
                    }
                    q = XMLGetNodeVal(doc, "//opPNGZPH");
                    sprintf(noteno,"%4s%1s%8s","0899", q == NULL ? "0":q, 
                            XMLGetNodeVal(doc, "//opNoteno"));
                    //FREE(q);
                    XMLSetNodeVal(doc, "//opPNGZHH", noteno);
                    ret = Accounting(ACCOUNT_INDEBITCHK_CDHP, doc);
                }
                else if(strcmp(bktype,"10")==0)//������ҵ��Ʊ
                    ret = Accounting(ACCOUNT_INDEBITCHK_SYHP, doc);
                else//����
                    ret = Accounting(ACCOUNT_INDEBITCHK_OTHER, doc);
            }
            else
            {
                return E_DB_NORECORD;
            }
        } else if (dcflag == OP_CREDITTRAN) { //�����δ���˿�����ȷ�ϼ���
            ret = db_query_str(tmp, sizeof(tmp), "SELECT result FROM acctjour %s and trncode='%d'", where, ACCOUNT_INCREDIT);
            if( ret == 0 )
            {
                /*ֻ����ȷ8248���˲ſ���ȷ������*/
                if( atoi(tmp) != 5 )
                    return E_DB_NORECORD;
                ret = Accounting(ACCOUNT_INCREDIT_CHK, doc);
            }
            else 
                return E_DB_NORECORD;
        } else
            return E_DB_NORECORD;
    }

    if (ret == E_APP_ACCOUNTSUCC)
        db_exec("UPDATE %s SET chkflag='1' %s", 
                strcmp(workdate, GetArchivedate()) > 0 ? "trnjour" : "htrnjour", where);

    return ret;
}

/*
 * ��������� 
 * ���� opDoc ƽ̨���� p ����
 */
int PF10_122(void *opDoc, char *p) 
{
    xmlDoc      *doc          = (xmlDoc *)opDoc;
    result_set  tpRS, ywRS;
    const char  *pSTDate      = XMLGetNodeVal(doc, "//opWorkdate");
    const char  *pSTRound     = XMLGetNodeVal(doc, "//opWorkround");
    const char  *pOriginator  = XMLGetNodeVal(doc, "//opOriginator");
    const char  *pInfo        = "";
    const char  *ptmp         = NULL;
    char        tblName[10]   = "trnjour";
    int         recordCount   = 0;
    int         i             = 0;

    if (strcmp(pSTDate, GetWorkdate()) > 0) {
        pInfo = "�������ڲ��ܴ��ڵ�ǰ��������";
        goto EXIT;
    }

    if (strcmp(pSTDate, GetExchgdate()) == 0) {
        if (strcmp(pSTRound, GetExchground()) >= 0) {
            pInfo = "���׳��α���С�ڵ�ǰ��������";
            goto EXIT;
        }
        if (atoi(GetExchground()) - atoi(pSTRound) == 1 && *GetSysStat() == '1') {
            pInfo = "�������δ������׳���1����ϵͳΪ����״̬����������";
            goto EXIT;
        }
        if (*GetExchground() == '1' && *GetSysStat() == '1') {
            pInfo = "�������ڵ��ڵ�ǰ���������ҵ�ǰ��������Ϊ1��ϵͳΪ����״̬��������";
            goto EXIT;
        }
    }

    if (strcmp(pSTDate, GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");

    //��ѯ������Ʊ����(�Ƿ�ָ���������?��������������?)
    ret = db_query(&tpRS, "SELECT acceptor, agreement FROM %s WHERE "
            "nodeid=%d and classid=1 and workdate='%s' and inoutflag='2' and workround='%s' and trncode='7'",
            tblName, OP_REGIONID, pSTDate, pSTRound);
    if (ret && ret != E_DB_NORECORD)
        goto EXIT;

    recordCount = db_row_count(&tpRS);
    for (i = 0; i < recordCount; i++)
    {
        ptmp = db_cell_by_name(&tpRS, i, "agreement");
        ret = db_exec("UPDATE %s SET tpflag='1' WHERE "
                "nodeid=%d and classid=1 and originator='%s' and refid='%s' and workdate='%8.8s' and inoutflag='1'",
                tblName, OP_REGIONID, db_cell_by_name(&tpRS, i, "acceptor"), ptmp+9, ptmp);
    }

    //��ѯ�Ѷ�������ǽ�������ҷǱ���Ʊ���׽�������(δ����Ʊ)
    ret = db_query(&ywRS, "SELECT * FROM %s WHERE nodeid=%d and classid=1 and clearstate='C' and truncflag='0' "
            "and dcflag='1' and inoutflag='1' and workdate='%s' and workround='%s' "
            "and originator='%s' and notetype not in('03', '04') and stflag!='1' and tpflag!='1'",
            tblName, OP_REGIONID, pSTDate, pSTRound, pOriginator);
    if (ret)
        goto EXIT;

    recordCount = db_row_count(&ywRS);
    for (i = 0; i < recordCount; i++)
    {
        XMLSetNodeVal(doc, "//opSettlamt",   db_cell_by_name(&ywRS, i, "settlamt"));
        XMLSetNodeVal(doc, "//opNodeid",     db_cell_by_name(&ywRS, i, "nodeid"));
        XMLSetNodeVal(doc, "//opWorkdate",   db_cell_by_name(&ywRS, i, "workdate"));
        XMLSetNodeVal(doc, "//opRefid",      db_cell_by_name(&ywRS, i, "refid"));
        XMLSetNodeVal(doc, "//opInoutflag",  "1");
        XMLSetNodeVal(doc, "//opOriginator", db_cell_by_name(&ywRS, i, "originator"));
        XMLSetNodeVal(doc, "//opBankno",     db_cell_by_name(&ywRS, i, "innerorganid"));
        XMLSetNodeVal(doc, "//opOperid",     db_cell_by_name(&ywRS, i, "acctoper"));
        XMLSetNodeVal(doc, "//opCurcode",    db_cell_by_name(&ywRS, i, "curcode"));
        XMLSetNodeVal(doc, "//opCurtype",    db_cell_by_name(&ywRS, i, "curtype"));
        XMLSetNodeVal(doc, "//opPayname",    db_cell_by_name(&ywRS, i, "payer"));
        XMLSetNodeVal(doc, "//opPaybank",    db_cell_by_name(&ywRS, i, "payingbank"));
        XMLSetNodeVal(doc, "//opBeneacct",   db_cell_by_name(&ywRS, i, "beneacct"));
        XMLSetNodeVal(doc, "//opBenename",   db_cell_by_name(&ywRS, i, "benename"));
        XMLSetNodeVal(doc, "//opNotetype",   db_cell_by_name(&ywRS, i, "notetype"));
        XMLSetNodeVal(doc, "//opNoteno",     db_cell_by_name(&ywRS, i, "noteno"));

        ret = Accounting(ACCOUNT_OUTDEBIT_ST, doc);
        if (ret == E_APP_ACCOUNTSUCC)
        {
            db_exec("UPDATE %s SET stflag='1' WHERE "
                    "nodeid=%d and inoutflag='1' and originator='%s' and refid='%s' and workdate='%s'",
                    tblName, OP_REGIONID, pOriginator, 
                    XMLGetNodeVal(doc, "//opRefid"), XMLGetNodeVal(doc, "//opWorkdate"));
        }
    }

EXIT:
    if (pInfo != NULL)
        XMLSetNodeVal(doc, "//opTCRetinfo", (char *)pInfo);
    return ret;
}

/*
 * ת�˻��ֽ𵥱�������
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_152(void *opDoc, char *p)
{
    xmlDoc *doc    = (xmlDoc *)opDoc;
    char   tmp[64] = {0};
    char   pwd[64] = {0};
    int    isHist;

    if (strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) > 0)
        isHist = 0;
    else
        isHist = 1;
    //�շѷ�ʽ1:�ֽ� 2:ת��
    if (*XMLGetNodeVal(doc, "//opEXBKTaxtype") == '1') 
        ret = fetchFee(ACCOUNT_CASHFEE, doc, isHist);
    else
    {
        OFP_Decrypt(XMLGetNodeVal(doc, "//opAgreement"), pwd);
        //pwd = XMLGetNodeVal(doc, "//opAgreement");
        memset(tmp, 0, sizeof(tmp));
        BKINFO("Ready BankPwdEncrypt(%s)...", pwd);
        if (ret = BankPwdEncrypt(pwd, tmp))
            return ret;
        XMLSetNodeVal(doc, "//opEXBKBankPwd", tmp);
        ret = fetchFee(ACCOUNT_ZZFEE, doc, isHist);
    }

    return ret;
}

/*
 * ����������
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_153(void *opDoc, char *p)
{
    xmlDoc     *doc           = (xmlDoc *)opDoc;
    result_set ywRS;
    double     fee            = 0;
    char       feeDate[9]     = {0}; 
    char       lastFeedate[9] = {0};
    int        succFlag       = 1;
    int        forbitSuccFlag = 0;
    int        recordCount    = 0, i = 0;

    forbitSuccFlag = atoi(XMLGetNodeVal(doc, "//opReserved")); //ǿ�Ƴɹ���־
    strcpy(feeDate, XMLGetNodeVal(doc, "//opWorkdate")); //�շѽ�ֹ����
    strcpy(lastFeedate, GetLastFeedate());

    if (strcmp(feeDate, GetArchivedate()) > 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "�շѽ�ֹ���ڲ��ܴ��ڹ鵵����");
        return 0;
    }
    if (strcmp(feeDate, lastFeedate) <= 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "�շѽ�ֹ���ڱ�������ϴ��շѽ�ֹ����");
        return 0;
    }

    ret = db_query(&ywRS, "SELECT workdate,refid,inoutflag,dcflag,originator,innerorganid,acctoper,"
            "curcode,curtype,payingacct,payer,payingbank,beneacct,benename,benebank,notetype,noteno FROM htrnjour WHERE "
            "nodeid=%d AND workdate between '%s' and '%s' AND inoutflag='1' AND feeflag!='1' AND clearstate='C'",
            OP_REGIONID, lastFeedate, feeDate);

    XMLSetNodeVal(doc, "//opNodeid", vstrcat("%d", OP_REGIONID));

    recordCount = db_row_count(&ywRS);
    for (i = 0; i < recordCount; i++)
    {
        char dcflag = *db_cell(&ywRS, i, 3);

        if ((fee = feeCalculate(dcflag, atoi(db_cell(&ywRS, i, 15)))) < 0)
            return E_DB_SELECT;
        if (fee == 0)
            continue;

        XMLSetNodeVal(doc, "//opSettlamt",   vstrcat("%.2lf", fee));
        XMLSetNodeVal(doc, "//opWorkdate",   db_cell(&ywRS, i, 0));
        XMLSetNodeVal(doc, "//opRefid",      db_cell(&ywRS, i, 1));
        XMLSetNodeVal(doc, "//opInoutflag",  db_cell(&ywRS, i, 2));
        XMLSetNodeVal(doc, "//opOriginator", db_cell(&ywRS, i, 4));
        XMLSetNodeVal(doc, "//opBankno",     db_cell(&ywRS, i, 5));
        XMLSetNodeVal(doc, "//opOperid",     db_cell(&ywRS, i, 6));
        XMLSetNodeVal(doc, "//opCurcode",    db_cell(&ywRS, i, 7));
        XMLSetNodeVal(doc, "//opCurtype",    db_cell(&ywRS, i, 8));
        XMLSetNodeVal(doc, "//opPayacct",    dcflag == '1' ? db_cell(&ywRS, i, 12) : db_cell(&ywRS, i, 9));
        XMLSetNodeVal(doc, "//opPayname",    dcflag == '1' ? db_cell(&ywRS, i, 13) : db_cell(&ywRS, i, 10));
        XMLSetNodeVal(doc, "//opPaybank",    dcflag == '1' ? db_cell(&ywRS, i, 14) : db_cell(&ywRS, i, 11));
        XMLSetNodeVal(doc, "//opBenename",   dcflag == '1' ? db_cell(&ywRS, i, 10) : db_cell(&ywRS, i, 13));
        XMLSetNodeVal(doc, "//opNotetype",   db_cell(&ywRS, i, 15));
        XMLSetNodeVal(doc, "//opNoteno",     db_cell(&ywRS, i, 16));

        if ((ret = fetchFee(ACCOUNT_ZZFEE, doc, 1)) != 0) {
            succFlag = 0;
            BKINFO("��������ʧ��,ret=[%d]refid=[%s]", ret, XMLGetNodeVal(doc, "//opRefid"));
            continue;
        }
    }

    if (succFlag || forbitSuccFlag) {
        //������������շ�����
        if ((ret = UpdLastFeedate(feeDate)) == 0) {
            if (recordCount == 0)
                return E_DB_NORECORD;
        }
    }

    return ret;
}

/*
 * ����¼��
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_154(void *opDoc, char *p)
{
    result_set  rs;
    xmlDoc      *doc           = (xmlDoc *)opDoc;
    int         rc;
    int         i;
    char        line[8192]     = {0};
    char        outline[8192]  = {0};
    char        *fields[256];
    char        caOutFile[256] = {0};
    int         fieldnum;
    char        fname[64];
    sprintf(fname, "%s/%s", getenv("FILES_DIR"), XMLGetNodeVal(doc, "//opReserved"));
    FILE        *fp            = fopen(fname, "r");

    if (fp == NULL)
    {
        SDKerrlog(ERRLOG , "������¼���ļ�ʧ��");
        return -1;
    }

    if (getFilesdirFile(caOutFile) == NULL) 
    {
        BKINFO("������ʱ�ļ�ʧ��");
        return -1;
    }
    FILE *fpout = fopen(caOutFile, "w");

    while(fgets(line, sizeof(line), fp) != NULL)
    {
        if (strlen(line) <= 1)
            continue;

        fieldnum = getcols(line, fields, 256, ',');

        rc = db_query(&rs, "select * from noteinfo where dcflag='%s' "
                "and notetype='%s'", fields[0], fields[3]);
        if (rc != 0)
        {
            continue;
        }


        //���浽���ݿ����
        if (db_exec("INSERT INTO trnjour (dcflag, originator, acceptor, notetype, noteno,"
                    " issuedate, payingacct, payer, payingbank, beneacct, benename, benebank,"
                    " settlamt, purpose,curcode,curtype,refid, clearstate, result, presdate, prestime, inoutflag,"
                    " workdate,nodeid,trncode,classid) VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s',"
                    " '%s','%s','%s',%s,'%s','%s','%s','%s','0',0,'%s','%s','1', '%s',10,'501',1)", 
                    fields[0],fields[1],fields[2],fields[3],fields[4],
                    fields[5],fields[6],fields[7],fields[8],fields[9],fields[10],fields[11],fields[12],
                    fields[13],fields[14],fields[15],fields[16],"now","yeye",GetWorkdate()) != 0)
        {
            SDKerrlog(ERRLOG , "Insert %s (%s) fail.", "trnjour", line);
            fclose(fp);
            return -1;
        }

        memset(outline, 0, sizeof outline);
        for (i = 0; i < fieldnum; i++)
        {
            if (fields[i] != NULL)
                strcat(outline, fields[i]);
            strcat(outline, ",");
        }
        outline[strlen(outline)-1] = '\n';

        if (fputs(outline, fpout) == EOF)
        {
            BKINFO("д����ʱ�ļ�ʧ��");
            continue;
        }
    }
    fclose(fp);
    fclose(fpout);

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));
    return 0;
}

/*
 * ƾ֤����
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_155(void *doc, char *p)
{
    int            ret               = 0;
    xmlDoc         *opDoc            = (xmlDoc *)doc;
    char           sAutoOrg[12+1]    = {0}; 
    char           sAutoOper[12+1]   = {0};
    char           sName1[81]        = {0};
    char           sName2[81]        = {0};
    char           sSqlStr[1024]     = {0};
    char           *q                = NULL;

    ret = callInterface(1607, opDoc);
    if(ret)
        return ret;

    return 0;
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
 * ��ӡ����������
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_780(void *opDoc, char *p)
{
    xmlDoc         *doc            = (xmlDoc *)opDoc;
    result_set     rs, rs2;
    FILE           *fp             = NULL;
    char           tmp[1024]       = {0};
    char           caParaFile[256] = {0}; 
    char           caDataFile[256] = {0};
    char           caOutFile[256]  = {0};
    char           startdate[9]    = {0}; 
    char           enddate[9]      = {0};
    int            recordCount     = 0;
    double         feeSum          = 0;
    int            i               = 0;

    strcpy(startdate, XMLGetNodeVal(doc, "//opStartdate"));
    strcpy(enddate, XMLGetNodeVal(doc, "//opEnddate"));

    BKINFO("��������:%s-%s ϵͳ����:%s ��������:%s", startdate, enddate, GetWorkdate(), getDate(0));

    if (startdate[0] == 0 || enddate[0] == 0)
        return E_PACK_GETVAL;

    ret = db_query(&rs, "SELECT workdate,refid,originator,acctno,amount,"
            "result,a.reserved1,name FROM feelist a,noteinfo "
            "WHERE feedate BETWEEN '%s' AND '%s' AND a.reserved2=notetype "
            "ORDER BY workdate", startdate, enddate);
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s-%s;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++) {
        feeSum += atof(db_cell(&rs, i, 4));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3),
                db_cell(&rs, i, 4),
                db_cell(&rs, i, 7),
                db_cell(&rs, i, 6),
                *db_cell(&rs, i, 5) == '1' ? "�ɹ�" : "ʧ��");
    }

    WriteRptRowCount(fp, recordCount);
    sprintf(tmp, "%.2lf", feeSum);
    WriteRptFooter(fp, "%d;%s;", recordCount, FormatMoney(tmp));
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/FeeList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("������ʱ�ļ����򱨱����ļ�ʧ��!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
 * ��ӡ�����嵥
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_781(void *opDoc, char *p)
{
    xmlDoc            *doc              = (xmlDoc *)opDoc;
    result_set        rs, rs2;
    FILE              *fp               = NULL;
    char              tbname[10]        = "trnjour"; 
    char              tmp[1024]         = {0};
    char              caParaFile[256]   = {0};
    char              caDataFile[256]   = {0}; 
    char              caOutFile[256]    = {0};
    char              settledate[16]    = {0};
    char              currdate[16]      = {0};
    int               classid           = 0; 
    int               clearround        = 0;
    int               recordCount       = 0;
    int               recordCount2      = 0;
    int               i                 = 0;
    int               j                 = 0;

    strcpy(settledate, GetSettledDateround());
    strcpy(currdate, getDate(0));
    classid = atoi(XMLGetNodeVal(doc, "//opClassid"));
    clearround = atoi(XMLGetNodeVal(doc, "//opClearround"));

    BKINFO("����:%d ����:%d ��ȡ�������ڳ���:%s ϵͳ����:%s ��������:%s", 
            classid, clearround, settledate, GetWorkdate(), currdate);
    settledate[8] = 0;

    //�������һ��δ����ʱ���˳�����ǰһ����󳡴�,�������Ϲ鵵ʱ�����˳�����0
    if ((clearround > atoi(settledate+9)) || strcmp(settledate, currdate) != 0) {
        XMLSetNodeVal(doc, "//opTCRetinfo", "δ����,�������ӡ�����嵥");
        return 0;
    }

    ret = db_query(&rs, "SELECT refid, originator, acceptor, inoutflag, dcflag, notetype, noteno,"
            "settlamt, curcode, beneacct, benename, payingacct, payer, clearstate, stflag, chkflag, tpflag, result FROM %s "
            "WHERE classid=%d AND ((originator='%s' and inoutflag='1') OR (acceptor='%s' and inoutflag='2'))"
            "  AND workdate='%s' and clearstate!='0' AND workround='%d' order by workround, clearstate, refid",
            tbname, classid,
            XMLGetNodeVal(doc, "//opOriginator"), XMLGetNodeVal(doc, "//opOriginator"),
            currdate, clearround);
    if (ret != 0)
        return ret;

    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s;%s;%s;%s;%d;%d;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"),
            settledate, classid, clearround, XMLGetNodeVal(doc, "//opOriginator"));

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

        ret = db_query(&rs2, "SELECT acctserial, trncode, result FROM acctjour WHERE " 
                "nodeid=%d AND workdate='%s' AND originator='%s' AND convert(decimal, refid)=%s AND inoutflag='%c'", 
                OP_REGIONID, currdate, db_cell(&rs, i, 1), 
                sdpStringTrimHeadChar(db_cell(&rs, i, 0), '0'), *db_cell(&rs, i, 3));
        if (ret) {
            if (ret == E_DB_NORECORD) {
                fprintf(fp, "%s;%s;%s;%s;%s;%s;\n", tmp, "", "δ����", "��", "", "");
                continue;
            } else {
                BKINFO("��ѯ������ˮʧ��,ret=%d", ret);
                return ret;
            }
        }

        recordCount2 = db_row_count(&rs2);
        for (j = 0; j < recordCount2; j++)
        { 
            fprintf(fp, "%s;%s;%s;%s;%s;%s;\n", tmp,
                    db_cell(&rs2, j, 0), 
                    getAccountDesc(db_cell(&rs2, j, 1)),
                    recordCount2 > 1 ? "��" : "��",
                    db_cell(&rs2, j, 1), "");
        }
    }

    WriteRptRowCount(fp, recordCount);
    WriteRptFooter(fp, "");
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/BankAcctList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("������ʱ�ļ����򱨱����ļ�ʧ��!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
 * ��ӡ������������ϸ
 * ����:opDoc ƽ̨���� p ����
 */
int PF10_782(void *opDoc, char *p)
{
    xmlDoc         *doc                  = (xmlDoc *)opDoc;
    result_set     rs, rs1, rs2;
    FILE           *fp                   = NULL;
    char           tmp[1024]             = {0};
    char           subject[4]            = {0};
    char           chinese[128]          = {0};
    char           caParaFile[256]       = {0};
    char           caDataFile[256]       = {0};
    char           caOutFile[256]        = {0};
    char           startdate[9]          = {0};
    char           enddate[9]            = {0};
    int            recordCount           = 0;
    double         feeSum                = 0;
    int            i                     = 0;

    strcpy(startdate, XMLGetNodeVal(doc, "//opStartdate"));
    strcpy(enddate, XMLGetNodeVal(doc, "//opEnddate"));

    BKINFO("��������:%s-%s ϵͳ����:%s ��������:%s", startdate, enddate, GetWorkdate(), getDate(0));

    if (startdate[0] == 0 || enddate[0] == 0)
        return E_PACK_GETVAL;

    ret = db_query(&rs, "SELECT workdate,refid,inoutflag,originator,acctno,amount,"
            "result,a.reserved1,name FROM feelist a,noteinfo "
            "WHERE feedate BETWEEN '%s' AND '%s' AND a.reserved2=notetype AND result='1' "
            "ORDER BY workdate", startdate, enddate);
    if (ret != 0)
        return ret;


    if (getFilesdirFile(caDataFile) == NULL || (fp = fopen(caDataFile, "w")) == NULL)
        return E_SYS_CALL;

    WriteRptHeader(fp, "%s-%s;%s;", getDate('/'), getTime(':'), XMLGetNodeVal(doc, "//opOperid"));

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++) {
        ret = db_query(&rs1, "SELECT dcflag, noteno, payer, benename, acctoper "
                " FROM htrnjour "
                " WHERE workdate='%s' AND refid='%s' AND inoutflag='%s' AND originator='%s' "
                " ORDER BY workdate", 
                db_cell(&rs, i, 0),
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 2),
                db_cell(&rs, i, 3)
                );

        strncpy(subject, &db_cell(&rs, i, 4)[10, 12], 3);
        MoneyToChinese(db_cell(&rs, i, 5), chinese);
        feeSum += atof(db_cell(&rs, i, 5));
        fprintf(fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n", 
                db_cell(&rs, i, 1),
                db_cell(&rs, i, 0),
                subject,
                *db_cell(&rs1, 0, 0) == '1' ? db_cell(&rs1, 0, 3) : db_cell(&rs1, 0, 2),
                db_cell(&rs, i, 4),
                db_cell(&rs1, 0, 1),
                db_cell(&rs, i, 5), 
                chinese,
                db_cell(&rs1, 0, 4),
                db_cell(&rs, i, 5)); 
    }

    WriteRptRowCount(fp, recordCount);
    sprintf(tmp, "%.2lf", feeSum);
    WriteRptFooter(fp, "%d;%s;", recordCount, FormatMoney(tmp));
    fclose(fp);

    sprintf(caParaFile, "%s/dat/%d/DetailFeeList.para", OP_HOME, TCOP_BANKID);
    if (getFilesdirFile(caOutFile) == NULL || (ret = PrintReportList(caParaFile, caDataFile, caOutFile)) != 0) {
        BKINFO("������ʱ�ļ����򱨱����ļ�ʧ��!ret=%d,%s", ret, OSERRMSG);
        return E_SYS_CALL;
    }

    XMLSetNodeVal(doc, "//opFilenames", basename(caOutFile));

    return 0;
}

/*
   ���ײ�ѯ����������
 */
int PF10_151(void *opDoc, char *p)
{
    xmlDoc        *doc              = (xmlDoc *)opDoc;
    result_set    rs;
    int           recordCount       = 0;
    int           i                 = 0;
    char          acctDesc[1024]    = {0};

    ret = db_query(&rs, "SELECT acctserial, trncode, result FROM acctjour WHERE %s", 
            GetSigleTrnjourWhere(doc));
    if (ret)
        return ret;

    recordCount = db_row_count(&rs);
    for (i = 0; i < recordCount; i++)
    { 
        sprintf(acctDesc+strlen(acctDesc), "|[%s]|[%s]", db_cell(&rs, i, 0), getAccountResult(db_cell(&rs, i, 2)) );
    }
    XMLSetNodeVal(doc, "//opTreserved2", acctDesc+1);

    return ret;
}

/*
 * �������
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_157(void *doc, char *p)
{
    int         ret              = 0;
    xmlDoc      *opDoc           = (xmlDoc *)doc;
    char          sql[512]         = {0};
    char          where[512]         = {0};
    char          refid[20]        = {0}; 
    char          originator[12]   = {0};
    char          acctorg[12]      = {0};
    char          acctoper[12]     = {0};
    char          trncode[6]       = {0};
    char          clearstate[2]       = {0};

    sprintf( where, " where nodeid=%d and originator='%s' and workdate='%s' and refid='%s' and inoutflag='2'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opOriginator"),
            XMLGetNodeVal(opDoc, "//opWorkdate"),XMLGetNodeVal(opDoc, "//opRefid")
           );
    memset( sql, 0, sizeof(sql) );
    sprintf( sql, "select clearstate from trnjour %s", where );
    ret =  db_query_str(clearstate, sizeof(clearstate), sql );
    if (ret != 0)
    {
        if( ret == E_DB_NORECORD )
            return E_APP_CZSUCC;
        else
            return E_DB;
    }
    else
    {
        if( atoi(clearstate) != 1 )
            return E_APP_CZSUCC;
    }

    memset( sql, 0, sizeof(sql) );
    sprintf( sql, "select  acctorg, acctoper,trncode from acctjour where nodeid=%d and  workdate='%s' and refid='%s' and inoutflag='%s' and originator='%s'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opRefid"),
            "2",XMLGetNodeVal(opDoc, "//opOriginator"));

    ret = db_query_strs( sql, acctorg, acctoper,trncode);
    if (ret != 0)
    {
        if( ret == E_DB_NORECORD )
            return E_APP_CZSUCC;
        else
            return E_DB;
    }

    BKINFO("****��������Ľ���:����[%s], ��ˮ��[%s], ���ڻ���[%s], ��Ա[%s], ���״���[%s]*****", 
            XMLGetNodeVal(opDoc, "//opWorkdate"), XMLGetNodeVal(opDoc, "//opRefid"), acctorg,acctoper,trncode );

    //�������ڻ���
    XMLSetNodeVal( opDoc, "//opInnerBank", acctorg); 
    XMLSetNodeVal( opDoc, "//opInnerorganid", acctorg); 
    //���˹�Ա
    XMLSetNodeVal( opDoc, "//opPDWSNO", acctoper); 
    XMLSetNodeVal( opDoc, "//opOperid", acctoper); 


    XMLSetNodeVal(opDoc, "//opInoutflag", "2");
    if ((ret = AccountCancel(ACCOUNT_CREDITCZ, 2, doc)) == 0) 
    {
        //������ˮΪʧ��
        db_exec( "update trnjour set clearstate='9' %s", where );
        return 0;
    }
    else 
        return E_APP_CZFAIL;

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
int PF10_111(void *doc, char *p)
{
    int           ret=0;
    xmlDoc        *opDoc           = (xmlDoc *)doc;
    char          sql[512]         = {0};
    char          refid[20]        = {0}; 
    char          originator[12]   = {0};
    char          inoutflag[20]    = {0}; 
    char          trncode[6]       = {0};
    char          termserial[20]   = {0};
    char          workdate[10]     = {0};
    char          hworkdate[10]    = {0};
    char          *pSerial         = NULL;
    char          *pBank           = NULL;

    /****************************************************************
      cop�����������Ľ���Ϊ����ҵ�����ҵ��, ���������Ϊ"8987", 
      add by lt@sunyard.com 2011-07-05
     ****************************************************/
    pSerial = XMLGetNodeVal(opDoc, "//opRefid");
    if( pSerial != NULL && !strncmp(pSerial, "99975", 5))
    {
        pBank = XMLGetNodeVal(opDoc, "//opInnerBank");
        if( strcmp(pBank, "8987") != 0 )
            XMLSetNodeVal( opDoc, "//opInnerBank", "8987"); 
    }
    //add end by lt@sunyard.com 2011-07-05
    //ԭ���׹�������
    strcpy( hworkdate, XMLGetNodeVal(opDoc, "//opJIOHRQ") );
    //ϵͳ��������
    strcpy( workdate, XMLGetNodeVal(opDoc, "//opWorkdate") );
    sprintf( sql, "select refid, originator, inoutflag, trncode from acctjour where workdate='%s' and acctserial='%s'",
            //XMLGetNodeVal(opDoc, "//opJIOHRQ"), XMLGetNodeVal(opDoc, "//opRefid") );
        hworkdate, XMLGetNodeVal(opDoc, "//opRefid") );

    ret = db_query_strs( sql, refid, originator, inoutflag, trncode, termserial );
    if (ret != 0)
        return E_DB;

    BKINFO("*****�ֹ������Ľ���:WorkDate[%s], RefId[%s], Originator[%s], InOutFlag[%s], TrnCode[%s]*****", 
            workdate, refid, originator, inoutflag, trncode );

    XMLSetNodeVal( opDoc, "//opOriginator", originator ); 
    XMLSetNodeVal( opDoc, "//opInoutflag", inoutflag ); 
    XMLSetNodeVal( opDoc, "//opRefid", refid); 
    //ԭ����������Ϊ��������,�������ڱ��浽opJIOHRQ�ֶ�
    XMLSetNodeVal( opDoc, "//opWorkdate", hworkdate); 
    XMLSetNodeVal( opDoc, "//opJIOHRQ", workdate); 
    XMLSetNodeVal( opDoc, "//opCZFLAG", "1"); 

    if ((ret = AccountCancel(atoi(trncode), 1, doc)) == 0) //������ǳ���
        return E_APP_CZSUCC;
    else 
        return E_APP_CZFAIL;

    return 0;
}

/*
 * Ʊ�ݺϷ��Լ��
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_158(void *doc, char *p)
{
    int         ret             = 0;
    int         notetype        = 0;
    char        *orgid          = NULL;
    char        sSqlStr[1024]   = {0}; 
    char        sAutoOrg[12+1]  = {0};
    char        sAutoOper[12+1] = {0};
    xmlDoc      *opDoc          = (xmlDoc *)doc;

    notetype = atoi(XMLGetNodeVal(doc, "//opNotetype"));

    sprintf(sSqlStr,"select autoorg, autooper from bankinfo where nodeid=%d and exchno='%s'", 
            OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
    ret = db_query_strs(sSqlStr, sAutoOrg, sAutoOper);
    if (ret != 0)
        return E_DB;

    XMLSetNodeVal(opDoc, "//opInnerBank", sAutoOrg);
    XMLSetNodeVal(opDoc, "//opOperid", sAutoOper);

    if( notetype == 31 ) 
        ret = CheckNoteInfo(ACCOUNT_CHECK_ZFMM, opDoc); //У��֧������
    else if( notetype == 32 )
        ret = CheckNoteInfo(ACCOUNT_CHECK_BPMY, opDoc); //У�鱾Ʊ��Ѻ

    /*
       XMLSetNodeVal(opDoc, "//opBKRetcode", "0000");
       XMLSetNodeVal(opDoc, "//opBKRetinfo", "���׳ɹ�");
     */

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
int PF10_405(void *doc, char *p)
{
    int         ret                  = 0;
    int         classid              = 0;
    xmlDoc      *opDoc               = (xmlDoc *)doc;
    char        sAutoOrg[12+1]       = {0};
    char        sAutoOper[12+1]      = {0};
    char        sName1[81]           = {0}; 
    char        sName2[81]           = {0};
    char        sSqlStr[1024]        = {0};
    char        *q                   = NULL;

    if ((q = XMLGetNodeVal(opDoc, "//opAcctName")) != NULL)
        strcpy(sName1, q);


    if ((q = XMLGetNodeVal(opDoc, "//opClassid")) != NULL)
        classid  = atoi(q);
    if( classid == 1 )
    {
        sprintf(sSqlStr,"select autoorg, autooper from bankinfo where nodeid=%d and exchno='%s'", 
                OP_REGIONID, XMLGetNodeVal(doc, "//opAcceptor"));
        ret = db_query_strs(sSqlStr, sAutoOrg, sAutoOper);
        if (ret != 0)
            return E_DB;
        XMLSetNodeVal(opDoc, "//opInnerBank", sAutoOrg);
        XMLSetNodeVal(opDoc, "//opOperid", sAutoOper);
        ret = callInterface(ACCOUNT_ACCTINFO_PUB1, opDoc);
    }
    if( classid == 2 )
    {
        XMLSetNodeVal(doc, "//opInnerBank", GetDefOrg()); //�ڲ�����
        XMLSetNodeVal(doc, "//opOperid", GetDefOper()); //��Ա
        ret = callInterface(ACCOUNT_ACCTINFO_PERSON, opDoc);
    }

    if(ret)
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "��������");
        return ret;
    }

    q = XMLGetNodeVal(doc, "//opBKRetcode");
    BKINFO("opBKRetcode=[%s]", q);
    if (strlen(q) && !sdpStringIsAllChar(q, '0'))
    {
        return 0;
    }
    if ((q = XMLGetNodeVal(opDoc, "//opCardName")) != NULL)
        strcpy(sName2, q);
    else
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "��������");
        return 0;
    }

    BKINFO("name1=[%s] name2=[%s]", sName1, sName2);
    if (sName1[0] == 0x00 || strcmp(sName1, sName2))
    {
        XMLSetNodeVal(opDoc, "//opBKRetcode", "8999");
        XMLSetNodeVal(opDoc, "//opBKRetinfo", "��������");
        return 0;
    }

    return 0;
}

/*
 * ����ѯ
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_903(void *doc, char *p)
{
    result_set          rs;
    xmlDoc              *opDoc           = (xmlDoc *)doc;
    char                *workdate        = NULL; 
    char                *bankid          = NULL;
    int                 workround, i, rc = 0;
    char                settledate[20]   = {0}; 
    char                condi[256]       = {0};
    int                 trtype_list[]    = { 1101, 1102, 1103, 1104 };

    workdate = XMLGetNodeVal(opDoc, "//opCleardate");
    workround = atoi(XMLGetNodeVal(opDoc, "//opClearround"));
    bankid = XMLGetNodeVal(opDoc, "//opOriginator");

    strcpy(settledate, GetSettledDateround());
    settledate[8] = 0;
    if ((workround> atoi(settledate+9)) || strcmp(settledate, workdate) != 0)
    {
        XMLSetNodeVal(doc, "//opTCRetcode", "8999");
        XMLSetNodeVal(doc, "//opTCRetinfo", "δ�����Ķ���,����������������!");
        return 0;
    }

    rc = db_query(&rs, "SELECT "
            "isnull" "(sum(pres_debit_total), 0.0),"
            "isnull" "(sum(acpt_credit_total), 0.0),"
            "isnull" "(sum(acpt_debit_total), 0.0),"
            "isnull" "(sum(pres_credit_total), 0.0),"
            "isnull" "(sum(balance), 0.0) FROM ebanksumm "
            "WHERE svcclass in (1, 2) and branchid='%s' "
            "and workdate='%s' and workround=%d",
            bankid, workdate, workround);
    if ( rc != 0 )
        return rc;

    // ������������
    for (i = 0; i < 4; i++)
    {
        sprintf(condi, "workdate='%s' and workround=%d and bankid='%s' "
                "and trtype='%d'", workdate, workround, bankid, trtype_list[i]);
        if (db_hasrecord("bankclear", condi) == 0)
        {
            rc = db_exec("insert into bankclear values"
                    "('%s', %d, '%s', %.2lf, '%d', '0')", workdate, workround, 
                    bankid, db_cell_d(&rs, 0, i), trtype_list[i]);
            if ( rc != 0 )
                return rc;
        }
    }

    XMLSetNodeVal(opDoc, "//opOreserved1", db_cell(&rs, 0, 0));
    XMLSetNodeVal(opDoc, "//opOreserved2", db_cell(&rs, 0, 1));
    XMLSetNodeVal(opDoc, "//opOreserved3", db_cell(&rs, 0, 2));
    XMLSetNodeVal(opDoc, "//opOreserved4", db_cell(&rs, 0, 3));
    XMLSetNodeVal(opDoc, "//opOreserved5", db_cell(&rs, 0, 4));

    return 0;
}

/*
 * �������ʽ�����
 * ���� doc ƽ̨����
 *      p ����
 * ��� pret ��������ֵ(0�ɹ� ��0ʧ��)
 *      plen ����
 * ���� NULL
 */
int PF10_904(void *doc, char *p)
{
    result_set            rs;
    xmlDoc                *opDoc            = (xmlDoc *)doc;
    char                  *workdate         = NULL; 
    char                  *bankid           = NULL;
    char                  path[100]         = {0};
    char                  hostserial[256]   = {0};
    int                   workround, i, ret = 0;
    char                  *pp               = NULL;

    memset(hostserial, 0, sizeof(hostserial));

    workdate = XMLGetNodeVal(opDoc, "//opCleardate");
    workround = atoi(XMLGetNodeVal(opDoc, "//opClearround"));
    bankid = XMLGetNodeVal(opDoc, "//opOriginator");

    ret = db_query(&rs, "select trtype, settlamt, flag from bankclear "
            "where workdate='%s' and workround=%d and bankid='%s'", 
            workdate, workround, bankid);
    if (ret != 0)
        return ret;

    for (i = 0; i < db_row_count(&rs); i++)
    {
        sprintf(path, "//opOreserved%d", db_cell_i(&rs, i, 0)%10);

        if (db_cell_i(&rs, i, 2) == 1)
        {
            XMLSetNodeVal(opDoc, path, "�Ѿ�����ɹ�");
            continue;
        }
        if (db_cell_d(&rs, i, 1) < 0.01)
        {
            db_exec("update bankclear set flag='1' where workdate='%s'"
                    " and workround=%d and bankid='%s' and trtype='%s'", 
                    workdate, workround, bankid, db_cell(&rs, i, 0));
            XMLSetNodeVal(opDoc, path, "����ɹ�");
            continue;
        }

        // ת�˽��
        XMLSetNodeVal(opDoc, "//opSettlamt", db_cell(&rs, i, 1));
        // ת����ˮ
        XMLSetNodeVal(opDoc, "//opHreserved5", vstrcat("%ld", GenAccountSerial()));

        // ����
        if (callInterface(db_cell_i(&rs, i, 0), doc)) 
            XMLSetNodeVal(opDoc, path, "ͨѸʧ��");
        else
        {
            pp = XMLGetNodeVal(doc, "//opBKRetcode");
            if (strlen(pp) && !sdpStringIsAllChar(pp, '0'))
                XMLSetNodeVal(opDoc, path, XMLGetNodeVal(doc, "//opBKRetinfo"));
            else
            {
                db_exec("update bankclear set flag='1' where workdate='%s'"
                        " and workround=%d and bankid='%s' and trtype='%s'", 
                        workdate, workround, bankid, db_cell(&rs, i, 0));
                XMLSetNodeVal(opDoc, path, "����ɹ�");
                strcat(hostserial, XMLGetNodeVal(doc, "//opHostSerial"));
                strcat(hostserial, "-");
            }
        }
    }
    XMLSetNodeVal(opDoc, "//opHostSerial", hostserial);
    db_free_result(&rs);

    return 0;
}
/*
 * ����ҵ�����صǼǲ�����
 * ���� doc ƽ̨����
 */
int PF10_654(void *opdoc, char *p)
{
    result_set          rs;
    result_set          rstmp;
    char                tblName[16]             = "trnjour";
    xmlDoc              *doc                    = (xmlDoc *)opdoc;
    //char workdate[9]={0};
    char                *workdate               = NULL;
    FILE                *fpout                  = NULL;
    char                path[100]               = {0};
    char                outline[8192]           = {0};
    char                caOutFile[256]          = {0};
    char                bankname[80]            = {0};
    int                 i, j, ret               = 0;
    char                dcflag                  = 0;
    char                exchground              = 0;
    char                filename[128]           = {0};
    char                TmpCmd[128]             = {0};

    BKINFO("����ҵ��ǼǱ�����...");
    //caOutFile �����ļ�·��(�ļ����������: tmpXXXXXX)
    if (getFilesdirFile(caOutFile) == NULL)
    {
        BKINFO("������ʱ�ļ�ʧ��");
        return -1;
    }
    fpout = fopen(caOutFile, "w");

    if (strcmp(XMLGetNodeVal(doc, "//opWorkdate"), GetArchivedate()) <= 0)
        strcpy(tblName, "htrnjour");

    workdate = XMLGetNodeVal(doc, "//opWorkdate");
    dcflag = XMLGetNodeVal(doc, "//opDcflag")[0];
    exchground = XMLGetNodeVal(doc, "//opExchground")[0];


    ret = db_query(&rs, "select workdate, exchground, refid, originator, "
            "originator,acceptor, notetype, noteno, dcflag, issuedate, "
            "payingacct, payer, payingbank, beneacct, benename, benebank, "
            "benebank, curcode, settlamt, agreement, purpose  from %s "
            "where workdate='%s' and dcflag='%c' and exchground like '%%%c%%' and inoutflag='2'"
            " and trncode != '7' and trncode!= '0007' and truncflag='0' ", 
            tblName, workdate, dcflag, exchground);
    if (ret != 0)
        return ret;

    for (i = 0; i < db_row_count(&rs); i++)
    {

        ret = db_query(&rstmp,"select result from acctjour where nodeid=10 and inoutflag='2'"
                " and originator='%s' and workdate='%s' and refid='%s'",
                db_cell_by_name(&rs,i,"originator"), db_cell_by_name(&rs,i,"workdate"), db_cell_by_name(&rs,i,"refid"));
        if(ret!=0 && ret!=E_DB_NORECORD)
        {
            db_free_result(&rs);
            return -1;
        }
        if(ret!=E_DB_NORECORD && *db_cell(&rstmp,0,0)!='2' && *db_cell(&rstmp,0,0)!='9')
        {
            db_free_result(&rstmp);
            continue;
        }
        db_free_result(&rstmp);
        for(j=0; j < db_col_count(&rs); j++)
        {
            if (j == 17)
            {
                db_query_str(bankname, sizeof(bankname), "select bankname from bankinfo "
                        "where nodeid=%d and exchno='%s'",OP_REGIONID, db_cell_by_name(&rs, i, "benebank"));
                fprintf(fpout, "%s|%s|", bankname,db_cell(&rs, i, j));
            }
            else if( j == 18 )
            {
                fprintf(fpout, "%.0lf|", atof(db_cell(&rs, i, j))*100);
            }
            else if ( j == db_col_count(&rs)-1)
                fprintf(fpout, "%s", db_cell(&rs, i, j));
            else
                fprintf(fpout, "%s|", db_cell(&rs, i, j));
        }
        fprintf(fpout, "\n");
    }
    fclose(fpout);

    sprintf( filename, "file/%s", basename(caOutFile) );
    BKINFO("����ҵ��ǼǱ������ļ�[%s]...", filename);
    XMLSetNodeVal(doc, "//opReserved", filename);
    sprintf( TmpCmd, "tftclient -dup -h3 -r%s %s -tFHIPP", filename, filename );
    system( TmpCmd );

    return 0;
}
/*У��֧������*/
int CheckNoteInfo(int bkcode, xmlDoc *doc)
{
    int             ret         = 0;
    char            *p          = NULL;
    char            noteno1[10], noteno2[10]; 

    p= sdpXmlSelectNodeText(doc, "//opNoteno");
    if (p!= NULL && strlen(p) > 8 ) {
        BKINFO("ƾ֤��̫��,�ض�[%s->%s]", p, p+strlen(p)-8);
        XMLSetNodeVal(doc, "//opNoteno", p+strlen(p)-8); //����ƾ֤�����8λ(�����8λ)
    }
    /*
       if(strlen(XMLGetNodeVal(doc, "//opNoteno")) > 8)
       {
       sprintf(noteno2,"%.8s",XMLGetNodeVal(doc, "//opNoteno")+8);
       }
       XMLSetNodeVal(doc, "//opNoteno", noteno2);
     */

    if (ret = callInterface(bkcode, doc))
    {
        BKINFO("У������ʧ��");
        XMLSetNodeVal(doc, "//opBKRetinfo", "֧���������");
        return E_MNG_PAYPWD;
    }

    if( bkcode == ACCOUNT_CHECK_ZFMM )
    {
        /*opYYCWDM:�ӵ�4λ��ʼ,��Ϊ"0010"��Ϊʧ��*/
        p = XMLGetNodeVal(doc, "//opYYCWDM");
        if( p )
        {
            if( strncmp( p+3, "0010", 4 ) )
            {
                BKINFO("У��֧���������opYYCWDM[%s]", p);
                XMLSetNodeVal(doc, "//opBKRetcode", "8999");
                if ( strlen(XMLGetNodeVal(doc, "//opOreserved2")) )
                    XMLSetNodeVal(doc, "//opBKRetinfo", XMLGetNodeVal(doc, "//opOreserved2"));
                else
                    XMLSetNodeVal(doc, "//opBKRetinfo", "֧���������");
                return E_MNG_PAYPWD;
            }
        }
        else
            return E_MNG_PAYPWD;
        BKINFO("У��֧������У��ɹ�opYYCWDM[%s]", p);
    }
    else if( bkcode == ACCOUNT_CHECK_BPMY )
    {
        p = XMLGetNodeVal(doc, "//opBKRetcode");
        BKINFO("opBKRetcode=[%s]", p);
        if( p )
        {
            if (strlen(p) && strcmp(p, "SUCCESS"))
            {
                XMLSetNodeVal(doc, "//opBKRetcode", "");
                XMLSetNodeVal(doc, "//opBKRetinfo", "֧���������");
                return E_MNG_PAYPWD;
            }
            BKINFO("У�鱾Ʊ��Ѻ�ɹ�!");
        }
        else
            return E_MNG_PAYPWD;
    }

    return ret;
}
