#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "pub.h"
#include "dbutil.h"
#include "tcop.h"
#include "other.h"
#include "region_common.h"

extern int OP_OPNODE;        //OPƽ̨��������ڵ��
extern int OP_NODEID;        //������ƽ̨�淶�ڵ��(4λ)
extern int OP_TCTCODE;       //����ͬ�ǽ�����
extern int OP_OPTCODE;       //�����׶�Ӧ��ƽ̨������
extern int OP_BKTCODE;       //�����׶�Ӧ�����ڽ�����
extern int OP_COMCODE;       //�������ʹ��ͨѶ������
extern int TCOP_BANKID;      //��ʶ��ƽ̨��������к�
extern int OP_DOINIT;        //��ʾ�Ƿ���Ҫ��Ԥ����Ľڵ�����
extern char OP_HOME[];       //OPƽ̨HOMEĿ¼
extern char OP_APPNAME[];

/* ��־������ͨѶǰ(�����������ǰ) */
#define COMMTOPH_BEFORE                         "1"
/* ��־������ͨѶ��(����ͨѶʧ�ܻ�ҵ��Ӧ��) */
#define COMMTOPH_AFTER                          "2"
/* ��־������Ӧ���(�������뽻��Ӧ���,����ͨѶʧ��) */
#define COMMRSPTOPH_AFTER                       "3"

#define OP_REGIONID    (OP_NODEID / 100)                //�������е�����
#define OP_BANKID      (OP_NODEID % 100)                //�������к�(����Ϊ����)
#define OP_PHNODEID    (OP_REGIONID*100+10)             //ǰ�ýڵ��
#define OP_BANKNODE    (OP_NODEID/100*100+TCOP_BANKID)  //�������нڵ�(4λ,��ʾ��ҵ���ж�)

#define OP_OPTBASE     (OP_OPTCODE/100*100)             //ƽ̨�������Ӧ�Ļ���������

//�Ƿ��������
#define isOutTran() ((OP_BANKID) != 10)

//�Ƿ����뽻��
#define isInTran()  ((OP_BANKID) == 10)        

typedef struct BankCommData {
    char head[128];
    int  headlen;
    char body[4096*2];
    int  bodylen;
    char tail[128];
    int  taillen;
    char files[128];
} BankCommData;

typedef struct RegionProcess {
    int tctcode;
    int (*pfunc)(xmlDoc*, xmlDoc **, char *);
} RegionProcess;

typedef struct BusinessProcess {
    int optcode;
    int (*pfunc)(void *, char *);
} BusinessProcess;

/**
 * ������ͨѶЭ����ؽӿ�
 * pCommData->body Ϊ�������ݻ�Ӧ������
 * pCommData->bodylen Ϊ�������ݳ��Ȼ�Ӧ�����ݳ���
 */
typedef int (*FPBankCommProtocol)(void *pCommData);

/**
 * �ӿ�ʵ��: ����ͨѶЭ��
 * ����ֵ:
 * ͨѶʧ��:<0
 * ͨѶ�ɹ�:��Ӧ���Ľ������ͱ�־
 */
extern int CommProtocol(void *pcd);

/**
 * �ӿ�ʵ��: ����ͨѶ��Ӧ��������Э��
 * ����ֵ���ݾ���ı�������
 */
extern int CommResponseProtocol(BankCommData *rsp);

//��ȡ������ͨѶ���������峤�ȵĺ����ӿ�
typedef int (*FPGetRecvBodyLength)(char *head);

/**
 * ������ͨѶ����(ͨ��ϵͳ����TCOP_BANKADDR=ip:port:timeout)��ȡͨѶ����)
 */
int CommToBank(BankCommData *preq, BankCommData *prsp, FPGetRecvBodyLength func);

int CommSocket(char *envName, unsigned char *req, int reqlen, unsigned char *rsp, int *rsplen);

/* 
 * �����ڽ��׵��ýӿ�
 * bktcode: ���еĽӿڽ�����
 * opDoc: ƽ̨��Ӧ�ı���
 */
int callInterface(int bktcode, xmlDoc *opDoc);

/*
 * ƽ̨�쳣������
 * 
 * rspbuf: ��Ӧ����
 * rsplen: ԭʼ���쳣�����Ӧ���ĵĳ���
 * operr: ƽ̨������
 *
 * ����: �ɹ� 0 ʧ�� ����
 */
typedef int (*FPSetException)(char *, int *, int);

/** 
 * �ӿ�ʵ��: �������������뽻�׳��ֵ��쳣������д���Ľӿ�
 */
extern int SetException(xmlDoc *, char *, int *, int);

extern RegionProcess G_RPROCESS[];   //ȫ�ֵ����⴦����
extern BusinessProcess G_BPROCESS[]; //ȫ��ҵ��⴦����

#endif
