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

 �˺�������ʹ��.���:ʹ�ú궨��ROOTNODE
 ����  2006/08/17

XMLNodePtr GetRootElement(XMLDocPtr xdDoc);
*******************************************************************/


/**********************************************************************
*Add a node to another node*
XMLNodePtr AddXMLChild(XMLNodePtr xnFather, XMLNodePtr xnChild);
����������ʹ��.�������:
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
��������XMLFunc.c���ڲ�ʹ��,�����ⲿ���򿪷�
SUNLAN 2006/08/16
**************************************************************/


/*****************************************************************************
���º����ڳ������޶���.ɾ��
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

***************************** ɾ������ SUNLAN 2006/08/16 **************/

//��XML�ļ����XML���ĺϷ���
XMLDocPtr  GetDocFromFile(char * filename, int* errcode);

/****************************************************************
���ڱ������ڳ�����δ����,��������2006/08/17ɾ��
//��XML�ļ����XML��
XMLDocPtr  GetDocFromFile2(char * filename);
****************************************************************/

//��������XML�ļ�
int   SaveXMLToFile(XMLDocPtr Doc,char *filename);

//����XML����
XMLDocPtr  MakeNewDoc(char *Head);

//��ָ�����Ȼ������м��XML���ĺϷ���
XMLDocPtr  GetDocFromBuff(const char * buff, int buff_len, int *errcode);

//��XML��д�뻺����,����buff����
int  WriteDocToBuff(XMLDocPtr Doc, unsigned char **buff); 

//�ͷ�XML��
void  FreeXMLDoc(XMLDocPtr Doc);

/****************************************************************************
 *  ��������д��ָ��XML�ļ�
 *
 *  ����������ʹ��
 *  �������: int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
int SetNodeValue(char *pFileName, char *pParentNode, char *pCurNode, const char *pCurValue);
*****************************************************************************/


/****************************************************************************
 *  ��ָ��XML�ļ��л�ȡ������
 *
 *  ����������ʹ��
 *  �������: int XMLGetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
char *GetNodeValue(char *pFileName, char *pParentNode, char *pCurNode,char *pCurVal);
*****************************************************************************/


/****************************************************************************
 *  ��������д��ָ��XML��
 *
 *  ����������ʹ��
 *  �������: int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *  2006/08/15 SUNLAN
 *
int MemSetNodeValue(XMLDocPtr xdDoc, char *pParentNode, char *pCurNode, const char *pCurValue);
*****************************************************************************/


/****************************************************************************
 *   ��ָ��XML���л�ȡ������
 *
 *   ����������ʹ��
 *   �������: int XMLGetNodeVal( xmlDoc *doc, const char *xpath, char *val );
 *   2006/08/15 SUNLAN
 *
char *MemGetNodeValue(XMLDocPtr xdDoc, char *pParentNode, char *pCurNode,char *pCurVal);
*****************************************************************************/


//���ݺ������ƻ�ȡ����ָ��
int GetFunc( char *funcname, int (**ppFunc)() );

//�ͷ�XML API�з�����ڴ�ռ�
void FreeXML(void *mem);


/*********************************************************************
char* XMLGetNodeVal( xmlDoc *doc, const char *xpath );
˵��:��ָ����·����XML��ȡ�ڵ��ֵ.·���ĸ�ʽ��:/aaa/bbb[@cc="dd"]/ee
    ��Ҫ��ѯ�Ľڵ����ΪҶ�ӽڵ�
����:
    doc: XML�ĵ�ָ��
    xpath: �ڵ�·��
����:
    �ɹ�ʱ���ؽڵ�ֵ,ʧ��ʱ����NULL.
*********************************************************************/
char* XMLGetNodeVal( xmlDoc *doc, const char *xpath );


/*********************************************************************
˵��:��ָ����·����XML�з��ؽڵ㼯.·���ĸ�ʽ��:/aaa/bbb[@cc="dd"]/ee
����:
    doc: XML�ĵ�ָ��
    xpath: �ڵ�·��
����:
    �ڳɹ�ʱ���ؽڵ��б��ָ��,ʧ��ʱ����NULL
    ���صĽڵ��б��еĽڵ�����������xmlNodePtr->nodeNr��,�ڵ��б�ͨ��
    xmlNodePtr->nodeTab����
*********************************************************************/
xmlNodeSetPtr XMLGetNodeSet( xmlDoc *doc, const char *xpath );


/*********************************************************************
˵��:��·������XML��ָ���ڵ��ֵ.·���ĸ�ʽ��:/aaa/bbb[@cc="dd"]/ee
    ��Ҫ���õĽڵ����ΪҶ�ӽڵ�
����:
    doc: XML�ĵ�ָ��
    xpath: �ڵ�·��
    val: �ڵ��ֵ
����:
    0 �ɹ�
   -1 ʧ��
*********************************************************************/
int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );


/*************************************************************************
˵��:��·��ָ����XML�ڵ��������µĽڵ�.·���ĸ�ʽ��:/aaa/bbb[@cc="dd"]/ee
����:
    doc: XML�ĵ�ָ��
    xpath: �ڵ�·��
    node: �½ڵ�
����:
    0 �ɹ�
   -1 ʧ��
*********************************************************************/
int XMLAppendNode( xmlDoc *doc, const char *xpath, xmlNode *node );


/*************************************************************************
����:����
˵��:��·��ָ����XML�ڵ��������µĽڵ㼰����.·���ĸ�ʽ��:/aaa/bbb[@cc="dd"]/ee
����:
    doc: XML�ĵ�ָ��
    xpath: �ڵ�·��
    nodename: �½ڵ������
    nodeprop: �ڵ�����.��ʽ��������XML��Լ����propname="propval".
              �������ƺ����Ե���󳤶�Ϊ63�ֽ�.Ŀǰ��ʱֻ��һ������һ������
    val: �½ڵ������
����:
    0 �ɹ�
   -1 ʧ��
�޸ļ�¼:
    2006/08/17 ���������ӶԽڵ����ԵĲ���
*********************************************************************/
int XMLAppendNodeVal( xmlDoc *doc, const char *xpath,
    const char *nodename, const char *nodeprop, const char *val );


/*************************************************************************
XMLNodePtr XMLAddField( xmlNodePtr pNode, char *pcName, char *pcVal )
����: Chen Jie
˵��: �ڵ�ǰ�ڵ�����һ���½ڵ�
����:
     pNode: xmlNodePtr ָ��
    pcName: �½ڵ������
     pcVal: �ڵ�����
����:
     �½ڵ��ָ��
�޸ļ�¼:
*********************************************************************/
xmlNodePtr XMLAddField( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
  char *XMLGetField( xmlNodePtr pNode, char *pcName, char *pcVal )
����: Chen Jie
˵��: ��ȡ��ǰ�ڵ������
����:
pNode: xmlNodePtr ָ��
pcName: �ڵ������
pcVal: �ڵ�����
����: �ڵ����ݵ�ָ��
�޸ļ�¼:
 *********************************************************************/
char *XMLGetField( xmlNodePtr pNode, char *pcName, char *pcVal);

/*************************************************************************
int XMLAddFieldProp( xmlNodePtr pNode, char *pcName, char *pcVal )
����: Chen Jie
˵��: �ڵ�ǰ�ڵ�����һ��������
����:
     pNode: xmlNodePtr ָ��
    pcName: �����Ե�����
     pcVal: ��������
����:
    -1 or 0
�޸ļ�¼:
*********************************************************************/
int XMLAddFieldProp( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
int XMLUpdField( xmlNodePtr pNode, char *pcName, char *pcVal )
����: Chen Jie
˵��: �޸ĵ�ǰ�ڵ���ָ��Ҷ�ӵ�ֵ
����:
     pNode: xmlNodePtr ָ��
    pcName: �ڵ������
     pcVal: �½ڵ�����
����:
     -1 or 0
�޸ļ�¼:
*********************************************************************/
int XMLUpdField( xmlNodePtr pNode, char *pcName, char *pcVal );

/*************************************************************************
char *GetField( xmlDocPtr doc, char *pcName)
����: Chen Jie
˵��: ȡ���ݿ��¼�еĳ�Աֵ
����:
      pNode: xml ָ��
      pcName: �ֶ���
����:
      ��Աֵ
�޸ļ�¼:
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
