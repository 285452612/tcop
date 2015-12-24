/**********************************************
    pubdef.h
    ���ö����ļ�
    Created by SUNLAN
    2006/06/22
**********************************************/

#ifndef __PUBDEF_H__
#define __PUBDEF_H__

#include "SDKpub.h"
#include "SDKxml.h"

#define ICCSDBNAME      "iccdb"

#ifndef ERRLOG
#define ERRLOG      logfname( "err" )
#endif
#ifndef SMPLOG
#define SMPLOG      logfname( "smp" )
#endif
#define TRNLOG      logfname( "trans" )
#define MNTNLOG     logfname( "mntn" )
#define XMLLOG      logfname( "xmlerror" )

#define SYS_NAME "��ͨͬ������ϵͳ"

#define TEMPLATE_PATH       "data"      /* $HOME/data */
#define RECON_DATA_PATH     "bankdata"  /* $HOME/bankdata */

typedef int *(*UFTPFunc)();

typedef struct func_list
{
    char *pcFuncName;
    int (*pFunc)();
} func_list;

xmlDocPtr xPrivate; // �ڲ���������
extern char gcaRequestFile  [ 600 ];   // ���ı������յ��ļ��б�
extern char gcaResponseFile [ 600 ];   // ������Ӧ���Է����ļ��б�
extern char gcaSendFile     [ 600 ];   // �����������͵��ļ��б�
extern char gcaRecvFile     [ 600 ];   // ���Ľ��յ���Ӧ���ļ��б�
extern char FileType     [ 2 ]; // �ļ����䷽ʽ 'A' ASCII��ʽ, 'B' BINARY��ʽ

#define CHAR_SPLIT '|'
/******************************
    ���ݱ������Ͷ���
******************************/
#define MSGRQ           0               /* ������ */
#define MSGRS           1               /* ��Ӧ���� */

/******************************
    �������ͳ��ȶ���
******************************/

#define COMMBUFF_MAX    32768           /* ���ͨ�����ݰ�����*/
#define TRNCODE_MAX     6               /* ���״�����󳤶� */
#define ORGID_MAX       12              /* ����������󳤶� */
#define NODE_MAX        8               /* �ڵ����󳤶� */
#define ORGNAME_MAX     80              /* ����������󳤶� */
#define CUSTNAME_MAX    ORGNAME_MAX     /* �ͻ�������󳤶� */
#define ACCT_MAX        32              /* ����˺ų��� */
#define REGION_MAX      6               /* �������������󳤶� */
#define TERMID_MAX      6               /* �ն˺���󳤶� */
#define RESULT_MAX      4               /* ���׽��������󳤶� */
#define MAC_MAX         32              /* MAC��󳤶� */
#define PAYKEY_KEN      10
#define SQLBUFF         4096

/***********************************************
    ���׷���־.�Խ��������˻�����Ϊ׼
***********************************************/
#define PART_DEBIT      1           /* �跽 */
#define PART_CREDIT     2           /* ���� */
#define PART_ZS         3           /* ָʾ�� */

/*****************************************
    �������Ͷ���
    RULE_XXX
*****************************************/
#define RULE_WORKDATE   1           /* ���������ù��� */
#define RULE_NOTE       2           /* ƾ֤Ҫ�ؼ����� */
#define RULE_OPER       3           /* ����ԱȨ�޹��� */
#define RULE_TRAN       4           /* ���Ľ��״������ */
#define RULE_PRETRAN    5           /* ǰ�ý��״������ */

/*****************************************
    ������������
    REGION_XX
*****************************************/
#define REGION_01       1           /* ʡ,������,ֱϽ�� */
#define REGION_02       2           /* ʡϽ��,���� */
#define REGION_03       3           /* ��Ͻ��,��,�ؼ��� */

/**********************************************
    ���Ҵ��붨��
    ����ISO 4217��׼
**********************************************/

#define CURR_RMB     156        /* ����� */
#define CURR_HKD     344        /* �۱� */
#define CURR_USD     840        /* ��Ԫ */
#define CURR_GBP     826        /* Ӣ�� */
#define CURR_EUR     978        /* ŷԪ */
#define CURR_JPY     392        /* ��Ԫ */

/************************************
    �������.����������ҳ������
    ��CURRT_��ͷ
************************************/
#define CURRT_EXCH_A    "2"     /* ��ҵ����ֻ� */
#define CURRT_PAPER_B   "3"     /* �����������ֳ� */
#define CURRT_EXCH_B    "4"     /* �����������ֻ� */
#define CURRT_PAPER_C   "0"     /* ���ھ�������ֳ�(�������) */
#define CURRT_EXCH_C    "6"     /* ���ھ�������ֻ� */

/*******************************************
    �ڵ㶨��
*******************************************/
#define CENTER_NODE         "000"
#define INTERFACE_NODE      "999"

/*******************************************
    �������Ͷ���
    OTYPE_XXX
*******************************************/
#define OTYPE_BANK        	"1"     /* ���л��� */

/*******************************************
    ����������
    OLEVEL_XXX
*******************************************/
#define OLEVEL_HQ			1       /* �ܲ�(������) */

/***********************************************
    �˻�״̬
    ACCTSTATE_XXX 
***********************************************/
#define ACCTSTATE_NOTPERMIT         '0'       /* δ��� */
#define ACCTSTATE_PERMITTED         '1'       /* ��� */
#define ACCTSTATE_FROZEN            '5'       /* ���� */

/***********************************************
    ����Ա���Ͷ���
    OPER_XXXX
***********************************************/
#define OPER_SYSADM         1       /* ϵͳ����Ա */
#define OPER_ADM            2       /* ҵ������ */
#define OPER_GNR            3       /* ��ͨ����Ա */

/**************************************************
    ����Ա״̬
    OPERSTAT_XXXX
**************************************************/
#define OPERSTAT_NOTUSED    '0'     /* δ���� */
#define OPERSTAT_NORMAL     '1'     /* ����(��ǩ��) */
#define OPERSTAT_LOGOUT     '2'     /* ��ǩ�� */
#define OPERSTAT_CANCEL     '9'     /* ��ע�� */

/**************************************************
    ����״̬
    ORGSTAT_XXXX
**************************************************/
#define ORGSTAT_NOTUSED    '0'     /* δ���� */
#define ORGSTAT_NORMAL     '1'     /* ���� */
#define ORGSTAT_CANCEL     '2'     /* ͣ�� */

/***************************************************
    ƾ֤���ඨ��
    NOTE_XXXX
***************************************************/
#define NOTE_CHECK          2       /* ת��֧Ʊ */
#define NOTE_REMIT          4       /* ���ƾ֤ */
#define NOTE_PROM           21      /* ���б�Ʊ */
#define NOTE_DRAFT          41      /* ȫ�����л�Ʊ */
#define NOTE_ZONE_DRAFT     42      /* ��ʡһ��/���������л�Ʊ */
#define NOTE_CORRXFER       44      /* ���˴�ת����ƾ֤
                                       Correspondent Transfer */
#define NOTE_CONPAY         46      /* ������֧�� Concentration Payment */
#define NOTE_COLLECTION     52      /* ����ƾ֤ */
#define NOTE_PDC            53      /* ���ڽ�/���� Prearranged Debit&Credit */
#define NOTE_FP             56      /* ��������֧��ƾ֤ Fiscal Payment */
#define NOTE_DRAWBACK       58      /* ˰���˻��� */
#define NOTE_TAXPAY         59      /* ˰�ѽɿ��� */
#define NOTE_SPCXFER        61      /* ����ת��ƾ֤ Special transfer */
#define NOTE_NETBANK        62      /* ���� */
#define NOTE_CASH           71      /* �ֽ��ȡ�� */
#define NOTE_PXFER          72      /* �����˻���ת�� personal transfer */
#define NOTE_POSXFER        75      /* ���п�POSת��(�տ���һ��һ��) */
#define NOTE_POSXFER_MULTI  76      /* ���п�POSת��(�տ��ж��һ��) */
#define NOTE_FORCHECK       81      /* ���ת��֧Ʊ */
#define NOTE_DRAFT_103      82      /* ���MT103 */
#define NOTE_DRAFT_202      83      /* ���MT202 */
#define NOTE_PFORXFER       84      /* ��������˻���ת�� */
#define NOTE_FORZS_OUT      88      /* ���ָʾ���� */
#define NOTE_FORZS_IN       89      /* ���ָʾ���� */
#define NOTE_BDC            91      /* ������/���� Batch Debit&Credit */

/***************************************************
    ҵ�����ඨ��
    CLASS_XXXX
***************************************************/
#define CLASS_UNIT              1
#define CLASS_PERSON            2
#define CLASS_FOREIGN           3
#define CLASS_BATCH             4

/***************************************************
    ϵͳ״̬����
    SYSSTAT_XXXX
***************************************************/
#define SYSSTAT_DISABLE     0       /* ϵͳ���� */
#define SYSSTAT_NORMAL      1       /* ϵͳ�ռ��������� */

/*****************************************
    ����״̬��־
    TRNSTAT_XXXXXXX
*****************************************/
#define TRNSTAT_NOTCHECKED      "0"     /* ����δ����.���ڶ���¼�� */
#define TRNSTAT_CHECKED         "1"     /* �����Ѹ���(����) */
#define TRNSTAT_PROCESSED       "2"     /* �����Ѵ��� */

/*****************************************
    �ʽ����㷽ʽ
    CLRTYP_XXXXXXX
*****************************************/
#define CLRTYP_DEBIT             1      /* ��ǽ������� */
#define CLRTYP_CREDIT            2      /* ���ǽ������� */
#define CLRTYP_POS_DEBIT         3      /* POS��ǽ������� */
#define CLRTYP_POS_CREDIT        4      /* POS���ǽ������� */

/*****************************************
    ���������־
    CLRSTAT_XXXXXXX
*****************************************/
//#define CLRSTAT_UNSETTLED       '0'     /* δ(��)���� */
//#define CLRSTAT_SETTLED         '1'     /* ����ɹ� */
//#define CLRSTAT_WAITFUNDHOLD    '2'     /* ��Ȧ�� */
//#define CLRSTAT_FUNDHOLD        '3'     /* ��Ȧ�� */
//#define CLRSTAT_INQUEUE_NOHOLD  '4'     /* δȦ��, ������ */
//#define CLRSTAT_INQUEUE_HOLD    '5'     /* ��Ȧ��, ������ */
//#define CLRSTAT_END             '6'     /* ����Ҫ���� */
//#define CLRSTAT_CANCELFAIL      '8'     /* ����ʧ�ܣ�Ȧ��δ��� */
//#define CLRSTAT_FAILED          '9'     /* ����ʧ�� */
//#define CLRSTAT_CHECKED         'C'     /* �Ѷ��� */
//#define CLRSTAT_UNDOED          'U'     /* �ѳ��� */

/*****************************************
    ��ά�����������
    SMPTYPE_XXXXXXX
*****************************************/
#define SMPTYPE_QUERY           "1"     /* ��ѯ */
#define SMPTYPE_RESEND          "2"     /* ���� */
#define SMPTYPE_REVERSE         "3"     /* ���� */
#define SMPTYPE_SYNC            "4"     /* ͬ�� */
#define SMPTYPE_HOLDFUND        "5"     /* ��Ȧ�� */

#define TIMES_RESEND            10      /* ��������*/
#define TIMES_QUERY             10      /* ��ѯ����*/
#define TIMES_UNDO              10      /* ��������*/


/*****************************************
    Ʊ����֤����
    AUTHTYPE_XXXXXX
*****************************************/
#define AUTHTYPE_PAYCODE            '1'     /* ֧������*/
#define AUTHTYPE_CORPCODE           '2'     /* ��λ����*/
#define AUTHTYPE_FOREIGNCODE        '3'     /* �����Ѻ*/
#define AUTHTYPE_PINCODE            '4'     /* �����˻����� */
#define AUTHTYPE_BANKCODE           '5'     /* ������Ѻ*/

/******************************************
    ���˱�־
    RECONFLAG_XXXX
    added by SUNLAN 2006/08/28
******************************************/
#define RECONFLAG_READY         '1'     /* �������������� */
#define RECONFLAG_GET           '2'     /* ���������ȡ�������� */
#define RECONFLAG_SUCC          '3'     /* ����������˳ɹ� */
#define RECONFLAG_FAILD         '4'     /* �����������ʧ��(��ƽ) */

/******************************************
    �����ļ��м�¼�ķ����־
    RECONREC_X
******************************************/
#define RECONREC_PRES           '1'     /* ������� */
#define RECONREC_ACPT           '2'     /* ���뽻�� */

/*****************************************
    �ʼ�����
    MAILTYPE_X
*****************************************/
#define MAILTYPE_MAIL           '0'
#define MAILTYPE_CXS            '1'
#define MAILTYPE_CFS            '2'
#define MAILTYPE_CXS_DRAFT      '3'
#define MAILTYPE_CFS_DRAFT      '4'

#define ACCTFROMBASE    "/UFTP/NoteInfo/AcctFromInfo"
#define ACCTTOBASE      "/UFTP/NoteInfo/AcctToInfo"
#define SETTLBASE       "/UFTP/SettlInfo"
#define MSGHDRRQBASE    "/UFTP/MsgHdrRq"
#define MSGHDRRSBASE    "/UFTP/MsgHdrRs"


#endif  /* __PUBDEF_H__ */
                                                   
