#ifndef TCOP_H_
#define TCOP_H_

#include "pub.h"
#include "app.h"
#include "optcode.h"
#include "tcutil.h"
#include "dbutil.h"

/** ƽ̨���������־ */
#define CLRSTAT_UNSETTLED    '0'        /* δ(��)���� */
#define CLRSTAT_SETTLED      '1'        /* ����ɹ� */
#define CLRSTAT_FAILED       '9'        /* ����ʧ�� */
#define CLRSTAT_UNKNOW       '7'        /* ����״̬δ֪ */
#define CLRSTAT_CHECKED      'C'        /* �Ѷ��� */

/** ��������־���� */
#define OP_OUTTRAN_FLAG      "1"        /* �����־ */
#define OP_INTRAN_FLAG       "2"        /* �����־ */

#define OP_DEBITTRAN         '1'        /* ��ǽ��� */
#define OP_CREDITTRAN        '2'        /* ���ǽ��� */

#define ORGLEVEL_CBANK       '1'        /* ������ */
#define ORGLEVEL_EBANK       '2'        /* ������ */
#define ORGLEVEL_OBANK       '3'        /* ������ */ 

/** ��Ա��¼״̬���� */
#define OPER_STATUS_LOGOFF  '0'         //δ��¼
#define OPER_STATUS_LOGIN   '1'         //�ѵ�¼
#define OPER_STATUS_CANCEL  '9'         //��ע��

#define OPER_LEVEL_COMMON   '1'         //��ͨ����Ա
#define OPER_LEVE_ADMIN     '2'         //ҵ������     
#define OPER_LEVE_SYSADM    '3'         //ϵͳ����Ա

/* ��ѯ�鸴��״̬ */
#define QUERY_STATE_NOREPLY '0'         //��ѯ��δ�ظ�
#define QUERY_STATE_REPLIED '1'         //��ѯ���ѻظ�

/*
 * ƽ̨����Ԥ�������
 *
 * ����: �ɹ� 0 ʧ�� ����
 */
int OPInitOPTran(xmlDoc *doc);


char *GetTrnjourWhere(xmlDoc *doc);
/*
 * ��ȡƽ̨���ʽ��׵�where���� 
 *
 * ע: �������й�������Ϊ����ʹ��ϵͳ�������� 
 */
char *GetSigleTrnjourWhere(xmlDoc *doc);

char *GetSigleQueryWhere(xmlDoc *doc);

char *GetSigleFreemsgWhere(xmlDoc *doc);

/*
 * ������뽻�׺Ϸ���
 * 
 * ���� �Ϸ����뽻�� 0 �쳣���� ����
 */ 
int CheckInTrans(xmlDoc *doc);

int OPAfterCommToPH(xmlDoc *doc);

int OPTranAfterCommToPH(xmlDoc *doc, int tcRet);
int OPInfoAfterCommToPH(xmlDoc *doc, int tcRet);
int OPAdmAfterCommToPH(xmlDoc *doc, int tcRet);

#endif
