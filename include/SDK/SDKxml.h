/*******************************************************
 * SDKxml.h
 * XML处理相关定义
 ******************************************************/

#ifndef __SDK_XML_H__
#define __SDK_XML_H__

#include "SDKpub.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/globals.h>
#include <libxml/xmlschemas.h>

/********************************************
        XML
********************************************/

struct XMLAttr {
    char *name;
    char *value;
};

#define DOCROOT     xmlDocGetRootElement

#define ConvertInput( in, enc ) (xmlChar*)EncodingConv( in, enc, "UTF-8" )
#define ConvertOutput( in, enc ) (xmlChar*)EncodingConv( in, "UTF-8", enc )

xmlDoc *XMLNewDocEnc( const char *root, const char *enc );
/**************************************************************
作者:孙澜
说明:创建新的XML文档
参数:
    root: XML根节点的名称
    enc: XML文档的编码
返回:
    创建成功时返回新XML文档指针,失败时返回NULL
**************************************************************/

char* XMLGetNodeVal( xmlDoc *doc, const char *xpath );
/*********************************************************************
作者:孙澜
说明:按指定的路径从XML中取节点的值.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要查询的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
返回:
    成功时返回节点值,失败时返回NULL.
*********************************************************************/

char* XMLGetNodeAttr( xmlDoc *doc, const char *xpath, const char *name );
/***************************************************************************
作者:孙澜
说明:按指定的路径从XML中取节点的属性.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要查询的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
    name: 节点属性的名称
返回:
    成功时返回节点属性值,失败时返回NULL.
***************************************************************************/

int XMLSetNodeAttr( xmlDoc *doc, const char *xpath, const char *name,
    const char *val );
/***************************************************************************
int XMLSetNodeAttr( xmlDoc *doc, const char *xpath, const char *name,
    const char *val );
作者:孙澜
说明:设置xpath所指定节点的属性.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要设置的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
    name: 节点属性的名称
    val: 节点属性值
返回:
    成功时返回0,失败时返回-1.
***************************************************************************/

xmlNodeSetPtr XMLGetNodeSet( xmlDoc *doc, const char *xpath );
/*********************************************************************
作者:孙澜
说明:按指定的路径从XML中返回节点集.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
返回:
    在成功时返回节点列表的指针,失败时返回NULL
    返回的节点列表中的节点数量保存在xmlNodePtr->nodeNr中,节点列表通过
    xmlNodePtr->nodeTab访问
*********************************************************************/

xmlNode *XMLGetNode( xmlDoc *doc, const char *xpath );
/*********************************************************************
xmlNode *XMLGetNode( xmlDoc *doc, const char *xpath )
作者:孙澜
说明:按指定的路径从XML中返回节点指针.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
返回:
    在成功时返回节点的指针,节点不存在或存在多个节点时返回NULL
*********************************************************************/

int XMLSetNodeVal( xmlDoc *doc, const char *xpath, char *val );
/*********************************************************************
作者:孙澜
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

int XMLSetNodeValLen( xmlDoc *doc, const char *xpath, char *val, int len );
/*********************************************************************
作者:孙澜
说明:按路径设置XML中指定节点的值.路径的格式如:/aaa/bbb[@cc="dd"]/ee
    所要设置的节点必须为叶子节点
参数:
    doc: XML文档指针
    xpath: 节点路径
    val: 节点的值
    len: val的长度
返回:
    0 成功
   -1 失败
*********************************************************************/

int XMLAppendNode( xmlDoc *doc, const char *xpath, xmlNode *node );
/*************************************************************************
作者:孙澜
说明:在路径指定的XML节点上增加新的节点.路径的格式如:/aaa/bbb[@cc="dd"]/ee
参数:
    doc: XML文档指针
    xpath: 节点路径
    node: 新节点
返回:
    0 成功
   -1 失败
*********************************************************************/

int XMLAppendNodeVal( xmlDoc *doc, const char *xpath,
    const char *nodename, const char *nodeprop, const char *val );
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

int XMLValidDoc( xmlDoc *doc, const char *schemafile );
/*************************************************************************
作者:孙澜
说明:使用Schema验证XML文档格式的有效性
参数:
    doc: XML文档指针
    schemafile: Schema文件名称
返回:
    0 成功
   -1 失败
*********************************************************************/

xmlDoc *XMLDumpNodeAsDoc( xmlNode *node );
/*************************************************************************
xmlDoc *XMLDumpNodeAsDoc( xmlNode *node )
作者:孙澜
说明:将源文档中的某一节点复制为新的XML文档.同时复制文档的属性
参数:
    node: 用于生成新XML文档的节点
返回:
    成功时返回新xmlDoc指针,失败时返回NULL
*********************************************************************/

char *XMLDumpNodeAsStr( xmlNode *node );
/*********************************************************************
char *XMLDumpNodeAsStr( xmlNode *node )
作者:孙澜
说明:将源文档中的某一节点复制为新的XML文档.同时复制文档的属性
参数:
    node: 用于生成新XML文档的节点
返回:
    成功时返回新xmlDoc指针,失败时返回NULL
*********************************************************************/

int XMLAddComment( xmlNode *node, const char *comment );
/*********************************************************************
说明:在节点上增加注释信息
参数:
    node: 需要增加注释的节点.当需要为XML文档增加注释信息时node为文档的
          根节点("/")
返回:
    成功时返回0,失败时返回-1
*********************************************************************/

char* XMLGetNodeComment( xmlDoc *doc, const char *xpath, int index );
/***************************************************************************
char* XMLGetNodeComment( xmlDoc *doc, const char *xpath, int index )
作者:孙澜
说明:按指定的路径从XML中取节点的注释信息.路径的格式如:/aaa/bbb
    所要查询的节点必须为叶子节点
参数:
    doc:    XML文档指针
    xpath:  节点路径
    index:  第n项注释,从1开始计数.当index=0时默认取第一项注释,否则取指定项
            的注释
返回:
    成功时返回节点属性值,失败时返回NULL.
***************************************************************************/

struct XMLAttr** XMLGetAttrList( xmlNode *node );
/*********************************************************************
说明:读取节点的属性列表.
参数:
    node: 需要增加注释的节点.
返回:
    成功时返回属性列表;如指定节点无属性时返回NULL
*********************************************************************/

void XMLFreeAttrList( struct XMLAttr **attrlist );
/*********************************************************************
说明:释放节点的属性列表.
参数:
    attrlist: 节点列表.
返回:
    无
*********************************************************************/


#endif /* __SDK_XML_H__ */

