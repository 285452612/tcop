/*
 *  %W% %G%
 *
 *  ͬ��Ʊ��ʵʱ����ϵͳ Release 2.00 ( CCS V2.00 for SYBASE)
 *  Copyright (C) 2000 �������Ŵ�ϵͳ�����������ι�˾
 *
 *  ���˽ӿ�ģʽpublic����, ����������ҵ���нӿ�ģʽ
 *  ����: file Created by zxq@sunyard.com at 2007/08/27
 *
 */

#include <time.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "fp.h"
#include "tp_code.h"
#include "public.h"
#include "global.h"
#include "err_code.h"
#include "netapi.h"
#include "s_hand_note.h"

long SQLCODE;
EXEC SQL include "../include/wuxixy.h";
EXEC SQL include "../include/flags.h";
EXEC SQL include "../include/s_exchgnote.h";
EXEC SQL include sqlca;
EXEC SQL whenever sqlerror call debugSQL(sqlca);
EXEC SQL whenever sqlwarning call debugSQL(sqlca);

void Prn_ExchgNote(struct pub_exchgnote_str *);
void ltos_date_oldadm(char *);
void HandToHandStr(hand_note *, s_hand_note *);
void Prn_HandStr(s_hand_note *);

EXEC SQL BEGIN DECLARE SECTION;
    struct AcctBankInfo AcctInfo;   /* ���˹�����Ϣ */
EXEC SQL END DECLARE SECTION;

/*** ���˽ӿ�ģʽ��������������� ***/
/* outinFlag:��������־(1:��� 2:����) tradetype:�����־('1':��'2':��) */
int AcctToBank(struct pub_exchgnote_str *s_str, int outinFlag, char tradetype)
{
    int result;
    char bufout[1000];  //��������
    char bufin[1000];   //��������
    int outlen=0;       //���ͳ���
    int packType;       //�����ڷ��͵İ�����
    int notetype;       //ƾ֤����
    int withholdFlag;   //������־
    char tradeid[5];        //���ڽ��״���
    int iFlag = 0;
    int iTradeCode = 0;

    giPackType = 0;
    Prn_ExchgNote(s_str);
#define PACK(type, reserved) packType=type, outlen = Pack_##type(s_str, bufout, reserved)

    if (result = InitAcctInterFace(s_str, outinFlag))
        return result;

    memset(bufin, ' ', sizeof(bufin));
    memset(bufout, ' ', sizeof(bufout));

    if(atoi(s_str->tradeid) == REQ_C2E_QRY_NOTEVALITY) //֧���������
    {
        PACK(1202, NULL);
        goto STEP;
    }
    notetype = atoi(s_str->notetype);
    fprintf(stderr, "%s|%d|AcctToBank:notetype=[%s]tradetype=[%c]\n", 
            __FILE__, __LINE__, s_str->notetype, tradetype);

    if (notetype == -1) //��ʾ���׽���(�ο���̨���մ���sendacct.cp)
        PACK(1102, NULL);
    else if(notetype == -2) //����ҵ��1102�߲�ͨʱ
    {
        //PACK(1631, NULL);
        PACK(1637, NULL);
    }
    else if(notetype == -3)     //��Ʊ��Ѻ
    {
        PACK(1649, NULL);
    }
    else if (notetype == TP)
    {
        if (outinFlag == 2) //������Ʊ
            PACK(1625, NULL);
        else
        {
            if (tradetype == '1') //��������Ʊ
                PACK(1623, "9");
            else if (tradetype == '2') //���������Ʊ
                PACK(1624, NULL);
            else
                return ERR_P_DATA_WRONG;
            if (outlen < 0)
                return ERR_P_DB_OPERATION;
            if (result = WSendRecv(bufout, outlen, bufin))
                return result;
            //InsertAcctSerial(s_str->outbank, s_str->outserial, bufin, packType);
            memset(bufout, 0, sizeof(bufout));
            memset(bufin, 0, sizeof(bufin));
            PACK(1626, tradetype); //ԭ����Ҫ��֤1623��1624���׺�1626����Ҫ��ͬһ������
            iTradeCode = 1626;
        }
    }
    else if (tradetype == '2')
    {
        if (outinFlag == 1)
        {
            if(memcmp(s_str->debtdetail1, "1653", 4) == 0)
            {
                PACK(1653, NULL);
                giPackType = 1653;
            }
            else if(memcmp(s_str->debtdetail1, "1664", 4) == 0)
            {
                PACK(1664, NULL);
                giPackType = 1664;
            }
            else
            {
                PACK(1622, NULL); //�������
                giPackType = 1622;
            }
            iFlag = 2;        //�������
        }
        else
        {
            if(memcmp(s_str->debtdetail1+10, "1632", 4) != 0)
                PACK(1624, NULL); //�������(������1632)
            else
                PACK(1632, NULL);
        }
    }
    else if (tradetype == '1')
    {
        withholdFlag = getWithHoldFlag(s_str->notetype);
        if (withholdFlag != 0 && withholdFlag != 1) //0:������ 1:����
            return withholdFlag;
        if (withholdFlag == 1)
        {
            /* MOD BY CCJIE 20081118 */
            if (outinFlag == 1 && atoi(s_str->notetype) != JLBP)
                PACK(1102, NULL); //�����ǽ�������(Ӧ����1102����2381)
            else if(outinFlag == 1 && atoi(s_str->notetype) == JLBP)
                PACK(1648, NULL);
            else if(outinFlag == 2 && atoi(s_str->notetype) == JLBP)
                PACK(1643, NULL);   //������Ʊ(����)
            else 
                PACK(1623, "1");  //�����ǽ�������(�Զ�����)
        }
        else if (withholdFlag == 0)
        {
            if (outinFlag == 1)
            {
                if (notetype == SSYSHP || notetype == BP)
                    PACK(1648, NULL); //�����ǻ�Ʊ�ཻ��
                else
                    PACK(1621, NULL); //�����Ƿǽ�������
                iFlag = 1;            //����ǽ������
            }
            else
            {
                memset(tradeid, 0, sizeof tradeid);
                memcpy(tradeid, s_str->debtdetail1, 4);
                fprintf(stderr, "%s %d| TRADEID:%s\n", __FILE__, __LINE__, tradeid);
                //if (notetype == SSYSHP || notetype == BP)
                if(atoi(tradeid) == 1643)
                    PACK(1643, NULL); //(��Ʊ��Ʊ����1643)
                else if(atoi(tradeid) == 1664)
                    PACK(1664, NULL); //(���гжһ�Ʊ����1664)
                else if(atoi(tradeid) == 1653)
                    PACK(1653, NULL); //(�и�����1653)
                else if(atoi(tradeid) == 1647)
                    PACK(1647, NULL); //(��Ʊ�Ҹ�1647)
                else
                    PACK(1623, "0");  //�����Ƿǽ�������(�ֹ�����)
            }
        }
    }

STEP:
    if (outlen < 0)
        return ERR_P_DB_OPERATION;

    AcctResult = WSendRecv(bufout, outlen, bufin);
    if((memcmp(BankErrCode, "1001", 4) == 0 && iTradeCode == 1626)) //������ڷ���1001�ʺŲ���ȷ����ʹ��ͳһ�ʺ������������
    {
        PACK(1626, tradetype);
        AcctResult = WSendRecv(bufout, outlen, bufin);
    }
    result = AcctResult;
    debugInt(AcctResult);
    strcpy(AcctResultStr, AcctResult == 0 ? "���˳ɹ�" : "����ʧ��");
    if (result == 0 && (iFlag == 1 || iFlag == 2))
    {
        InsertAcctSerial(s_str->outbank, s_str->outserial, bufin, packType);
    }

    return result;
}

/* ��ʼ�����˹�����Ϣ outinFlag:��������־(1:��� 2:����) */
int InitAcctInterFace(struct pub_exchgnote_str *s_str, int outinFlag)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char Sbankno[7];
    long Iserial;
    EXEC SQL END DECLARE SECTION;

    //memset(BankErrCode, 0, sizeof BankErrCode);        //��ʼ���ڷ��ش������
    memset(&AcctInfo, 0, sizeof(AcctInfo));
    strcpy(Sbankno, outinFlag == 1 ? s_str->outbank: s_str->inbank);
    fprintf(stderr, "%s|%d|InitAcctInterFace:outinFlag=[%d]bankno=[%s]outserial=[%s]\n", 
            __FILE__, __LINE__, outinFlag, Sbankno, s_str->outserial);

    EXEC SQL select jgdm, jgmc, zzgyh, qszh, zszh, zfzh, tpzszh into :AcctInfo.AcctJgdm, :AcctInfo.AcctJgmc, :AcctInfo.AcctZzgy, :AcctInfo.AcctQszh, :AcctInfo.AcctZszh, :AcctInfo.AcctZfzh, :AcctInfo.AcctTpzszh from hnjg where bankno=:Sbankno;
    if(SQLCODE)
    {
        debugSQL(sqlca);
        return ERR_P_DB_OPERATION;
    }
    trim_all(AcctInfo.AcctJgdm);    //��������
    trim_all(AcctInfo.AcctJgdm);    //��������
    trim_all(AcctInfo.AcctZzgy);    //������Ա
    trim_all(AcctInfo.AcctQszh);    //�����˺�
    trim_all(AcctInfo.AcctZszh);    //�����˺�
    trim_all(AcctInfo.AcctZfzh);    //�ݸ��˺�
    trim_all(AcctInfo.AcctTpzszh);  //��Ʊ�����˺�

    Iserial = atoi(s_str->outserial);
    if (outinFlag == 2)
    { 
        //������ʵ���ÿ���(�Զ�������������Ա,�ֹ��������õ�ǰ����Ա)
        if(atoi(s_str->tradeid) == REQ_C2E_QRY_NOTEVALITY || atoi(s_str->notetype) == ZFMMZP)
        {
            exec sql select zzgyh into :AcctInfo.AcctOperNo from hnjg  where bankno=:Sbankno;
        }
        else
            memcpy(AcctInfo.AcctOperNo, s_str->operator, sizeof(AcctInfo.AcctOperNo)-1);
    }
    else
    {
        exec sql select acct_oper into :AcctInfo.AcctOperNo
            from out_note where out_serial=:Iserial and out_bank=:Sbankno;
    }
    if(SQLCODE)
    {
        debugSQL(sqlca);
        //return ERR_P_DB_OPERATION; //�����������벻���Ҳ���Ա��ʧ���������Ҫ����
    }
    //���˹�Ա
    trim_all(AcctInfo.AcctOperNo); 
    //���˹�Ա����(������Ϣ,����Ҫ��ͨ��getOperName��ȡ��ֱ���޸�����SQL���)
    trim_all(AcctInfo.AcctOperName); 

    //ǰ�û��յ���ǰ39�ֽڽ����������������Ƶ�ǰ����(��֪�������,�ݱ���)
    if(atoi(s_str->tradeid) == 1 || atoi(s_str->tradeid) == 2)
        memcpy(s_str->debtdetail1,s_str->debtdetail1+32, sizeof(s_str->debtdetail1)-32);

    return 0;
}

/*** ���˽ӿ�ģʽ����������˳��� ***/
//tradetype:�����־ '1':�� '2':��
int AcctOutCZ(struct pub_exchgnote_str *s_str, char tradetype, char *pDate, int packtype)
{
    struct gj1106i1 packin;
    struct AcctAck ack;
    char bufout[1000];
    char bufin[1000];
    int result;
    //int packtype;

    memset(bufout,' ',sizeof(bufout));
    memset(bufin,' ',sizeof(bufin));
    memset(&packin, 0, sizeof(packin));

    //acctype:����ֱ�Ӵ�s_str��ȡ
    if (result = getAcctAckInfo(&ack, s_str->outbank, s_str->outserial, pDate, packtype))
        return result;
    strcpy(packin.jydm, "1106");
    strcpy(packin.dqdh, DQDH);
    strcpy(packin.jgdh, AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy, AcctInfo.AcctOperNo); //�����Ա��
    //strcpy(packin.jygy, AcctInfo.AcctZzgy); //������Ա��
    strcpy(packin.zddh, "ttywxtc "); //�ն˴���
    strcpy(packin.sqgy, s_str->operator);//��Ȩ��Ա
    strcpy(packin.sqmm, ""); //��Ȩ����
    strcpy(packin.czgyls, ack.gylsh); //���˹�Ա��ˮ
    sprintf(packin.jyje, "%.2f", atof(s_str->amount)/100.00); //���׽��

    memcpy(bufout,&packin,sizeof(packin));

    result = WSendRecv(bufout, sizeof(packin), bufin);
    if (result != 0)
        strcpy(AcctCZResultStr, "����ʧ��"); 
    else
        strcpy(AcctCZResultStr, "�����ɹ�");

    //�����˽��������������ظ�ǰ̨
    if (tradetype == '2')
        sprintf(s_str->debtdetail1, "����:[%s]����:[%s]", AcctResultStr, AcctCZResultStr);
    else if (tradetype == '1')
        sprintf(s_str->debtdetail1, "����:[%s]", AcctResultStr);

    return result;
}

/*** ��1102���� ����������ҵ���� ***/
int Pack_1102(struct pub_exchgnote_str *s_str, char *buf, char *reserved)
{
    struct gj1102i1 packin;
    struct AcctAck ack;
    int stFlag = 0; //���ױ�־(1��ʾ���׽���)
    int result;
    int packtype;
    EXEC SQL begin declare section;
    char pDate[11], acctround[2];
    EXEC SQL end declare section;

    memset(&packin,0,sizeof(packin));
    if (atoi(s_str->notetype) == -1)
        stFlag = 1;

    strcpy(packin.jydm, "1102");
    strcpy(packin.dqdh, DQDH);
    strcpy(packin.jgdh, AcctInfo.AcctJgdm); //��������

    if (stFlag == 0)
    {
        strcpy(packin.jygy,AcctInfo.AcctOperNo); //������Ա��
        strcpy(packin.dfzhdh, AcctInfo.AcctQszh); //�Է��ʻ�����
    }
    else
    {
        memset(&ack, 0, sizeof(ack));
        if(atoi(s_str->debtdetail1) == BP || atoi(s_str->debtdetail1) == SSYSHP)      //����Ҫ�������׽���
            packtype= 1648;
        else
            packtype = 1621;
        //acctype:����ֱ�Ӵ�s_str��ȡ,�޸ĺ�̨ȡ���ݿ��еĲ���(1623��1643)
        memset(acctround, 0, sizeof acctround);
        memset(pDate, 0, sizeof pDate);
        GetParaVal("acctround", "0024", acctround);
        if(atoi(acctround) > 2)             //����
            GetParaVal("sys_date", "0003", pDate);
        else                                //����
            EXEC SQL select max(trade_date) into :pDate from hddb..in_note;
        ltos_date_oldadm(pDate);
        memcpy(Acct_Date, pDate, 8);
        memcpy(Acct_OutBank, s_str->outbank, 6);
        Acct_OutSerial = atol(s_str->outserial);
        Acct_PackType = packtype;
        if (result = getAcctAckInfo(&ack,s_str->outbank,s_str->outserial,pDate, packtype))
            return result;
        strcpy(packin.jygy, s_str->operator); //���׹�Ա��
        trim_all(ack.xzxh);
        if(strlen(ack.xzxh) == 0)
        {
            fprintf(stderr, "%s %d| ���׽���ʱ�������Ϊ��!\n", __FILE__, __LINE__);
            fprintf(stderr, "%s %d| date:%s\nbankno:%s\nserial:%s\npacktype:%d\n", __FILE__, __LINE__, pDate, s_str->outbank, s_str->outserial, packtype);
            return 0;
        }
        strcpy(packin.xzxh, ack.xzxh); //�������
        strcpy(packin.dfzhdh, AcctInfo.AcctZszh); //�Է��ʻ�����
    }
    strcpy(packin.zddh, "ttywxtc "); //�ն˴���
    packin.jdbj = '1'; //�����־
    packin.xzbz = '1'; //��ת��־
    memcpy(packin.pzzl, "99", 2);
    trim_all(s_str->debitacct);
    strcpy(packin.zhdh, s_str->debitacct); //�ʻ�����
    sprintf(packin.jyje, "%.2f",atof(s_str->amount)/100.00); //���׽��
    strcpy(packin.zydh, "104"); //ժҪ���� 104 (ת�ʽ�)
    strncpy(packin.xjxmdh, "", sizeof(packin.xjxmdh)); //�ֽ���Ŀ����
    //ltos_date(packin.tcrq,s_str->issuedate); //TCRQ ��������
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*��1631���� ���ڷǱ������ʺŽ��м���*/
int Pack_1631(struct pub_exchgnote_str *s_str, char *buf, char *reserved)
{
    struct gj1631i1 packin;
    struct AcctAck ack;
    char pJgh[4];
    int result;
    int packtype;
    EXEC SQL begin declare section;
    char pDate[11], acctround[2];
    EXEC SQL end declare section;

    memset(&packin,0,sizeof(packin));

    strcpy(packin.jydm, "1631");
    strcpy(packin.dqdh, DQDH);
    strcpy(packin.jgdh, AcctInfo.AcctJgdm); //��������

        memset(&ack, 0, sizeof(ack));
        if(atoi(s_str->debtdetail1) == BP || atoi(s_str->debtdetail1) == SSYSHP)      //����Ҫ�������׽���
            packtype= 1648;
        else
            packtype = 1621;
        //acctype:����ֱ�Ӵ�s_str��ȡ,�޸ĺ�̨ȡ���ݿ��еĲ���(1623��1643)
        memset(acctround, 0, sizeof acctround);
        memset(pDate, 0, sizeof pDate);
        GetParaVal("acctround", "0024", acctround);
        if(atoi(acctround) > 2)             //����
            GetParaVal("sys_date", "0003", pDate);
        else                                //����
            EXEC SQL select max(trade_date) into :pDate from hddb..in_note;
        ltos_date_oldadm(pDate);
        memcpy(Acct_Date, pDate, 8);
        memcpy(Acct_OutBank, s_str->outbank, 6);
        Acct_OutSerial = atol(s_str->outserial);
        Acct_PackType = packtype;
        if (result = getAcctAckInfo(&ack,s_str->outbank,s_str->outserial,pDate, packtype))
            return result;
        memcpy(packin.gydh, s_str->operator, sizeof packin.gydh); //���׹�Ա��
        trim_all(ack.xzxh);
        if(strlen(ack.xzxh) == 0)
        {
            fprintf(stderr, "%s %d| ���׽���ʱ�������Ϊ��!\n", __FILE__, __LINE__);
            fprintf(stderr, "%s %d| date:%s\nbankno:%s\nserial:%s\npacktype:%d\n", __FILE__, __LINE__, pDate, s_str->outbank, s_str->outserial, packtype);
            return 0;
        }
        memcpy(packin.xzxh, ack.xzxh, sizeof packin.xzxh); //�������

    strcpy(packin.zddh, "ttywxtc "); //�ն˴���
    packin.rzfs = '0'; //���ʷ�ʽ
    packin.xzbz = '1'; //��ת��־
    memcpy(packin.pzzl, "99", 2);
    memcpy(packin.zhdh, AcctInfo.AcctZszh, sizeof packin.zhdh); //�ʻ�����
    packin.qkfs = '3'; //ȡ�ʽ
    sprintf(packin.jyje, "%.2f",atof(s_str->amount)/100.00); //���׽��
    memcpy(packin.skdwzh, s_str->debitacct, sizeof packin.skdwzh); //�տλ�ʺ�
    memcpy(packin.skdwmc, s_str->debitname, sizeof packin.skdwmc); //�տλ����
    memset(pJgh, 0, sizeof pJgh);
    memcpy(pJgh, s_str->debitacct+2, 3);
    packin.ylxz = '2'; //��·ѡ��
    sprintf(packin.dqjg, "%s%s", DQDH, pJgh);
    strcpy(packin.bz, "����ҵ��,�����ص�");
    memcpy(packin.hbzl, "01", 2);
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1621���ģ������Ƿǽ������ʱ��ģ�������������ҵ���� ***/
int Pack_1621(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct tr1621i1 packin;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1621");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm);           //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo);
    strcpy(packin.zddh,"ttywxtc ");         //�ն˴���
    trim_all(s_str->debitacct);
    strcpy(packin.zhdh,s_str->debitacct);   //�ʻ�����
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    //�Է�ƾ֤����ת��Ϊ����ƾ֤����
    //trans_pzxh(s_str->notetype, s_str->noteno, packin.dfpzzl, packin.dfpzxh);
    memcpy(packin.dfpzzl, "99", 2);
    strcpy(packin.dfpzxh, "");              //�Է�ƾ֤��� ��
    strcpy(packin.dfzh,s_str->creditacct);  //�Է��ʺ�
    strcpy(packin.dfjhdh,"00002");          //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    strcpy(packin.zydh,"104");              //ժҪ����
    packin.ylxz = '1';                      //��·ѡ��,1-ͬ��Ʊ��
    memcpy(packin.dfzhmc,s_str->debitname,50); //�Է��˺�����
    memcpy(packin.sy,s_str->purpose,50);    //����  
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1622���ģ�������Ǽ��ʱ��ģ�������������ҵ���� ***/
int Pack_1622(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct tr1622i1 packin;
    char tmp[20];

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1622");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //¼���Ա����
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->creditacct);
    strcpy(packin.zhdh,s_str->creditacct); //�ʻ�����
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    ltos_date(packin.qfrq,s_str->issuedate); //ǩ������
    trim_all(s_str->noteno); //ƾ֤��ż�ƾ֤����ת��
    if (strlen(s_str->noteno) && atol(s_str->noteno) != 0)
        trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    else
        memcpy(packin.pzzl, "99", 2);
    if(atoi(s_str->notetype) == 1)
    {
        memset(tmp, 0, sizeof tmp);
        memcpy(tmp, s_str->debtdetail1, sizeof tmp);
        memcpy(packin.pzzl, tmp, 2);
        packin.qkfs = tmp[2]; //ȡ�ʽ
        memset(tmp, 0, sizeof tmp);
        memcpy(tmp, s_str->debtdetail1+3, sizeof tmp);
        memcpy(packin.xzpmm, tmp, sizeof packin.xzpmm); //֧������
        memcpy(packin.zpxe, "0", 1); //�޶�
    }
    else
    {
        //memcpy(packin.zpxe, " ");
        memcpy(packin.pzzl, "99", 2);
        packin.qkfs = '3';
    }

    strcpy(packin.dfzh,s_str->debitacct); //�Է��ʺ�
    //strcpy(packin.dfjhdh,s_str->debiterbank); //�Է���������
    strcpy(packin.dfjhdh,"00002"); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    strcpy(packin.zydh,"103"); //ժҪ����
    strcpy(packin.gdzh,""); //�����˺�
    packin.ylxz = '1'; //��·ѡ��,1-ͬ��Ʊ��
    memcpy(packin.dfzhmc,s_str->debitname,50); //�Է��˺�����
    memcpy(packin.sy,s_str->purpose,50); //���� 
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1623���ģ������Ǽ��ʱ��ģ�������������ҵ���� ***/
int Pack_1623(struct pub_exchgnote_str *s_str,char *buf, char *withholdflag)
{
    struct tr1623i1 packin;
    int tpFlag = 0; //��Ʊ����

    if (atoi(s_str->notetype) == TP)
        tpFlag = 1;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1623");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    if (tpFlag)
        strcpy(packin.jygy,AcctInfo.AcctOperNo); //���˹�Ա
    else if (atoi(withholdflag) == 1) //����
        strcpy(packin.jygy,AcctInfo.AcctZzgy); //������Ա��
    else //�ǽ���
        strcpy(packin.jygy,s_str->operator); 
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->creditacct);
    if (tpFlag)
        strcpy(packin.zhdh,AcctInfo.AcctZfzh); //�ʻ�����
    else
        strcpy(packin.zhdh,s_str->creditacct); //�ʻ�����
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    if(atoi(withholdflag) != 1)     //�ǽ���
        strcpy(packin.zpxe,"  "); //�޶�
    else                            //����
        strcpy(packin.zpxe, "0");
    ltos_date(packin.qfrq,s_str->issuedate); //ǩ������
    //ת��Ϊ����ƾ֤����
    trim_all(s_str->noteno); //ƾ֤��
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    if(atoi(s_str->notetype) == ZP || atoi(s_str->notetype) == ZFMMZP)
        memcpy(packin.pzzl, "02", 2);
    else
        memcpy(packin.pzzl, "99", 2);
    if(atoi(s_str->notetype) == ZFMMZP)
        packin.qkfs = '1';  //ƾ����ȡ��
    else
        packin.qkfs = '3'; //ȡ�ʽ
    strcpy(packin.xzpmm,s_str->secret); //֧������
    if (tpFlag)
        memcpy(packin.zydh,"123", 3); //ժҪ����
    else {
        strcpy(packin.dfzh,s_str->debitacct); //�Է��ʺ�
        memcpy(packin.zydh,"103", 3); //ժҪ����
    }
    memcpy(packin.dfjhdh,"00002",5); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    packin.ylxz = '1'; //��·ѡ��,1-ͬ��Ʊ��

    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1624���ģ�������Ǽ��ʱ��ģ�������������ҵ���� ***/
int Pack_1624(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct tr1624i1 packin;
    int tpFlag = 0; //��Ʊ����

    if (atoi(s_str->notetype) == 19)
        tpFlag = 1;

    memset(&packin,0,sizeof(packin));

    strcpy(packin.jydm,"1624");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //��Ա��
    //strcpy(packin.jygy,s_str->operator); //��Ա��
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    if (tpFlag)
        strcpy(packin.zhdh, AcctInfo.AcctTpzszh); //�ʻ�����
    else {
        trim_all(s_str->debitacct);
        strcpy(packin.zhdh,s_str->debitacct); //�ʻ�����
    }
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    //ƾ֤����ת��Ϊ����ƾ֤����
    trim_all(s_str->noteno); //ƾ֤��
    //trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    memcpy(packin.pzzl, "99", 2);
    if (tpFlag)
        strcpy(packin.dfzh, ""); //�Է��ʺ�
    else
        strcpy(packin.dfzh, s_str->creditacct); //�Է��ʺ�
    strcpy(packin.dfjhdh,"00002"); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    if (tpFlag)
        memcpy(packin.zydh,"123", 3); //ժҪ����
    else
        memcpy(packin.zydh,"104", 3); //ժҪ����
    packin.ylxz = '1'; //��·ѡ��,1-ͬ��Ʊ��
    memcpy(packin.dfzhmc,s_str->creditname,50); //�Է��˺�����
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}
/*** ��1632���ģ�������Ǽ��ʱ��� ��Ʊ��������������ҵ���� ***/
int Pack_1632(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct tr1632i1 packin;
    char pDate[11];
    int tpFlag = 0; //��Ʊ����

    if (atoi(s_str->notetype) == 19)
        tpFlag = 1;

    memset(&packin,0,sizeof(packin));

    strcpy(packin.jydm,"1632");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    memcpy(packin.gydh, s_str->operator, sizeof packin.gydh);
    //strcpy(packin.gydh,AcctInfo.AcctZzgy); ������Ա��
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->noteno); //ƾ֤��
    //trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    memcpy(packin.pzzl, "99", 2);
    if (tpFlag)
        strcpy(packin.zhdh, AcctInfo.AcctTpzszh); //�ʻ�����
    else {
        trim_all(s_str->creditacct);
        strcpy(packin.zhdh,s_str->debitacct); //�ʻ�����
    }
    strcpy(packin.hrhjgh, AcctInfo.AcctJgdm);
    memcpy(packin.skdwmc, s_str->debitname, sizeof packin.skdwmc);
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    //ƾ֤����ת��Ϊ����ƾ֤����
    if (tpFlag)
        strcpy(packin.fkdwzh, ""); //�Է��ʺ�
    else
        strcpy(packin.fkdwzh, s_str->creditacct); //�Է��ʺ�
    memcpy(packin.fkdwmc, s_str->creditname, sizeof packin.fkdwmc);
    packin.ylxz = '1'; //��·ѡ��,1-ͬ��Ʊ��
    strcpy(packin.dfjhdh,"00002"); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    if (tpFlag)
        memcpy(packin.zydh,"123", 3); //ժҪ����
    else
        memcpy(packin.zydh,"108", 3); //ժҪ����
    memset(pDate, 0, sizeof pDate);
    memcpy(pDate, s_str->issuedate, sizeof pDate - 1);
    ltos_date_oldadm(pDate);
    trim_all(pDate);
    memcpy(packin.fbrq, pDate, 8);
    memcpy(packin.hbzl, "01", 2);
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1625���� ���������������Ʊ  1625����(�������Ʊ,���������) */
int Pack_1625(struct pub_exchgnote_str *s_str,char *buf, char *resved)
{
    struct tr1625i1 packin;
    struct AcctAck ack;
    int result;
    EXEC SQL begin declare section;
    char pDate[11];
    char notetype[3];
    char yls[11];
    char trade_type[3];
    char outbank[7];
    char reserved[201];
    long outserial;
    char withholdflag[2];
    EXEC SQL end declare section;
    char acctround[2];
    int tradeid;

    memset(acctround, 0, sizeof acctround);
    memset(trade_type, 0, sizeof trade_type);
    memset(outbank, 0, sizeof outbank);
    memset(yls, 0, sizeof yls);
    memset(notetype, 0, sizeof notetype);
    memset(withholdflag, 0, sizeof withholdflag);
    memset(pDate, 0, sizeof pDate);
    strcpy(outbank, s_str->outbank);
    outserial = atol(s_str->outserial);
    GetParaVal("acctround", "0024", acctround);
    debugStr(acctround);
    if(atoi(acctround) == 1)
    {
        GetParaVal("sys_date", "0003", pDate);
        if(memcmp(pDate, s_str->debtdetail1, sizeof pDate - 1) != 0) //�ǽ���
        {
            memset(pDate, 0, sizeof pDate);
            EXEC SQL select max(trade_date) into :pDate from hddb..in_note;
            EXEC SQL select trade_type, reserved into :trade_type, :reserved from hddb..in_note where trade_date = :pDate and out_serial = :outserial and out_bank = :outbank;
        }
        else  //����
        {
            EXEC SQL select trade_type, reserved into :trade_type, :reserved from cddb..in_note where out_serial = :outserial and out_bank = :outbank;
        }
        memset(pDate, 0, sizeof pDate);
        EXEC SQL select max(trade_date) into :pDate from hddb..in_note; //ԭƾ֤��������
    }
    else if(atoi(acctround) > 2)
    {
        GetParaVal("sys_date", "0003", pDate);
        EXEC SQL select trade_type, reserved into :trade_type, :reserved from cddb..in_note where out_serial = :outserial and out_bank = :outbank;
    }
    else        //�ڶ���
    {
        EXEC SQL select withholdflag, reserved into :withholdflag, :reserved from cddb..in_note where out_serial = :outserial and out_bank = :outbank;
        trim_all(withholdflag);
        if(atoi(withholdflag) == 1)         //����ȡ��������
            GetParaVal("sys_date", "0003", pDate);
        else                                //�ǽ���ȡ��������
            EXEC SQL select max(trade_date) into :pDate from hddb..in_note;
    }
    trim_all(reserved);
    debugStr(reserved);
    memcpy(notetype, reserved, 2);
    memcpy(yls, reserved+14, sizeof yls);
    trim_all(yls);

    ltos_date_oldadm(pDate);
    //��������Ҫ����ԭ����ƾ֤����ͽ�����־���ж�(1621��1648,��ͨ����ѯԭ�������ݿ�)
    memset(&ack, 0, sizeof ack);
    if(atoi(trade_type) == 2)
        tradeid = 1622;
    else
    {
        if(atoi(notetype) == BP || atoi(notetype) == SSYSHP)
        {
            result = Pack_1623(s_str,buf, withholdflag);
            return result;
        }
        else
        {
            if(s_str->tradeid[1] == '1')
            {
                tradeid = 1621;
                if (result = getAcctAckInfo(&ack, s_str->inbank, yls, pDate,  tradeid))  //��ѯ������ˮ
                    return 0;
            }
        }
    }
    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1625");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //¼���Ա����
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    if(s_str->tradeid[1] != '2')
        memcpy(packin.zhdh, s_str->creditacct, sizeof packin.zhdh);
    else
        memcpy(packin.zhdh, s_str->debitacct, sizeof packin.zhdh);
    //memcpy(packin.zhdh, AcctInfo.AcctQszh, sizeof packin.zhdh);
    memcpy(packin.xzxh, ack.xzxh, sizeof packin.xzxh); //�������
    packin.jdbj = s_str->tradeid[1] - 1; //0:��  1:��
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    trim_all(s_str->noteno); //ƾ֤��ż�ƾ֤����ת��
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    memcpy(packin.pzzl, "99", 2);
    //strcpy(packin.dfzh,s_str->debitacct);
    strcpy(packin.dfjhdh, "00002"); //�Է�������
    strcpy(packin.zydh,"123"); //ժҪ����
    packin.ylxz = '1'; //��·ѡ��,1-ͬ��Ʊ��
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1626���� ���������н跽 1626(������Ʊ) */
int Pack_1626(struct pub_exchgnote_str *s_str,char *buf, char tradetype)
{
    struct tr1626i1 packin;
    struct AcctAck ack;
    int result;

    /*
    if (result = getAcctAckInfo(&ack, s_str->inbank, s_str->outserial, 
                tradetype == '1' ? 1623 : 1624))
        return result;
        */
    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1626");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //¼���Ա����
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->debitacct);
    trim_all(s_str->creditacct);
    if(memcmp(BankErrCode, "1001", 4) == 0)
        strcpy(packin.zhdh, tradetype == '1' ? AcctInfo.AcctZfzh: AcctInfo.AcctTpzszh); // �ʻ�����
    else
        strcpy(packin.zhdh, tradetype == '1' ? s_str->debitacct: s_str->creditacct); // �ʻ�����
    if (tradetype == '1') //��
        packin.jdbj = '0'; //0:��  1:��
    else
        packin.jdbj = '1'; //0:��  1:��
    trim_all(s_str->noteno); //ƾ֤��ż�ƾ֤����ת��
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    strcpy(packin.pzzl, tradetype == '1'  ? "99" : "02"); //02:֧Ʊ(�跽ʱ) 99(����ʱ)
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    strcpy(packin.dfzh, ""); //s_str->debitacct); //�Է��ʺ�
    //memcpy(packin.dfzhmc,s_str->debitname,50); //�Է��˺�����
    strcpy(packin.dfjhdh, "00002"); //�Է�������
    strcpy(packin.zydh,"123"); //ժҪ����
    strcpy(packin.sy,""); //����
    strcpy(packin.hbzl, "01");
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1648���ģ������ǻ�Ʊ����ʱ��ģ�������������ҵ���� ***/
int Pack_1648(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1648i1 packin;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1648");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //������Ա��
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->debitacct);
    strcpy(packin.zhdh, s_str->debitacct);
    if (atoi(s_str->notetype) == BP || atoi(s_str->notetype) == JLBP)
        packin.choice = '2'; //��Ҫ�ж�: ѡ�� 0�����л�Ʊ 1:���б�Ʊ  2:���б�Ʊ
    else 
        packin.choice = '0'; 
    trim_all(s_str->noteno);
    strcpy(packin.hphm, s_str->noteno);
    strcpy(packin.dfzh,"0"); //�Է��ʺ�
    if(atoi(s_str->notetype) == BP || atoi(s_str->notetype) == JLBP)
        strcpy(packin.zydh,"207"); //ժҪ����
    else
        strcpy(packin.zydh,"114");
    strcpy(packin.xzxh, ""); //������� ��
    strcpy(packin.dfjhdh,"00002"); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    strcpy(packin.sxf, "0"); //������ ��
    strcpy(packin.fkdwmc,"0"); //���λ����
    /* ADD BY CCJIE 20080312 1648���ױ��ı䶯 */
    strcpy(packin.skdwzh,s_str->debitacct); //�տλ�ʺ�
    strcpy(packin.skdwmc,s_str->debitname); //�տλ����
    strcpy(packin.wzhphm, s_str->noteno); //��Ʊ����
    packin.hryl = '1';
    ltos_date(packin.hpqfrq,s_str->issuedate); //��������
    /* ADD END */
    /* add by ccjie 20081124 */
    memcpy(packin.khdqdh, "  ", 2);
    /* add end */
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1643���� (��Ʊ��Ʊ����) */
int Pack_1643(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1643i1 packin;
    char pzxh[8];

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1643");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    if(atoi(s_str->notetype) != JLBP)
        memcpy(packin.jygy,AcctInfo.AcctOperNo, sizeof packin.jygy); //������Ա��
    else
        memcpy(packin.jygy,AcctInfo.AcctZzgy, sizeof packin.jygy); //������Ա��
    //strcpy(packin.jygy,s_str->operator); 
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trim_all(s_str->noteno); //ƾ֤��ż�ƾ֤����ת��
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    //strcpy(packin.hphm, ""); //��Ʊ����
    memset(pzxh, 0, sizeof pzxh);
    memcpy(pzxh, packin.pzxh, sizeof packin.pzxh);
    if(atoi(s_str->notetype) == BP || atoi(s_str->notetype) == JLBP)
        sprintf(packin.hphm, "35%s", pzxh);
    else if(atoi(s_str->notetype) == SSYSHP)
        sprintf(packin.hphm, "08%s", pzxh);
    else if(atoi(s_str->notetype) == YHHP)
        sprintf(packin.hphm, "03%s", pzxh);
    sprintf(packin.sjje,"%.2f",atof(s_str->amount)/100.00); //ʵ�ʽ��
    /*
        trim_all(s_str->creditacct);
        strcpy(packin.zhdh,s_str->creditacct); //�ʻ�����
        strcpy(packin.skdwzh,s_str->creditacct); //�տλ�ʺ�
        strcpy(packin.skdwmc,s_str->creditname); //�տλ����
    */
    packin.ylxz = '1'; //��·ѡ�� 0:�޹�,1:ͬ��Ʊ��
    strcpy(packin.dfjhdh,"00002"); //�Է�������(��һ��00001,�ڶ���00002,���ڲ��ֳ�ֱ����00002)
    ltos_date(packin.fbrq,s_str->issuedate); //��������
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/*** ��1664���ģ������Ǽ��ʱ��� ���гжһ�Ʊ���壩������������ҵ���� ***/
int Pack_1664(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1664i1 packin;

    memset(&packin,0,sizeof(packin));
    memcpy(packin.jydm,"1664", 4);
    memcpy(packin.dqdh,DQDH, 2);
    memcpy(packin.jgdh,AcctInfo.AcctJgdm, 3); //��������
    memcpy(packin.jygy,AcctInfo.AcctOperNo, sizeof packin.jygy); //������Ա��
    //memcpy(packin.jygy,s_str->operator, 4); 
    memcpy(packin.zddh,"ttywxtc ", 8); //�ն˴���
    trim_all(s_str->noteno); //ƾ֤��ż�ƾ֤����ת��
    trans_pzxh(s_str->notetype, s_str->noteno, packin.zfpzzl, packin.zfpzhm);
    strcpy(packin.pjhm, packin.zfpzhm);
    memcpy(packin.zfpzzl, "99", 2);
    ltos_date(packin.wtrq,s_str->issuedate);
    //memcpy(packin.wtrq, s_str->issuedate, 8);
    memcpy(packin.skdwzh, s_str->debitacct, 32);
    memcpy(packin.skdwmc, s_str->debitname, sizeof packin.skdwmc);
    packin.ylxz = '1';  //��·ѡ�� 1Ʊ�ݽ���
    strcpy(packin.dfjhdh, "00002");
    memcpy(packin.tshm, s_str->debtdetail1+4, 10);//���պ���

    memcpy(buf,&packin,sizeof(packin));
    return sizeof(packin);
}

/*** ��1653���ģ������Ǽ��ʱ��� �и����������������ҵ���� ***/
int Pack_1653(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1653i1 packin;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1653");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    memcpy(packin.gydh,AcctInfo.AcctOperNo, sizeof packin.gydh); //������Ա��
    //strcpy(packin.gydh,s_str->operator); 
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    //memcpy(packin.pzzl, "05", 2);
    memcpy(packin.tshm, s_str->debtdetail1+4, sizeof packin.tshm); //���պ���
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    //strcpy(packin.jyje, s_str->amount);
    strcpy(packin.zhdh, s_str->creditacct); //�ʻ�����
    packin.tszy = '0';
    packin.ylxz = '1';
    strcpy(packin.dfjhdh, "00002");
    memcpy(packin.fjzs, "0", 1);
    strcpy(packin.skdwzh, s_str->debitacct);
    memcpy(packin.skdwmc, s_str->debitname, sizeof packin.skdwmc);
    packin.rzfs = '3';  //���ʷ�ʽ
    
    memcpy(buf,&packin,sizeof(packin));
    return sizeof(packin);
}

/*** ��1647���ģ������Ǽ��ʱ��� ��Ʊ�Ҹ���������������ҵ���� ***/
int Pack_1647(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1647i1 packin;
    char pAmount[16];
    char reservedbuf[201];
    int i,j,iFlag=0;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"1647");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    memcpy(packin.jygy,AcctInfo.AcctOperNo, sizeof packin.jygy); //������Ա��
    //strcpy(packin.jygy,s_str->operator); 
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    memcpy(packin.pzzl, "03", 2);
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    //strcpy(packin.jyje, s_str->amount);
    strcpy(packin.skdwzh, s_str->debitacct);
    memcpy(packin.skdwmc, s_str->debitname, sizeof packin.skdwmc);
    strcpy(packin.fkdwzh, s_str->creditacct);
    memcpy(packin.fkdwmc, s_str->creditname, sizeof packin.fkdwmc);
    packin.hryl = '1';
    strcpy(packin.jhdh, "00002");
    packin.jhlx = '3';
    ltos_date_oldadm(s_str->issuedate);
    memcpy(packin.hpqfrq, s_str->issuedate, sizeof packin.hpqfrq);
    //ȡ��Ʊ���
    memset(reservedbuf, 0, sizeof reservedbuf);
    strcpy(reservedbuf, s_str->reserved);
    memset(pAmount, 0, sizeof pAmount);
    for(i=0,j=0;i<strlen(reservedbuf);i++)
    {
        if(reservedbuf[i] != '\\' && iFlag == 0)
            continue;
        else if(reservedbuf[i] == '\\' && iFlag == 0)
        {
            iFlag = 1;
            continue;
        }
        if(reservedbuf[i] == '\\')
        {
            pAmount[15] = 0;
            break;
        }
        pAmount[j] = reservedbuf[i];
        j++;
    }
    trim_all(pAmount);
    sprintf(packin.hpje,"%.2f",atof(pAmount)/100.00); //��Ʊ���
    memcpy(packin.wzhphm, packin.pzzl, 2);
    memcpy(packin.wzhphm+2, s_str->noteno+strlen(s_str->noteno)-8, 8);
    memcpy(packin.hpmy, "0", 1);

    memcpy(buf,&packin,sizeof(packin));
    return sizeof(packin);
}

/*** ��2381���ģ������ǽ������ʱ��ģ�������������ҵ���� ***/
int Pack_2381(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dl2381i1 packin;

    memset(&packin,0,sizeof(packin));
    strcpy(packin.jydm,"2381");
    strcpy(packin.dqdh,DQDH);
    strcpy(packin.jgdh,AcctInfo.AcctJgdm); //��������
    strcpy(packin.jygy,AcctInfo.AcctOperNo); //������Ա��
    strcpy(packin.zddh,"ttywxtc "); //�ն˴���
    packin.jdbj = '1'; //�����־
    packin.xzbz = '1'; //��ת��־
    trim_all(s_str->debitacct);
    strcpy(packin.zhdh,s_str->debitacct); //�ʻ�����
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    strcpy(packin.dfzhdh, AcctInfo.AcctQszh); //�Է��ʻ�����
    strcpy(packin.zydh, "104"); //ժҪ���� 104 (ת�ʽ�)
    strncpy(packin.xjxmdh, "11111", sizeof(packin.xjxmdh)); //�ֽ���Ŀ���� 11111
    ltos_date(packin.tcrq,s_str->issuedate); //TCRQ ��������
    strcpy(packin.xtgzh, ""); //XTGZH ϵͳ���ٺ�
    
    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}
/**********��1637����******************/
int Pack_1637(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct tr1637i1 packin;
    struct AcctAck ack;
    int result;
    int packtype;
    EXEC SQL begin declare section;
    char pDate[11], acctround[2];
    char sJgmc[60+1];
    char pJgh[4];
    EXEC SQL end declare section;

    memset(&packin,0,sizeof(packin));

    strcpy(packin.jydm, "1637");
    strcpy(packin.dqdh, DQDH);
    strcpy(packin.jgdh, AcctInfo.AcctJgdm); //��������
    memset(&ack, 0, sizeof(ack));
    if(atoi(s_str->debtdetail1) == BP || atoi(s_str->debtdetail1) == SSYSHP)      //����Ҫ�������׽���
        packtype= 1648;
    else
        packtype = 1621;
    //acctype:����ֱ�Ӵ�s_str��ȡ,�޸ĺ�̨ȡ���ݿ��еĲ���(1623��1643)
    memset(acctround, 0, sizeof acctround);
    memset(pDate, 0, sizeof pDate);
    GetParaVal("acctround", "0024", acctround);
    if(atoi(acctround) > 2)             //����
        GetParaVal("sys_date", "0003", pDate);
    else                                //����
        EXEC SQL select max(trade_date) into :pDate from hddb..in_note;
    ltos_date_oldadm(pDate);
    memcpy(Acct_Date, pDate, 8);
    memcpy(Acct_OutBank, s_str->outbank, 6);
    Acct_OutSerial = atol(s_str->outserial);
    Acct_PackType = packtype;
    if (result = getAcctAckInfo(&ack,s_str->outbank,s_str->outserial,pDate, packtype))
        return result;
    memcpy(packin.jygy, s_str->operator, sizeof packin.jygy); //���׹�Ա��
    strcpy(packin.zddh, "ttywxtc "); //�ն˴���
    memcpy(packin.app_ver, "01", 2);
    packin.xzbz = '1'; //��ת��־
    packin.fkxykbz = '0';
    memcpy(packin.pzzl, "99", 2);
    memcpy(packin.zhdh, AcctInfo.AcctZszh, sizeof packin.zhdh); //�ʻ�����
    trim_all(ack.xzxh);
    if(strlen(ack.xzxh) == 0)
    {
        fprintf(stderr, "%s %d| ���׽���ʱ�������Ϊ��!\n", __FILE__, __LINE__);
        fprintf(stderr, "%s %d| date:%s\nbankno:%s\nserial:%s\npacktype:%d\n", __FILE__, __LINE__, pDate, s_str->outbank, s_str->outserial, packtype);
        return 0;
    }
    memcpy(packin.xzxh, ack.xzxh, sizeof packin.xzxh); //�������
    packin.qkfs = '3'; //ȡ�ʽ
    //memcpy(packin.zffkrmc, s_str->creditname, sizeof packin.zffkrmc);
    strcpy(packin.zffkrmc, "ͬ������������˿���");
    sprintf(packin.jyje, "%.2f",atof(s_str->amount)/100.00); //���׽��
    packin.ylxz = '2'; //��·ѡ��
    memcpy(packin.zfskrzh, s_str->debitacct, sizeof packin.zfskrzh); //�տλ�ʺ�
    memcpy(packin.zfskrmc, s_str->debitname, sizeof packin.zfskrmc); //�տλ����
    //sprintf(packin.zfsbhh, "%s%s", DQDH, AcctInfo.AcctJgdm);
    memcpy(packin.zfsbhh, s_str->debitacct, 5);
    memset(pJgh, 0, sizeof pJgh);
    memcpy(pJgh, s_str->debitacct+2, 3);
    memset(sJgmc, 0, sizeof sJgmc);
    EXEC SQL select jgmc into :sJgmc from cddb..hnjg where jgdm = :pJgh;
    //memcpy(packin.zfsbhm, AcctInfo.AcctJgmc, sizeof packin.zfsbhm);
    memcpy(packin.zfsbhm, sJgmc, sizeof packin.zfsbhm);
    packin.rzfs = '0'; //���ʷ�ʽ
    packin.sffs = '1';
    packin.fyjsfs = '2';
    memcpy(packin.ydf, "0", 1);
    memcpy(packin.sxf, "0", 1);
    strcpy(packin.bz, "����ҵ��,�����ص�");
    memcpy(packin.hbzl, "01", 2);

    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}
/* ��Ʊ��Ѻ */
int Pack_1649(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct dh1649i1 packin;
    char pDate[11];
    char bank[13];

    memset(&packin, 0, sizeof packin);
    memcpy(packin.jydm, "1649", 4);
    memcpy(packin.dqdh, DQDH, 2);
    memcpy(packin.jgdh,AcctInfo.AcctJgdm,sizeof packin.jgdh); //��������
    memcpy(packin.jygy,AcctInfo.AcctZzgy,sizeof packin.jygy); //������Ա��
    memcpy(packin.zddh, "ttywxtc ", sizeof packin.zddh);
    packin.czgn = '4';
    memset(pDate, 0, sizeof pDate);
    ltos_date(pDate,s_str->issuedate); // ��������
    memcpy(packin.hpqfrq, pDate, sizeof packin.hpqfrq);
    memcpy(packin.hpqfdq, DQDH, 2);
    //trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    trim_all(s_str->noteno);
    memcpy(packin.hphm, s_str->noteno, sizeof packin.hphm);
    sprintf(packin.hpje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    trim_all(s_str->secret);
    memcpy(packin.hpmy, s_str->secret, sizeof packin.hpmy);
    memset(bank, 0, sizeof bank);
    GetProfileString("../etc/server.ini", "Exchg To Bank", s_str->inbank, bank);
    memcpy(packin.qfwdh, bank, sizeof packin.qfwdh);
    //memcpy(packin.skrzh, s_str->debitacct, sizeof packin.skrzh);
    //memcpy(packin.skrmc, s_str->debitname, sizeof packin.skrmc);
    packin.sybz = '0';

    memcpy(buf,&packin,sizeof(packin));
    return sizeof(packin);
}

/*��֧֯��������鱨��1202*/
int Pack_1202(struct pub_exchgnote_str *s_str,char *buf, char *reserved)
{
    struct bm1202i1 packin;
    char pDate[11];

    memset(&packin, 0, sizeof packin);
    memcpy(packin.jydm, "1202", 4);
    memcpy(packin.dqdh, DQDH, 2);
    memcpy(packin.jgdh,AcctInfo.AcctJgdm,sizeof packin.jgdh); //��������
    memcpy(packin.jygy,AcctInfo.AcctOperNo,sizeof packin.jygy); //������Ա��
    memcpy(packin.zddh, "ttywxtc ", sizeof packin.zddh);
    memcpy(packin.zhdh, s_str->creditacct, sizeof packin.zhdh);
    trans_pzxh(s_str->notetype, s_str->noteno, packin.pzzl, packin.pzxh);
    memcpy(packin.pzzl, "02", sizeof packin.pzzl);
    memset(pDate, 0, sizeof pDate);
    memcpy(pDate, s_str->issuedate, sizeof(pDate)-1);
    ltos_date(pDate,s_str->issuedate); // ��������
    memcpy(packin.qfrq, pDate, sizeof packin.qfrq);
    sprintf(packin.jyje,"%.2f",atof(s_str->amount)/100.00); //���׽��
    strcpy(packin.zpxe, "0");
    memcpy(packin.zpmm, s_str->secret, sizeof packin.zpmm);

    memcpy(buf,&packin,sizeof(packin));

    return sizeof(packin);
}

/* ����ͬ��ƾ֤���ͺ�ƾ֤����ת�����ڼ���ƾ֤���ͺ�ƾ֤��� */
int trans_pzxh(char *notetype, char *noteno, char *hntype, char *pzxh)
{
    EXEC SQL BEGIN DECLARE SECTION;
        char Snotetype[3];
        char Sdsbs[3];
        char Spzdjbz[2];
        char Spzzl[3];
    EXEC SQL END DECLARE SECTION;
    int pzzl;
    char tmp[30];
    
    strcpy(Snotetype,notetype);
    exec sql select pzzl, dsbs, pzdjbz into :Spzzl, :Sdsbs,:Spzdjbz
        from hn_notetype where tcpzzl=:Snotetype;
    if(SQLCODE)
    {
        debugSQL(sqlca);
        debugStr(Snotetype);
        return -1;
    }

    trim_all(Sdsbs);
    trim_all(Spzdjbz);
    trim_all(Spzzl);
    pzzl = atoi(Spzzl);
    memcpy(hntype, Spzzl, 2); //ת������ƾ֤����

    //memcpy(pzxh, noteno+strlen(noteno)-7, 7); Ĭ��ȡ��7λ(����Ĭ��ƾ֤�����Ҫ�پ���)
    if(strlen(noteno) >7)
        memcpy(tmp, noteno+strlen(noteno)-7, 7);
    else
        memcpy(tmp, noteno, strlen(noteno));
    sprintf(pzxh, "%07d", atoi(tmp));

    if (Spzdjbz[0] == '1' || pzzl == 60 || pzzl == 61 || pzzl == 62 || pzzl == 83)
    {
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
    fprintf(stderr, "%s|%d|notetype=[%s]noteno=[%s]hntype=[%s]hnxh=[%s]\n", 
            __FILE__, __LINE__, notetype, noteno, hntype, pzxh);

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
    for (i=0; i<7; i++)
        seven[i] = sevenBak[6-i];
}

int getAcctAckInfo(struct AcctAck *pack, char *outbank, char *outserial, char *pDate, int packtype)
{
    EXEC SQL begin declare section;
        struct AcctAck ack;
        char bankno[7];
        long serial;
        long type;
        char jyrq[9];
    EXEC SQL end declare section;

    strcpy(bankno, outbank);
    serial = atoi(outserial);
    memset(jyrq, 0, sizeof jyrq);
    memcpy(jyrq, pDate, sizeof jyrq);
    type = packtype;
    memset(&ack, 0, sizeof(ack));
    exec sql select * into :ack from acct_serial 
        where bankno=:bankno and serial=:serial and packtype=:type and jyrq=:jyrq;
    fprintf(stderr, "%s|%d|getAcctAckInfo:outbank=[%s]outserial=[%s]acctype=[%d]\njyrq=[%s]\n", 
            __FILE__, __LINE__, outbank, outserial, packtype, jyrq);
    if(SQLCODE) {
        debugSQL(sqlca);
        return -1;
    }
    *pack = ack;
    return 0;
}

int InsertAcctSerial(char *outbank, char *outserial, char *bufin, int packtype)
{
    EXEC SQL begin declare section;
        char bankno[7];
        long serial;
        long type;
        struct AcctAck ack;
        char pDate[11];
        char acctround[2];
        int iRound;
    EXEC SQL end declare section;

    memset(&ack, 0, sizeof(ack));
    MakeAcctAck(&ack, bufin, packtype); 

    strcpy(bankno, outbank);
    serial = atoi(outserial);
    type = packtype;

    memset(pDate, 0, sizeof pDate);
    GetParaVal("sys_date", "0003", pDate);
    ltos_date_oldadm(pDate);
    trim_all(pDate);
    memset(acctround, 0, sizeof acctround);
    GetParaVal("acctround", "0024", acctround);
    trim_all(acctround);
    iRound = atoi(acctround);

    EXEC SQL insert into acct_serial values (
            :bankno, :serial, :type, :ack.msgid, :ack.cfcs, :pDate, :ack.jysj, :ack.gylsh, :ack.xzxh,
            :ack.pjxh, :ack.jhrq, :iRound, :ack.jyrq, 0, :ack.wdmc, :ack.khmc);
    fprintf(stderr, "%s|%d|InsertAcctSerial:bankno=[%s]serial=[%ld]type=[%ld]\n",
            __FILE__, __LINE__, bankno, serial, type);
    if(SQLCODE)
    {
        debugSQL(sqlca);
        //return ERR_P_DB_OPERATION;
    }

    return 0;
}

/***�����ڼ�����������Ϣ��������Ӧ�� ***/
int WSendRecv(char *bufout, int outlen, char *bufin)
{
    int result = 0;
    char tmp[6];

    dispbuf(bufout,outlen); //��ʾ��������
    result = wsend_recv(bufout,outlen,bufin);
    if(result<=0)
    {
        //��Ϣ��������ϵͳʧ��
        return ERR_E_OTHER; //������ϵͳ�ڲ���      
    }
    dispbuf(bufin,result); //��ʾ��������

    //ǰ��5λΪ������룬ת��Ϊͬ�Ǵ������
    memset(tmp,0,sizeof(tmp));
    memcpy(tmp,bufin,5);
    trim_all(tmp);
    if(atoi(tmp) == 8888)   //����Խ�����Ʊ��Ѻ��  ���ڶ���Ѻ������ȷ�������
        memcpy(ErrStr, bufin+11, 12);
    fprintf(stderr, "%s %d| bank ack result:%s\n", __FILE__, __LINE__, tmp);
    memcpy(BankErrCode, tmp, strlen(tmp));
    BankErrCode[strlen(tmp)]=0;
    result = transRet(atoi(tmp)); //�����Ƿ�ɹ�

    return result;
}

//���ݼ��ʽ����������Ӧ����ת�ɶ�Ӧ���ݿ����Ӧ�ṹ
int MakeAcctAck(struct AcctAck *ack, char *bufin, int acctType)
{
    struct tr1621o1 *out1621 = (struct tr1621o1*)bufin;
    struct tr1622o1 *out1622 = (struct tr1622o1*)bufin;
    struct gj1102o1 *out1102 = (struct gj1102o1*)bufin;
#define SetAcctAck(name, trname) strncpy(ack->name,  trname, sizeof(trname))
    switch (acctType) {
        case 1621:
        case 1648:
            SetAcctAck(jyrq, out1621->jyrq);
            SetAcctAck(gylsh, out1621->gylsh);
            SetAcctAck(xzxh, out1621->xzxh);
            break;
        case 1622:
        //case 1623:
        //case 1624:
            SetAcctAck(jyrq, out1622->jyrq);
            SetAcctAck(gylsh, out1622->gylsh);
            break;
        //case 1102:
            //break;
    }
    return 0;
}

/*�����Ǽ��ʺ�̨����*/
void BankInDebit(struct data_packet *pdat)
{
    int result = 0;
    char result_str[3];
    int dlen =mtoi(pdat->dlen, 4); 
    struct pub_exchgnote s_bin;
    EXEC SQL begin declare section;
    struct pub_exchgnote_str s_str;
    char acctround[2];
    char tradedate[11];
    int outserial;
    EXEC SQL end declare section;
    char tmp[64];
    
    memset(&s_bin, 0 ,sizeof(s_bin));
    memset(&s_str, 0 ,sizeof(s_str));

    memcpy(&s_bin, pdat->data_buf, dlen);
    PubPacketToStr(&s_bin, &s_str);
    fprintf(stderr, "%s|%d|dlen=[%d]tradeid=[%s]outbank=[%s]inbank=[%s]outserial=[%s]\n",
            __FILE__, __LINE__, dlen, s_str.tradeid, s_str.outbank, s_str.inbank, s_str.outserial);
    outserial = atoi(s_str.outserial);

    if (!OpenDatabase())
        result = ERR_P_DB_OPERATION;
    else if (AcctInterFace == 1)
    {
        memset(acctround, 0, sizeof acctround);
        GetParaVal("acctround", "0024", acctround);
        trim_all(acctround);
        debugStr(acctround);
        if(atoi(acctround) == 1)
        {
            memset(tradedate, 0, sizeof tradedate);
            EXEC SQL select max(trade_date) into :tradedate from hddb..in_note;
            EXEC SQL select reserved into :s_str.reserved from hddb..in_note where out_serial = :outserial and out_bank = :s_str.outbank and trade_date = :tradedate;
        }
        else
            EXEC SQL select reserved into :s_str.reserved from in_note where out_serial = :outserial and out_bank = :s_str.outbank;
        s_str.tradeid[1] = '1'; //���
        result = AcctToBank(&s_str, 2, '1');
        if(result) {
            updateoutnoteflag2(s_str, NOTE_CHECKFAILED, result);
        }
        //��һ���������ɹ��ĸ�����ʷ���е�ԭ����result=09
        if(atoi(acctround) == 1 && result == 0)
        {
            memset(tradedate, 0, sizeof tradedate);
            EXEC SQL select max(trade_date) into :tradedate from hddb..in_note;
            EXEC SQL update hddb..in_note set result = '09' where out_bank=:s_str.outbank and out_serial=:outserial and trade_date=:tradedate;
        }
        else if(atoi(acctround) != 1 && result == 0)
        {
            debugStr(s_str.outbank);
            debugInt(outserial);
            EXEC SQL update cddb..in_note set result = '09' where out_bank=:s_str.outbank and out_serial=:outserial;
        }
    }
    CloseDatabase();
    fprintf(stderr,"%s|%d|result=%d\n", __FILE__, __LINE__, result);
    sprintf(s_bin.result,"%02d",result);

    memset(s_bin.debtdetail1, 0, sizeof s_bin.debtdetail1);
    memset(s_bin.debtdetail2, 0, sizeof s_bin.debtdetail2);
    //���ڷ��صĴ��� ��###��ͷ ��ǰ̨����ֻ��
    Find_OneLine(1,BankErrCode,"../dat/errcode.dat",s_bin.debtdetail1);
    GetFJBank(s_bin.debtdetail1,2,'|',s_bin.debtdetail2);
    memset(s_bin.debtdetail1, 0, sizeof s_bin.debtdetail1);
    sprintf(s_bin.debtdetail1, "###%s", s_bin.debtdetail2);
    memset(s_bin.debtdetail2, 0, sizeof s_bin.debtdetail2);

    dlen = sizeof(struct pub_exchgnote);
    memset(tmp, 0, sizeof tmp);
    sprintf(tmp, "%04d", dlen);
    memcpy(pdat->dlen, tmp, 4);
    s_bin.tradeid[0] = ACK_TRADE;
    memcpy(pdat->data_buf,&s_bin,dlen);

    memset(tmp,0,sizeof(tmp));
    sprintf(tmp,"%3d",SYS_ServerNum);
    pdat->mtype = mtoi(pdat->orgnodeno,sizeof(pdat->orgnodeno));
    memcpy(pdat->snodeno,tmp,sizeof(pdat->snodeno));
    memcpy(pdat->dnodeno,pdat->orgnodeno,sizeof(pdat->dnodeno));
    memcpy(pdat->ackpid,pdat->reserved,sizeof(pdat->ackpid));

    itom(pdat->orgpid,getpid(),10);

    pdat->request = ACK_MODE;

    pdat->data_buf[dlen]=0;
    SendMsgByKey(sendkey,exbshmkey,pdat,SEND_AS_SERVER);
    //AckToPubEX(pdat,&s_str,result);
}

/*������Ǽ��ʺ�̨����*/
void BankInCredit(struct data_packet *pdat)
{
    int result = 0;
    char result_str[3];
    int dlen =mtoi(pdat->dlen, 4); 
    struct pub_exchgnote s_bin;
    EXEC SQL begin declare section;
    struct pub_exchgnote_str s_str;
    char acctround[2];
    char tradedate[11];
    int outserial;
    EXEC SQL end declare section;
    char pSysDate[11];
    char tmp[64];
    
    memset(&s_bin, 0 ,sizeof(s_bin));
    memset(&s_str, 0 ,sizeof(s_str));

    memcpy(&s_bin, pdat->data_buf, dlen);
    PubPacketToStr(&s_bin, &s_str);
    fprintf(stderr, "%s|%d|dlen=[%d]tradeid=[%s]outbank=[%s]inbank=[%s]outserial=[%s]\n",
            __FILE__, __LINE__, dlen, s_str.tradeid, s_str.outbank, s_str.inbank, s_str.outserial);
    outserial = atoi(s_str.outserial);

    if (!OpenDatabase())
        result = ERR_P_DB_OPERATION;
    else if (AcctInterFace == 1)
    {
        memset(pSysDate, 0, sizeof pSysDate);
        GetParaVal("sys_date", "0003", pSysDate);
        ltos_date_oldadm(pSysDate);
        memset(s_str.issuedate, 0, sizeof s_str.issuedate);
        strcpy(s_str.issuedate, pSysDate);
        s_str.tradeid[1] = '2'; //����
        result = AcctToBank(&s_str, 2, '2');
        if(result)
        {
            updateoutnoteflag2(s_str, NOTE_CHECKFAILED, result);
        }
        memset(acctround, 0, sizeof acctround);
        GetParaVal("acctround", "0024", acctround);
        if(atoi(acctround) == 1 && result == 0 && atoi(s_str.notetype) != JZD)
        {
            memset(tradedate, 0, sizeof tradedate);
            GetParaVal("sys_date", "0003", tradedate);
            //�ֹ�ȷ�ϳɹ�����result=09
            if(memcmp(tradedate, s_str.debtdetail1, sizeof tradedate - 1) == 0)    //����
                EXEC SQL update cddb..in_note set result = '09' where out_bank = :s_str.outbank and out_serial = :outserial;
            else
            {
                memset(tradedate, 0, sizeof tradedate);
                EXEC SQL select max(trade_date) into :tradedate from hddb..in_note;
                EXEC SQL update hddb..in_note set result = '09' where out_bank=:s_str.outbank and out_serial=:outserial and trade_date=:tradedate;
            }
        }
        else if(atoi(acctround) == 1 && result == 0 && atoi(s_str.notetype) == JZD)
        {
                EXEC SQL update cddb..in_note set result = '09' where out_bank = :s_str.outbank and out_serial = :outserial;
        }
        else if(atoi(acctround) != 1 && result == 0)
        {
            EXEC SQL update cddb..in_note set result = '09' where out_bank=:s_str.outbank and out_serial=:outserial;
        }
    }
    CloseDatabase();
    fprintf(stderr,"%s|%d|result=%d\n", __FILE__, __LINE__, result);
    sprintf(s_bin.result,"%02d",result);

    memset(s_bin.debtdetail1, 0, sizeof s_bin.debtdetail1);
    memset(s_bin.debtdetail2, 0, sizeof s_bin.debtdetail2);
    //���ڷ��صĴ��� ��###��ͷ ��ǰ̨����ֻ��
    Find_OneLine(1,BankErrCode,"../dat/errcode.dat",s_bin.debtdetail1);
    GetFJBank(s_bin.debtdetail1,2,'|',s_bin.debtdetail2);
    memset(s_bin.debtdetail1, 0, sizeof s_bin.debtdetail1);
    sprintf(s_bin.debtdetail1, "###%s", s_bin.debtdetail2);
    memset(s_bin.debtdetail2, 0, sizeof s_bin.debtdetail2);

    dlen = sizeof(struct pub_exchgnote);
    memset(tmp, 0, sizeof tmp);
    sprintf(tmp, "%04d", dlen);
    memcpy(pdat->dlen, tmp, 4);
    s_bin.tradeid[0] = ACK_TRADE;
    memcpy(pdat->data_buf,&s_bin,dlen);

    memset(tmp,0,sizeof(tmp));
    sprintf(tmp,"%3d",SYS_ServerNum);
    pdat->mtype = mtoi(pdat->orgnodeno,sizeof(pdat->orgnodeno));
    memcpy(pdat->snodeno,tmp,sizeof(pdat->snodeno));
    memcpy(pdat->dnodeno,pdat->orgnodeno,sizeof(pdat->dnodeno));
    memcpy(pdat->ackpid,pdat->reserved,sizeof(pdat->ackpid));

    itom(pdat->orgpid,getpid(),10);

    pdat->request = ACK_MODE;

    pdat->data_buf[dlen]=0;
    SendMsgByKey(sendkey,exbshmkey,pdat,SEND_AS_SERVER);
    //AckToPubEX(pdat,&s_str,result);
}

/*** ���ݻ�������ȡ�������Ƶ���Ϣ,������������ҵ�ȼ��˽ӿ� ***/
int GetJgXX(char *AcctJgdm,char *AcctJgmc,char *AcctZzgy,char *AcctZszh)
{
    EXEC SQL BEGIN DECLARE SECTION;
        char S1[11];
        char S2[61];
        char S3[11];
        char S4[33];
    EXEC SQL END DECLARE SECTION;
    
    fprintf(stderr, "AcctJgdm=[%s]AcctJgmc=[%s]AcctZzgy=[%s]AcctZszh=[%s]\n",
            AcctJgdm, AcctJgmc, AcctZzgy, AcctZszh);
    strcpy(S1,AcctJgdm);
    OpenDatabase();
    exec sql select jgmc,zzgyh,zszh into :S2,:S3,:S4 
        from hnjg where jgdm = :S1 ;
        if(SQLCODE)
        {
        debugSQL(sqlca);
        debugStr(S1);
        CloseDatabase();    
                return -1;
        }
    trim_all(S2);
    trim_all(S3);
    trim_all(S4);
    strcpy(AcctJgmc,S2);
    strcpy(AcctZzgy,S3);
    strcpy(AcctZszh,S4);

    CloseDatabase();    
    return 0;
}

/* ����ƾ֤����ȡ������־, ���� 0:�ǽ��� 1:���� ����:������ */
int getWithHoldFlag(char *notetype)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char Snotetype[3];
    char flag[2];
    EXEC SQL END DECLARE SECTION;
    memset(Snotetype, 0, sizeof(Snotetype));
    memcpy(Snotetype, notetype, 2);

    exec sql select withholdflag into :flag
        from note_type where note_type=:Snotetype;
    if (SQLCODE)
    {
        debugSQL(sqlca);
        debugStr(notetype);
        return ERR_P_DB_OPERATION;
    }
    return flag[0] - '0';
}

/* ���ݲ���Ա��ȡ����Ա�����*/
int getOperName(char *oper, char *name)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char Soper[5];
    char Sname[9];
    EXEC SQL END DECLARE SECTION;
    strcpy(Soper, oper);
    memset(Sname,0,sizeof(Sname));

    exec sql select name into :Sname
        from operator where operator=:Soper;
    if(SQLCODE)
    {
        debugSQL(sqlca);
        debugStr(Soper);
        return ERR_P_DB_OPERATION;
    }
    strcpy(name, Sname);
    return 0;
}

#if 0
int AcctInRec(struct pub_exchgnote_str *s_str, char tradetype)
{
    int acctType;
    char bufout[1000];
    char bufin[1000];
    int result;
    int outlen=0; //���ͳ���
    int validNoteFlag = -1;
    int notetype;

    if (result = InitAcctInterFace(s_str))
        return result;

    memset(bufout,' ',sizeof(bufout));
    memset(bufin,' ',sizeof(bufin));
    notetype = atoi(s_str->notetype);

    fprintf(stderr, "notetype=[%d]tradetype=[%c]\n", notetype, tradetype);
    if (notetype == 19) {
        outlen = Pack_1625(s_str,bufout);
        acctType = 1625;
    } else if(tradetype == '2') { //����
        outlen = Pack_1624(s_str,bufout);
        acctType = 1624;
    } else if(tradetype == '1') { //���
        switch(notetype)
        {
            /** ����Ʊ�� */
            case JZD:
            case LZDZPZ:
            case TSCFL:
            case SKTH:
            case CZWHK:
            case ZFMMZP:    //֧������֧Ʊ
                outlen = Pack_1623(s_str,bufout,1); //ʵʱ�ͺ�̨����
                acctType = 1623;
                break;
            /** �ǽ���Ʊ�� */
            case ZP:
            case TCWTSK:
            case TYWTSK:
            case TZZZ:
            case SKJKS:
            case GZ:
            case XH:
            case DH:
            case SKJK2:
            case YHHP:
                outlen = Pack_1623(s_str,bufout,0);//��Ʊ���ɹ�Ա�ͺ�̨����
                acctType = 1623;
                break;
            case BP:
            case SSYSHP:
                //outlen = Pack_1643(s_str,bufout); //��Ʊ��Ʊ����
                acctType = 1643;
                break;
            default:
                return -1;
        }
    }
    result = AcctResult = WSendRecv(bufout, outlen, bufin);
    strcpy(AcctResultStr, AcctResult == 0 ? "���˳ɹ�" : "����ʧ��");

    return result;
}

/*** ���˽ӿ�ģʽ����������� ***/
//tradetype:�����־ '1':�� '2':��
int AcctOutRec(struct pub_exchgnote_str *s_str, char tradetype)
{
    int acctType;
    char bufout[1000];
    char bufin[1000];
    int result;
    int outlen=0; //���ͳ���
    int validNoteFlag = -1;
    int notetype;

    if (result = InitAcctInterFace(s_str))
        return result;

    memset(bufout,' ',sizeof(bufout));
    memset(bufin,' ',sizeof(bufin));
    notetype = atoi(s_str->notetype);

    fprintf(stderr, "notetype=[%s]tradetype=[%c]\n", s_str->notetype, tradetype);
    if (notetype == 19) { //��Ʊ����
        if (tradetype == '2')
            acctType = 1624, outlen = Pack_1624(s_str, bufout);
        else
            acctType = 1623, outlen = Pack_1623(s_str, bufout, 9);
        if (outlen < 0)
            return ERR_P_DB_OPERATION;
        result = WSendRecv(bufout, outlen, bufin);
        if (result != 0)
            return result;
        memset(bufout, 0, sizeof(bufout));
        outlen = Pack_1626(s_str,bufout,acctType);
        acctType = 1626;
    } else if(tradetype == '2') { //����
        outlen = Pack_1622(s_str,bufout);
        acctType = 1622;
    }
    else if(tradetype == '1') //���
    {
        switch(notetype)
        {
            case -1: //��ʾ���׽���
                outlen = Pack_1102(s_str,bufout);
                acctType = 1102;
                break;
            /** ����Ʊ�� */
            case JZD:
            case LZDZPZ:
            case TSCFL:
            case SKTH:
            case CZWHK:
            case ZFMMZP:    //֧������֧Ʊ
                outlen = Pack_1102(s_str,bufout);
                acctType = 1102;
                break;
            /** �ǽ���Ʊ�� */
            case ZP:
            case TCWTSK:
            case TYWTSK:
            case TZZZ:
            case SKJKS:
            case GZ:
            case XH:
            case DH:
            case YHHP:
            case SKJK2:
                outlen = Pack_1621(s_str,bufout);
                acctType = 1621;
                break;
            /** ��Ʊ�� */
            case SSYSHP:
            case BP:
                outlen = Pack_1648(s_str,bufout);
                acctType = 1648;
                break;
            default:
                return -1;
        }
    }
    if (outlen < 0)
        return ERR_P_DB_OPERATION;

    result = AcctResult = WSendRecv(bufout, outlen, bufin);
    strcpy(AcctResultStr, AcctResult == 0 ? "���˳ɹ�" : "����ʧ��");
    if (result == 0) {
        InsertAcctSerial(s_str->outbank, s_str->outserial, bufin, acctType);
    }

    return result;
}
#endif 

/** ��ӡ���������� **/
int dispbuf(char *buf,int buflen)
{
    int j=0;
    debugStr("Disp buf ...");
    for(j=0;j<buflen;j++)
        if(buf[j]==0) fprintf(stderr,"|");
        else fprintf(stderr,"%c",buf[j]);
    fprintf(stderr,"\n");
    debugStr("Disp buf end ...");
}

void ltos_date_oldadm(char *ldate)
{
    int i = 0;
    int j = 0;
    char sdate[11];

    while (ldate[j] != '\0')
    {
        if (ldate[j] == '/')
        j++;
        sdate[i++] = ldate[j++];
    }
    sdate[i] = '\0';
    strcpy(ldate, sdate);
}

int UpdateInTrans(int serial,int flag, s_hand_note *s_hand)
{
    EXEC SQL begin declare section;
    IN_NOTE    DB_InNote;
    char out_bank[7];
    char noteno[21];
    int out_serial;
    char acctround[2];
    char tradedate[11],ptmp[9];
    char acct[33],acctname[81];
    EXEC SQL end declare section;
    char sendbuf[2048];
    int result;

    //��ȡ����
    out_serial=serial;

    fprintf(stderr, "%s %d| noteno:%s\n", __FILE__, __LINE__, s_hand->noteno);
    memset(out_bank,0,sizeof(out_bank));
    memcpy(out_bank, s_hand->outbank, sizeof(out_bank)-1);
    memset(tradedate,0,sizeof(tradedate));
    strcpy(tradedate,s_hand->tradedate);
    strcpy(acct,s_hand->debitacct);
    strcpy(acctname,s_hand->filename);
    strcpy(noteno,s_hand->noteno);
    trim_all(tradedate);
    trim_all(out_bank);
    trim_all(acct);
    trim_all(acctname);
    trim_all(noteno);

    //����ԭ���뽻��

    fprintf(stderr, "%s %d| flag:%d\n", __FILE__, __LINE__, flag);
    fprintf(stderr, "%s %d| out_serial:%d\n", __FILE__, __LINE__, out_serial);
    fprintf(stderr, "%s %d| out_bank:%s\n", __FILE__, __LINE__, out_bank);
    if (flag==0)
    {
        EXEC SQL  select *  from in_note into  :DB_InNote  where out_serial=:out_serial and
        out_bank=:out_bank;
        if ( SQLCODE && SQLCODE != 100 )
        {
            return ERR_P_DB_OPERATION;
        }

        if( SQLCODE == 100 )
        {
            return ERR_P_NO_RECORD;
        }
    }
    else
    {
        EXEC SQL  select *  from hddb..in_note into  :DB_InNote  where out_serial=:out_serial and  out_bank=:out_bank and  trade_date=:tradedate;
        if ( SQLCODE && SQLCODE != 100 )
        {
            return ERR_P_DB_OPERATION;
        }

        if( SQLCODE == 100 )
        {
            return ERR_P_NO_RECORD;
        }
    }

    if (flag==0)
    {
        EXEC SQL  update in_note set debit_acct=:acct, debit_name=:acctname ,note_no=:noteno,debtdetail2='M'  where out_serial=:out_serial and  out_bank=:out_bank;
        if ( SQLCODE && SQLCODE != 100 )
        {
            return ERR_P_DB_OPERATION;
        }
    }
    else
    {
        EXEC SQL  update hddb..in_note set  debit_acct=:acct, debit_name=:acctname,note_no=:noteno,debtdetail2='M'  where out_serial=:out_serial and trade_date=:tradedate and  out_bank=:out_bank;
        if ( SQLCODE && SQLCODE != 100 )
        {
            return ERR_P_DB_OPERATION;
        }
    }

    return  result;
}


/*����ҵ���޸ĺ�̨����*/
void UpdateInNote(struct data_packet *pdat)
{
    int result = 0,dlen;
    int  out_serial;
    int flag;
    hand_note s_bin;
    s_hand_note s_str;
    char tmp[64];

    memset(&s_bin, 0, sizeof s_bin);
    memcpy(&s_bin, pdat->data_buf, sizeof s_bin);
    memset(&s_str, 0, sizeof s_str);
    HandToHandStr(&s_bin, &s_str);
    Prn_HandStr(&s_str);

    if(!OpenDatabase())
    {
        result = ERR_P_DB_OPERATION;
        goto exit;
    }

    out_serial=atoi(s_str.outserial);
    flag=atoi(s_str.bankresult);
    result=UpdateInTrans(out_serial,flag,&s_str);
exit:
    CloseDatabase();
    sprintf(s_bin.result,"%02d",result);
    dlen = sizeof(hand_note);
    memset(tmp, 0, sizeof tmp);
    sprintf(tmp, "%04d", dlen);
    memcpy(pdat->dlen, tmp, 4);
    s_bin.tradeid[0] = ACK_TRADE;
    memcpy(pdat->data_buf,&s_bin,dlen);

    memset(tmp,0,sizeof(tmp));
    sprintf(tmp,"%3d",SYS_ServerNum);
    pdat->mtype = mtoi(pdat->orgnodeno,sizeof(pdat->orgnodeno));
    memcpy(pdat->snodeno,tmp,sizeof(pdat->snodeno));
    memcpy(pdat->dnodeno,pdat->orgnodeno,sizeof(pdat->dnodeno));
    memcpy(pdat->ackpid,pdat->reserved,sizeof(pdat->ackpid));

    itom(pdat->orgpid,getpid(),10);

    pdat->request = ACK_MODE;

    pdat->data_buf[dlen]=0;
    SendMsgByKey(sendkey,exbshmkey,pdat,SEND_AS_SERVER);
    //AckToPubEX(pdat,&s_bin,result);
}

void GetFJBank(char *FJ,int w1,char w2,char *res)
{
    char temp[3000];
    char wtemp[3000];
    int Pos=0;
    int J=0;
    int K=0;

    memset(wtemp,0,sizeof(wtemp));
    memset(temp,0,sizeof(temp));

    strcpy(wtemp,FJ);
    while(1)
    {
        if(Pos>=strlen(wtemp)) break;
        if(wtemp[Pos]==w2) {J++;Pos++;continue; }
        if(J==w1) break;
        if(J==w1-1) temp[K++]=wtemp[Pos];
        Pos++;
    }
    strcpy(res,temp);
}

BOOL Find_OneLine(int ColNo,char *SeaStr,char *SeaFile,char *LineStr)
{
    FILE *fp;
    char str[4096+1];
    char idxstr[4096+1];
    char tmpstr[4096+1];
    BOOL ret=FALSE;
    int i,j,k,m,n;

    if((fp=fopen(SeaFile,"r"))==NULL) return FALSE;

    trim_all(SeaStr);
    while (!feof(fp))
    {
        if(fgets(str,4096,fp)==NULL)
        {
            fclose(fp);
            return ret;
        }
        strcpy(tmpstr,str);
        strcpy(idxstr,(char *)strtok(tmpstr,"|"));
        for(i=2;i<=ColNo;i++)
            strcpy(idxstr,(char *)strtok(NULL,"|"));
        trim_all(idxstr);

        if (strcmp(SeaStr,idxstr)==0)
        {
            ret= TRUE;
            strcpy(LineStr,str);
            break;
        }
    }
    fclose(fp);
    return ret;
}

/*�������ת��*/
int transRet(int bankresult)
{
    char pIniFileName[128], pTmp[128], retcode[7],msg[10];

    memset(pIniFileName, 0, sizeof(pIniFileName));
    memset(pTmp, 0, sizeof(pTmp));

    if( bankresult == 0)
        return 0;

    sprintf(retcode,"%d",bankresult);

    sprintf(pIniFileName,"%s/dat/errcode.dat",getenv("HOME"));
    Find_OneLine(1,retcode,pIniFileName,pTmp);
    trim_all(pTmp);
    trim_all(retcode);
    if (strlen(pTmp)<strlen(retcode))
    {
        return 50; //��������
    }
    else
    {
        GetFJBank(pTmp,3,'|',msg);
        trim_all(msg);
        if(strlen(msg) == 0)
        {
            return 50; //��������
        }
        return (atoi(msg));
    }
}

/*���߹������Ӧ��*/
int ApparatusManage(struct data_packet *pdat)
{
    int result = 0;
    char tmp[4];

    memset(tmp,0,4);
    sprintf(tmp,"%3d",SYS_ServerNum);
    pdat->mtype = mtoi(pdat->orgnodeno,sizeof(pdat->orgnodeno));
    memcpy(pdat->snodeno,tmp,sizeof(pdat->snodeno));
    memcpy(pdat->dnodeno,pdat->orgnodeno,sizeof(pdat->dnodeno));
    memcpy(pdat->ackpid,pdat->reserved,sizeof(pdat->ackpid));

    itom(pdat->orgpid,getpid(),10);

    pdat->request = ACK_MODE;
    pdat->data_buf[0]= ACK_TRADE;

    if(InterFace == 1)
        pdat->mtype = 10;   
    result = SendMsgByKey(sendkey,exbshmkey,pdat,SEND_AS_SERVER);
}

void HandToHandStr(hand_note *s_bin, s_hand_note *s_str)
{
    memcpy(s_str->tradeid, s_bin->tradeid, sizeof s_bin->tradeid);
    memcpy(s_str->tradedate, s_bin->tradedate, sizeof s_bin->tradedate);
    memcpy(s_str->acctround, s_bin->acctround, sizeof s_bin->acctround);
    memcpy(s_str->outbank, s_bin->outbank, sizeof s_bin->outbank);
    memcpy(s_str->outserial, s_bin->outserial, sizeof s_bin->outserial);
    memcpy(s_str->doflag, s_bin->doflag, sizeof s_bin->doflag);
    memcpy(s_str->needflag, s_bin->needflag, sizeof s_bin->needflag);
    memcpy(s_str->localbank, s_bin->localbank, sizeof s_bin->localbank);
    memcpy(s_str->localoper, s_bin->localoper, sizeof s_bin->localoper);
    memcpy(s_str->bankresult, s_bin->bankresult, sizeof s_bin->bankresult);
    memcpy(s_str->filename, s_bin->filename, sizeof s_bin->filename);
    memcpy(s_str->creditacct, s_bin->creditacct, sizeof s_bin->creditacct);
    memcpy(s_str->debitacct, s_bin->debitacct, sizeof s_bin->debitacct);
    memcpy(s_str->noteno, s_bin->noteno, sizeof s_bin->noteno);
    memcpy(s_str->amount, s_bin->amount, sizeof s_bin->amount);
    memcpy(s_str->notetype, s_bin->notetype, sizeof s_bin->notetype);
    memcpy(s_str->result, s_bin->result, sizeof s_bin->result);
}

void Prn_HandStr(s_hand_note *s_str)
{
    fprintf(stderr, "Print s_hand_note Begin\n");
    fprintf(stderr, "tradeid:%s\n", s_str->tradeid);
    fprintf(stderr, "tradedate:%s\n", s_str->tradedate);
    fprintf(stderr, "acctround:%s\n", s_str->acctround);
    fprintf(stderr, "outbank:%s\n", s_str->outbank);
    fprintf(stderr, "outserial:%s\n", s_str->outserial);
    fprintf(stderr, "doflag:%s\n", s_str->doflag);
    fprintf(stderr, "needflag:%s\n", s_str->needflag);
    fprintf(stderr, "localbank:%s\n", s_str->localbank);
    fprintf(stderr, "localoper:%s\n", s_str->localoper);
    fprintf(stderr, "bankresult:%s\n", s_str->bankresult);
    fprintf(stderr, "filename:%s\n", s_str->filename);
    fprintf(stderr, "creditacct:%s\n", s_str->creditacct);
    fprintf(stderr, "debitacct:%s\n", s_str->debitacct);
    fprintf(stderr, "noteno:%s\n", s_str->noteno);
    fprintf(stderr, "amount:%s\n", s_str->amount);
    fprintf(stderr, "notetype:%s\n", s_str->notetype);
    fprintf(stderr, "result:%s\n", s_str->result);
    fprintf(stderr, "Print s_hand_note End\n");
}

void Prn_ExchgNote(struct pub_exchgnote_str *s_str)
{
    fprintf(stderr, "Print Exchgnote Begin\n");
    fprintf(stderr, "tradeid:%s\n", s_str->tradeid);
    fprintf(stderr, "outserial:%s\n", s_str->outserial);
    fprintf(stderr, "outbank:%s\n", s_str->outbank);
    fprintf(stderr, "inbank:%s\n", s_str->inbank);
    fprintf(stderr, "notetype:%s\n", s_str->notetype);
    fprintf(stderr, "noteno:%s\n", s_str->noteno);
    fprintf(stderr, "issuedate:%s\n", s_str->issuedate);
    fprintf(stderr, "debitacct:%s\n", s_str->debitacct);
    fprintf(stderr, "debitname:%s\n", s_str->debitname);
    fprintf(stderr, "debiterbank:%s\n", s_str->debiterbank);
    fprintf(stderr, "creditacct:%s\n", s_str->creditacct);
    fprintf(stderr, "creditname:%s\n", s_str->creditname);
    fprintf(stderr, "crediterbank:%s\n", s_str->crediterbank);
    fprintf(stderr, "oppbank:%s\n", s_str->oppbank);
    fprintf(stderr, "oppbankname:%s\n", s_str->oppbankname);
    fprintf(stderr, "amount:%s\n", s_str->amount);
    fprintf(stderr, "limitamount:%s\n", s_str->limitamount);
    fprintf(stderr, "secret:%s\n", s_str->secret);
    fprintf(stderr, "digest:%s\n", s_str->digest);
    fprintf(stderr, "purpose:%s\n", s_str->purpose);
    fprintf(stderr, "reserved:%s\n", s_str->reserved);
    fprintf(stderr, "netaddr:%s\n", s_str->netaddr);
    fprintf(stderr, "hostaddr:%s\n", s_str->hostaddr);
    fprintf(stderr, "termno:%s\n", s_str->termno);
    fprintf(stderr, "operator:%s\n", s_str->operator);
    fprintf(stderr, "result:%s\n", s_str->result);
    fprintf(stderr, "alarmflag:%s\n", s_str->alarmflag);
    fprintf(stderr, "debtdetail1:%s\n", s_str->debtdetail1);
    fprintf(stderr, "debtdetail2:%s\n", s_str->debtdetail2);
    fprintf(stderr, "Print Exchgnote End\n");
}

