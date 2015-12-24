#ifndef TCOP_H_
#define TCOP_H_

#include "pub.h"
#include "app.h"
#include "optcode.h"
#include "tcutil.h"
#include "dbutil.h"

/** 平台交易清算标志 */
#define CLRSTAT_UNSETTLED    '0'        /* 未(待)清算 */
#define CLRSTAT_SETTLED      '1'        /* 清算成功 */
#define CLRSTAT_FAILED       '9'        /* 清算失败 */
#define CLRSTAT_UNKNOW       '7'        /* 清算状态未知 */
#define CLRSTAT_CHECKED      'C'        /* 已对账 */

/** 提出提入标志定义 */
#define OP_OUTTRAN_FLAG      "1"        /* 提出标志 */
#define OP_INTRAN_FLAG       "2"        /* 提入标志 */

#define OP_DEBITTRAN         '1'        /* 借记交易 */
#define OP_CREDITTRAN        '2'        /* 贷记交易 */

#define ORGLEVEL_CBANK       '1'        /* 清算行 */
#define ORGLEVEL_EBANK       '2'        /* 交换行 */
#define ORGLEVEL_OBANK       '3'        /* 开户行 */ 

/** 柜员登录状态定义 */
#define OPER_STATUS_LOGOFF  '0'         //未登录
#define OPER_STATUS_LOGIN   '1'         //已登录
#define OPER_STATUS_CANCEL  '9'         //已注销

#define OPER_LEVEL_COMMON   '1'         //普通操作员
#define OPER_LEVE_ADMIN     '2'         //业务主管     
#define OPER_LEVE_SYSADM    '3'         //系统管理员

/* 查询查复书状态 */
#define QUERY_STATE_NOREPLY '0'         //查询书未回复
#define QUERY_STATE_REPLIED '1'         //查询书已回复

/*
 * 平台交易预处理程序
 *
 * 返回: 成功 0 失败 其它
 */
int OPInitOPTran(xmlDoc *doc);


char *GetTrnjourWhere(xmlDoc *doc);
/*
 * 获取平台单笔交易的where条件 
 *
 * 注: 若报文中工作日期为空则使用系统工作日期 
 */
char *GetSigleTrnjourWhere(xmlDoc *doc);

char *GetSigleQueryWhere(xmlDoc *doc);

char *GetSigleFreemsgWhere(xmlDoc *doc);

/*
 * 检查提入交易合法性
 * 
 * 返回 合法提入交易 0 异常提入 其它
 */ 
int CheckInTrans(xmlDoc *doc);

int OPAfterCommToPH(xmlDoc *doc);

int OPTranAfterCommToPH(xmlDoc *doc, int tcRet);
int OPInfoAfterCommToPH(xmlDoc *doc, int tcRet);
int OPAdmAfterCommToPH(xmlDoc *doc, int tcRet);

#endif
