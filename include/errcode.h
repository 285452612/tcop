/*********************************************************
  errcode.h
  错误代码定义
  Created by SUNLAN
  2005/06/11
 *********************************************************/

#ifndef __ERRCODE_H__
#define __ERRCODE_H__

typedef struct {
    int  iErrCode;
    char *sErrMacro;
    char *sErrDesc;
} ST_ERRMSG;

#ifndef MAX_ErrCode
#define MAX_ErrCode 9999
#endif

#define ERRCODE_LEN         4               /* 错误代码最大长度 */

static ST_ERRMSG UFTP_errlist[] = {
#define E_SUCC              0
    { E_SUCC,                "E_SUCC",             "交易成功" },

#define E_GNR_BASE          100               /* 通用错误 General Errors */
/* 101~200 平台性错误 */
#define E_GNR_SYS_CLOSE         E_GNR_BASE+1
    { E_GNR_SYS_CLOSE,      "E_GNR_SYS_CLOSE",      "系统关闭" },
#define E_GNR_NOT_TRADETIME     E_GNR_BASE+2   
    { E_GNR_NOT_TRADETIME,      "E_GNR_NOT_TRADETIME",     "非交易时间" },
#define E_GNR_FUNC_DISABLE      E_GNR_BASE+3
    { E_GNR_FUNC_DISABLE,   "E_GNR_FUNC_DISABLE",   "该业务未开通" },
#define E_GNR_FUNC_NOTFOUND     E_GNR_BASE+4
    { E_GNR_FUNC_NOTFOUND,  "E_GNR_FUNC_NOTFOUND",  "无该项业务" },
#define E_GNR_WORKDATE          E_GNR_BASE+5   
    { E_GNR_WORKDATE,       "E_GNR_WORKDATE",       "工作日期错" },
#define E_GNR_ROUND             E_GNR_BASE+6  
    { E_GNR_ROUND,          "E_GNR_ROUND",          "场次错" },
#define E_GNR_NOTREADY          E_GNR_BASE+7
    { E_GNR_NOTREADY,       "E_GNR_NOTREADY",       "数据未准备好" },
#define E_GNR_WINDCLOSED        E_GNR_BASE+10 
    { E_GNR_WINDCLOSED,     "E_GNR_WINDCLOSED",     "清算窗口已关闭" },
#define E_GNR_WINDNOTCLOSED     E_GNR_BASE+11
    { E_GNR_WINDNOTCLOSED,  "E_GNR_WINDNOTCLOSED",  "清算窗口未关闭" },
#define E_GNR_TIMESETTING       E_GNR_BASE+12  
    { E_GNR_TIMESETTING,    "E_GNR_TIMESETTING",    "时间设置错误" },
#define E_GNR_SYS_OPENED        E_GNR_BASE+13
    { E_GNR_SYS_OPENED,     "E_GNR_SYS_OPENED",     "系统已开启" },
#define E_GNR_NEED_NOT_UPD      E_GNR_BASE+17
    { E_GNR_NEED_NOT_UPD,   "E_GNR_NEED_NOT_UPD",   "已经是最新的参数" },


/* 201~300 通讯相关错误 */
#define E_COMM_BASE         200
#define E_GNR_COMM          E_COMM_BASE+1
    { E_GNR_COMM,           "E_GNR_COMM",          "通讯故障" },
#define E_GNR_COMM_SEND     E_COMM_BASE+2
    { E_GNR_COMM_SEND,      "E_GNR_COMM_SEND",     "通讯发送失败" },
#define E_GNR_COMM_RECV     E_COMM_BASE+3
    { E_GNR_COMM_RECV,      "E_GNR_COMM_RECV",     "接收应答失败" },
#define E_GNR_TIMEOUT       E_COMM_BASE+4
    { E_GNR_TIMEOUT,        "E_GNR_TIMEOUT",       "交易超时" },
#define E_GNR_CENTER_BUSSY  E_COMM_BASE+5
    { E_GNR_CENTER_BUSSY,   "E_GNR_CENTER_BUSSY",   "结算中心忙" },
#define E_GNR_CENTER_COMM   E_COMM_BASE+7
    { E_GNR_CENTER_COMM,    "E_GNR_CENTER_COMM",     "前置机通讯故障" },

/* 301~400 数据报文相关错误 */
#define E_DATA_BASE         300
#define E_GNR_DATAFMT       E_DATA_BASE+1
    { E_GNR_DATAFMT,        "E_GNR_DATAFMT",       "报文格式错" },
#define E_GNR_DATAMAC       E_DATA_BASE+2
    { E_GNR_DATAMAC,        "E_GNR_DATAMAC",       "报文校验错" },
#define E_GNR_FILEFMT       E_DATA_BASE+3
    { E_GNR_FILEFMT,        "E_GNR_FILEFMT",       "文件格式错" },
#define E_GNR_FILEMAC       E_DATA_BASE+4
    { E_GNR_FILEMAC,        "E_GNR_FILEMAC",       "文件校验错" },
#define E_GNR_DATA_REQUIRED E_DATA_BASE+5  
    { E_GNR_DATA_REQUIRED,  "E_GNR_DATA_REQUIRED", "业务要素不全" },
#define E_GNR_TRTYPE        E_DATA_BASE+6   
    { E_GNR_TRTYPE,         "E_GNR_TRTYPE",        "交易码错误" },
#define E_GNR_INVALID_DATA  E_DATA_BASE+7   
    { E_GNR_INVALID_DATA,   "E_GNR_INVALID_DATA",   "无效报文数据" },
#define E_GNR_TESTKEY_CHK   E_DATA_BASE+8   
    { E_GNR_TESTKEY_CHK,    "E_GNR_TESTKEY_CHK",    "核银行押错" },


/* 801~899操作员相关 */
#define E_OPER_BASE         800     
#define E_OPER_NOT_EXIST    E_OPER_BASE+1   
    { E_OPER_NOT_EXIST,     "E_OPER_NOT_EXIST",    "操作员不存在" },
#define E_OPER_EXIST        E_OPER_BASE+2   
    { E_OPER_EXIST,         "E_OPER_EXIST",        "操作员已存在" },
#define E_OPER_REVOKE       E_OPER_BASE+3   
    { E_OPER_REVOKE,        "E_OPER_REVOKE",       "操作员已注销" },
#define E_OPER_PASSWD       E_OPER_BASE+4   
    { E_OPER_PASSWD,        "E_OPER_PASSWD",       "操作员口令错" },
#define E_OPER_LOGIN        E_OPER_BASE+5   
    { E_OPER_LOGIN,         "E_OPER_LOGIN",        "操作员已签到" },
#define E_OPER_LOGOUT       E_OPER_BASE+6   
    { E_OPER_LOGOUT,        "E_OPER_LOGOUT",       "操作员已签退" },
#define E_OPER_EXPIRE       E_OPER_BASE+7   
    { E_OPER_EXPIRE,        "E_OPER_EXPIRE",       "操作员到期" },
#define E_OPER_PERMIT       E_OPER_BASE+8   
    { E_OPER_PERMIT,        "E_OPER_PERMIT",       "操作员无此权限" },
#define E_OPER_STATE        E_OPER_BASE+9   
    { E_OPER_STATE,         "E_OPER_STATE",        "操作员状态错" },
#define E_OPER_CLASS        E_OPER_BASE+10  
    { E_OPER_CLASS,         "E_OPER_CLASS",        "操作员级别错" },


/* 系统内部错 901～999 */
#define E_SYS_BASE         900           

/* 系统级错误 901~910 */    
#define E_GNR_MEMORY            E_SYS_BASE+1
    { E_GNR_MEMORY,         "E_GNR_MEMORY",         "分配内存出错" },
#define E_GNR_SYSINVOKE         E_SYS_BASE+2
    { E_GNR_SYSINVOKE,      "E_GNR_SYSINVOKE",      "系统调用失败" },
#define E_GNR_FILE_OPEN         E_SYS_BASE+3
    { E_GNR_FILE_OPEN,      "E_GNR_FILE_OPEN",      "文件打开失败" },

/* 数据库级错误 911~920 */    
#define E_GNR_CENTER_DB     E_SYS_BASE+11
    { E_GNR_CENTER_DB,      "E_GNR_CENTER_DB",      "结算中心数据库操作错" },
#define E_GNR_DB            E_GNR_CENTER_DB
#define E_GNR_DB_OPEN       E_SYS_BASE+12 
    { E_GNR_DB_OPEN,        "E_GNR_DB_OPEN",       "打开数据库失败" },
#define E_GNR_RECNOTFOUND   E_SYS_BASE+13 
    { E_GNR_RECNOTFOUND,    "E_GNR_RECNOTFOUND",   "指定记录不存在" },
#define E_GNR_RECMODI       E_SYS_BASE+14 
    { E_GNR_RECMODI,        "E_GNR_RECMODI",       "原记录已经被修改" },

/* 921~940 XML处理相关错误 */
#define E_GNR_XML_NORECORD  E_SYS_BASE+21    
    { E_GNR_XML_NORECORD,   "E_GNR_XML_NORECORD",  "XML中无符合条件记录" },
#define E_GNR_XML_NOTLEAF   E_SYS_BASE+22    
    { E_GNR_XML_NOTLEAF,    "E_GNR_XML_NOTLEAF",   "指定记录非叶子节点" },
#define E_GNR_XML_MULTINODE E_SYS_BASE+23    
    { E_GNR_XML_MULTINODE,  "E_GNR_XML_MULTINODE", "存在多个XML节点" },
#define E_GNR_XML_NODETYPE  E_SYS_BASE+24   
    { E_GNR_XML_NODETYPE,   "E_GNR_XML_NODETYPE",  "XML节点类型错误" },
#define E_GNR_XML_ADDNODE   E_SYS_BASE+25    
    { E_GNR_XML_ADDNODE,    "E_GNR_XML_ADDNODE",   "增加XML节点失败" },
#define E_GNR_XML_VALIDATE  E_SYS_BASE+26 
    { E_GNR_XML_VALIDATE,   "E_GNR_XML_VALIDATE",  "XML验证失败" },
#define E_GNR_XML_FORMAT    E_SYS_BASE+27 
    { E_GNR_XML_FORMAT,     "E_GNR_XML_FORMAT",    "XML文件格式错误" },
#define E_GNR_XML_PACKERR   E_SYS_BASE+28   
    { E_GNR_XML_PACKERR,    "E_GNR_XML_PACKERR",   "XML报文不合法" },

/* 941~960 规则、配置参数相关错误 */
#define E_GNR_WINDERR           E_SYS_BASE+41
    { E_GNR_WINDERR,        "E_GNR_WINDERR",        "取清算窗口参数失败" },
#define E_GNR_RULE_NOT_FOUND    E_SYS_BASE+42
    { E_GNR_RULE_NOT_FOUND, "E_GNR_RULE_NOT_FOUND", "没有匹配的检查规则" },
#define E_GNR_RULE_ERR          E_SYS_BASE+43
    { E_GNR_RULE_ERR,       "E_GNR_RULE_ERR",       "检查规则配置不合法" },
#define E_GNR_DEST_NOTFOUND     E_SYS_BASE+44 
    { E_GNR_DEST_NOTFOUND,  "E_GNR_DEST_NOTFOUND",  "结算中心未找到接收方" },

/* 981~990 密押机相关错误 */
#define E_GNR_TESTKEY_GEN   E_SYS_BASE+81    
    { E_GNR_TESTKEY_GEN,    "E_GNR_TESTKEY_GEN",    "中心加押错误" },
#define E_GNR_TESTKEY_CON   E_SYS_BASE+82    
    { E_GNR_TESTKEY_CON,    "E_GNR_TESTKEY_CON",    "连接密押设备失败" },
#define E_GNR_TESTKEY_INF   E_SYS_BASE+83    
    { E_GNR_TESTKEY_INF,    "E_GNR_TESTKEY_INF",    "获取密押设备信息失败" },

#define E_SYS_BASE_END          999            /* 系统内部错误 */

/* 1001~1999 机构相关错误 */
#define E_ORG_BASE          1000            /* 机构相关 */

#define E_ORG_NOT_EXIST         E_ORG_BASE+1    
    { E_ORG_NOT_EXIST,          "E_ORG_NOT_EXIST",       "机构不存在" },
#define E_ORG_ORIGN_NOT_EXIST   E_ORG_BASE+2    
    { E_ORG_ORIGN_NOT_EXIST,    "E_ORG_ORIGN_NOT_EXIST", "提出机构不存在" },
#define E_ORG_ACPT_NOT_EXIST    E_ORG_BASE+3   
    { E_ORG_ACPT_NOT_EXIST,     "E_ORG_ACPT_NOT_EXIST",  "提入机构不存在" },
#define E_ORG_PAYING_NOT_EXIST  E_ORG_BASE+4    
    { E_ORG_PAYING_NOT_EXIST,   "E_ORG_PAYING_NOT_EXIST","付款行不存在" },
#define E_ORG_BENE_NOT_EXIST    E_ORG_BASE+5    
    { E_ORG_BENE_NOT_EXIST,     "E_ORG_BENE_NOT_EXIST",  "收款行不存在" },
#define E_ORG_EXIST             E_ORG_BASE+6    /* 用于机构新增 */
    { E_ORG_EXIST,              "E_ORG_EXIST",           "机构已存在" },
#define E_ORG_ORIGN_REVOKE      E_ORG_BASE+7    
    { E_ORG_ORIGN_REVOKE,       "E_ORG_ORIGN_REVOKE",    "提出机构已注销" },
#define E_ORG_ACPT_REVOKE       E_ORG_BASE+8    
    { E_ORG_ACPT_REVOKE,        "E_ORG_ACPT_REVOKE",     "提入机构已注销" },
#define E_ORG_PAYING_REVOKE     E_ORG_BASE+9   
    { E_ORG_PAYING_REVOKE,      "E_ORG_PAYING_REVOKE",   "付款机构已注销" },
#define E_ORG_BENE_REVOKE       E_ORG_BASE+10    
    { E_ORG_BENE_REVOKE,        "E_ORG_BENE_REVOKE",     "收款机构已注销" },
#define E_ORG_LOGIN             E_ORG_BASE+11   
    { E_ORG_LOGIN,              "E_ORG_LOGIN",           "机构已签到" },
#define E_ORG_LOGOUT            E_ORG_BASE+12   
    { E_ORG_LOGOUT,             "E_ORG_LOGOUT",          "机构已签退" },
#define E_ORG_ORIGN_LOGOUT      E_ORG_BASE+13  
    { E_ORG_ORIGN_LOGOUT,       "E_ORG_ORIGN_LOGOUT",    "提出机构已签退" },
#define E_ORG_ACPT_LOGOUT       E_ORG_BASE+14   
    { E_ORG_ACPT_LOGOUT,        "E_ORG_ACPT_LOGOUT",     "提入机构已签退" },
#define E_ORG_PAYING_LOGOUT     E_ORG_BASE+15    
    { E_ORG_PAYING_LOGOUT,      "E_ORG_PAYING_LOGOUT",   "付款机构已签退" },
#define E_ORG_BENE_LOGOUT       E_ORG_BASE+16   
    { E_ORG_BENE_LOGOUT,        "E_ORG_BENE_LOGOUT",     "收款机构已签退" },
#define E_ORG_STATE             E_ORG_BASE+17    
    { E_ORG_STATE,              "E_ORG_STATE",           "机构状态错" },
#define E_ORG_PERMIT            E_ORG_BASE+18    
    { E_ORG_PERMIT,             "E_ORG_PERMIT",          "机构无此权限" },
#define E_ORG_ORIGN_PERMIT      E_ORG_BASE+19   
    { E_ORG_ORIGN_PERMIT,       "E_ORG_ORIGN_PERMIT",    "提出机构无此权限" },
#define E_ORG_ACPT_PERMIT       E_ORG_BASE+20   
    { E_ORG_ACPT_PERMIT,        "E_ORG_ACPT_PERMIT",     "提入机构无此权限" },
#define E_ORG_PAYING_PERMIT     E_ORG_BASE+21   
    { E_ORG_PAYING_PERMIT,      "E_ORG_PAYING_PERMIT",   "付款行无此权限" },
#define E_ORG_BENE_PERMIT       E_ORG_BASE+22   
    { E_ORG_BENE_PERMIT,        "E_ORG_BENE_PERMIT",     "收款行无此权限" },
#define E_ORG_CLASS             E_ORG_BASE+23    
    { E_ORG_CLASS,              "E_ORG_CLASS",           "机构级别错误" },
#define E_ORG_PARENT            E_ORG_BASE+24    
    { E_ORG_PARENT,             "E_ORG_PARENT",          "上级机构错误" },
#define E_ORG_ATTECH_NOT_EXIST  E_ORG_BASE+25    
    { E_ORG_ATTECH_NOT_EXIST,  "E_ORG_ATTECH_NOT_EXIST","机构所属管辖行不存在"},
#define E_ORG_NODEID_NOT_EXIST  E_ORG_BASE+26    
    { E_ORG_NODEID_NOT_EXIST,  "E_ORG_NODEID_NOT_EXIST","机构所属节点号不存在"},


/* 2001 ~ 2999 账户相关 */
#define E_ACCT_BASE         2000            /* 账户相关 */

#define E_ACCT_NOT_EXIST        E_ACCT_BASE+1   
    { E_ACCT_NOT_EXIST,         "E_ACCT_NOT_EXIST",         "账户不存在" },
#define E_ACCT_PAYING_NOT_EXIST     E_ACCT_BASE+2   
    { E_ACCT_PAYING_NOT_EXIST,  "E_ACCT_PAYING_NOT_EXIST",  "付款账户不存在" },
#define E_ACCT_BENE_NOT_EXIST   E_ACCT_BASE+3   
    { E_ACCT_BENE_NOT_EXIST,    "E_ACCT_BENE_NOT_EXIST",    "收款账户不存在" },
#define E_ACCT_EXIST            E_ACCT_BASE+4   
    { E_ACCT_EXIST,             "E_ACCT_EXIST",             "账户已登记" },
#define E_ACCT_PERMIT           E_ACCT_BASE+5  
    { E_ACCT_PERMIT,            "E_ACCT_PERMIT",            "账户无权限" },
#define E_ACCT_PAYING_PERMIT    E_ACCT_BASE+6  
    { E_ACCT_PAYING_PERMIT,     "E_ACCT_PAYING_PERMIT",     "付款账户无权限" },
#define E_ACCT_BENE_PERMIT      E_ACCT_BASE+7   
    { E_ACCT_BENE_PERMIT,       "E_ACCT_BENE_PERMIT",       "收款账户无权限" },
#define E_ACCT_NAME             E_ACCT_BASE+12   
    { E_ACCT_NAME,              "E_ACCT_NAME",              "账户名称不符" },
#define E_ACCT_PAYING_NAME      E_ACCT_BASE+13   
    { E_ACCT_PAYING_NAME,       "E_ACCT_PAYING_NAME",      "付款账户名称不符" },
#define E_ACCT_BENE_NAME        E_ACCT_BASE+14   
    { E_ACCT_BENE_NAME,         "E_ACCT_BENE_NAME",        "收款账户名称不符" },
#define E_ACCT_REVOKE           E_ACCT_BASE+15   
    { E_ACCT_REVOKE,            "E_ACCT_REVOKE",            "账户已注销" },
#define E_ACCT_PAYING_REVOKE    E_ACCT_BASE+16   
    { E_ACCT_PAYING_REVOKE,     "E_ACCT_PAYING_REVOKE",     "付款账户已注销" },
#define E_ACCT_BENE_REVOKE      E_ACCT_BASE+17  
    { E_ACCT_BENE_REVOKE,       "E_ACCT_BENE_REVOKE",       "收款账户已注销" },
#define E_ACCT_EXPIRE           E_ACCT_BASE+19   
    { E_ACCT_EXPIRE,            "E_ACCT_EXPIRE",            "账户已过期" },
#define E_ACCT_PAYING_EXPIRE    E_ACCT_BASE+20   
    { E_ACCT_PAYING_EXPIRE,     "E_ACCT_PAYING_EXPIRE",     "付款账户已过期" },
#define E_ACCT_BENE_EXPIRE      E_ACCT_BASE+21   
    { E_ACCT_BENE_EXPIRE,       "E_ACCT_BENE_EXPIRE",       "收款账户已过期" },
#define E_ACCT_STATE            E_ACCT_BASE+25  
    { E_ACCT_STATE,             "E_ACCT_STATE",             "账户状态错误" },
#define E_ACCT_PAYING_STATE     E_ACCT_BASE+26  
    { E_ACCT_PAYING_STATE,      "E_ACCT_PAYING_STATE",     "付款账户状态错误" },
#define E_ACCT_BENE_STATE       E_ACCT_BASE+27  
    { E_ACCT_BENE_STATE,        "E_ACCT_BENE_STATE",       "收款账户状态错误" },
#define E_ACCT_TYPE             E_ACCT_BASE+29  
    { E_ACCT_TYPE,              "E_ACCT_TYPE",              "账户类型错误" },
#define E_ACCT_PAYING_TYPE      E_ACCT_BASE+30  
    { E_ACCT_PAYING_TYPE,       "E_ACCT_PAYING_TYPE",      "付款账户类型错误" },
#define E_ACCT_BENE_TYPE        E_ACCT_BASE+31  
        { E_ACCT_BENE_TYPE,     "E_ACCT_BENE_TYPE",        "付款账户类型错误" },
#define E_ACCT_NAME_DIFF        E_ACCT_BASE+32   
    { E_ACCT_NAME_DIFF,         "E_ACCT_NAME_DIFF",        "收付款户名不一致" },

/* 3001 ~ 3999 业务相关 */
#define E_TR_BASE           3000            /* 业务相关 */

#define E_TR_CLEARACCT_NOT_EXIST    E_TR_BASE+1 
    { E_TR_CLEARACCT_NOT_EXIST, "E_TR_CLEARACCT_NOT_EXIST", "清算账户不存在" },
#define E_TR_CLEARAMT_NOT_ENOUGH    E_TR_BASE+2 
    { E_TR_CLEARAMT_NOT_ENOUGH, "E_TR_CLEARAMT_NOT_ENOUGH", "清算账户余额不足"},
#define E_TR_SERIALDUP              E_TR_BASE+3     
    { E_TR_SERIALDUP,       "E_TR_SERIALDUP",       "交易流水重复" },
#define E_TR_NOTENODUP              E_TR_BASE+4     
    { E_TR_NOTENODUP,       "E_TR_NOTENODUP",       "提出凭证重复" },
#define E_TR_PROCESSED              E_TR_BASE+5    
    { E_TR_PROCESSED,       "E_TR_PROCESSED",       "交易已经处理" },
#define E_TR_STATE_UNKNOWN          E_TR_BASE+6     
    { E_TR_STATE_UNKNOWN,   "E_TR_STATE_UNKNOWN",   "交易状态不确定" },
#define E_TR_STATE                  E_TR_BASE+7
    { E_TR_STATE,           "E_TR_STATE",           "原交易失败" },
#define E_TR_ROLLBACK               E_TR_BASE+8
    { E_TR_ROLLBACK,        "E_TR_ROLLBACK",        "原交易已冲正" },
#define E_TR_REVOKE                 E_TR_BASE+9
    { E_TR_REVOKE,          "E_TR_REVOKE",          "原交易已撤消" },
#define E_TR_NOTEXIST               E_TR_BASE+10
    { E_TR_NOTEXIST,        "E_TR_NOTEXIST",        "原交易不存在" },
#define E_TR_ORGPWD                 E_TR_BASE+12
    { E_TR_ORGPWD,          "E_TR_ORGPWD",          "单位密码错" },
#define E_TR_AMT_EXCEED             E_TR_BASE+16
    { E_TR_AMT_EXCEED,      "E_TR_AMT_EXCEED",      "交易金额超限" },
#define E_TR_CURCODE                E_TR_BASE+17
    { E_TR_CURCODE,         "E_TR_CURCODE",         "币种或钞汇类型错" },
#define E_TR_SUM                    E_TR_BASE+18
    { E_TR_SUM,             "E_TR_SUM",             "批量数据总分不平" },
#define E_TR_HOLDAMT_NOT_ENOUGH     E_TR_BASE+19
    { E_TR_HOLDAMT_NOT_ENOUGH,  "E_TR_HOLDAMT_NOT_ENOUGH",  "圈存余额不足" },
#define E_TR_ELEMENT_NOT_MATCH      E_TR_BASE+20
    { E_TR_ELEMENT_NOT_MATCH,   "E_TR_ELEMENT_NOT_MATCH",   "要素复核不一致" },
#define E_QUERY_PLEASE              E_TR_BASE+21
    { E_QUERY_PLEASE,           "E_QUERY_PLEASE",   "无响应, 请稍候查询" },

/* 3101 ~ 3199 票据相关 */
#define E_NOTE_BASE         E_TR_BASE+100            /* 票据相关 */
#define E_NOTE_TYPE         E_NOTE_BASE+1   
    { E_NOTE_TYPE,          "E_NOTE_TYPE",         "票据类型错误" },
#define E_NOTE_REVOKE       E_NOTE_BASE+3   
    { E_NOTE_REVOKE,        "E_NOTE_REVOKE",       "票据类型已注销" },
#define E_NOTE_DUEDATE      E_NOTE_BASE+5
    { E_NOTE_DUEDATE,       "E_NOTE_DUEDATE",      "票据未到有效期"},

/* E_TR_BASE+500之后代码专用于柜员终端处理系统 */

#define E_TR_NOTCHECKED     E_TR_BASE+501
    { E_TR_NOTCHECKED,      "E_TR_NOTCHECKED",    "交易未经复核" },
#define E_TR_EXIST          E_TR_BASE+502
    { E_TR_EXIST,           "E_TR_EXIST",         "原交易已存在" },
#define E_TR_AMT_FORMAT     E_TR_BASE+503
    { E_TR_AMT_FORMAT,      "E_TR_AMT_FORMAT",    "金额格式错误" },
#define E_TR_PRINT          E_TR_BASE+504
    { E_TR_PRINT,          "E_TR_PRINT",          "生成打印文件失败" },


/* 日终相关3901-3999 */
#define E_DAYEND_BASE       E_TR_BASE+900         /* 日终相关3901-3999 */
#define E_DAYEND_SETTFILE   E_DAYEND_BASE+1
    { E_DAYEND_SETTFILE,    "E_DAYEND_SETTFILE",  "对账文件不存在" },
#define E_DAYEND_NODATA     E_DAYEND_BASE+2
    { E_DAYEND_NODATA,      "E_DAYEND_NODATA",      "无本场次交易数据" },
#define E_DAYEND_SETTFAILED E_DAYEND_BASE+3
    { E_DAYEND_SETTFAILED,  "E_DAYEND_SETTFAILED","对账出错" },
#define E_DAYEND_DBBAKFILE   E_DAYEND_BASE+4
    { E_DAYEND_DBBAKFILE,   "E_DAYEND_DBBAKFILE",  "数据备份文件不存在" },
#define E_DAYEND_NOT_CHECKED   E_DAYEND_BASE+5
    { E_DAYEND_NOT_CHECKED, "E_DAYEND_NOT_CHECKED",  "轧差对账尚未完成" },
#define E_DAYEND_ARCHIVE   E_DAYEND_BASE+6
    { E_DAYEND_ARCHIVE,    "E_DAYEND_ARCHIVE",  "日终数据归档失败" },
#define E_YEAREND_CONDITION   E_DAYEND_BASE+7
    { E_YEAREND_CONDITION, "E_YEAREND_CONDITION",  "不符合年终数据库切换条件" },
#define E_LOADDATE   E_DAYEND_BASE+8
    { E_LOADDATE,    "E_LOADDATE",  "导入日期小于等于已归档日期" },
#define E_RCPT_NOTEXIST   E_DAYEND_BASE+9
    { E_RCPT_NOTEXIST,    "E_RCPT_NOTEXIST",  "对账回执文件不存在" },


/*8001-8999提入行使用*/
#define E_ACPT_BASE       8000         

#define E_ACPT_ORG_NOT_EXIST    E_ACPT_BASE+33   
    { E_ACPT_ORG_NOT_EXIST,     "E_ACPT_ORG_NOT_EXIST",  "提入机构不存在" },

#define E_ACPT_NOTE_TYPE        E_ACPT_BASE+36   
    { E_ACPT_NOTE_TYPE,         "E_ACPT_NOTE_TYPE",      "票据类型错误" },

#define E_ACPT_NOTE_USED        E_ACPT_BASE+38   
    { E_ACPT_NOTE_USED,         "E_ACPT_NOTE_USED",       "票据已使用" },

#define E_ACPT_SERIALDUP        E_ACPT_BASE+43   
    { E_ACPT_SERIALDUP,         "E_ACPT_SERIALDUP",       "交易流水重复" },

#define E_ACPT_TR_STATE_UNKNOWN E_ACPT_BASE+48   
    { E_ACPT_TR_STATE_UNKNOWN,  "E_ACPT_TR_STATE_UNKNOWN", "交易结果不确定" },
#define E_WAITING_CONFIRMATION  E_ACPT_TR_STATE_UNKNOWN 

#define E_ACPT_DATA_REQUIRED    E_ACPT_BASE+51  
    { E_ACPT_DATA_REQUIRED,     "E_ACPT_DATA_REQUIRED",  "数据要素不全" },

#define E_ACPT_NOTE_INVALID     E_ACPT_BASE+52   
    { E_ACPT_NOTE_INVALID,      "E_ACPT_NOTE_INVALID",   "票据号无效" },
#define E_NOTE_INVALID          E_ACPT_NOTE_INVALID   

#define E_ACPT_NOTE_EXPIRE      E_ACPT_BASE+53   
    { E_ACPT_NOTE_EXPIRE,       "E_ACPT_NOTE_EXPIRE",    "票据已过期" },
#define E_NOTE_EXPIRE           E_ACPT_NOTE_BASE

#define E_ACPT_NOTE_LOSS        E_ACPT_BASE+54   
    { E_ACPT_NOTE_LOSS,         "E_ACPT_NOTE_LOSS",      "票据已挂失止付" },

#define E_ACPT_ACCT_FROZEN      E_ACPT_BASE+55   
    { E_ACPT_ACCT_FROZEN,       "E_ACPT_ACCT_FROZEN",    "账户已冻结" },

#define E_ACPT_PAYPWD           E_ACPT_BASE+56   
    { E_ACPT_PAYPWD,            "E_ACPT_PAYPWD",         "支付密码错误" },

#define E_ACPT_ACCT_BALANCE     E_ACPT_BASE+57
    { E_ACPT_ACCT_BALANCE,      "E_ACPT_ACCT_BALANCE",   "账户余额不足" },

#define E_ACPT_ACCT_NOT_EXIST   E_ACPT_BASE+58
    { E_ACPT_ACCT_NOT_EXIST,    "E_ACPT_ACCT_NOT_EXIST", "提入行账户不存在" },

#define E_ACPT_TR_NOT_EXIST     E_ACPT_BASE+59
    { E_ACPT_TR_NOT_EXIST,      "E_ACPT_TR_NOT_EXIST",   "原交易不存在" },

#define E_ACPT_TR_FAILED        E_ACPT_BASE+60
    { E_ACPT_TR_FAILED,         "E_ACPT_TR_FAILED",      "原交易失败" },

#define E_ACPT_ORGPWD           E_ACPT_BASE+61
    { E_ACPT_ORGPWD,            "E_ACPT_ORGPWD",         "单位密码错误" },

#define E_ACPT_TESTKEY_CHK      E_ACPT_BASE+62   
    { E_ACPT_TESTKEY_CHK,       "E_ACPT_TESTKEY_CHK",    "核中心密押错误" },

#define E_ACPT_NOTE_DUEDATE     E_ACPT_BASE+63
    { E_ACPT_NOTE_DUEDATE,      "E_ACPT_NOTE_DUEDATE",   "票据未到有效期"},

#define E_ACPT_AGREEMENT        E_ACPT_BASE+65
    { E_ACPT_AGREEMENT,         "E_ACPT_AGREEMENT",      "协议号不符" },

#define E_ACPT_TR_AMT_EXCEED    E_ACPT_BASE+73
    { E_ACPT_TR_AMT_EXCEED,     "E_ACPT_TR_AMT_EXCEED",  "交易金额超限" },

#define E_ACPT_CLOSED           E_ACPT_BASE+103   
    { E_ACPT_CLOSED,            "E_ACPT_CLOSED",        "提入机构系统已关闭" },

#define E_ACPT_CURCODE          E_ACPT_BASE+117   
    { E_ACPT_CURCODE,           "E_ACPT_CURCODE",        "币种或钞汇类型错误" },

#define E_ACPT_ACCTPWD          E_ACPT_BASE+156   
    { E_ACPT_ACCTPWD,           "E_ACPT_ACCTPWD",        "个人账户密码错误" },

#define E_ACPT_COMM_SEND        E_ACPT_BASE+202
    { E_ACPT_COMM_SEND,         "E_ACPT_COMM_SEND",     "通讯故障(发送失败)" },

#define E_ACPT_COMM_RECV        E_ACPT_BASE+203
    { E_ACPT_COMM_RECV,        "E_ACPT_COMM_RECV",   "接收提入行应答失败" },

#define E_ACPT_TIMEOUT          E_ACPT_BASE+204
    { E_ACPT_TIMEOUT,           "E_ACPT_TIMEOUT",       "提入行交易超时" },

#define E_ACPT_BUSSY            E_ACPT_BASE+205
    { E_ACPT_BUSSY,             "E_ACPT_BUSSY",          "对方系统忙" },

#define E_GNR_PRES_DB           E_ACPT_BASE+301
    { E_GNR_PRES_DB,            "E_GNR_PRES_DB",         "前置机数据库操作错" },

#define E_ACPT_DATAFMT          E_ACPT_BASE+401
    { E_ACPT_DATAFMT,           "E_ACPT_DATAFMT",        "数据报文格式错" },

#define E_ACPT_DEFAULT          E_ACPT_BASE+999 
    { E_ACPT_DEFAULT,           "E_ACPT_DEFAULT",        "接收机构内部错误" },

    /*9999 结算中心其他错误 */
#define E_CENTER_DEFAULT    MAX_ErrCode
    { E_CENTER_DEFAULT,     "E_CENTER_DEFAULT",   "结算中心其他错误" },
#define E_DEFAULT           E_CENTER_DEFAULT

};

/***************************************************
    以下定义与错误设置连接相关
***************************************************/
#ifndef SetError
void SetError( int err );
#endif
/*****************************************
用途:设置错误代码
传入参数:
    err 错误代码
传出参数:
    无
*****************************************/

int GetError();
/*****************************************
用途:取错误信息
传入参数:
    无
传出参数:
    错误信息
*****************************************/

void ClearError();
/*****************************************
用途:清除错误信息
传入参数:
    无
传出参数:
    无
*****************************************/

char *errmsg( int errnum );    /* 错误信息说明 */

#endif /* __ERRCODE_H__ */
