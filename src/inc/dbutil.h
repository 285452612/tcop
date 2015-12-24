#ifndef DBUTIL_H_
#define DBUTIL_H_

#include "udb.h"
#include "table.h"

char *GetSysPara(char *paraname);
int UpdSysPara(char *paraname, char *paraval);

#define GetSysStat()                GetSysPara("SYSSTAT")           //取当前系统状态
#define GetDefOper()                GetSysPara("DEFOPER")           //取自动记账柜员
#define GetDefOrg()                 GetSysPara("DEFORG")            //取自动记账机构
#define GetAuthDevId()              GetSysPara("AUTHDEVID")         //取密押机设备编号
#define GetSysName()                GetSysPara("SYSNAME")           //取系统名称
#define GetCBankno()                GetSysPara("CBANKNO")           //取清算行号
#define GetWorkdate()               GetSysPara("CURWORKDATE")       //取当前工作日期
#define GetPreWorkdate()            GetSysPara("PREWORKDATE")       //取前一工作日期
#define GetRound()                  GetSysPara("CURROUND")          //取当前工作场次
#define GetORGBank()                GetSysPara("ORG_BANK")          //取个人密钥行内机构
#define GetORGCenter()              GetSysPara("ORG_CENTER")        //取个人密钥中心机构
#define GetBankPerFlag()            GetSysPara("BANKPER_FLAG")      //取个人账户标志
#define GetFSAcctNo()               GetSysPara("FS_ACCTNO")         //非税帐户
#define GetArchivedate()            GetSysPara("ARCHIVEDATE")       //取归档日期
#define GetClasslist()              GetSysPara("CLASSLIST")         //取业务列表
#define GetCleardate()              GetSysPara("CLEARDATE")         //取清算日期
#define GetPreCleardate()           GetSysPara("PRECLEARDATE")      //取前一清算日期
#define GetLastFeedate()            GetSysPara("LASTFEEDATE")       //取最后收手续费日期
#define GetPreLastFeedate()         GetSysPara("PRELASTFEEDATE")    //取上一次最后收手续费日期
#define GetPIKInfo()                GetSysPara("PIKINFO")           //取PIK信息
#define GetMMKIndex()               GetSysPara("MMKINDEX")          //取主密钥MMK索引号
#define GetMaxRound()               GetSysPara("MAXROUND")          //取最大场次
#define GetAmtLimit()               GetSysPara("AMTLIMIT")          //取来账最大金额上限

#define GetRoundTrnend()            GetSysPara("ROUNDS_TRNEND")     //场次对账未决标志列表 10101 (多场)
#define GetSettlmsgDateround()      GetSysPara("SETTLMSG_DATEROUND")//取对账通知工作日期场次 20091230-1
#define GetSettledDateround()       GetSysPara("SETTLED_DATEROUND") //取已取对账工作日期场次 20091230-1

#define GetExchgdate()              GetSysPara("EXCHGDATE")         //取当前交换日期
#define GetPreExchgdate()           GetSysPara("PREEXCHGDATE")      //取前一交换日期
#define GetExchground()             GetSysPara("EXCHGROUND")        //取当前交换场次
#define GetClearround()             GetSysPara("CLEARROUND")        //取清算场次

#define UpdSysStat(val)             UpdSysPara("SYSSTAT", val)
#define UpdWorkdate(val)            UpdSysPara("CURWORKDATE", val)
#define UpdPreWorkdate(val)         UpdSysPara("PREWORKDATE", val)
#define UpdCleardate(val)           UpdSysPara("CLEARDATE", val)
#define UpdPreCleardate(val)        UpdSysPara("PRECLEARDATE", val)
#define UpdRound(val)               UpdSysPara("CURROUND", val)
#define UpdPreRound(val)            UpdSysPara("PREROUND", val)
#define UpdClearround(val)          UpdSysPara("CLEARROUND", val)
#define UpdRoundTrnend(val)         UpdSysPara("ROUNDS_TRNEND", val)
#define UpdArchivedate(val)         UpdSysPara("ARCHIVEDATE", val)
#define UpdSettlmsgDateround(val)   UpdSysPara("SETTLMSG_DATEROUND", val)
#define UpdSettledDateround(val)    UpdSysPara("SETTLED_DATEROUND", val)
#define UpdLastFeedate(val)         UpdSysPara("LASTFEEDATE", val)
#define UpdPreLastFeedate(val)      UpdSysPara("PRELASTFEEDATE", val)
#define UpdPIKInfo(val)             UpdSysPara("PIKINFO", val)
#define UpdMMKIndex(val)            UpdSysPara("MMKINDEX", val)

#define UpdExchgdate(val)           UpdSysPara("EXCHGDATE", val)
#define UpdPreExchgdate(val)        UpdSysPara("PREEXCHGDATE", val)
#define UpdExchground(val)          UpdSysPara("EXCHGROUND", val)

//定义数据库操作标志
#define DB_OPERATOR_INSERT      'C'
#define DB_OPERATOR_UPDATE      'U'
#define DB_OPERATOR_QUERY       'R'
#define DB_OPERATOR_DELETE      'D'

#define DB_COLTYPE_STR      'S'     //string类型
#define DB_COLTYPE_INT      'I'     //int类型
#define DB_COLTYPE_CHAR     'C'     //char类型
#define DB_COLTYPE_DBL      'D'     //double类型
#define DB_COLTYPE_XML      'X'     //XMLNODE类型
#define DB_COLTYPE_FLT      'F'     //float类型

int IsRoundEnd(int round);

/*
 * 成功 邮件ID字符串 失败 NULL
 */
char *GetMailId();

/*
 * 根据ID标识(0为无ID)在指定表中插入一条记录
 *
 * 成功: 0 失败: 其它
 */
int InsertTableByID(xmlDoc *doc, char *tableName, int id);

int QueryTableByID(xmlDoc *doc, char *tableName, int id, char *where);

int QueryTableByIDToFile(const char *filename, char *tableName, int id, char *where);

int QueryTableByIDToXML(xmlDoc *doc, char *tableName, int id, char *where);

/*
 * 根据ID标识(0为无ID)和where查询条件更新指定表中的记录
 *
 * where: WHERE子句(不含where)
 *
 * 返回: 成功 0 其它 失败
 */
int UpdateTableByID(xmlDoc *doc, char *tableName, int id, char *where);

#define InsertTable(doc, tableName)                     InsertTableByID(doc, tableName, OP_OPTCODE)
#define QueryTable(doc, tableName, where)               QueryTableByID(doc, tableName, OP_OPTCODE, where)
#define QueryTableToFile(filename, tableName, where)    QueryTableByIDToFile(filename, tableName, OP_OPTCODE, where)
#define UpdateTable(doc, tableName, where)              UpdateTableByID(doc, tableName, OP_OPTCODE, where)

#define InsertTrnjour(doc)                  InsertTableByID(doc, "trnjour", 0)
#define InsertAcctjour(doc)                 InsertTableByID(doc, "acctjour", 0)

/*
 * 获取数据库操作的配置的节点
 * 
 * operator: CURD中的某个字符表示操作标志
 * id: 可为0
 *
 * 返回: 成功 需要操作的XML节点指针 失败 NULL
 */
xmlNodePtr GetTBOperatorNode(xmlNodeSetPtr *pnsPtr, char *tableName, char operator, int id);

int GetDBNodeAttrs(xmlNodePtr pDBNode, char *coltype, char *rule);

#define OpenOPDB()  db_connect(NULL)

#define CloseOPDB() db_close()

#endif
