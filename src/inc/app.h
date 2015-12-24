#ifndef APP_H_
#define APP_H_

#include "pub.h"
#include "pack.h"
#include "interface.h"

#ifndef ERRLOG
#define ERRLOG     logfname("errlog")
#endif

extern char OP_APPTYPE;                     /* 应用类型 */
extern char OP_APPPK;                       /* 应用是否加押核押 */
extern int  OP_APPEXTEND;                   /* 平台内部应用是否需要扩展 */

extern char G_REQFILE[];                    /* 请求文件列表 */
extern char G_RSPFILE[];                    /* 响应文件列表 */

/* 应用类型 */
#define APPTYPE_LOCAL_SERVER        'L'     /* 本地应用(不调用动态处理) */
#define APPTYPE_OPBANK_SERVER       'I'     /* 接口应用(需要调用动态处理) */
#define APPTYPE_OPBANK_AUTOSVR      'C'     /* 接口应用(自动调用一次行内相关交易) */
#define APPTYPE_OUTTRANS_SERVER     'O'     /* 提出应用 */
#define APPTYPE_REGION_SERVER       'R'     /* 地区应用 */
#define APPTYPE_UNDEF               '0'     /* 应用类型未定义 */

#define APPPK_CHECK                 'Y'     /* 需要加押核押 */
#define APPPK_UNCHECK               'N'     /* 不需要加押核押 */

int initApp(char *reqnode, char *reqtcode, char *);

xmlNodePtr requestDispatcher(xmlNodeSet *ns, char *reqbuf);

/*
 * 业务处理主程序
 *
 * 输入:
 * reqbuf: 请求报文
 *
 * 输出:
 * rspbuf: 应答报文
 *
 * plen: 请求报文长度或应答报文长度
 *
 * 返回:失败 其它 成功 0
 */
int appsMain(xmlDoc **doc, char *reqbuf, char *rspbuf, int *plen);

/*
 * 平台应用处理
 *
 * 返回: 成功 0
 */
int opAppMain(xmlDoc *opDoc);

/* 
 * 提出应用处理
 *
 * 返回: 成功 0
 */
int outAppMain(xmlDoc *opDoc, char *reqbuf, char *rspbuf, int *plen);

int opRegionTrans(char *reqbuf, char *rspbuf, int *plen);

int opUndefInTrans(char *reqbuf, char *rspbuf, int *plen);

int opException(xmlDoc *, char *rspbuf, int *rsplen, int operr);

int OP_DoInit(char *reqbuf, int *reqlen);

int OP_DoFinish(xmlDoc *opDoc, int opret);

int DigestHandle(char *reqbuf, int *plen);

#endif
