#ifndef PACK_H_
#define PACK_H_

#include "util.h"
#include "element.h"

//报文类型
#define PACKTYPE_XML                'X'     //XML报文类型
#define PACKTYPE_STRUCT             'S'     //结构(定长)报文类型
#define PACKTYPE_TCOP               'P'     //平台报文类型(无需转换)

#define PACKMAPNODE_SRC             'S'     //该映射节点名为源报文中的节点名
#define PACKMAPNODE_DST             'D'     //该映射节点名为目标报文中的节点名

#define PACK_REQ2OP                 'A'     //到OP的请求报文(网点的提出或前置的提入)转换到OP后的XML报文
#define PACK_BANKREQ                'B'     //银行接口交易的请求XML报文
#define PACK_BANKRSP                'C'     //银行接口交易的响应XML报文
#define PACK_OP2RSP                 'D'     //OP应答时的响应XML报文
#define PACK_REQSTRUCTXML           'E'     //请求结构报文对应的XML报文
#define PACK_RSPSTRUCTXML           'F'     //响应结构报文对应的XML报文
#define PACK_OPBODY                 'G'     //OP报文体
#define PACK_TCPACK                 'H'     //同城交易的原始报文
#define PACK_RSP2OP                 'I'     //同城应答报文对应的OP报文
#define PACK_BANKXMLRSP             'J'     //银行接口交易的响应XML报文

#define PACK_MAPOP2RSP              'L'     //OP报文到同城的应答报文映射
#define PACK_MAPOP2BANK             'M'     //OP报文到银行的接口请求报文映射
#define PACK_MAPBANK2OP             'N'     //银行的接口应答报文到OP报文的映射
#define PACK_MAPTCXML2OP            'O'     //同城XML报文到OP报文的映射
#define PACK_MAPOP2TCXML            'P'     //OP报文到同城XML请求报文映射

extern char OP_TCPACKTYPE;                  //同城报文类型
extern char OP_PHPACKTYPE;                  //前置报文类型
extern char OP_BKPACKTYPE;                  //银行报文类型

#define GetOPVal(name)  XMLGetNodeVal(doc, "//"#name)

xmlDoc *getOPTemplateDoc(int optcode);

#define getOPDoc()  getOPTemplateDoc(OP_OPTCODE)

xmlDoc *getTemplateDocBase(char packattr, int bktcode, int optcode, int tctcode, char *xpath);

#define getTemplateDoc(packattr, bktcode, xpath)  getTemplateDocBase(packattr, bktcode, OP_OPTCODE, OP_TCTCODE, xpath)

#define getTCTemplateDoc(tctcode, xpath) getTemplateDocBase(PACK_TCPACK, 0, OP_OPTCODE, tctcode, xpath)

/*
 * 转换请求报文(同城提出或提入交易)到OP定义报文
 *
 * 返回: 成功 OP报文 失败 NULL
 */
xmlDoc *ConvertREQ2OP(char *reqbuf, char *reqfile, int *plen);

xmlDoc *ConvertREQ2TCXML(char *reqbuf, char *reqfile, int *plen);

char *ConvertOP2REQ(xmlDoc *opDoc, char *reqbuf, char *reqfile, int *plen);

/*
 * 转换OP报文到交易应答报文
 *
 * reqbuf: 为结构通讯报文预留(返回内容先从请求内容拷贝)
 *
 * 返回 应答报文长度 失败 <=0
 */
int ConvertOP2RSP(xmlDoc *opDoc, char *reqbuf, int reqlen, char *rspbuf, char *rspfile);

/*
 * 转换前置应答报文(同城提出响应)到OP定义报文
 *
 * 返回: 成功 OP报文 失败 NULL
 */
xmlDoc *ConvertPHRsp2OP(xmlDoc *opDoc, char *rspbuf, char *rspfile);

/*
 * 转换OP报文到银行接口通讯请求报文
 *
 * 返回: 成功 通讯请求报文长度 失败 <0
 */
int ConvertOP2BANK(char *buf, int bktcode, xmlDoc *opDoc);

/*
 * 转换银行接口通讯应答报文到OP报文
 *
 * responseFlag: 当应答分多种情况时需要传输应答解析标志
 *
 * 返回: 成功 OP报文 失败 NULL
 */
xmlDoc *ConvertBANK2OP(xmlDoc *opDoc, int bktcode, char *buf, int responseFlag);

/*
 * 转换同城报文(提出提入请求,提出响应)到OP定义报文
 *
 * tcbuf: 同城请求或应答报文
 * tcfile: 同城请求或应答文件名
 * mapNSXPath: 同城XML报文与OP报文的转换映射关系集合的XPATH
 *
 * 返回: 成功 OP报文 失败 NULL
 */
xmlDoc *ConvertTCPack2OP(xmlDoc *opDoc, char *tcbuf, int *plen, char *tcfile, char *mapNSXPath);

/*
 * 转换同城XML报文(提出提入或提出响应)到OP定义报文
 *
 * mapNSXPath: 同城报文与OP报文转换的映射关系的XPATH
 *
 * 返回: 成功 opDoc 失败 NULL
 */
xmlDoc *ConvertTCXML2OP(xmlDoc *opDoc, xmlDoc *tcDoc, char *mapNSXPath);

/*
 * 转换XML报文到XML报文
 *
 * dstDoc: 目标XML报文
 * srcDoc: 源XML报文
 * mapNSPtr: 转换规则映射节点集
 * mapWay: PACKMAPNODE_SRC 或 PACKMAPNODE_DST
 * ppath: 规则节点名在目标报文或源报文中的位置的父路径
 * fp: 转换时对节点值进行处理的函数
 * 
 * 返回: 成功 dstDoc 失败 NULL
 */
xmlDoc *ConvertXML2XML(xmlDoc *dstDoc, xmlDoc *srcDoc, xmlNodeSetPtr mapNSPtr, 
        char mapWay, char *ppath, FPProcessNodeValue fp);

/*
 * 转换OP报文到接口XML报文
 *
 * 返回: 成功 bkDoc 失败 NULL
 */
xmlDoc *ConvertOP2BankXML(int bktcode, xmlDoc *bkDoc, xmlDoc *opDoc);

/*
 * 转换接口XML报文到OP报文
 *
 * 返回: 成功 opDoc 失败 NULL
 */
xmlDoc *ConvertBankXML2OP(xmlDoc *opDoc, int bktcode, xmlDoc *bkDoc);

/*
 * 转换OP报文到同城应答XML报文
 *
 * 返回: 成功 tcDoc 失败 NULL
 */
xmlDoc *ConvertOP2TcRspXML(xmlDoc *tcDoc, xmlDoc *opDoc);

/*
 * 转换同城应答XML报文到结构报文
 *
 * 返回: 成功 结构报文长度 失败 <0
 */
int ConvertTcRspXML2Struct(char *buf, char *tcDoc);

/*
 * 转换结构体到XML节点集
 *
 * nsPtr 目标XML节点集
 * buf 结构体包文
 *
 * 成功: 0 失败: 错误码
 */
int ConvertStruct2Nodes(xmlNodeSetPtr nsPtr, const char *buf);

/*
 * 转换XML节点集到结构体
 *
 * buf 目标结构体包文
 * nsPtr 源XML节点集
 *
 * 成功:结构体长度 失败: <0
 */ 
int ConvertNodes2Struct(char *buf, xmlNodeSetPtr nsPtr);

int ConvertOP2TCStruct(char *buf, int *plen, int tctcode, xmlDoc *opDoc);

/*
 * 保存报文
 */
void SavePack(xmlDoc *doc, char savepackType, int bktcode);


#endif
