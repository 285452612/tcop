#ifndef SDP_H_
#define SDP_H_

#include "SDKxml.h"

#define DOCENC(doc) doc->encoding==NULL ? "UTF-8" : (char*)doc->encoding

#define sdpTrimHeadZero(src)  sdpStringTrimHeadChar(src, '0')

//ȥ��ָ���ַ�����������trim�ַ���ͷ���ַ�
char *sdpStringTrimHeadChar(char *src, char trim);

//ȥ��ָ���ַ�����������trim�ַ���β���ַ�
char *sdpStringTrimTailChar(char *src, char trim);

//ȥ��ָ���ַ���������ǰ���ͺ󵼿հ��ַ�
char *sdpStringTrim(char *src);

int sdpMemcpy(void *dst, int count, void *src1, int src1Len, ...);

//��ָ���ַ����������,len:Ŀ���ַ������� padway:L�� R�Ҳ� padder:�����ַ�
char *sdpStringPad(const char *src, int len, char padway, char padder);

//��ָ���ַ������зָ� words:��ŷָ�����Ӵ�ָ������ maxwords:���ָ������� delim:�ָ���
int sdpStringSplit(char *line, char *words[], int maxwords, int delim);

int sdpStringIsAllChar(char *str, char c);

int sdpFileLinesForeach(const char *pfile, int breakFlag, 
    int (*pfunc)(char *line, char *reserved), char *reserved);

void sdpDebugXmlDoc(xmlDoc *doc);

char *sdpXmlDocGetDeclare(xmlDoc *doc);

char *sdpXmlNodeDump2Str(xmlNode *node, int convertFlag);

char *sdpXmlNodeGetAttrText(xmlNode *pNode, char *attrName);

xmlNodeSetPtr sdpXmlSelectNodes(xmlDoc *doc, const char *xpath);

xmlNodePtr sdpXmlSelectNode(xmlDoc *doc, const char *xpath);

char *sdpXmlSelectNodeText(xmlDoc *doc, const char *xpath);

int sdpXmlNodeSetText(xmlDoc *doc, const char *xpath, const char *value);

void sdpXmlNodeAddChildren(xmlNodePtr parent, xmlNodeSetPtr nsPtr);

void sdpXmlNodeNew(xmlDoc *doc, const char *xpath, const char *value);

#endif
