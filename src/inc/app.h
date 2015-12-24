#ifndef APP_H_
#define APP_H_

#include "pub.h"
#include "pack.h"
#include "interface.h"

#ifndef ERRLOG
#define ERRLOG     logfname("errlog")
#endif

extern char OP_APPTYPE;                     /* Ӧ������ */
extern char OP_APPPK;                       /* Ӧ���Ƿ��Ѻ��Ѻ */
extern int  OP_APPEXTEND;                   /* ƽ̨�ڲ�Ӧ���Ƿ���Ҫ��չ */

extern char G_REQFILE[];                    /* �����ļ��б� */
extern char G_RSPFILE[];                    /* ��Ӧ�ļ��б� */

/* Ӧ������ */
#define APPTYPE_LOCAL_SERVER        'L'     /* ����Ӧ��(�����ö�̬����) */
#define APPTYPE_OPBANK_SERVER       'I'     /* �ӿ�Ӧ��(��Ҫ���ö�̬����) */
#define APPTYPE_OPBANK_AUTOSVR      'C'     /* �ӿ�Ӧ��(�Զ�����һ��������ؽ���) */
#define APPTYPE_OUTTRANS_SERVER     'O'     /* ���Ӧ�� */
#define APPTYPE_REGION_SERVER       'R'     /* ����Ӧ�� */
#define APPTYPE_UNDEF               '0'     /* Ӧ������δ���� */

#define APPPK_CHECK                 'Y'     /* ��Ҫ��Ѻ��Ѻ */
#define APPPK_UNCHECK               'N'     /* ����Ҫ��Ѻ��Ѻ */

int initApp(char *reqnode, char *reqtcode, char *);

xmlNodePtr requestDispatcher(xmlNodeSet *ns, char *reqbuf);

/*
 * ҵ����������
 *
 * ����:
 * reqbuf: ������
 *
 * ���:
 * rspbuf: Ӧ����
 *
 * plen: �����ĳ��Ȼ�Ӧ���ĳ���
 *
 * ����:ʧ�� ���� �ɹ� 0
 */
int appsMain(xmlDoc **doc, char *reqbuf, char *rspbuf, int *plen);

/*
 * ƽ̨Ӧ�ô���
 *
 * ����: �ɹ� 0
 */
int opAppMain(xmlDoc *opDoc);

/* 
 * ���Ӧ�ô���
 *
 * ����: �ɹ� 0
 */
int outAppMain(xmlDoc *opDoc, char *reqbuf, char *rspbuf, int *plen);

int opRegionTrans(char *reqbuf, char *rspbuf, int *plen);

int opUndefInTrans(char *reqbuf, char *rspbuf, int *plen);

int opException(xmlDoc *, char *rspbuf, int *rsplen, int operr);

int OP_DoInit(char *reqbuf, int *reqlen);

int OP_DoFinish(xmlDoc *opDoc, int opret);

int DigestHandle(char *reqbuf, int *plen);

#endif
