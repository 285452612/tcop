/**********************************************
    pubdef.h
    公用定义文件
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

#define SYS_NAME "南通同城清算系统"

#define TEMPLATE_PATH       "data"      /* $HOME/data */
#define RECON_DATA_PATH     "bankdata"  /* $HOME/bankdata */

typedef int *(*UFTPFunc)();

typedef struct func_list
{
    char *pcFuncName;
    int (*pFunc)();
} func_list;

xmlDocPtr xPrivate; // 内部公共变量
extern char gcaRequestFile  [ 600 ];   // 中心被动接收的文件列表
extern char gcaResponseFile [ 600 ];   // 中心响应给对方的文件列表
extern char gcaSendFile     [ 600 ];   // 中心主动发送的文件列表
extern char gcaRecvFile     [ 600 ];   // 中心接收到响应的文件列表
extern char FileType     [ 2 ]; // 文件传输方式 'A' ASCII方式, 'B' BINARY方式

#define CHAR_SPLIT '|'
/******************************
    数据报文类型定义
******************************/
#define MSGRQ           0               /* 请求报文 */
#define MSGRS           1               /* 响应报文 */

/******************************
    数据类型长度定义
******************************/

#define COMMBUFF_MAX    32768           /* 最大通信数据包长度*/
#define TRNCODE_MAX     6               /* 交易代码最大长度 */
#define ORGID_MAX       12              /* 机构代码最大长度 */
#define NODE_MAX        8               /* 节点号最大长度 */
#define ORGNAME_MAX     80              /* 机构名称最大长度 */
#define CUSTNAME_MAX    ORGNAME_MAX     /* 客户名称最大长度 */
#define ACCT_MAX        32              /* 最大账号长度 */
#define REGION_MAX      6               /* 行政区域代码最大长度 */
#define TERMID_MAX      6               /* 终端号最大长度 */
#define RESULT_MAX      4               /* 交易结果代码最大长度 */
#define MAC_MAX         32              /* MAC最大长度 */
#define PAYKEY_KEN      10
#define SQLBUFF         4096

/***********************************************
    交易方标志.以结算中心账户处理为准
***********************************************/
#define PART_DEBIT      1           /* 借方 */
#define PART_CREDIT     2           /* 贷方 */
#define PART_ZS         3           /* 指示类 */

/*****************************************
    规则类型定义
    RULE_XXX
*****************************************/
#define RULE_WORKDATE   1           /* 工作日设置规则 */
#define RULE_NOTE       2           /* 凭证要素检查规则 */
#define RULE_OPER       3           /* 操作员权限规则 */
#define RULE_TRAN       4           /* 中心交易处理规则 */
#define RULE_PRETRAN    5           /* 前置交易处理规则 */

/*****************************************
    行政区划级别
    REGION_XX
*****************************************/
#define REGION_01       1           /* 省,自治区,直辖市 */
#define REGION_02       2           /* 省辖市,地区 */
#define REGION_03       3           /* 市辖区,县,县级市 */

/**********************************************
    货币代码定义
    采用ISO 4217标准
**********************************************/

#define CURR_RMB     156        /* 人民币 */
#define CURR_HKD     344        /* 港币 */
#define CURR_USD     840        /* 美元 */
#define CURR_GBP     826        /* 英镑 */
#define CURR_EUR     978        /* 欧元 */
#define CURR_JPY     392        /* 日元 */

/************************************
    货币类别.用于区分外币钞汇类别
    以CURRT_打头
************************************/
#define CURRT_EXCH_A    "2"     /* 企业外币现汇 */
#define CURRT_PAPER_B   "3"     /* 境外居民外币现钞 */
#define CURRT_EXCH_B    "4"     /* 境外居民外币现汇 */
#define CURRT_PAPER_C   "0"     /* 境内居民外币现钞(含人民币) */
#define CURRT_EXCH_C    "6"     /* 境内居民外币现汇 */

/*******************************************
    节点定义
*******************************************/
#define CENTER_NODE         "000"
#define INTERFACE_NODE      "999"

/*******************************************
    机构类型定义
    OTYPE_XXX
*******************************************/
#define OTYPE_BANK        	"1"     /* 银行机构 */

/*******************************************
    机构级别定义
    OLEVEL_XXX
*******************************************/
#define OLEVEL_HQ			1       /* 总部(清算行) */

/***********************************************
    账户状态
    ACCTSTATE_XXX 
***********************************************/
#define ACCTSTATE_NOTPERMIT         '0'       /* 未许可 */
#define ACCTSTATE_PERMITTED         '1'       /* 许可 */
#define ACCTSTATE_FROZEN            '5'       /* 冻结 */

/***********************************************
    操作员类型定义
    OPER_XXXX
***********************************************/
#define OPER_SYSADM         1       /* 系统管理员 */
#define OPER_ADM            2       /* 业务主管 */
#define OPER_GNR            3       /* 普通操作员 */

/**************************************************
    操作员状态
    OPERSTAT_XXXX
**************************************************/
#define OPERSTAT_NOTUSED    '0'     /* 未启用 */
#define OPERSTAT_NORMAL     '1'     /* 正常(已签到) */
#define OPERSTAT_LOGOUT     '2'     /* 已签退 */
#define OPERSTAT_CANCEL     '9'     /* 已注销 */

/**************************************************
    机构状态
    ORGSTAT_XXXX
**************************************************/
#define ORGSTAT_NOTUSED    '0'     /* 未启用 */
#define ORGSTAT_NORMAL     '1'     /* 启用 */
#define ORGSTAT_CANCEL     '2'     /* 停用 */

/***************************************************
    凭证种类定义
    NOTE_XXXX
***************************************************/
#define NOTE_CHECK          2       /* 转账支票 */
#define NOTE_REMIT          4       /* 汇款凭证 */
#define NOTE_PROM           21      /* 银行本票 */
#define NOTE_DRAFT          41      /* 全国银行汇票 */
#define NOTE_ZONE_DRAFT     42      /* 三省一市/区域性银行汇票 */
#define NOTE_CORRXFER       44      /* 来账代转补充凭证
                                       Correspondent Transfer */
#define NOTE_CONPAY         46      /* 代理集中支付 Concentration Payment */
#define NOTE_COLLECTION     52      /* 托收凭证 */
#define NOTE_PDC            53      /* 定期借/贷记 Prearranged Debit&Credit */
#define NOTE_FP             56      /* 财政国库支付凭证 Fiscal Payment */
#define NOTE_DRAWBACK       58      /* 税款退还书 */
#define NOTE_TAXPAY         59      /* 税费缴款书 */
#define NOTE_SPCXFER        61      /* 特种转账凭证 Special transfer */
#define NOTE_NETBANK        62      /* 网银 */
#define NOTE_CASH           71      /* 现金存取款 */
#define NOTE_PXFER          72      /* 个人账户间转账 personal transfer */
#define NOTE_POSXFER        75      /* 银行卡POS转账(收款行一借一贷) */
#define NOTE_POSXFER_MULTI  76      /* 银行卡POS转账(收款行多借一贷) */
#define NOTE_FORCHECK       81      /* 外币转账支票 */
#define NOTE_DRAFT_103      82      /* 电汇MT103 */
#define NOTE_DRAFT_202      83      /* 电汇MT202 */
#define NOTE_PFORXFER       84      /* 个人外币账户间转账 */
#define NOTE_FORZS_OUT      88      /* 外币指示付款 */
#define NOTE_FORZS_IN       89      /* 外币指示收账 */
#define NOTE_BDC            91      /* 批量借/贷记 Batch Debit&Credit */

/***************************************************
    业务种类定义
    CLASS_XXXX
***************************************************/
#define CLASS_UNIT              1
#define CLASS_PERSON            2
#define CLASS_FOREIGN           3
#define CLASS_BATCH             4

/***************************************************
    系统状态定义
    SYSSTAT_XXXX
***************************************************/
#define SYSSTAT_DISABLE     0       /* 系统禁用 */
#define SYSSTAT_NORMAL      1       /* 系统日间正常运行 */

/*****************************************
    交易状态标志
    TRNSTAT_XXXXXXX
*****************************************/
#define TRNSTAT_NOTCHECKED      "0"     /* 交易未复核.限于二次录入 */
#define TRNSTAT_CHECKED         "1"     /* 交易已复核(发送) */
#define TRNSTAT_PROCESSED       "2"     /* 交易已处理 */

/*****************************************
    资金清算方式
    CLRTYP_XXXXXXX
*****************************************/
#define CLRTYP_DEBIT             1      /* 借记交易清算 */
#define CLRTYP_CREDIT            2      /* 贷记交易清算 */
#define CLRTYP_POS_DEBIT         3      /* POS借记交易清算 */
#define CLRTYP_POS_CREDIT        4      /* POS贷记交易清算 */

/*****************************************
    交易清算标志
    CLRSTAT_XXXXXXX
*****************************************/
//#define CLRSTAT_UNSETTLED       '0'     /* 未(待)清算 */
//#define CLRSTAT_SETTLED         '1'     /* 清算成功 */
//#define CLRSTAT_WAITFUNDHOLD    '2'     /* 待圈存 */
//#define CLRSTAT_FUNDHOLD        '3'     /* 已圈存 */
//#define CLRSTAT_INQUEUE_NOHOLD  '4'     /* 未圈存, 待清算 */
//#define CLRSTAT_INQUEUE_HOLD    '5'     /* 已圈存, 待清算 */
//#define CLRSTAT_END             '6'     /* 不需要清算 */
//#define CLRSTAT_CANCELFAIL      '8'     /* 清算失败，圈存未解除 */
//#define CLRSTAT_FAILED          '9'     /* 清算失败 */
//#define CLRSTAT_CHECKED         'C'     /* 已对账 */
//#define CLRSTAT_UNDOED          'U'     /* 已冲正 */

/*****************************************
    自维护表操作类型
    SMPTYPE_XXXXXXX
*****************************************/
#define SMPTYPE_QUERY           "1"     /* 查询 */
#define SMPTYPE_RESEND          "2"     /* 补发 */
#define SMPTYPE_REVERSE         "3"     /* 冲正 */
#define SMPTYPE_SYNC            "4"     /* 同步 */
#define SMPTYPE_HOLDFUND        "5"     /* 待圈存 */

#define TIMES_RESEND            10      /* 补发次数*/
#define TIMES_QUERY             10      /* 查询次数*/
#define TIMES_UNDO              10      /* 冲正次数*/


/*****************************************
    票据验证类型
    AUTHTYPE_XXXXXX
*****************************************/
#define AUTHTYPE_PAYCODE            '1'     /* 支付密码*/
#define AUTHTYPE_CORPCODE           '2'     /* 单位密码*/
#define AUTHTYPE_FOREIGNCODE        '3'     /* 异地密押*/
#define AUTHTYPE_PINCODE            '4'     /* 个人账户密码 */
#define AUTHTYPE_BANKCODE           '5'     /* 银行密押*/

/******************************************
    对账标志
    RECONFLAG_XXXX
    added by SUNLAN 2006/08/28
******************************************/
#define RECONFLAG_READY         '1'     /* 对账数据已生成 */
#define RECONFLAG_GET           '2'     /* 清算机构已取对账数据 */
#define RECONFLAG_SUCC          '3'     /* 清算机构对账成功 */
#define RECONFLAG_FAILD         '4'     /* 清算机构对账失败(不平) */

/******************************************
    对账文件中记录的方向标志
    RECONREC_X
******************************************/
#define RECONREC_PRES           '1'     /* 提出交易 */
#define RECONREC_ACPT           '2'     /* 提入交易 */

/*****************************************
    邮件类型
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
                                                   
