#ifndef DBUTIL_H_
#define DBUTIL_H_

#include "udb.h"
#include "table.h"

char *GetSysPara(char *paraname);
int UpdSysPara(char *paraname, char *paraval);

#define GetSysStat()                GetSysPara("SYSSTAT")           //ȡ��ǰϵͳ״̬
#define GetDefOper()                GetSysPara("DEFOPER")           //ȡ�Զ����˹�Ա
#define GetDefOrg()                 GetSysPara("DEFORG")            //ȡ�Զ����˻���
#define GetAuthDevId()              GetSysPara("AUTHDEVID")         //ȡ��Ѻ���豸���
#define GetSysName()                GetSysPara("SYSNAME")           //ȡϵͳ����
#define GetCBankno()                GetSysPara("CBANKNO")           //ȡ�����к�
#define GetWorkdate()               GetSysPara("CURWORKDATE")       //ȡ��ǰ��������
#define GetPreWorkdate()            GetSysPara("PREWORKDATE")       //ȡǰһ��������
#define GetRound()                  GetSysPara("CURROUND")          //ȡ��ǰ��������
#define GetORGBank()                GetSysPara("ORG_BANK")          //ȡ������Կ���ڻ���
#define GetORGCenter()              GetSysPara("ORG_CENTER")        //ȡ������Կ���Ļ���
#define GetBankPerFlag()            GetSysPara("BANKPER_FLAG")      //ȡ�����˻���־
#define GetFSAcctNo()               GetSysPara("FS_ACCTNO")         //��˰�ʻ�
#define GetArchivedate()            GetSysPara("ARCHIVEDATE")       //ȡ�鵵����
#define GetClasslist()              GetSysPara("CLASSLIST")         //ȡҵ���б�
#define GetCleardate()              GetSysPara("CLEARDATE")         //ȡ��������
#define GetPreCleardate()           GetSysPara("PRECLEARDATE")      //ȡǰһ��������
#define GetLastFeedate()            GetSysPara("LASTFEEDATE")       //ȡ���������������
#define GetPreLastFeedate()         GetSysPara("PRELASTFEEDATE")    //ȡ��һ�����������������
#define GetPIKInfo()                GetSysPara("PIKINFO")           //ȡPIK��Ϣ
#define GetMMKIndex()               GetSysPara("MMKINDEX")          //ȡ����ԿMMK������
#define GetMaxRound()               GetSysPara("MAXROUND")          //ȡ��󳡴�
#define GetAmtLimit()               GetSysPara("AMTLIMIT")          //ȡ�������������

#define GetRoundTrnend()            GetSysPara("ROUNDS_TRNEND")     //���ζ���δ����־�б� 10101 (�ೡ)
#define GetSettlmsgDateround()      GetSysPara("SETTLMSG_DATEROUND")//ȡ����֪ͨ�������ڳ��� 20091230-1
#define GetSettledDateround()       GetSysPara("SETTLED_DATEROUND") //ȡ��ȡ���˹������ڳ��� 20091230-1

#define GetExchgdate()              GetSysPara("EXCHGDATE")         //ȡ��ǰ��������
#define GetPreExchgdate()           GetSysPara("PREEXCHGDATE")      //ȡǰһ��������
#define GetExchground()             GetSysPara("EXCHGROUND")        //ȡ��ǰ��������
#define GetClearround()             GetSysPara("CLEARROUND")        //ȡ���㳡��

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

//�������ݿ������־
#define DB_OPERATOR_INSERT      'C'
#define DB_OPERATOR_UPDATE      'U'
#define DB_OPERATOR_QUERY       'R'
#define DB_OPERATOR_DELETE      'D'

#define DB_COLTYPE_STR      'S'     //string����
#define DB_COLTYPE_INT      'I'     //int����
#define DB_COLTYPE_CHAR     'C'     //char����
#define DB_COLTYPE_DBL      'D'     //double����
#define DB_COLTYPE_XML      'X'     //XMLNODE����
#define DB_COLTYPE_FLT      'F'     //float����

int IsRoundEnd(int round);

/*
 * �ɹ� �ʼ�ID�ַ��� ʧ�� NULL
 */
char *GetMailId();

/*
 * ����ID��ʶ(0Ϊ��ID)��ָ�����в���һ����¼
 *
 * �ɹ�: 0 ʧ��: ����
 */
int InsertTableByID(xmlDoc *doc, char *tableName, int id);

int QueryTableByID(xmlDoc *doc, char *tableName, int id, char *where);

int QueryTableByIDToFile(const char *filename, char *tableName, int id, char *where);

int QueryTableByIDToXML(xmlDoc *doc, char *tableName, int id, char *where);

/*
 * ����ID��ʶ(0Ϊ��ID)��where��ѯ��������ָ�����еļ�¼
 *
 * where: WHERE�Ӿ�(����where)
 *
 * ����: �ɹ� 0 ���� ʧ��
 */
int UpdateTableByID(xmlDoc *doc, char *tableName, int id, char *where);

#define InsertTable(doc, tableName)                     InsertTableByID(doc, tableName, OP_OPTCODE)
#define QueryTable(doc, tableName, where)               QueryTableByID(doc, tableName, OP_OPTCODE, where)
#define QueryTableToFile(filename, tableName, where)    QueryTableByIDToFile(filename, tableName, OP_OPTCODE, where)
#define UpdateTable(doc, tableName, where)              UpdateTableByID(doc, tableName, OP_OPTCODE, where)

#define InsertTrnjour(doc)                  InsertTableByID(doc, "trnjour", 0)
#define InsertAcctjour(doc)                 InsertTableByID(doc, "acctjour", 0)

/*
 * ��ȡ���ݿ���������õĽڵ�
 * 
 * operator: CURD�е�ĳ���ַ���ʾ������־
 * id: ��Ϊ0
 *
 * ����: �ɹ� ��Ҫ������XML�ڵ�ָ�� ʧ�� NULL
 */
xmlNodePtr GetTBOperatorNode(xmlNodeSetPtr *pnsPtr, char *tableName, char operator, int id);

int GetDBNodeAttrs(xmlNodePtr pDBNode, char *coltype, char *rule);

#define OpenOPDB()  db_connect(NULL)

#define CloseOPDB() db_close()

#endif
