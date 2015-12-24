#include "pub.h"

char *sdpStringTrimHeadChar(char *src, char trim)
{
    register char *pos;

    if (src == NULL)
        return src;

    for (pos = src; *pos != '\0' && trim == *pos; pos++);

    if (pos > src)
        memmove(src, pos, strlen(pos) + 1);

    return src;
}

char *sdpStringTrimTailChar(char *src, char trim)
{
    int len;

    if (src == NULL)
        return src;

    len = strlen(src);

    while (len > 0 && src[len - 1] == trim)
        src[--len] = '\0';

    return src;
}

char *sdpStringTrimHead(char *src)
{
    register char *pos;

    if (src == NULL)
        return src;

    for (pos = src; *pos != '\0' && isspace(*pos); pos++);

    if (pos > src)
        memmove(src, pos, strlen(pos) + 1);

    return src;
}

char *sdpStringTrimTail(char *src)
{
    int len;

    if (src == NULL)
        return src;

    len = strlen(src);

    while (len > 0 && isspace(src[len - 1]))
        src[--len] = '\0';

    return src;
}

char *sdpStringTrim(char *src)
{
    return sdpStringTrimHead(sdpStringTrimTail(src));
}

int sdpMemcpy(void *dst, int count, void *src1, int src1Len, ...)
{
    va_list ap;
    int len = src1Len;
    int tmpLen = 0;
    void *p = NULL;
    int i = 0;

    va_start(ap, src1Len);
    memcpy(dst, src1, src1Len);
    for (i = 1; i < count; i++) {
        p = va_arg(ap, void *);
        tmpLen = va_arg(ap, int);
        memcpy(dst+len, p, tmpLen);
        len += tmpLen;
    }
    va_end(ap);

    return len;
}

char *sdpStringPad(const char *src, int len, char padway, char padder)
{
    static char buf[1024] = {0};

    memset(buf, padder, len);
    if (padway == 'L')
        memcpy(buf + len - strlen(src), src, strlen(src));
    else if (padway = 'R')
        memcpy(buf, src, strlen(src));

    buf[len] = 0;

    return buf;
}

int sdpStringSplit(char *line, char *words[], int maxwords, int delim)
{
    char *p = line, *p2;
    int nwords = 0;

    while (*p != '\0')
    {
        words[nwords++] = p;
        if (nwords >= maxwords)
            return nwords;

        p2 = strchr(p, delim);
        if (p2 == NULL)
            break;
        *p2 = '\0';
        p = p2 + 1;
        if (*p == 0) 
            words[nwords++] = p;
    }

    return nwords;
}

int sdpStringIsAllChar(char *str, char c)
{
    char *p = str;

    while (*p++ == c);
    return *--p == 0;
}

int sdpFileLinesForeach(const char *pfile, int breakFlag, int (*pfunc)(char *line, char *reserved), char *reserved)
{
    FILE *fp = NULL;
    char line[2048] = {0};
    int ret = 0;

    if ((fp = fopen(pfile, "r")) == NULL)
        return -1;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        sdpStringTrim(line);
        if (pfunc(line, reserved) && breakFlag)
        {
            ret = -1;
            break;
        }
        memset(line, 0, sizeof(line));
    }

    fclose(fp);
    return ret;
}

char *sdpXmlDocGetDeclare(xmlDoc *doc)
{
    static char buf[256] = {0};

    sprintf(buf, "<?xml version='1.0' encoding='%s'?>", doc->encoding);
    return buf;
}

char *sdpXmlNodeDump2Str(xmlNode *node, int convertFlag)
{
    xmlBufferPtr p = NULL;

    returnIfNull(node, NULL);

    p = xmlBufferCreate();
    xmlKeepBlanksDefault(0);
    if (xmlNodeDump(p, node->doc, node, 1, 0) < 0)
    {
        INFO("sdpXmlNodeDump2Str error!nodename=[%s]", node->name);
        return NULL;
    }
    if (convertFlag)
        return ConvertOutput(p->content, DOCENC(node->doc));

    return p->content;
}

char *sdpXmlNodeGetAttrText(xmlNode *pNode, char *attrName)
{
    xmlAttr *pAttr = pNode->properties;

    while (pAttr != NULL)
    {
        if (strcmp(pAttr->name, attrName) == 0) 
            return xmlNodeGetContent(pAttr->children);
        pAttr = pAttr->next;
    }
    /*
    if ((pAttr = xmlHasProp(pNode, attrName)) != NULL)
        return pAttr->children->content;
        */

    return NULL;
}

xmlNodeSetPtr sdpXmlSelectNodes(xmlDoc *doc, const char *xpath)
{
    xmlXPathContextPtr contextPtr = NULL; 
    xmlXPathObjectPtr xpathObj = NULL;   
    xmlNodeSetPtr nsPtr = NULL; 

    if (NULL != (contextPtr = xmlXPathNewContext(doc))) 
    {
        if (NULL != (xpathObj = xmlXPathEvalExpression(xpath, contextPtr))) 
        {
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
                nsPtr = xpathObj->nodesetval;
        }       
        xmlXPathFreeContext(contextPtr);
    }

    return nsPtr;
}

xmlNodePtr sdpXmlSelectNode(xmlDoc *doc, const char *xpath)
{
    xmlNodeSetPtr nsPtr = sdpXmlSelectNodes(doc, xpath); 

    return NULL == nsPtr ? NULL : nsPtr->nodeTab[0];
}

char *sdpXmlSelectNodeText(xmlDoc *doc, const char *xpath)
{
    xmlNodeSetPtr nsPtr = NULL;

    //存在多个符合条件节点默认返回第一个节点的值
    if ((nsPtr = sdpXmlSelectNodes(doc, xpath)) == NULL)
        return NULL;

    //判断是否为空节点
    if (nsPtr->nodeTab[0]->children == NULL)
        return strdup("");

    //判断节点是否为叶子节点
    if (!xmlNodeIsText(nsPtr->nodeTab[0]->children))
    {
        INFO("非叶子节点 xpath:[%s]", xpath);
        return NULL;
    }

    return (char*)ConvertOutput(xmlNodeGetContent(nsPtr->nodeTab[0]), DOCENC(doc));
}

int sdpXmlNodeSetText(xmlDoc *doc, const char *xpath, const char *value)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    xmlNodeSetPtr tmpNSPtr = NULL;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(ConvertInput(xpath, DOCENC(doc)), context);
    xmlXPathFreeContext(context);

    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        INFO("指定记录不存在 xpath:[%s]", xpath);
        return -1;
    }

    tmpNSPtr = result->nodesetval;

    if (tmpNSPtr->nodeNr != 1)
    {
        INFO("存在多条记录 xpath:[%s]", xpath);
        return -2;
    }

    /* 判断节点是否为叶子节点.当节点的child为TEXT Node或无child时表示该
       节点为叶子节点.原先考虑使用xmlIsBlankNode()函数判断节点是否为空节点,
       但实际应用发现该函数存在问题.根据Xpath的规则: All the whitespace
       between the elements is treated as whitespace text nodes in XPath. 
    */
    if (xmlGetLastChild(tmpNSPtr->nodeTab[0]) == NULL || xmlNodeIsText(tmpNSPtr->nodeTab[0]->children))
    {
        xmlNodeSetContent(tmpNSPtr->nodeTab[0], ConvertInput(value, DOCENC(doc)));
        return 0;
    } 
    INFO("非叶子节点 xpath:[%s]", xpath);

    return -3;
}

void sdpXmlNodeAddChildren(xmlNodePtr parent, xmlNodeSetPtr nsPtr)
{
    int i = 0;

    if (nsPtr != NULL)
    {
        for (i = 0; i < nsPtr->nodeNr; i++)
            xmlAddChild(parent, nsPtr->nodeTab[i]);
    }
}

void sdpXmlNodeNew(xmlDoc *doc, const char *xpath, const char *value)
{       
    char *p = NULL;
    char tmp[256] = {0};

    strcpy(tmp, xpath);
    if ((p = strrchr(tmp, '/')) == NULL)
        return;

    *p = 0;
    //XMLAppendNode(doc, tmp, xmlNewDocNode(doc, NULL, p+1, value));
    XMLAppendNode(doc, tmp, xmlNewDocNode(doc, NULL, p+1, ConvertInput(value, DOCENC(doc))));
} 

void sdpDebugXmlDoc(xmlDoc *doc)
{
    int len = 0;
    unsigned char *pbuf = NULL;

    xmlDocDumpFormatMemory(doc, &pbuf, &len, 1);
    DBUG("DOC[%s]", pbuf);
}
