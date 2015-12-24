/*
 *Description : Head file for the XMLFunc.c 
 *Author	  : TQZ
 *Date        : 2006-5-25
 */
//#include <iconv.h>
//#include <libxml/debugXML.h>
//#include <libxml/xmlschemastypes.h>
//#include <libxml/xpathInternals.h>
//
#ifndef __XMLFUNC_H__
#define __XMLFUNC_H__

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>

#include <libxml/xpath.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlmemory.h>

#define M_PARSE_ERR_XML_FAIL                     999    /*Parse failed without DTD or Schema*/
#define M_PARSE_ERR_SCHEMA_FAIL                  1000   /*Parse Schema failed*/
#define M_PARSE_ERR_FAIL_WITHSCHEMA              1001   /*Parse failed with Schema*/
#define M_PARSE_ERR_GEN_INTERERR                 1002   /*Parse failed with Schema and validation generated an internal error*/
#define M_PARSE_ERR_SUCEEDED                     0      /*Parse suceeded*/
#define XMLFILE_ENCODING                              "ISO-8859-1"
#define XMLPARSE_ENCODING                             "ISO-8859-1"
#define MY_ENCODING                                   "ISO-8859-1"
#define GB_ENCODING                                   "GB18030"
#define LIBXML2_LOG				      "LIBXML2.log" 

typedef xmlNode *XMLNodePtr;    /*A type of pointer to a XML document node*/
typedef xmlDoc *XMLDocPtr;		/*A type of pointer to a XML document*/

#define ROOTNODE xmlDocGetRootElement

/*Create a memory XML document*/
XMLDocPtr CreateXMLDoc(char *pRootNode);

/*Create a memory node*/
XMLNodePtr CreateXMLNode(char *pXMLNode, char *pContent);

/*******************************************************************
Get the root element of the xml doc

 此函数不再使用.替代:使用宏定义ROOTNODE
 孙澜  2006/08/17

XMLNodePtr GetRootElement(XMLDocPtr xdDoc);
*******************************************************************/


/**********************************************************************
*Add a node to another node*
XMLNodePtr AddXMLChild(XMLNodePtr xnFather, XMLNodePtr xnChild);
本函数不再使用.替代函数:
int XMLAppendNode( xmlDoc *doc, const char *xpath, xmlNode *node );
SUNLAN 2006/08/17
**********************************************************************/

/*Function: Save a XML doc as specific encoding*/
int SaveXMLFile(XMLDocPtr xdDoc, const char *pEncoding, char *pFileName);
xmlDocPtr XMLParseFile(char *filename);

/*Parse a file XML doc*/
int ParseXMLDoc(char *pXMLFileName, char *pSchemaFileName);

/*Parse a file XML doc modal and initialize it*/
XMLDocPtr ParseXMLDocModal(char *pModalFileName);

/*
 *Function: Parse a memory XML doc
 *Param   : xdDoc - the memory XML pointer; pSchemaFileName - the schema file name,this param could be NULL,in 
 *          this case, the parser would ignore the schema vilidation; the file name should include the full path
 *return  : 0 if success, else it will return non-zero
 */
int ParseMemXMLDoc(XMLDocPtr xdDoc, char *pSchemaFileName);


/**************************************************************
Converts @in into UTF-8 for processing with libxml2 APIs
xmlChar *ConvertInput(const char *in, const char *encoding);
本函数在XMLFunc.c中内部使用,不对外部程序开放
SUNLAN 2006/08/16
**************************************************************/


/*****************************************************************************
以下函数在程序中无定义.删除
SUNLAN 2006/08/16 

**
 * asciiToUTF8:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of ASCII chars
 * @inlen:  the length of @in
 *
 * Take a block of ASCII chars in and try to convert it to an UTF-8
 * block of chars out.
 * Returns 0 if success, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     as the return value is positive, else unpredictiable.
 * The value of @outlen after return is the number of ocetes consumed.
 *
int
asciiToUTF8(unsigned char* out, int *outlen, const unsigned char* in, int *inlen);

*convert a encoding charset to UTF-8 encoding charset*
unsigned char * ConvertToUTF (unsigned char *strIn, char *strEncoding);

unsigned char* ConvertFromUTF (unsigned char *strIn, char *strEncoding);

***************************** 删除结束 SUNLAN 2006/08/16 **************/

//从XML文件检测XML树的合法性
XMLDocPtr  GetDocFromFile(char * filename, int* errcode);

/****************************************************************
由于本函数在程序中未定义,由孙澜于2006/08/17删除
//从XML文件获得XML树
XMLDocPtr  GetDocFromFile2(char * filename);
****************************************************************/

//保存树到XML文件
int   SaveXMLToFile(XMLDocPtr Doc,char *filename);

//生成XML空树
XMLDocPtr  MakeNewDoc(char *Head);

//从指定长度缓冲区中检测XML树的合法性
XMLDocPtr  GetDocFromBuff(const char * buff, int buff_len, int *errcode);

//将XML树写入缓冲区,返回buff长度
int  WriteDocToBuff(XMLDocPtr Doc, unsigned char **buff); 

//释放XML树
void  FreeXMLDoc(XMLDocPtr Doc);

/****************************************************************************
 *  将数据项写入指定XML文件
 *
 *  本函数不再使用
 *  替代函数: int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
int SetNodeValue(char *pFileName, char *pParentNode, char *pCurNode, const char *pCurValue);
*****************************************************************************/


/****************************************************************************
 *  从指定XML文件中获取数据项
 *
 *  本函数不再使用
 *  替代函数: int XMLGetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
char *GetNodeValue(char *pFileName, char *pParentNode, char *pCurNode,char *pCurVal);
*****************************************************************************/


/****************************************************************************
 *  将数据项写入指定XML树
 *
 *  本函数不再使用
 *  替代函数: int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
int MemSetNodeValue(XMLDocPtr xdDoc, char *pParentNode, char *pCurNode, const char *pCurValue);
*****************************************************************************/


/****************************************************************************
 *   从指定XML树中获取数据项
 *
 *   本函数不再使用
 *   替代函数: int XMLGetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *   2006/08/15 SUNLAN
 *
char *MemGetNodeValue(XMLDocPtr xdDoc, char *pParentNode, char *pCurNode,char *pCurVal);
*****************************************************************************/


//根据函数名称获取函数指针
int GetFunc( char *funcname, int (**ppFunc)() );

//释放XML API中分配的内存空间
void FreeXML(void *mem);


/*********************************************************************
char* XMLGetNodeVal( xmlDoc *doc, const char *xpath );
说明:按指定的路径从XML中取节点的值.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要查询的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
返回:
    成功时返回节点值,失败时返回NULL.
*********************************************************************/
char* XMLGetNodeVal( xmlDoc *doc, const char *xpath );


/*********************************************************************
说明:按指定的路径从XML中返回节点集.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
返回:
    在成功时返回节点列表的指针,失败时返回NULL
    返回的节点列表中的节点数量保存在xmlNodePtr->nodeNr中,节点列表通过
    xmlNodePtr->nodeTab访问
*********************************************************************/
xmlNodeSetPtr XMLGetNodeSet( xmlDoc *doc, const char *xpath );


/*********************************************************************
说明:按路径设置XML中指定节点的值.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要设置的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
    val: 节点的值
返回:
    0 成功
   -1 失败
*********************************************************************/
int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );


/*************************************************************************
说明:在路径指定的XML节点上增加新的节点.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
    node: 新节点
返回:
    0 成功
   -1 失败
*********************************************************************/
int XMLAppendNode( xmlDoc *doc, const char *xpath, xmlNode *node );


/*************************************************************************
作者:孙澜
说明:在路径指定的XML节点上增加新的节点及内容.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
    nodename: 新节点的名称
    nodeprop: 节点属性.格式必须遵照XML所约定的propname="propval".
              属性名称和属性的最大长度为63字节.目前暂时只能一次增加一个属性
    val: 新节点的内容
返回:
    0 成功
   -1 失败
修改记录:
    2006/08/17 由孙澜增加对节点属性的操作
*********************************************************************/
int XMLAppendNodeVal( xmlDoc *doc, const char *xpath,
    const char *nodename, const char *nodeprop, const char *val );


/*************************************************************************
XMLNodePtr XMLAddField( xmlNodePtr pNode, char *pcName, char *pcVal )
作者: Chen Jie
说明: 在当前节点增加一个新节点
参数:
     pNode: xmlNodePtr 指针
    pcName: 新节点的名称
     pcVal: 节点内容
返回:
     新节点的指针
修改记录:
*********************************************************************/
xmlNodePtr XMLAddField( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
  char *XMLGetField( xmlNodePtr pNode, char *pcName, char *pcVal )
作者: Chen Jie
说明: 获取当前节点的内容
参数:
pNode: xmlNodePtr 指针
pcName: 节点的名称
pcVal: 节点内容
返回: 节点内容的指针
修改记录:
 *********************************************************************/
char *XMLGetField( xmlNodePtr pNode, char *pcName, char *pcVal);

/*************************************************************************
int XMLAddFieldProp( xmlNodePtr pNode, char *pcName, char *pcVal )
作者: Chen Jie
说明: 在当前节点增加一个新属性
参数:
     pNode: xmlNodePtr 指针
    pcName: 新属性的名称
     pcVal: 属性内容
返回:
    -1 or 0
修改记录:
*********************************************************************/
int XMLAddFieldProp( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
int XMLUpdField( xmlNodePtr pNode, char *pcName, char *pcVal )
作者: Chen Jie
说明: 修改当前节点中指定叶子的值
参数:
     pNode: xmlNodePtr 指针
    pcName: 节点的名称
     pcVal: 新节点内容
返回:
     -1 or 0
修改记录:
*********************************************************************/
int XMLUpdField( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
char *GetField( xmlDocPtr doc, char *pcName)
作者: Chen Jie
说明: 取数据库记录中的成员值
参数:
      pNode: xml 指针
      pcName: 字段名
返回:
      成员值
修改记录:
 *********************************************************************/
char *GetField( char *pcName, xmlDoc *doc );

char *GetUFTPField( const char *field, xmlDoc *doc );

int XMLAppendNodeSet( xmlDocPtr doc, const char *xpath, xmlNodeSetPtr nodeset );

xmlDocPtr TrnjourToUFTPDoc( xmlDocPtr rs );

int XMLDocReplaceNode( xmlDocPtr xmlReq, xmlDocPtr xmlRsp, char *xpath );

char *GetFieldByRow( int iRow, char *pcName, xmlDocPtr doc );

unsigned char *encoding_conv( char *bufin, const char *now, const char *enc );
int XMLSetVal( xmlDocPtr doc, const char *xpath, char *val );
int xmlSetVal( xmlDocPtr doc, const char *xpath, char *val );
char *XMLGetVal(xmlDocPtr doc, const char *xpath, char *val);
char *xmlGetVal(xmlDocPtr doc, const char *xpath, char *val);
char *XMLGetDynVal( xmlDocPtr doc, const char *xpath);
int         XMLGetValToInt(xmlDocPtr doc, const char *xpath);
double      XMLGetValToDbl(xmlDocPtr doc, const char *xpath);
xmlXPathObjectPtr getnodeset(xmlDocPtr doc, const char *xpath);
int XMLAddNode( xmlDocPtr doc, const char *xpath, xmlNodePtr cur );
char *XMLNodeDump(xmlNodePtr cur);
char *xmlNodeListDump(xmlNodePtr cur);
xmlNodePtr XMLGetStaticNode(xmlDocPtr doc, const char *xpath);

int RowCount( xmlDocPtr doc );
int SetUFTPField( const char *field, char *val, xmlDoc *doc );
void XMLFreeDoc(xmlDocPtr doc);
int XMLReplaceNode( xmlDocPtr doc, const char *xpath, xmlNodePtr cur );
int XMLAppendContent(xmlNodePtr cur, char *content);
int XMLRemoveBlankNode(xmlDocPtr doc, char *path);

#endif
