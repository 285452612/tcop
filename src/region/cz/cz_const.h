#ifndef CZ_CONST_H_
#define CZ_CONST_H_

#include "tcpapi.h"
#include "interface.h"
#include "lswitch.h"

/*----------------------------------------------------------------------*/
/*  交换记录                                                            */
/*  入口参数:                                                           */
/*      HostName:人行结点机名字,此名字需定义在 /etc/hosts 中            */
/*      Port:结点机服务端口,默认 1400                                   */
/*         如果结点机服务端口改变,此处需与服务端口一致                  */
/*  JYM:根据不同的交易填写不同的交易码                                  */
/*              "11":插入凭证       "12":取凭证(立即更新标志)           */
/*              "13":取凭证(不更新)     "14":更新凭证标志               */  
/*      "13" 与 "14" 配套使用,以下雷同                                  */
/*          "21":插入回执       "22":取回执(立即更新标志)               */
/*      "23":取回执(不更新) "24":更新回执标志                           */
/*          "31":插入帐号       "32":取帐号(立即更新标志)               */
/*      "33":取帐号(不更新) "34":更新帐号标志                           */
/*          "42":取清算明细(立即更新标志)                               */
/*          "43":取清算明细(不更新) "44":更新清算明细标志               */
/*          "52":取对帐数据(立即更新标志)                               */
/*          "53":取对帐数据(不更新) "54":更新对帐数据标志               */
/*      Buffer:交换记录的缓冲区                                         */
/*  SendLength:发送的长度                                               */
/*  ReceiveLength:接收的长度                                            */
/*  ErrorMessage:错误信息(200字节以内)                                  */
/*  返回值:                                                             */
/*      0:正确                                                          */
/*      其它:错误 ,错误解释在ErrorMessage                               */
/*      对于取数据:返回 DATABASE_NODATA(10)表示当前库中无数据           */
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
