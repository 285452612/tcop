#ifndef CZ_CONST_H_
#define CZ_CONST_H_

#include "tcpapi.h"
#include "interface.h"
#include "lswitch.h"

/*----------------------------------------------------------------------*/
/*  ������¼                                                            */
/*  ��ڲ���:                                                           */
/*      HostName:���н�������,�������趨���� /etc/hosts ��            */
/*      Port:��������˿�,Ĭ�� 1400                                   */
/*         �����������˿ڸı�,�˴��������˿�һ��                  */
/*  JYM:���ݲ�ͬ�Ľ�����д��ͬ�Ľ�����                                  */
/*              "11":����ƾ֤       "12":ȡƾ֤(�������±�־)           */
/*              "13":ȡƾ֤(������)     "14":����ƾ֤��־               */  
/*      "13" �� "14" ����ʹ��,������ͬ                                  */
/*          "21":�����ִ       "22":ȡ��ִ(�������±�־)               */
/*      "23":ȡ��ִ(������) "24":���»�ִ��־                           */
/*          "31":�����ʺ�       "32":ȡ�ʺ�(�������±�־)               */
/*      "33":ȡ�ʺ�(������) "34":�����ʺű�־                           */
/*          "42":ȡ������ϸ(�������±�־)                               */
/*          "43":ȡ������ϸ(������) "44":����������ϸ��־               */
/*          "52":ȡ��������(�������±�־)                               */
/*          "53":ȡ��������(������) "54":���¶������ݱ�־               */
/*      Buffer:������¼�Ļ�����                                         */
/*  SendLength:���͵ĳ���                                               */
/*  ReceiveLength:���յĳ���                                            */
/*  ErrorMessage:������Ϣ(200�ֽ�����)                                  */
/*  ����ֵ:                                                             */
/*      0:��ȷ                                                          */
/*      ����:���� ,���������ErrorMessage                               */
/*      ����ȡ����:���� DATABASE_NODATA(10)��ʾ��ǰ����������           */
/*----------------------------------------------------------------------*/
int SwitchRecord(char *HostName, int Port, char *JYM, char *Buffer,
        int SendLength, int *ReceiveLength, char *ErrorMessage);

typedef struct ExchgInfo {
    char exchgNo[12+1];
    char prehostAddr[40+1];
    int prehostPort;
} ExchgInfo;

typedef struct TransInfo {
    int commcode;
    char tctcode[4];
    int timePeriod;
    int exchgnoPos;
} TransInfo;

extern struct ExchgInfo G_ExchgInfos[10];
extern struct TransInfo G_TransInfos[40];

extern char G_REQFILE[];
extern char G_RSPFILE[];

#endif
