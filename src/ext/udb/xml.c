#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/globals.h>
#include <libxml/xmlschemas.h>

#ifndef _MIN
#define _MIN(a, b) ( (a)<(b) ? (a) : (b) )
#endif

#define ConvIn(in) (xmlChar *)encoding_conv((char *)in, "GB18030", "UTF-8")
#define ConvOut(in) (char *)encoding_conv((char *)in, "UTF-8", "GB18030")

#define UTF8_TO_GB18030(src, src_size, dest, dest_size) \
    Iconv(src, src_size, "UTF-8", dest, dest_size, "GB18030")
#define GB18030_TO_UTF8(src, src_size, dest, dest_size) \
    Iconv(src, src_size, "GB18030", dest, dest_size, "UTF-8")

static int CheckMultiBytes(const char *pSrc)
{
    int i;
    if (pSrc == NULL)
        return 0;
    for (i = 0; i < strlen(pSrc); i++)
        if (*(pSrc+i) & 0x80)
            return 1;
    return 0;
}

int Iconv(char *src, int src_size, const char *src_enc, 
        char *dest, int dest_size, const char *dest_enc)
{
    iconv_t cd;
    size_t inbytesleft = (size_t) src_size;
    size_t outbytesleft =(size_t) dest_size;

    memset(dest, 0, dest_size);
    cd = iconv_open(dest_enc, src_enc);
    if ( cd == (iconv_t)-1)
    {
        fprintf(stderr, "iconv_open(%s, %s) Fail.", dest_enc, src_enc);
        return -1;
    }

    if (iconv(cd, &src, &inbytesleft, &dest, &outbytesleft) == -1)
    {
        fprintf(stderr, "iconv(%s, %s) Fail.", dest_enc, src_enc);
        iconv_close(cd);
        return -1;
    }

    iconv_close(cd);

    return 0;
}

unsigned char *encoding_conv(char *bufin, const char *now, const char *enc)
{
    iconv_t cd;
    size_t inlen, outlen;
    char *bufout, *cur;
    char **pin = &bufin;
    char **pout;

    inlen = strlen( bufin );
    outlen = inlen * 2 + 1;

    bufout = (char *)malloc( outlen );
    if ( bufout == NULL )
        return NULL;
    memset( bufout, 0, outlen );

    // 如果当前编码与待转换编码相同，则不作转换
    if (!strcasecmp(enc, now))
    {
        strcpy(bufout, bufin);
        return bufout;
    }

    cd = iconv_open( enc, now );
    if ( cd == (iconv_t)-1)
    {
        fprintf(stderr, "iconv_open(%s, %s) Fail.", enc, now);
        free( bufout );
        return NULL;
    }

    cur = bufout;
    pout = (char **)&bufout;
    if (iconv(cd, (char **)pin, &inlen, pout, &outlen) == -1 )
    {
        fprintf(stderr, "iconv(%s, %s) Fail.", enc, now);
        free( cur );
        iconv_close(cd);
        return NULL;
    }

    iconv_close(cd);

    return (unsigned char *)cur;
}

/*********************************************
说明:   新建一个指定编码的XML文档           
参数:
        root: 根节点名称
        encoding: 编码名称, 如 UTF-8, GB18030, ISO-8859-1等
返回:   文档指针xmlDocPtr
*********************************************/
xmlDocPtr XmlNewDocEnc(char *root, const char *encoding)
{
    const char *title = "<?xml version=\"1.0\" encoding=\"";
    char *buf = NULL;
    xmlDocPtr doc = NULL;
    int len;

    len = strlen(title) + strlen(encoding) + strlen(root) + 7;
    if ((buf = malloc(len)) != NULL)
    {
        snprintf(buf, len, "%s%s\"?><%s/>", title, encoding, root);
        doc = xmlParseMemory(buf, len);
        free(buf);
    }
    return doc;
}


xmlNodePtr XmlNewTextChild(xmlNodePtr node, char *name, char *val)
{
    xmlNodePtr pNewNode = NULL;
    xmlChar *pEncVal = NULL;
    int flag = 0;

    if (val == NULL)
    {
        pNewNode = xmlNewTextChild(node, NULL, BAD_CAST name, NULL);
        return pNewNode;
    }
    if (CheckMultiBytes(val))
    {
        pEncVal = (xmlChar *)encoding_conv((char *)val, "GB18030", "UTF-8");
        flag = 1;
    }
    else
        pEncVal = BAD_CAST val;

    pNewNode = xmlNewTextChild(node, NULL, BAD_CAST name, pEncVal);
    if (flag)
        free(pEncVal);

    return pNewNode;
}

xmlXPathObjectPtr getnodeset(xmlDocPtr doc, const char *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result = NULL;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression((xmlChar *)xpath, context);
    if (result == NULL)
    {
        xmlXPathFreeContext(context);
        return NULL;
    }

    if(xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        xmlXPathFreeContext(context);
        return NULL;
    }

    xmlXPathFreeContext(context);
    return result;
}

char *XmlGetString(xmlDocPtr doc, const char *xpath, char *val, size_t size)
{
    xmlXPathObjectPtr result;
    char *content = NULL;

    memset(val, 0, size);
    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            content = (char *)xmlNodeListGetString( doc, 
                    result->nodesetval->nodeTab[0]->children, 1 );
            if ( content )
            {
                if (CheckMultiBytes(content))
                {
                    UTF8_TO_GB18030(content, strlen(content), val, size);
                }
                else
                {
                    strncpy(val, content, _MIN(size, strlen(content)));
                }
                free(content);
            }
        }
        xmlXPathFreeObject(result);
    }

    return val;
}


// 移动节点指针到指定名称的子节点
xmlNodePtr XmlFetchNode(xmlNodePtr pnNode, const char *xKey)
{
    xmlNodePtr pCurNode;
    pCurNode = pnNode->xmlChildrenNode;
    while(pCurNode != NULL) {
        if( !xmlStrcmp(pCurNode->name, BAD_CAST xKey) ) break;
        pCurNode = pCurNode->next;
    }
    return pCurNode;
}

/*********************************************************************
说明:按路径设置XML中指定节点的值.路径的格式如:/aaa/bbb[@cc="dd"]/ee
     所要设置的节点必须为叶子节点
参数:
      doc: XML文档指针
    xpath: 节点路径
      val: 节点的值
返回: 0 成功 -1 失败
*********************************************************************/
int XmlSetString(xmlDocPtr doc, const char *xpath, char *val)
{
    xmlNodePtr cur = NULL;
    xmlNodePtr node = NULL;
    char *ptr = NULL;
    char *token = NULL;
    xmlChar *pEncVal = NULL;
    int flag = 0;
    int rc = -1;

    if (val == NULL)
        return 0;
    if (CheckMultiBytes(val))
    {
        pEncVal = ConvIn(val);
        flag = 1;
    }
    else
        pEncVal = BAD_CAST val;

    if (xpath[0] != '/')
        goto func_handler;
    if ((ptr = strdup(xpath+1)) == NULL)
        goto func_handler;
    if ((node = xmlDocGetRootElement(doc)) == NULL)
        goto func_handler;

    // 允许xpath忽略根节点
    token = strsep(&ptr, "/");
    if (*token != 0x00 && strcmp(token, (char *)node->name))
        goto func_handler;

    while((token = strsep(&ptr, "/")) != NULL)
    {
        if ((cur = XmlFetchNode(node, token)) == NULL)
            node = xmlNewTextChild(node, NULL, BAD_CAST token, NULL);
        else
            node = cur;
    }
    xmlNodeSetContent(node, pEncVal);
    free(ptr);
    rc = 0;
func_handler:
    if (flag)
        free(pEncVal);
    return rc;
}

int XmlSetInteger(xmlDocPtr doc, const char *xpath, int val)
{
    char buf[12];
    sprintf(buf, "%d", val);

    return XmlSetString(doc, xpath, buf);
}

int XmlSetNodeString(xmlNodePtr node, const char *name, char *val)
{
    xmlNodePtr pCurNode;
    xmlChar *pEncVal = NULL;
    int flag = 0;

    if (val == NULL)
        return 0;
    if (CheckMultiBytes(val))
    {
        pEncVal = ConvIn(val);
        flag = 1;
    }
    else
        pEncVal = BAD_CAST val;

    pCurNode = node->children;
    while( pCurNode != NULL )
    {
        if (!strcasecmp((char*)pCurNode->name, name))
        {
            if (pCurNode->children->content == NULL)
                xmlNodeAddContent(pCurNode->children, pEncVal);
            else
                xmlNodeSetContent(pCurNode->children, pEncVal);
            break;
        }
        pCurNode = pCurNode->next;
    }
    if (pCurNode == NULL)
    {
        xmlNewTextChild(node, NULL, BAD_CAST name, pEncVal);
    }

    if (flag)
        free(pEncVal);

    return 0;
}

char *XmlGetStringDup( xmlDocPtr doc, const char *xpath)
{
    xmlXPathObjectPtr result;
    char *content = NULL;
    char *enc = NULL;

    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            content = (char *)xmlNodeListGetString( doc, 
                    result->nodesetval->nodeTab[0]->children, 1 );
            if ( content )
            {
                if (CheckMultiBytes(content))
                {
                    enc = ConvOut(content);
                    free(content);
                }
                else
                    enc = content;
            }
        }
        xmlXPathFreeObject(result);
    }

    if (enc == NULL)
        enc = strdup("");

    return enc;
}

int XmlGetInteger(xmlDocPtr doc, const char *xpath)
{
    xmlXPathObjectPtr result;
    char *content = NULL;
    int val = -1;

    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            content = (char *)xmlNodeListGetString( doc, 
                    result->nodesetval->nodeTab[0]->children, 1 );
            if ( content )
            {
                val = atoi(content);
                free(content); 
                content = NULL;
            }
        }
        xmlXPathFreeObject(result);
    }

    return val;
}

double XmlGetFloat(xmlDocPtr doc, const char *xpath)
{
    xmlXPathObjectPtr result;
    char *content = NULL;
    double val = (double)0;

    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            content = (char *)xmlNodeListGetString( doc, 
                    result->nodesetval->nodeTab[0]->children, 1 );
            if ( content )
            {
                val = atof(content);
                free(content);
                content = NULL;
            }
        }
        xmlXPathFreeObject(result);
    }

    return val;
}

char *XmlNodeGetContent(xmlNodePtr node)
{
    xmlChar *pContent = NULL;
    char *pEncVal = NULL;
    int flag = 0;

    if ((pContent = xmlNodeGetContent(node)) == NULL)
        return NULL;

    if (CheckMultiBytes((char *)pContent))
    {
        pEncVal = ConvOut(pContent);
        flag = 1;
    }
    else
        pEncVal = (char *)pContent;

    if (flag)
        xmlFree(pContent);

    return pEncVal;
}

char *XmlNodeDump(xmlNodePtr cur)
{
    xmlBufferPtr buf;
    char *p = NULL;

    if (cur == NULL)
        return NULL;

    buf = xmlBufferCreate();

    if (xmlNodeDump(buf, cur->doc, cur, 0, 0) < 0)
    {
        xmlBufferFree(buf);
        return NULL;
    }

    if (CheckMultiBytes((char *)buf->content))
        p = ConvOut(buf->content);
    else
        p = strdup((char *)buf->content);

    xmlBufferFree(buf);

    return p;
}

int XmlGetNodeString(xmlNodePtr node, const char *name, char *val, size_t size)
{
    xmlNodePtr pCurNode;
    char *p=NULL;
    int len = 0;
    int rc = -1;

    memset(val, 0, size);
    pCurNode = node->children;
    while ( pCurNode != NULL )
    {
        if ( !strcasecmp( ( char *)pCurNode->name, name ) )
        {
            if ( pCurNode->children != NULL)
            {
                /*
                p = (char *)xmlNodeGetContent(pCurNode);
                if (*p == 0)
                    break;
                free(p); p = NULL;
                */

                if (pCurNode->children->content != NULL)
                    p = XmlNodeGetContent(pCurNode->children);
                else
                    p = XmlNodeDump(pCurNode);
                if ( p != NULL )
                {
                    len = _MIN(strlen(p), size);
                    strncpy(val, p, len);
                    free( p ); p = NULL;
                    rc = 0;
                }
            }
            break;
        }
        pCurNode = pCurNode->next;
    }

    return rc;
}
int XmlGetNodeInteger(xmlNodePtr node, const char *name)
{
    xmlNodePtr pCurNode;
    xmlChar *p = NULL;
    int result = -1;

    pCurNode = node->children;
    while ( pCurNode != NULL )
    {
        if ( !strcasecmp( ( char *)pCurNode->name, name ) )
        {
            if ( pCurNode->children != NULL 
                    && pCurNode->children->content != NULL)
            {
                p = xmlNodeGetContent(pCurNode->children);
                result = atoi((char *)p);
                xmlFree(p);
            }
            break;
        }
        pCurNode = pCurNode->next;
    }

    return result;
}

// 替换新节点到doc指定位置
int XmlReplaceNode( xmlDocPtr doc, const char *xpath, xmlNodePtr cur )
{
    xmlXPathObjectPtr result;
    xmlNodePtr mynode;

    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            mynode = xmlCopyNode(cur, 1);
            xmlReplaceNode(result->nodesetval->nodeTab[0], mynode);
            xmlXPathFreeObject (result);
            return 0;
        }
        else
        {
            xmlXPathFreeObject (result);
            return -1;
        }
    }

    return -1;
}

/*********************************************************************
  说明:按指定的路径从XML doc中返回节点, 路径的格式如:/aaa/bbb[@cc="dd"]/ee
  参数:
doc: XML文档指针
xpath: 节点路径
返回:
在成功时返回节点指针,失败时返回NULL
 *********************************************************************/
xmlNodePtr XmlLocateNode(xmlDocPtr doc, const char *xpath)
{
    xmlXPathObjectPtr result;
    xmlNodePtr mynode = NULL;

    if ((result = getnodeset(doc, xpath)) == NULL)
        return NULL;

    mynode = result->nodesetval->nodeTab[0];

    xmlXPathFreeObject(result);
    return mynode;
}

xmlNodePtr XmlLocateNodeNew(xmlDocPtr doc, const char *xpath)
{
    xmlNodePtr cur = NULL;
    xmlNodePtr node = NULL;
    char *ptr = NULL;
    char *token = NULL;
    char *cp = NULL;

    if (xpath[0] != '/')
        return NULL;
    if ((ptr = strdup(xpath)) == NULL)
        return NULL;

    node = xmlDocGetRootElement(doc);
    token = strtok_r(ptr, "/", &cp);
    while((token = strtok_r(NULL, "/", &cp)) != NULL)
    {
        if ((cur = XmlFetchNode(node, token)) == NULL)
        {
            node = xmlNewTextChild(node, NULL, BAD_CAST token, NULL);
        }
        else
            node = cur;
    }

    free(ptr);
    return node;
}

char *XmlNodeGetAttrText(xmlNodePtr node, char *name)
{
    xmlAttrPtr attr = node->properties;
    xmlChar *pContent;
    char *enc;

    while (attr != NULL)
    {
        if (strcasecmp(attr->name, name) == 0)
        {
            pContent = xmlNodeGetContent(attr->children);
            if (CheckMultiBytes((char *)pContent))
            {
                enc = ConvOut(pContent);
                xmlFree(pContent);
                return enc;
            }
            else
                return (char *)pContent;
        }
        attr = attr->next;
    }
    return NULL;
}

int XmlSetVal( xmlDocPtr doc, const char *xpath, char *val )
{
    xmlXPathObjectPtr result;
    xmlChar *enc;
    int ret = -1;

    result = getnodeset(doc, xpath);
    if ( result )
    {
        if ( result->nodesetval->nodeNr == 1 )
        {
            enc = (xmlChar *)encoding_conv(val, "GB18030", "UTF-8");
            if ( enc == NULL )
            {
                if ((enc = strdup("")) == NULL)
                {
                    xmlXPathFreeObject (result);
                    return -1;
                }
            }
            if (result->nodesetval->nodeTab[0]->children != NULL)
                xmlNodeSetContent(result->nodesetval->nodeTab[0], enc);
            else
                xmlNodeAddContent(result->nodesetval->nodeTab[0], enc);
            free( enc );
            ret = 0;
        }
        xmlXPathFreeObject (result);
    }

    return ret;
}

