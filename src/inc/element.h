#ifndef ELEMENT_H_
#define ELEMENT_H_

#include "pub.h"

/* ��ʼ���ֵ��ļ� */
xmlDoc *initDictDoc();

#define OPPACK_ENCODEING        "GB18030"

#define XMLDECLARE(ENCODING)    "<?xml version='1.0' encoding='"ENCODING"'?>"

#define XMLROOTDOCSTRING        XMLDECLARE(OPPACK_ENCODEING)"<ROOT/>"

#define NEWROOTDOC()            (xmlRecoverMemory(XMLROOTDOCSTRING, strlen(XMLROOTDOCSTRING)))

/*
 * ����Խڵ�ֵ���д���ĺ���ָ������
 *
 * pnode: ��Ҫ����Ľڵ�
 * value: ��ֵ
 *
 * ����: �ɹ� ������ֵ ʧ�� NULL 
 */
typedef char *(*FPProcessNodeValue)(xmlNodePtr pnode, const char *value);

/*
 * ��ָ��BUF����ָ���ڵ�ֵ(�Խڵ��ֵȫ������Trim����)
 *
 * pnode: �ڵ������ֵ��ж���ı��Ľڵ�
 * buf: Դ���ݵ���ʼ��ַ
 *
 * ����: �ɹ� ���õ�Ŀ��ֵ�ĳ��� ʧ�� -1
 *
 * ע:����ýڵ������ֵ���FMT���Ա�ʾ�ĳ�����0���ʾ��Ԫ��ȡbuf�е�������г���
 */
int SetNodeValueFromBuff(xmlNodePtr pnode, char *buf);

/*
 * �����ױ����нڵ��ֵ
 *
 * pnode: ���Ľڵ�ָ��
 * value: ���Ľڵ�Դֵ
 *
 * ����: �����ı��Ľڵ�ֵ ʧ�� NULL
 *
 * ע:pnode�ڵ������ֵ��ж���
 */
char *ProcessTranNodeValue(xmlNodePtr pnode, const char *value);

/*
 * �����������ֵ��ж���Ľڵ��ֵ
 *
 * dictname: ���Ľڵ�����
 * value: ���Ľڵ�Դֵ
 *
 * ����: �����ı��Ľڵ�ֵ ʧ�� NULL
 *
 * ע:dictname���ֵ�����δ�ҵ���Ӧ�����򷵻�ԭֵ
 */
char *ProcessDictNodeValue(const char *dictname, char *value, int *plen);

/*
 * �������ʽ�������ԵĽڵ��ֵ
 *
 * pnode: ����ʽ�������ԵĽڵ�ָ��
 * value: ���Ľڵ�Դֵ
 *
 * ����: �����ı��Ľڵ�ֵ ʧ�� NULL
 */
char *ProcessFormatNodeValue(xmlNodePtr pnode, const char *value, int *plen);

/*
 * �������ݿ����ñ����нڵ��ֵ
 *
 * pnode: ���Ľڵ�ָ��
 * value: ���Ľڵ�Դֵ
 *
 * ����: �����ı��Ľڵ�ֵ ʧ�� NULL
 */
#define ProcessDBNodeValue(pnode, value)  ProcessFormatNodeValue(pnode, value, NULL)

/*
 * ��̬����ڵ�ֵ
 * value: Դֵ
 * pFMT: �ڵ��FMT����ֵ
 * pVALUE: �ڵ��VALUE����ֵ
 * pHFUNC: �ڵ��HFUNC����ֵ
 * pDESC: �ڵ��DESC����ֵ
 *
 * ����: �ɹ� ������ֵ ʧ�� NULL
 */
char *ProcessNodeValueBase(const char *value, int *plen, const char *pFMT, 
        const char *pVALUE, const char *pHFUNC, const char *pDESC);

#endif
