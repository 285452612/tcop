#include "element.h"
#include "interface.h"

static xmlDoc *DICTDOC = NULL;

xmlDoc *initDictDoc()
{
    char xpath[256] = {0};

    if (DICTDOC == NULL)
    {
        sprintf(xpath, "%s/conf/%d/dict%d.xml", OP_HOME, OP_BANKNODE, OP_REGIONID);
        if (access(xpath, 0) != 0)
            sprintf(xpath, "%s/conf/%d/dict.xml", OP_HOME, OP_BANKNODE);
        if ((DICTDOC = xmlRecoverFile(xpath)) == NULL)
            INFO("初始化字典文件[%s]失败", xpath);
    }
    return DICTDOC;
}

int SetNodeValueFromBuff(xmlNodePtr pnode, char *buf)
{
    xmlNodePtr dictNode = NULL;
    char tmp[2048] = {0};
    const char *p = NULL;
    char xpath[64] = {0};
    int fmtlen = 0;

    sprintf(xpath, "//%s", pnode->name);
    if ((dictNode = sdpXmlSelectNode(DICTDOC, xpath)) == NULL) {
        DBUG("字典中未找到指定节点[%s]的定义,忽略", xpath);
        return 0;
    }

    p = sdpXmlNodeGetAttrText(dictNode, "FMT");
    returnIfNullLoger(p, -2, "字典中指定节点[%s]未定义[FMT]属性", xpath);

    //不定长:如果FMT中定义的该元素的长度是0则表示该元素取buf中的最后所有长度
    if ((fmtlen = atoi(p + 2)) == 0)
        fmtlen = strlen(buf);

    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, buf, fmtlen);

    DBUG("定长转节点:%20s=%d[%s]", pnode->name, fmtlen, tmp);

    xmlNodeSetContent(pnode, ConvertInput(sdpStringTrim(tmp), OPPACK_ENCODEING));

    return fmtlen;
}

char *ProcessTranNodeValue(xmlNodePtr pnode, const char *value)
{
    return ProcessDictNodeValue(pnode->name, (char *)value, NULL);
}

char *ProcessDictNodeValue(const char *dictname, char *value, int *plen)
{
    xmlNodePtr pnode = NULL;
    char xpath[128] = {0};

    sprintf(xpath, "//%s", dictname);
    if ((pnode = sdpXmlSelectNode(DICTDOC, xpath)) == NULL)
        return value;
    return ProcessFormatNodeValue(pnode, value, plen);
}

char *ProcessFormatNodeValue(xmlNodePtr pnode, const char *value, int *plen)
{
    const char *pFMT = NULL;
    const char *pVALUE = NULL;
    const char *pHFUNC = NULL;
    const char *pDESC = NULL;

    returnIfNull(pnode, NULL);

    if ((pDESC = sdpXmlNodeGetAttrText(pnode, "DESC")) != NULL)
        pDESC = ConvertOutput(pDESC, OPPACK_ENCODEING);
    pFMT = sdpXmlNodeGetAttrText(pnode, "FMT");
    if ((pVALUE = sdpXmlNodeGetAttrText(pnode, "VALUE")) != NULL)
        pVALUE = ConvertOutput(pVALUE, OPPACK_ENCODEING);
    pHFUNC = sdpXmlNodeGetAttrText(pnode, "HFUNC");

    return ProcessNodeValueBase(value, plen, pFMT, pVALUE, pHFUNC, pDESC);
}

char *ProcessNodeValueBase(const char *value, int *plen, const char *pFMT, 
        const char *pVALUE, const char *pHFUNC, const char *pDESC)
{
    static char dst[1024] = {0};
    char tmp[512] = {0};
    int ret = 0;

    memset(dst, 0, sizeof(dst));
    //处理默认值
    if ((value == NULL || value[0] == 0) && pVALUE != NULL)
        strcpy(dst, pVALUE);
    else if (value != NULL)
        strcpy(dst, value);

    //处理动态调用
    if (pHFUNC != NULL) 
    {
        memset(tmp, 0, sizeof(tmp));
        if (strcmp(pHFUNC, "OPFgetCurrentDate") == 0)
            strcpy(tmp, getDate(0));
        else if (strcmp(pHFUNC, "OPFgetCurrentTime") == 0)
            strcpy(tmp, getTime(0));
        else if (strcmp(pHFUNC, "OPFconvertAmount") == 0)
            strcpy(tmp, convertAmount(dst));
        else if (strcmp(pHFUNC, "OPFconvertDate") == 0)
            strcpy(tmp, convertDate(dst));
        else if (strcmp(pHFUNC, "OPFconvertTime") == 0)
            strcpy(tmp, convertTime(dst));
        else 
        {
            callLibraryFunction(&ret, pHFUNC, dst, tmp); 
            if (ret != 0)
                return NULL;
        }
        strcpy(dst, tmp);
    }

    //处理格式补齐
    if (pFMT != NULL)
    {
        char moflag = pFMT[0];
        char conflag = pFMT[1];
        int itemlen = atoi(pFMT+2);
        char pader = pFMT[strlen(pFMT) - 1];
        char padway = pFMT[strlen(pFMT) - 2];

        if (pader == 'L' || pader == 'R') {
            //支持填充字符串结束符'\0'
            padway = pader;
            pader = 0; 
        }
        if (padway != 'L' && padway != 'R')
            pader = padway = 0;

        switch (conflag)
        {
            case '=':
                if (moflag == 'M' && strlen(dst) != itemlen)
                {
                    if (strlen(dst) < itemlen) {
                        strcpy(dst, sdpStringPad(dst, itemlen, padway, pader));
                        if (pader == 0)
                            *plen = itemlen;
                    } else 
                        dst[itemlen] = 0;
                }
                break;

            case '-':
                if (moflag == 'M') {
                    if (dst[0] == 0 || strlen(dst) > itemlen)
                        return NULL;
                } else if (moflag == 'O') {
                }
                break;

            case '+':
                break;
        }
        //DBUG("%20s:[%s] conflag:%c moflag:%c itemlen:%-3d padway:%c pader:%c", pDESC, dst, conflag, moflag, itemlen, padway, pader);
        DBUG("FMT[%8s]%20s:[%s]", pFMT, pDESC, dst);
    }

EXIT:
    return dst;
}
