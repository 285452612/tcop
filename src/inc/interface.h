#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "pub.h"
#include "dbutil.h"
#include "tcop.h"
#include "other.h"
#include "region_common.h"

extern int OP_OPNODE;        //OP平台本身虚拟节点号
extern int OP_NODEID;        //请求交易平台规范节点号(4位)
extern int OP_TCTCODE;       //请求同城交易码
extern int OP_OPTCODE;       //请求交易对应的平台交易码
extern int OP_BKTCODE;       //请求交易对应的行内交易码
extern int OP_COMCODE;       //提出交易使用通讯交易码
extern int TCOP_BANKID;      //标识该平台部署的银行号
extern int OP_DOINIT;        //表示是否需要做预处理的节点请求
extern char OP_HOME[];       //OP平台HOME目录
extern char OP_APPNAME[];

/* 标志与人行通讯前(用于提出请求前) */
#define COMMTOPH_BEFORE                         "1"
/* 标志与人行通讯后(包含通讯失败或业务应答) */
#define COMMTOPH_AFTER                          "2"
/* 标志与人行应答后(用于提入交易应答后,包含通讯失败) */
#define COMMRSPTOPH_AFTER                       "3"

#define OP_REGIONID    (OP_NODEID / 100)                //联机银行地区号
#define OP_BANKID      (OP_NODEID % 100)                //联机银行号(可能为中心)
#define OP_PHNODEID    (OP_REGIONID*100+10)             //前置节点号
#define OP_BANKNODE    (OP_NODEID/100*100+TCOP_BANKID)  //联机银行节点(4位,表示商业银行端)

#define OP_OPTBASE     (OP_OPTCODE/100*100)             //平台交易码对应的基本交易码

//是否提出交易
#define isOutTran() ((OP_BANKID) != 10)

//是否提入交易
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
 * 与行内通讯协议加载接口
 * pCommData->body 为请求数据或应答数据
 * pCommData->bodylen 为请求数据长度或应答数据长度
 */
typedef int (*FPBankCommProtocol)(void *pCommData);

/**
 * 接口实现: 银行通讯协议
 * 返回值:
 * 通讯失败:<0
 * 通讯成功:响应报文解析类型标志
 */
extern int CommProtocol(void *pcd);

/**
 * 接口实现: 银行通讯响应报文设置协议
 * 返回值根据具体的报文配置
 */
extern int CommResponseProtocol(BankCommData *rsp);

//获取与行内通讯接收数据体长度的函数接口
typedef int (*FPGetRecvBodyLength)(char *head);

/**
 * 与银行通讯函数(通过系统变量TCOP_BANKADDR=ip:port:timeout)获取通讯参数)
 */
int CommToBank(BankCommData *preq, BankCommData *prsp, FPGetRecvBodyLength func);

int CommSocket(char *envName, unsigned char *req, int reqlen, unsigned char *rsp, int *rsplen);

/* 
 * 与行内交易调用接口
 * bktcode: 银行的接口交易码
 * opDoc: 平台对应的报文
 */
int callInterface(int bktcode, xmlDoc *opDoc);

/*
 * 平台异常处理函数
 * 
 * rspbuf: 响应报文
 * rsplen: 原始或异常处理后应答报文的长度
 * operr: 平台错误码
 *
 * 返回: 成功 0 失败 其它
 */
typedef int (*FPSetException)(char *, int *, int);

/** 
 * 接口实现: 针对所有提出提入交易出现的异常情况进行处理的接口
 */
extern int SetException(xmlDoc *, char *, int *, int);

extern RegionProcess G_RPROCESS[];   //全局地区库处理器
extern BusinessProcess G_BPROCESS[]; //全局业务库处理器

#endif
