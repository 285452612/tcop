#include "dbutil.h"
#include "element.h"
#include "interface.h"

static char GSQLBUF[SQLBUFF_MAX] = {0};
static int ret = 0;

char *GetSysPara(char *paraname)
{
    static char dst[256] = {0};

    ret = db_query_str(dst, sizeof(dst), 
            "SELECT paraval FROM syspara WHERE nodeid=%d AND paraname='%s'", 
            OP_REGIONID, paraname);
    if (ret != 0)
        return NULL;

    return dst;
}

int UpdSysPara(char *paraname, char *paraval)
{
    return db_exec("UPDATE syspara SET paraval='%s' WHERE nodeid=%d AND paraname='%s'", 
            paraval, OP_REGIONID, paraname);
}

int IsRoundEnd(int round)
{
    char *p = GetRoundTrnend();

    if (p != NULL && strlen(p) >= round)
        return (p[round-1] == '1');
    return 0;
}

xmlNodePtr GetTBOperatorNode(xmlNodeSetPtr *pcolsNSPtr, char *tableName, char operator, int id)
{
    static xmlDoc *dbDoc = NULL;
    xmlNodePtr tbNode = NULL;
    xmlNodeSetPtr tbNodes = NULL;
    char xpath[256] = {0};
    char path[256] = {0};

    sprintf(path, "%s/conf/%d/db%d.xml", OP_HOME, OP_BANKNODE, OP_OPTBASE/100);
    if (access(path, 0) != 0)
        sprintf(path, "%s/conf/%d/db.xml", OP_HOME, OP_BANKNODE);

    dbDoc = xmlRecoverFile(path); 
    returnIfNullLoger(dbDoc, NULL, "初始化数据库操作配置文件[%s]失败", path);

    if (id == 0)
        sprintf(xpath, "(//%s[@%c and not(@ID)]", tableName, operator);
    else 
        sprintf(xpath, "(//%s[contains(@ID,'%d') and @%c]", tableName, id, operator);

    sprintf(xpath+strlen(xpath), "|//*[@ALIAS='%s'])", tableName);

    tbNodes = XMLGetNodeSet(dbDoc, xpath);
    returnIfNullLoger(tbNodes, NULL, "初始化数据库配置[%s]操作节点[%s]失败", path, xpath);
    if (tbNodes->nodeNr > 1) {
        strcat(xpath, vstrcat("[@TCID='%d']", OP_TCTCODE));
        tbNode = XMLGetNode(dbDoc, xpath);
        returnIfNullLoger(tbNode, NULL, "初始化数据库配置[%s]操作节点[%s]失败", path, xpath);
    } else
        tbNode = tbNodes->nodeTab[0];

    switch (operator)
    {
        case DB_OPERATOR_INSERT:
            strcat(xpath, "/*[@C or not(@R|@U)]"); break;
        case DB_OPERATOR_UPDATE:
            strcat(xpath, "/*[@U or not(@C|@R)]"); break;
        case DB_OPERATOR_QUERY:
            strcat(xpath, "/*[@R or not(@C|@U)]"); break;
    }
    DBUG("操作数据库配置文件[%s]节点集 xpath:%s", path, xpath);
    *pcolsNSPtr = XMLGetNodeSet(dbDoc, xpath);

    return tbNode;
}

int InsertTableByID(xmlDoc *doc, char *tableName, int id)
{
    xmlNodePtr tbNode = NULL;
    xmlNodePtr tmpNode = NULL;
    xmlNodePtr pNode = NULL;
    xmlNodeSetPtr colsNSPtr = NULL;
    char xpath[256] = {0};
    const char *p = NULL;
    int isPartCols = 1;     //默认是指定列插入
    int cols = 0;
    char coltype = 'U';
    int i = 0, len = 0;
    char tmp[2048]={0};

    if ((tbNode = GetTBOperatorNode(&colsNSPtr, tableName, DB_OPERATOR_INSERT, id)) == NULL)
        return E_PACK_GETVAL;

    if ((p = sdpXmlNodeGetAttrText(tbNode, "COLS")) != NULL && strcmp(p, "ALL") == 0)
       isPartCols = 0;

    if (isPartCols) 
    {
        len = sprintf(GSQLBUF, "INSERT INTO %s(", tableName);
        for (i = 0; i < colsNSPtr->nodeNr; ++i)
            len += sprintf(GSQLBUF + len, "%s,", colsNSPtr->nodeTab[i]->name);
        len--;
        len += sprintf(GSQLBUF + len, ") VALUES (");
    } else 
        len = sprintf(GSQLBUF, "INSERT INTO %s VALUES (", tableName);

    for (i = 0; i < colsNSPtr->nodeNr; ++i)
    {
        tmpNode = colsNSPtr->nodeTab[i];
        GetDBNodeAttrs(tmpNode, &coltype, xpath);

        if (*xpath == 0)
            continue;

        if (coltype == DB_COLTYPE_XML)
        {
            if ((pNode = XMLGetNode(doc, xpath)) == NULL)
                 len += sprintf(GSQLBUF + len, "'',");
            else
               len += sprintf(GSQLBUF + len, "'%s',", XMLDumpNodeAsStr(pNode));
           continue;
        }

        p = ProcessDBNodeValue(tmpNode, strlen(xpath) ? XMLGetNodeVal(doc, xpath) : NULL);
        returnIfNull(p, E_PACK_CFG);

        switch (coltype)
        {
            case DB_COLTYPE_STR:
            case DB_COLTYPE_CHAR:
                /*add by lt@sunyard.com 2012-8-27*/
                if( strchr(p, '\'') != NULL )
                {
                    db_esc_str(tmp, p);
                    p=tmp;
                }
                /*add end 2012-8-27*/
                len += sprintf(GSQLBUF + len, "'%s',", p); break;

            case DB_COLTYPE_INT:
                len += sprintf(GSQLBUF + len, "%s,", p); break;

            case DB_COLTYPE_DBL:
            case DB_COLTYPE_FLT:
                len += sprintf(GSQLBUF + len, "%.2lf,", atof(p)); break;

            default:
                INFO("数据库操作配置中未找到预定义的列类型!coltype=[%c]", coltype);
                return E_PACK_CFG;
        }
    }
    strcpy(GSQLBUF + len - 1, ")");

    return db_exec("%s", GSQLBUF);
}

int QueryTableByIDToXML(xmlDoc *doc, char *tableName, int id, char *where)
{
    xmlDoc *retDoc = NULL;
    xmlNodeSetPtr mapNSPtr = NULL;
    xmlNodeSetPtr recordNSPtr = NULL;
    char tmp[256] = {0};
    FILE *fp = NULL;

    DBUG("QueryTableByIDToXML WHERE:[%s]", where); 

    if ((ret = db_query_xml(&retDoc, "SELECT * FROM %s WHERE %s", tableName, where)) != 0) 
        return ret;

    if (GetTBOperatorNode(&mapNSPtr, tableName, DB_OPERATOR_QUERY, id) == NULL)
        return E_PACK_CFG;

    recordNSPtr = XMLGetNodeSet(retDoc, "//RECORD");

    if (recordNSPtr->nodeNr > 0)
    {
        if (getFilesdirFile(tmp) == NULL || (fp = fopen(tmp, "w")) == NULL)
            return E_SYS_CALL;
        xmlDocDump(fp, retDoc);
        XMLSetNodeVal(doc, "//opFilenames", basename(tmp));
        return 0;
    }

    return E_DB_SELECT;
}

int QueryTableByID(xmlDoc *doc, char *tableName, int id, char *where)
{
    xmlDoc *retDoc = NULL;
    xmlNodeSetPtr mapNSPtr = NULL;
    xmlNodeSetPtr recordNSPtr = NULL;
    xmlNodeSetPtr tmpNSPtr = NULL;
    xmlNodePtr tmpNode = NULL;
    const char *p = NULL;
    char xpath[128] = {0};
    int i = 0, j = 0;

    DBUG("QueryTableByID WHERE:[%s]", where);

    if ((ret = db_query_xml(&retDoc, "SELECT * FROM %s WHERE %s", tableName, where)) != 0) 
        return ret;

    if (GetTBOperatorNode(&mapNSPtr, tableName, DB_OPERATOR_QUERY, id) == NULL)
        return E_PACK_CFG;

    recordNSPtr = XMLGetNodeSet(retDoc, "//RECORD");

    for (i = 0; i < recordNSPtr->nodeNr; i++)  //记录循环处理 
    {
        for (j = 0; j < mapNSPtr->nodeNr; j++) //映射节点集循环处理
        {
            tmpNode = mapNSPtr->nodeTab[j];
            sprintf(xpath, "//RECORD[@rowid='%d']/%s", i+1, tmpNode->name);
            if ((p = sdpXmlNodeGetAttrText(tmpNode, "TYPE")) != NULL)
            {
                if (*p == DB_COLTYPE_XML)
                {
                    p = XMLGetNodeVal(retDoc, xpath);
                    if (sdpXmlSelectNodes(doc, vstrcat("%s/*", sdpXmlNodeGetAttrText(tmpNode, "RULE"))) == NULL)
                        XMLAppendNode(doc, sdpXmlNodeGetAttrText(tmpNode, "RULE"),
                                DOCROOT(xmlRecoverMemory(ConvertInput(p, OPPACK_ENCODEING), strlen(p))));
                    else 
                        xmlReplaceNode(XMLGetNode(doc, sdpXmlNodeGetAttrText(tmpNode, "RULE")), 
                                DOCROOT(xmlRecoverMemory(ConvertInput(p, OPPACK_ENCODEING), strlen(p))));
                }
            } else {
                p = ProcessDBNodeValue(tmpNode, XMLGetNodeVal(retDoc, xpath));
                returnIfNull(p, E_PACK_CONVERT);
                XMLSetNodeVal(doc, sdpXmlNodeGetAttrText(tmpNode, "RULE"), (char *)p);
            }
        }
        break; //暂时只支持单笔返回
    }

    return 0;
}

int QueryTableByIDToFile(const char *filename, char *tableName, int id, char *where)
{
    FILE *fp = NULL;
    xmlNodeSetPtr mapNSPtr = NULL;
    xmlNodePtr tbNode = NULL;
    char *sep = NULL;
    char *order = NULL;
    char *pWhere = NULL;
    const char *p = NULL;
    int isPartCols = 1;     //默认是指定列插入
    int len = 0, i = 0;

    if ((tbNode = GetTBOperatorNode(&mapNSPtr, tableName, DB_OPERATOR_QUERY, id)) == NULL)
        return E_PACK_GETVAL;

    if ((fp = fopen(filename, "w")) == NULL)
        return E_SYS_CALL;

    if ((sep = sdpXmlNodeGetAttrText(tbNode, "SEP")) == NULL)
        sep = "|";

    order = sdpXmlNodeGetAttrText(tbNode, "ORDER");
    pWhere = sdpXmlNodeGetAttrText(tbNode, "WHERE");

    if ((p = sdpXmlNodeGetAttrText(tbNode, "COLS")) != NULL && strcmp(p, "ALL") == 0)
       isPartCols = 0;

    if (isPartCols) 
    {
        len = sprintf(GSQLBUF, "SELECT ");
        for (i = 0; i < mapNSPtr->nodeNr; ++i)
            len += sprintf(GSQLBUF + len, "%s,", mapNSPtr->nodeTab[i]->name);
        len--;
        len += sprintf(GSQLBUF + len, " FROM %s", tableName);
    } else 
        len += sprintf(GSQLBUF, "SELECT * FROM %s", tableName);

    if (where != NULL && strlen(where)) {
        len += sprintf(GSQLBUF + len, " WHERE %s", where);
        if (pWhere != NULL && strlen(pWhere))
            len += sprintf(GSQLBUF + len, " AND %s", pWhere);
    } else if (pWhere != NULL && strlen(pWhere))
        len += sprintf(GSQLBUF + len, " WHERE %s", pWhere);

    if (order != NULL)
        len += sprintf(GSQLBUF + len, " ORDER BY %s", order);

    DBUG("QueryTableByIDToFile SQL:[%s]", GSQLBUF);

    ret = db_query_file(fp, sep, GSQLBUF);

    fclose(fp);

    return ret;
}

int UpdateTableByID(xmlDoc *doc, char *tableName, int id, char *where)
{
    xmlNodePtr tbNode = NULL;
    xmlNodePtr tmpNode = NULL;
    xmlNodeSetPtr colsNSPtr = NULL;
    char xpath[256] = {0};
    const char *p = NULL;
    char coltype = 'U';
    int i = 0, len = 0;

    if ((tbNode = GetTBOperatorNode(&colsNSPtr, tableName, DB_OPERATOR_UPDATE, id)) == NULL)
        return E_PACK_GETVAL;

    len = sprintf(GSQLBUF, "UPDATE %s SET ", tableName);

    for (i = 0; i < colsNSPtr->nodeNr; ++i)
    {
        tmpNode = colsNSPtr->nodeTab[i];
        GetDBNodeAttrs(tmpNode, &coltype, xpath);

        if (coltype == DB_COLTYPE_XML) 
        {
           p = XMLDumpNodeAsStr(XMLGetNode(doc, xpath));
           len += sprintf(GSQLBUF + len, "%s='%s',", tmpNode->name, p);
           continue;
        }

        if (xpath != NULL && xpath[0] != 0)
            p = ProcessDBNodeValue(tmpNode, XMLGetNodeVal(doc, xpath));
        else
            p = ProcessDBNodeValue(tmpNode, NULL);

        returnIfNull(p, E_PACK_CFG);

        switch (coltype)
        {
            case DB_COLTYPE_STR:
            case DB_COLTYPE_CHAR:
                len += sprintf(GSQLBUF + len, "%s='%s',", tmpNode->name, p); break;

            case DB_COLTYPE_INT:
                len += sprintf(GSQLBUF + len, "%s=%s,", tmpNode->name, p); break;

            case DB_COLTYPE_DBL:
            case DB_COLTYPE_FLT:
                len += sprintf(GSQLBUF + len, "%s=%.2lf,", tmpNode->name, atof(p)); break;

            default:
                INFO("数据库操作配置中未找到预定义的列类型!coltype=[%c]", coltype);
                return E_PACK_CFG;
        }
    }
    sprintf(GSQLBUF + len - 1, " WHERE %s", where);

    return db_exec(GSQLBUF);
}

int GetDBNodeAttrs(xmlNodePtr pDBNode, char *coltype, char *rule)
{
    char *p = NULL;

    p = sdpXmlNodeGetAttrText(pDBNode, "TYPE");
    *coltype = (p == NULL ? DB_COLTYPE_STR : *p);

    if ((p = sdpXmlNodeGetAttrText(pDBNode, "RULE")) != NULL)
        strcpy(rule, p);
    else
        *rule = 0;

    return 0;
}

