/*********************************************************
  errcode.h
  ������붨��
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

#define ERRCODE_LEN         4               /* ���������󳤶� */

static ST_ERRMSG UFTP_errlist[] = {
#define E_SUCC              0
    { E_SUCC,                "E_SUCC",             "���׳ɹ�" },

#define E_GNR_BASE          100               /* ͨ�ô��� General Errors */
/* 101~200 ƽ̨�Դ��� */
#define E_GNR_SYS_CLOSE         E_GNR_BASE+1
    { E_GNR_SYS_CLOSE,      "E_GNR_SYS_CLOSE",      "ϵͳ�ر�" },
#define E_GNR_NOT_TRADETIME     E_GNR_BASE+2   
    { E_GNR_NOT_TRADETIME,      "E_GNR_NOT_TRADETIME",     "�ǽ���ʱ��" },
#define E_GNR_FUNC_DISABLE      E_GNR_BASE+3
    { E_GNR_FUNC_DISABLE,   "E_GNR_FUNC_DISABLE",   "��ҵ��δ��ͨ" },
#define E_GNR_FUNC_NOTFOUND     E_GNR_BASE+4
    { E_GNR_FUNC_NOTFOUND,  "E_GNR_FUNC_NOTFOUND",  "�޸���ҵ��" },
#define E_GNR_WORKDATE          E_GNR_BASE+5   
    { E_GNR_WORKDATE,       "E_GNR_WORKDATE",       "�������ڴ�" },
#define E_GNR_ROUND             E_GNR_BASE+6  
    { E_GNR_ROUND,          "E_GNR_ROUND",          "���δ�" },
#define E_GNR_NOTREADY          E_GNR_BASE+7
    { E_GNR_NOTREADY,       "E_GNR_NOTREADY",       "����δ׼����" },
#define E_GNR_WINDCLOSED        E_GNR_BASE+10 
    { E_GNR_WINDCLOSED,     "E_GNR_WINDCLOSED",     "���㴰���ѹر�" },
#define E_GNR_WINDNOTCLOSED     E_GNR_BASE+11
    { E_GNR_WINDNOTCLOSED,  "E_GNR_WINDNOTCLOSED",  "���㴰��δ�ر�" },
#define E_GNR_TIMESETTING       E_GNR_BASE+12  
    { E_GNR_TIMESETTING,    "E_GNR_TIMESETTING",    "ʱ�����ô���" },
#define E_GNR_SYS_OPENED        E_GNR_BASE+13
    { E_GNR_SYS_OPENED,     "E_GNR_SYS_OPENED",     "ϵͳ�ѿ���" },
#define E_GNR_NEED_NOT_UPD      E_GNR_BASE+17
    { E_GNR_NEED_NOT_UPD,   "E_GNR_NEED_NOT_UPD",   "�Ѿ������µĲ���" },


/* 201~300 ͨѶ��ش��� */
#define E_COMM_BASE         200
#define E_GNR_COMM          E_COMM_BASE+1
    { E_GNR_COMM,           "E_GNR_COMM",          "ͨѶ����" },
#define E_GNR_COMM_SEND     E_COMM_BASE+2
    { E_GNR_COMM_SEND,      "E_GNR_COMM_SEND",     "ͨѶ����ʧ��" },
#define E_GNR_COMM_RECV     E_COMM_BASE+3
    { E_GNR_COMM_RECV,      "E_GNR_COMM_RECV",     "����Ӧ��ʧ��" },
#define E_GNR_TIMEOUT       E_COMM_BASE+4
    { E_GNR_TIMEOUT,        "E_GNR_TIMEOUT",       "���׳�ʱ" },
#define E_GNR_CENTER_BUSSY  E_COMM_BASE+5
    { E_GNR_CENTER_BUSSY,   "E_GNR_CENTER_BUSSY",   "��������æ" },
#define E_GNR_CENTER_COMM   E_COMM_BASE+7
    { E_GNR_CENTER_COMM,    "E_GNR_CENTER_COMM",     "ǰ�û�ͨѶ����" },

/* 301~400 ���ݱ�����ش��� */
#define E_DATA_BASE         300
#define E_GNR_DATAFMT       E_DATA_BASE+1
    { E_GNR_DATAFMT,        "E_GNR_DATAFMT",       "���ĸ�ʽ��" },
#define E_GNR_DATAMAC       E_DATA_BASE+2
    { E_GNR_DATAMAC,        "E_GNR_DATAMAC",       "����У���" },
#define E_GNR_FILEFMT       E_DATA_BASE+3
    { E_GNR_FILEFMT,        "E_GNR_FILEFMT",       "�ļ���ʽ��" },
#define E_GNR_FILEMAC       E_DATA_BASE+4
    { E_GNR_FILEMAC,        "E_GNR_FILEMAC",       "�ļ�У���" },
#define E_GNR_DATA_REQUIRED E_DATA_BASE+5  
    { E_GNR_DATA_REQUIRED,  "E_GNR_DATA_REQUIRED", "ҵ��Ҫ�ز�ȫ" },
#define E_GNR_TRTYPE        E_DATA_BASE+6   
    { E_GNR_TRTYPE,         "E_GNR_TRTYPE",        "���������" },
#define E_GNR_INVALID_DATA  E_DATA_BASE+7   
    { E_GNR_INVALID_DATA,   "E_GNR_INVALID_DATA",   "��Ч��������" },
#define E_GNR_TESTKEY_CHK   E_DATA_BASE+8   
    { E_GNR_TESTKEY_CHK,    "E_GNR_TESTKEY_CHK",    "������Ѻ��" },


/* 801~899����Ա��� */
#define E_OPER_BASE         800     
#define E_OPER_NOT_EXIST    E_OPER_BASE+1   
    { E_OPER_NOT_EXIST,     "E_OPER_NOT_EXIST",    "����Ա������" },
#define E_OPER_EXIST        E_OPER_BASE+2   
    { E_OPER_EXIST,         "E_OPER_EXIST",        "����Ա�Ѵ���" },
#define E_OPER_REVOKE       E_OPER_BASE+3   
    { E_OPER_REVOKE,        "E_OPER_REVOKE",       "����Ա��ע��" },
#define E_OPER_PASSWD       E_OPER_BASE+4   
    { E_OPER_PASSWD,        "E_OPER_PASSWD",       "����Ա�����" },
#define E_OPER_LOGIN        E_OPER_BASE+5   
    { E_OPER_LOGIN,         "E_OPER_LOGIN",        "����Ա��ǩ��" },
#define E_OPER_LOGOUT       E_OPER_BASE+6   
    { E_OPER_LOGOUT,        "E_OPER_LOGOUT",       "����Ա��ǩ��" },
#define E_OPER_EXPIRE       E_OPER_BASE+7   
    { E_OPER_EXPIRE,        "E_OPER_EXPIRE",       "����Ա����" },
#define E_OPER_PERMIT       E_OPER_BASE+8   
    { E_OPER_PERMIT,        "E_OPER_PERMIT",       "����Ա�޴�Ȩ��" },
#define E_OPER_STATE        E_OPER_BASE+9   
    { E_OPER_STATE,         "E_OPER_STATE",        "����Ա״̬��" },
#define E_OPER_CLASS        E_OPER_BASE+10  
    { E_OPER_CLASS,         "E_OPER_CLASS",        "����Ա�����" },


/* ϵͳ�ڲ��� 901��999 */
#define E_SYS_BASE         900           

/* ϵͳ������ 901~910 */    
#define E_GNR_MEMORY            E_SYS_BASE+1
    { E_GNR_MEMORY,         "E_GNR_MEMORY",         "�����ڴ����" },
#define E_GNR_SYSINVOKE         E_SYS_BASE+2
    { E_GNR_SYSINVOKE,      "E_GNR_SYSINVOKE",      "ϵͳ����ʧ��" },
#define E_GNR_FILE_OPEN         E_SYS_BASE+3
    { E_GNR_FILE_OPEN,      "E_GNR_FILE_OPEN",      "�ļ���ʧ��" },

/* ���ݿ⼶���� 911~920 */    
#define E_GNR_CENTER_DB     E_SYS_BASE+11
    { E_GNR_CENTER_DB,      "E_GNR_CENTER_DB",      "�����������ݿ������" },
#define E_GNR_DB            E_GNR_CENTER_DB
#define E_GNR_DB_OPEN       E_SYS_BASE+12 
    { E_GNR_DB_OPEN,        "E_GNR_DB_OPEN",       "�����ݿ�ʧ��" },
#define E_GNR_RECNOTFOUND   E_SYS_BASE+13 
    { E_GNR_RECNOTFOUND,    "E_GNR_RECNOTFOUND",   "ָ����¼������" },
#define E_GNR_RECMODI       E_SYS_BASE+14 
    { E_GNR_RECMODI,        "E_GNR_RECMODI",       "ԭ��¼�Ѿ����޸�" },

/* 921~940 XML������ش��� */
#define E_GNR_XML_NORECORD  E_SYS_BASE+21    
    { E_GNR_XML_NORECORD,   "E_GNR_XML_NORECORD",  "XML���޷���������¼" },
#define E_GNR_XML_NOTLEAF   E_SYS_BASE+22    
    { E_GNR_XML_NOTLEAF,    "E_GNR_XML_NOTLEAF",   "ָ����¼��Ҷ�ӽڵ�" },
#define E_GNR_XML_MULTINODE E_SYS_BASE+23    
    { E_GNR_XML_MULTINODE,  "E_GNR_XML_MULTINODE", "���ڶ��XML�ڵ�" },
#define E_GNR_XML_NODETYPE  E_SYS_BASE+24   
    { E_GNR_XML_NODETYPE,   "E_GNR_XML_NODETYPE",  "XML�ڵ����ʹ���" },
#define E_GNR_XML_ADDNODE   E_SYS_BASE+25    
    { E_GNR_XML_ADDNODE,    "E_GNR_XML_ADDNODE",   "����XML�ڵ�ʧ��" },
#define E_GNR_XML_VALIDATE  E_SYS_BASE+26 
    { E_GNR_XML_VALIDATE,   "E_GNR_XML_VALIDATE",  "XML��֤ʧ��" },
#define E_GNR_XML_FORMAT    E_SYS_BASE+27 
    { E_GNR_XML_FORMAT,     "E_GNR_XML_FORMAT",    "XML�ļ���ʽ����" },
#define E_GNR_XML_PACKERR   E_SYS_BASE+28   
    { E_GNR_XML_PACKERR,    "E_GNR_XML_PACKERR",   "XML���Ĳ��Ϸ�" },

/* 941~960 �������ò�����ش��� */
#define E_GNR_WINDERR           E_SYS_BASE+41
    { E_GNR_WINDERR,        "E_GNR_WINDERR",        "ȡ���㴰�ڲ���ʧ��" },
#define E_GNR_RULE_NOT_FOUND    E_SYS_BASE+42
    { E_GNR_RULE_NOT_FOUND, "E_GNR_RULE_NOT_FOUND", "û��ƥ��ļ�����" },
#define E_GNR_RULE_ERR          E_SYS_BASE+43
    { E_GNR_RULE_ERR,       "E_GNR_RULE_ERR",       "���������ò��Ϸ�" },
#define E_GNR_DEST_NOTFOUND     E_SYS_BASE+44 
    { E_GNR_DEST_NOTFOUND,  "E_GNR_DEST_NOTFOUND",  "��������δ�ҵ����շ�" },

/* 981~990 ��Ѻ����ش��� */
#define E_GNR_TESTKEY_GEN   E_SYS_BASE+81    
    { E_GNR_TESTKEY_GEN,    "E_GNR_TESTKEY_GEN",    "���ļ�Ѻ����" },
#define E_GNR_TESTKEY_CON   E_SYS_BASE+82    
    { E_GNR_TESTKEY_CON,    "E_GNR_TESTKEY_CON",    "������Ѻ�豸ʧ��" },
#define E_GNR_TESTKEY_INF   E_SYS_BASE+83    
    { E_GNR_TESTKEY_INF,    "E_GNR_TESTKEY_INF",    "��ȡ��Ѻ�豸��Ϣʧ��" },

#define E_SYS_BASE_END          999            /* ϵͳ�ڲ����� */

/* 1001~1999 ������ش��� */
#define E_ORG_BASE          1000            /* ������� */

#define E_ORG_NOT_EXIST         E_ORG_BASE+1    
    { E_ORG_NOT_EXIST,          "E_ORG_NOT_EXIST",       "����������" },
#define E_ORG_ORIGN_NOT_EXIST   E_ORG_BASE+2    
    { E_ORG_ORIGN_NOT_EXIST,    "E_ORG_ORIGN_NOT_EXIST", "�������������" },
#define E_ORG_ACPT_NOT_EXIST    E_ORG_BASE+3   
    { E_ORG_ACPT_NOT_EXIST,     "E_ORG_ACPT_NOT_EXIST",  "�������������" },
#define E_ORG_PAYING_NOT_EXIST  E_ORG_BASE+4    
    { E_ORG_PAYING_NOT_EXIST,   "E_ORG_PAYING_NOT_EXIST","�����в�����" },
#define E_ORG_BENE_NOT_EXIST    E_ORG_BASE+5    
    { E_ORG_BENE_NOT_EXIST,     "E_ORG_BENE_NOT_EXIST",  "�տ��в�����" },
#define E_ORG_EXIST             E_ORG_BASE+6    /* ���ڻ������� */
    { E_ORG_EXIST,              "E_ORG_EXIST",           "�����Ѵ���" },
#define E_ORG_ORIGN_REVOKE      E_ORG_BASE+7    
    { E_ORG_ORIGN_REVOKE,       "E_ORG_ORIGN_REVOKE",    "���������ע��" },
#define E_ORG_ACPT_REVOKE       E_ORG_BASE+8    
    { E_ORG_ACPT_REVOKE,        "E_ORG_ACPT_REVOKE",     "���������ע��" },
#define E_ORG_PAYING_REVOKE     E_ORG_BASE+9   
    { E_ORG_PAYING_REVOKE,      "E_ORG_PAYING_REVOKE",   "���������ע��" },
#define E_ORG_BENE_REVOKE       E_ORG_BASE+10    
    { E_ORG_BENE_REVOKE,        "E_ORG_BENE_REVOKE",     "�տ������ע��" },
#define E_ORG_LOGIN             E_ORG_BASE+11   
    { E_ORG_LOGIN,              "E_ORG_LOGIN",           "������ǩ��" },
#define E_ORG_LOGOUT            E_ORG_BASE+12   
    { E_ORG_LOGOUT,             "E_ORG_LOGOUT",          "������ǩ��" },
#define E_ORG_ORIGN_LOGOUT      E_ORG_BASE+13  
    { E_ORG_ORIGN_LOGOUT,       "E_ORG_ORIGN_LOGOUT",    "���������ǩ��" },
#define E_ORG_ACPT_LOGOUT       E_ORG_BASE+14   
    { E_ORG_ACPT_LOGOUT,        "E_ORG_ACPT_LOGOUT",     "���������ǩ��" },
#define E_ORG_PAYING_LOGOUT     E_ORG_BASE+15    
    { E_ORG_PAYING_LOGOUT,      "E_ORG_PAYING_LOGOUT",   "���������ǩ��" },
#define E_ORG_BENE_LOGOUT       E_ORG_BASE+16   
    { E_ORG_BENE_LOGOUT,        "E_ORG_BENE_LOGOUT",     "�տ������ǩ��" },
#define E_ORG_STATE             E_ORG_BASE+17    
    { E_ORG_STATE,              "E_ORG_STATE",           "����״̬��" },
#define E_ORG_PERMIT            E_ORG_BASE+18    
    { E_ORG_PERMIT,             "E_ORG_PERMIT",          "�����޴�Ȩ��" },
#define E_ORG_ORIGN_PERMIT      E_ORG_BASE+19   
    { E_ORG_ORIGN_PERMIT,       "E_ORG_ORIGN_PERMIT",    "��������޴�Ȩ��" },
#define E_ORG_ACPT_PERMIT       E_ORG_BASE+20   
    { E_ORG_ACPT_PERMIT,        "E_ORG_ACPT_PERMIT",     "��������޴�Ȩ��" },
#define E_ORG_PAYING_PERMIT     E_ORG_BASE+21   
    { E_ORG_PAYING_PERMIT,      "E_ORG_PAYING_PERMIT",   "�������޴�Ȩ��" },
#define E_ORG_BENE_PERMIT       E_ORG_BASE+22   
    { E_ORG_BENE_PERMIT,        "E_ORG_BENE_PERMIT",     "�տ����޴�Ȩ��" },
#define E_ORG_CLASS             E_ORG_BASE+23    
    { E_ORG_CLASS,              "E_ORG_CLASS",           "�����������" },
#define E_ORG_PARENT            E_ORG_BASE+24    
    { E_ORG_PARENT,             "E_ORG_PARENT",          "�ϼ���������" },
#define E_ORG_ATTECH_NOT_EXIST  E_ORG_BASE+25    
    { E_ORG_ATTECH_NOT_EXIST,  "E_ORG_ATTECH_NOT_EXIST","����������Ͻ�в�����"},
#define E_ORG_NODEID_NOT_EXIST  E_ORG_BASE+26    
    { E_ORG_NODEID_NOT_EXIST,  "E_ORG_NODEID_NOT_EXIST","���������ڵ�Ų�����"},


/* 2001 ~ 2999 �˻���� */
#define E_ACCT_BASE         2000            /* �˻���� */

#define E_ACCT_NOT_EXIST        E_ACCT_BASE+1   
    { E_ACCT_NOT_EXIST,         "E_ACCT_NOT_EXIST",         "�˻�������" },
#define E_ACCT_PAYING_NOT_EXIST     E_ACCT_BASE+2   
    { E_ACCT_PAYING_NOT_EXIST,  "E_ACCT_PAYING_NOT_EXIST",  "�����˻�������" },
#define E_ACCT_BENE_NOT_EXIST   E_ACCT_BASE+3   
    { E_ACCT_BENE_NOT_EXIST,    "E_ACCT_BENE_NOT_EXIST",    "�տ��˻�������" },
#define E_ACCT_EXIST            E_ACCT_BASE+4   
    { E_ACCT_EXIST,             "E_ACCT_EXIST",             "�˻��ѵǼ�" },
#define E_ACCT_PERMIT           E_ACCT_BASE+5  
    { E_ACCT_PERMIT,            "E_ACCT_PERMIT",            "�˻���Ȩ��" },
#define E_ACCT_PAYING_PERMIT    E_ACCT_BASE+6  
    { E_ACCT_PAYING_PERMIT,     "E_ACCT_PAYING_PERMIT",     "�����˻���Ȩ��" },
#define E_ACCT_BENE_PERMIT      E_ACCT_BASE+7   
    { E_ACCT_BENE_PERMIT,       "E_ACCT_BENE_PERMIT",       "�տ��˻���Ȩ��" },
#define E_ACCT_NAME             E_ACCT_BASE+12   
    { E_ACCT_NAME,              "E_ACCT_NAME",              "�˻����Ʋ���" },
#define E_ACCT_PAYING_NAME      E_ACCT_BASE+13   
    { E_ACCT_PAYING_NAME,       "E_ACCT_PAYING_NAME",      "�����˻����Ʋ���" },
#define E_ACCT_BENE_NAME        E_ACCT_BASE+14   
    { E_ACCT_BENE_NAME,         "E_ACCT_BENE_NAME",        "�տ��˻����Ʋ���" },
#define E_ACCT_REVOKE           E_ACCT_BASE+15   
    { E_ACCT_REVOKE,            "E_ACCT_REVOKE",            "�˻���ע��" },
#define E_ACCT_PAYING_REVOKE    E_ACCT_BASE+16   
    { E_ACCT_PAYING_REVOKE,     "E_ACCT_PAYING_REVOKE",     "�����˻���ע��" },
#define E_ACCT_BENE_REVOKE      E_ACCT_BASE+17  
    { E_ACCT_BENE_REVOKE,       "E_ACCT_BENE_REVOKE",       "�տ��˻���ע��" },
#define E_ACCT_EXPIRE           E_ACCT_BASE+19   
    { E_ACCT_EXPIRE,            "E_ACCT_EXPIRE",            "�˻��ѹ���" },
#define E_ACCT_PAYING_EXPIRE    E_ACCT_BASE+20   
    { E_ACCT_PAYING_EXPIRE,     "E_ACCT_PAYING_EXPIRE",     "�����˻��ѹ���" },
#define E_ACCT_BENE_EXPIRE      E_ACCT_BASE+21   
    { E_ACCT_BENE_EXPIRE,       "E_ACCT_BENE_EXPIRE",       "�տ��˻��ѹ���" },
#define E_ACCT_STATE            E_ACCT_BASE+25  
    { E_ACCT_STATE,             "E_ACCT_STATE",             "�˻�״̬����" },
#define E_ACCT_PAYING_STATE     E_ACCT_BASE+26  
    { E_ACCT_PAYING_STATE,      "E_ACCT_PAYING_STATE",     "�����˻�״̬����" },
#define E_ACCT_BENE_STATE       E_ACCT_BASE+27  
    { E_ACCT_BENE_STATE,        "E_ACCT_BENE_STATE",       "�տ��˻�״̬����" },
#define E_ACCT_TYPE             E_ACCT_BASE+29  
    { E_ACCT_TYPE,              "E_ACCT_TYPE",              "�˻����ʹ���" },
#define E_ACCT_PAYING_TYPE      E_ACCT_BASE+30  
    { E_ACCT_PAYING_TYPE,       "E_ACCT_PAYING_TYPE",      "�����˻����ʹ���" },
#define E_ACCT_BENE_TYPE        E_ACCT_BASE+31  
        { E_ACCT_BENE_TYPE,     "E_ACCT_BENE_TYPE",        "�����˻����ʹ���" },
#define E_ACCT_NAME_DIFF        E_ACCT_BASE+32   
    { E_ACCT_NAME_DIFF,         "E_ACCT_NAME_DIFF",        "�ո������һ��" },

/* 3001 ~ 3999 ҵ����� */
#define E_TR_BASE           3000            /* ҵ����� */

#define E_TR_CLEARACCT_NOT_EXIST    E_TR_BASE+1 
    { E_TR_CLEARACCT_NOT_EXIST, "E_TR_CLEARACCT_NOT_EXIST", "�����˻�������" },
#define E_TR_CLEARAMT_NOT_ENOUGH    E_TR_BASE+2 
    { E_TR_CLEARAMT_NOT_ENOUGH, "E_TR_CLEARAMT_NOT_ENOUGH", "�����˻�����"},
#define E_TR_SERIALDUP              E_TR_BASE+3     
    { E_TR_SERIALDUP,       "E_TR_SERIALDUP",       "������ˮ�ظ�" },
#define E_TR_NOTENODUP              E_TR_BASE+4     
    { E_TR_NOTENODUP,       "E_TR_NOTENODUP",       "���ƾ֤�ظ�" },
#define E_TR_PROCESSED              E_TR_BASE+5    
    { E_TR_PROCESSED,       "E_TR_PROCESSED",       "�����Ѿ�����" },
#define E_TR_STATE_UNKNOWN          E_TR_BASE+6     
    { E_TR_STATE_UNKNOWN,   "E_TR_STATE_UNKNOWN",   "����״̬��ȷ��" },
#define E_TR_STATE                  E_TR_BASE+7
    { E_TR_STATE,           "E_TR_STATE",           "ԭ����ʧ��" },
#define E_TR_ROLLBACK               E_TR_BASE+8
    { E_TR_ROLLBACK,        "E_TR_ROLLBACK",        "ԭ�����ѳ���" },
#define E_TR_REVOKE                 E_TR_BASE+9
    { E_TR_REVOKE,          "E_TR_REVOKE",          "ԭ�����ѳ���" },
#define E_TR_NOTEXIST               E_TR_BASE+10
    { E_TR_NOTEXIST,        "E_TR_NOTEXIST",        "ԭ���ײ�����" },
#define E_TR_ORGPWD                 E_TR_BASE+12
    { E_TR_ORGPWD,          "E_TR_ORGPWD",          "��λ�����" },
#define E_TR_AMT_EXCEED             E_TR_BASE+16
    { E_TR_AMT_EXCEED,      "E_TR_AMT_EXCEED",      "���׽���" },
#define E_TR_CURCODE                E_TR_BASE+17
    { E_TR_CURCODE,         "E_TR_CURCODE",         "���ֻ򳮻����ʹ�" },
#define E_TR_SUM                    E_TR_BASE+18
    { E_TR_SUM,             "E_TR_SUM",             "���������ֲܷ�ƽ" },
#define E_TR_HOLDAMT_NOT_ENOUGH     E_TR_BASE+19
    { E_TR_HOLDAMT_NOT_ENOUGH,  "E_TR_HOLDAMT_NOT_ENOUGH",  "Ȧ������" },
#define E_TR_ELEMENT_NOT_MATCH      E_TR_BASE+20
    { E_TR_ELEMENT_NOT_MATCH,   "E_TR_ELEMENT_NOT_MATCH",   "Ҫ�ظ��˲�һ��" },
#define E_QUERY_PLEASE              E_TR_BASE+21
    { E_QUERY_PLEASE,           "E_QUERY_PLEASE",   "����Ӧ, ���Ժ��ѯ" },

/* 3101 ~ 3199 Ʊ����� */
#define E_NOTE_BASE         E_TR_BASE+100            /* Ʊ����� */
#define E_NOTE_TYPE         E_NOTE_BASE+1   
    { E_NOTE_TYPE,          "E_NOTE_TYPE",         "Ʊ�����ʹ���" },
#define E_NOTE_REVOKE       E_NOTE_BASE+3   
    { E_NOTE_REVOKE,        "E_NOTE_REVOKE",       "Ʊ��������ע��" },
#define E_NOTE_DUEDATE      E_NOTE_BASE+5
    { E_NOTE_DUEDATE,       "E_NOTE_DUEDATE",      "Ʊ��δ����Ч��"},

/* E_TR_BASE+500֮�����ר���ڹ�Ա�ն˴���ϵͳ */

#define E_TR_NOTCHECKED     E_TR_BASE+501
    { E_TR_NOTCHECKED,      "E_TR_NOTCHECKED",    "����δ������" },
#define E_TR_EXIST          E_TR_BASE+502
    { E_TR_EXIST,           "E_TR_EXIST",         "ԭ�����Ѵ���" },
#define E_TR_AMT_FORMAT     E_TR_BASE+503
    { E_TR_AMT_FORMAT,      "E_TR_AMT_FORMAT",    "����ʽ����" },
#define E_TR_PRINT          E_TR_BASE+504
    { E_TR_PRINT,          "E_TR_PRINT",          "���ɴ�ӡ�ļ�ʧ��" },


/* �������3901-3999 */
#define E_DAYEND_BASE       E_TR_BASE+900         /* �������3901-3999 */
#define E_DAYEND_SETTFILE   E_DAYEND_BASE+1
    { E_DAYEND_SETTFILE,    "E_DAYEND_SETTFILE",  "�����ļ�������" },
#define E_DAYEND_NODATA     E_DAYEND_BASE+2
    { E_DAYEND_NODATA,      "E_DAYEND_NODATA",      "�ޱ����ν�������" },
#define E_DAYEND_SETTFAILED E_DAYEND_BASE+3
    { E_DAYEND_SETTFAILED,  "E_DAYEND_SETTFAILED","���˳���" },
#define E_DAYEND_DBBAKFILE   E_DAYEND_BASE+4
    { E_DAYEND_DBBAKFILE,   "E_DAYEND_DBBAKFILE",  "���ݱ����ļ�������" },
#define E_DAYEND_NOT_CHECKED   E_DAYEND_BASE+5
    { E_DAYEND_NOT_CHECKED, "E_DAYEND_NOT_CHECKED",  "���������δ���" },
#define E_DAYEND_ARCHIVE   E_DAYEND_BASE+6
    { E_DAYEND_ARCHIVE,    "E_DAYEND_ARCHIVE",  "�������ݹ鵵ʧ��" },
#define E_YEAREND_CONDITION   E_DAYEND_BASE+7
    { E_YEAREND_CONDITION, "E_YEAREND_CONDITION",  "�������������ݿ��л�����" },
#define E_LOADDATE   E_DAYEND_BASE+8
    { E_LOADDATE,    "E_LOADDATE",  "��������С�ڵ����ѹ鵵����" },
#define E_RCPT_NOTEXIST   E_DAYEND_BASE+9
    { E_RCPT_NOTEXIST,    "E_RCPT_NOTEXIST",  "���˻�ִ�ļ�������" },


/*8001-8999������ʹ��*/
#define E_ACPT_BASE       8000         

#define E_ACPT_ORG_NOT_EXIST    E_ACPT_BASE+33   
    { E_ACPT_ORG_NOT_EXIST,     "E_ACPT_ORG_NOT_EXIST",  "�������������" },

#define E_ACPT_NOTE_TYPE        E_ACPT_BASE+36   
    { E_ACPT_NOTE_TYPE,         "E_ACPT_NOTE_TYPE",      "Ʊ�����ʹ���" },

#define E_ACPT_NOTE_USED        E_ACPT_BASE+38   
    { E_ACPT_NOTE_USED,         "E_ACPT_NOTE_USED",       "Ʊ����ʹ��" },

#define E_ACPT_SERIALDUP        E_ACPT_BASE+43   
    { E_ACPT_SERIALDUP,         "E_ACPT_SERIALDUP",       "������ˮ�ظ�" },

#define E_ACPT_TR_STATE_UNKNOWN E_ACPT_BASE+48   
    { E_ACPT_TR_STATE_UNKNOWN,  "E_ACPT_TR_STATE_UNKNOWN", "���׽����ȷ��" },
#define E_WAITING_CONFIRMATION  E_ACPT_TR_STATE_UNKNOWN 

#define E_ACPT_DATA_REQUIRED    E_ACPT_BASE+51  
    { E_ACPT_DATA_REQUIRED,     "E_ACPT_DATA_REQUIRED",  "����Ҫ�ز�ȫ" },

#define E_ACPT_NOTE_INVALID     E_ACPT_BASE+52   
    { E_ACPT_NOTE_INVALID,      "E_ACPT_NOTE_INVALID",   "Ʊ�ݺ���Ч" },
#define E_NOTE_INVALID          E_ACPT_NOTE_INVALID   

#define E_ACPT_NOTE_EXPIRE      E_ACPT_BASE+53   
    { E_ACPT_NOTE_EXPIRE,       "E_ACPT_NOTE_EXPIRE",    "Ʊ���ѹ���" },
#define E_NOTE_EXPIRE           E_ACPT_NOTE_BASE

#define E_ACPT_NOTE_LOSS        E_ACPT_BASE+54   
    { E_ACPT_NOTE_LOSS,         "E_ACPT_NOTE_LOSS",      "Ʊ���ѹ�ʧֹ��" },

#define E_ACPT_ACCT_FROZEN      E_ACPT_BASE+55   
    { E_ACPT_ACCT_FROZEN,       "E_ACPT_ACCT_FROZEN",    "�˻��Ѷ���" },

#define E_ACPT_PAYPWD           E_ACPT_BASE+56   
    { E_ACPT_PAYPWD,            "E_ACPT_PAYPWD",         "֧���������" },

#define E_ACPT_ACCT_BALANCE     E_ACPT_BASE+57
    { E_ACPT_ACCT_BALANCE,      "E_ACPT_ACCT_BALANCE",   "�˻�����" },

#define E_ACPT_ACCT_NOT_EXIST   E_ACPT_BASE+58
    { E_ACPT_ACCT_NOT_EXIST,    "E_ACPT_ACCT_NOT_EXIST", "�������˻�������" },

#define E_ACPT_TR_NOT_EXIST     E_ACPT_BASE+59
    { E_ACPT_TR_NOT_EXIST,      "E_ACPT_TR_NOT_EXIST",   "ԭ���ײ�����" },

#define E_ACPT_TR_FAILED        E_ACPT_BASE+60
    { E_ACPT_TR_FAILED,         "E_ACPT_TR_FAILED",      "ԭ����ʧ��" },

#define E_ACPT_ORGPWD           E_ACPT_BASE+61
    { E_ACPT_ORGPWD,            "E_ACPT_ORGPWD",         "��λ�������" },

#define E_ACPT_TESTKEY_CHK      E_ACPT_BASE+62   
    { E_ACPT_TESTKEY_CHK,       "E_ACPT_TESTKEY_CHK",    "��������Ѻ����" },

#define E_ACPT_NOTE_DUEDATE     E_ACPT_BASE+63
    { E_ACPT_NOTE_DUEDATE,      "E_ACPT_NOTE_DUEDATE",   "Ʊ��δ����Ч��"},

#define E_ACPT_AGREEMENT        E_ACPT_BASE+65
    { E_ACPT_AGREEMENT,         "E_ACPT_AGREEMENT",      "Э��Ų���" },

#define E_ACPT_TR_AMT_EXCEED    E_ACPT_BASE+73
    { E_ACPT_TR_AMT_EXCEED,     "E_ACPT_TR_AMT_EXCEED",  "���׽���" },

#define E_ACPT_CLOSED           E_ACPT_BASE+103   
    { E_ACPT_CLOSED,            "E_ACPT_CLOSED",        "�������ϵͳ�ѹر�" },

#define E_ACPT_CURCODE          E_ACPT_BASE+117   
    { E_ACPT_CURCODE,           "E_ACPT_CURCODE",        "���ֻ򳮻����ʹ���" },

#define E_ACPT_ACCTPWD          E_ACPT_BASE+156   
    { E_ACPT_ACCTPWD,           "E_ACPT_ACCTPWD",        "�����˻��������" },

#define E_ACPT_COMM_SEND        E_ACPT_BASE+202
    { E_ACPT_COMM_SEND,         "E_ACPT_COMM_SEND",     "ͨѶ����(����ʧ��)" },

#define E_ACPT_COMM_RECV        E_ACPT_BASE+203
    { E_ACPT_COMM_RECV,        "E_ACPT_COMM_RECV",   "����������Ӧ��ʧ��" },

#define E_ACPT_TIMEOUT          E_ACPT_BASE+204
    { E_ACPT_TIMEOUT,           "E_ACPT_TIMEOUT",       "�����н��׳�ʱ" },

#define E_ACPT_BUSSY            E_ACPT_BASE+205
    { E_ACPT_BUSSY,             "E_ACPT_BUSSY",          "�Է�ϵͳæ" },

#define E_GNR_PRES_DB           E_ACPT_BASE+301
    { E_GNR_PRES_DB,            "E_GNR_PRES_DB",         "ǰ�û����ݿ������" },

#define E_ACPT_DATAFMT          E_ACPT_BASE+401
    { E_ACPT_DATAFMT,           "E_ACPT_DATAFMT",        "���ݱ��ĸ�ʽ��" },

#define E_ACPT_DEFAULT          E_ACPT_BASE+999 
    { E_ACPT_DEFAULT,           "E_ACPT_DEFAULT",        "���ջ����ڲ�����" },

    /*9999 ���������������� */
#define E_CENTER_DEFAULT    MAX_ErrCode
    { E_CENTER_DEFAULT,     "E_CENTER_DEFAULT",   "����������������" },
#define E_DEFAULT           E_CENTER_DEFAULT

};

/***************************************************
    ���¶�������������������
***************************************************/
#ifndef SetError
void SetError( int err );
#endif
/*****************************************
��;:���ô������
�������:
    err �������
��������:
    ��
*****************************************/

int GetError();
/*****************************************
��;:ȡ������Ϣ
�������:
    ��
��������:
    ������Ϣ
*****************************************/

void ClearError();
/*****************************************
��;:���������Ϣ
�������:
    ��
��������:
    ��
*****************************************/

char *errmsg( int errnum );    /* ������Ϣ˵�� */

#endif /* __ERRCODE_H__ */
