#ifndef SDP_H_
#define SDP_H_

#include "SDKxml.h"

#define DOCENC(doc) doc->encoding==NULL ? "UTF-8" : (char*)doc->encoding

#define sdpTrimHeadZero(src)  sdpStringTrimHeadChar(src, '0')

//去掉指定字符串的所有以trim字符开头的字符
char *sdpStringTrimHeadChar(char *src, char trim);

//去掉指定字符串的所有以trim字符结尾的字符
char *sdpStringTrimTailChar(char *src, char trim);

//去掉指定字符串的所有前导和后导空白字符
char *sdpStringTrim(char *src);

int sdpMemcpy(void *dst, int count, void *src1, int src1Len, ...);

//对指定字符串进行填充,len:目标字符串长度 padway:L左补 R右补 padder:填充的字符
char *sdpStringPad(const char *src, int len, char padway, char padder);

//对指定字符串进行分隔 words:存放分隔后的子串指针数组 maxwords:最大分隔的列数 delim:分隔符
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
