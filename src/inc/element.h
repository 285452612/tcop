#ifndef ELEMENT_H_
#define ELEMENT_H_

#include "pub.h"

/* 初始化字典文件 */
xmlDoc *initDictDoc();

#define OPPACK_ENCODEING        "GB18030"

#define XMLDECLARE(ENCODING)    "<?xml version='1.0' encoding='"ENCODING"'?>"

#define XMLROOTDOCSTRING        XMLDECLARE(OPPACK_ENCODEING)"<ROOT/>"

#define NEWROOTDOC()            (xmlRecoverMemory(XMLROOTDOCSTRING, strlen(XMLROOTDOCSTRING)))

/*
 * 定义对节点值进行处理的函数指针类型
 *
 * pnode: 需要处理的节点
 * value: 现值
 *
 * 返回: 成功 处理后的值 失败 NULL 
 */
typedef char *(*FPProcessNodeValue)(xmlNodePtr pnode, const char *value);

/*
 * 从指定BUF设置指定节点值(对节点的值全部会做Trim处理)
 *
 * pnode: 节点名在字典中定义的报文节点
 * buf: 源数据的起始地址
 *
 * 返回: 成功 设置的目标值的长度 失败 -1
 *
 * 注:如果该节点名在字典中FMT属性表示的长度是0则表示该元素取buf中的最后所有长度
 */
int SetNodeValueFromBuff(xmlNodePtr pnode, char *buf);

/*
 * 处理交易报文中节点的值
 *
 * pnode: 报文节点指针
 * value: 报文节点源值
 *
 * 返回: 处理后的报文节点值 失败 NULL
 *
 * 注:pnode节点名在字典中定义
 */
char *ProcessTranNodeValue(xmlNodePtr pnode, const char *value);

/*
 * 处理报文中在字典中定义的节点的值
 *
 * dictname: 报文节点名称
 * value: 报文节点源值
 *
 * 返回: 处理后的报文节点值 失败 NULL
 *
 * 注:dictname在字典中若未找到相应定义则返回原值
 */
char *ProcessDictNodeValue(const char *dictname, char *value, int *plen);

/*
 * 处理带格式配置属性的节点的值
 *
 * pnode: 带格式配置属性的节点指针
 * value: 报文节点源值
 *
 * 返回: 处理后的报文节点值 失败 NULL
 */
char *ProcessFormatNodeValue(xmlNodePtr pnode, const char *value, int *plen);

/*
 * 处理数据库配置报文中节点的值
 *
 * pnode: 报文节点指针
 * value: 报文节点源值
 *
 * 返回: 处理后的报文节点值 失败 NULL
 */
#define ProcessDBNodeValue(pnode, value)  ProcessFormatNodeValue(pnode, value, NULL)

/*
 * 动态处理节点值
 * value: 源值
 * pFMT: 节点的FMT属性值
 * pVALUE: 节点的VALUE属性值
 * pHFUNC: 节点的HFUNC属性值
 * pDESC: 节点的DESC属性值
 *
 * 返回: 成功 处理后的值 失败 NULL
 */
char *ProcessNodeValueBase(const char *value, int *plen, const char *pFMT, 
        const char *pVALUE, const char *pHFUNC, const char *pDESC);

#endif
