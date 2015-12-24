#include "app.h"
#include "tcop.h"
#include "comm.h"
#include "dbutil.h"
#include <stdlib.h>

#include "tcpapi.h"

int appsMain(xmlDoc **doc, char *reqbuf, char *rspbuf, int *plen)
{
    xmlDoc *opDoc = NULL;
    int reqlen = *plen;
    int ret = E_OTHER;

    if (OP_APPTYPE == APPTYPE_UNDEF && isOutTran())
    {
        //无相应OP平台应用类型的提出交易默认为转发交易
        INFO("开始转发交易[%d]...", OP_TCTCODE);
        return TransferToPH(reqbuf, G_REQFILE, rspbuf, G_RSPFILE, plen);
    }

    if ((ret = OpenOPDB()) != 0)
        exit(-1);

    if (OP_DOINIT && (ret = OP_DoInit(reqbuf, plen)) != 0) 
        goto EXIT;

    switch (OP_APPTYPE) 
    {
        case APPTYPE_LOCAL_SERVER:
        case APPTYPE_OPBANK_SERVER:
        case APPTYPE_OUTTRANS_SERVER:
        case APPTYPE_OPBANK_AUTOSVR:
            opDoc = ConvertREQ2OP(reqbuf, G_REQFILE, plen);
            break;
        case APPTYPE_REGION_SERVER:
            if ((ret = opRegionTrans(reqbuf, rspbuf, plen)) != 0)
                INFO("地区应用类型的交易[%d]处理失败,ret[%d]", OP_TCTCODE, ret);
            if (ret < 0)
                ret = E_OTHER;
            INFO("region_server ret=[%d].", ret);
            return ret;
        default:
            INFO("默认不处理的提入应用[%d]", OP_TCTCODE);
            ret = opUndefInTrans(reqbuf, rspbuf, plen);
            return ret;
    }

    if (opDoc == NULL) 
    {
        ret = E_PACK_CONVERT;
        goto EXIT;
    }

    *doc = opDoc;

    if ((ret = OPInitOPTran(opDoc)) != 0)
        goto EXIT;

    //加押核押处理
    if ((ret = DigestHandle(reqbuf, plen)) != 0)
        goto EXIT;

    switch (OP_APPTYPE)
    {
        case APPTYPE_OPBANK_AUTOSVR:
            if ((ret = callInterface(OP_BKTCODE, opDoc)) == 0)
                ret = OPAfterCommToPH(opDoc);
            break;
        case APPTYPE_OPBANK_SERVER:
            ret = opAppMain(opDoc);
            break;
        case APPTYPE_OUTTRANS_SERVER:
            ret = outAppMain(opDoc, reqbuf, rspbuf, plen);
            break;
        case APPTYPE_LOCAL_SERVER:
            //本地应用为平台预定义的处理程序(如:录入时不送行内的情况)
            break;
    }

EXIT:
    if ((*plen = ConvertOP2RSP(opDoc, reqbuf, reqlen, rspbuf, G_RSPFILE)) < 0)
        return E_PACK_CONVERT;

    return ret;
}

//获取前置通讯节点(需要根据tcp.ini转换)
int GetPHCommNode(char *reqnode)
{
    FILE *fp = NULL;
    char line[256] = {0};
    char phIP[32] = {0};
    const char *p = NULL;
    int phnode = -2;

    if ((fp = fopen(vstrcat("%s/etc/tcp.ini", getenv("HOME")), "r")) == NULL)
        return -1;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        sdpStringTrim(line);

        //注:针对平台的前置通讯节点(4位)的配置需放在针对中心的前置通讯节点(3位)配置后面
        if (phIP[0] != 0) {
            if ((p = strstr(line, phIP)) != NULL)
            {
                phnode = atoi(line);
                break;
            }
        } else if (line[3] == '=' && memcmp(line, reqnode, 3) == 0) {
            p = strchr(line, ':');
            memcpy(phIP, line + 4, p - line - 4);
        }
        memset(line, 0, sizeof(line));
    }

    fclose(fp);

    return 0;
}

int initApp(char *reqnode, char *reqtcode, char *reqbuf)
{
    xmlDoc *doc = NULL;
    xmlNode *pNode = NULL;
    xmlNodeSet *pNodeSet = NULL;
    char xpath[256] = {0};
    char *p = NULL;

    p = getenv("TCOP_BANKID");
    returnIfNull(p, E_PACK_INIT);
    TCOP_BANKID = atoi(p);

    OP_TCTCODE = atoi(reqtcode);

    //根据发送节点和TCOP_BANKID查找对应OPNODE
    if (access(p = vstrcat("%s/conf/main.xml", OP_HOME), 0) != 0)
        returnIfNullLoger(NULL, E_PACK_INIT, "未找到应用配置文件:%s", p);

    doc = xmlRecoverFile(p);
    returnIfNull(doc, E_PACK_INIT);

    pNode = XMLGetNode(doc, vstrcat("//BANK[@ID='%d']/NODE[@COMMNODE='%s']", TCOP_BANKID, reqnode));
    returnIfNullLoger(pNode, E_PACK_INIT, "未找到相应通讯节点的配置 reqnode:[%s]BANK id=[%d]", reqnode, TCOP_BANKID);

    if ((p = sdpXmlNodeGetAttrText(pNode, "OPNODE")) != NULL)
        OP_NODEID = atoi(p);

    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_TC")) != NULL)
        OP_TCPACKTYPE = *p;
    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_BK")) != NULL)
        OP_BKPACKTYPE = *p;

    pNode = XMLGetNode(doc, vstrcat("//BANK[@ID='%d']/NODE[@OPNODE='%d']", TCOP_BANKID, OP_PHNODEID));
    returnIfNullLoger(pNode, E_PACK_INIT, "未找到相应通讯节点的配置 PHNODE:[%d]BANK id=[%d]", OP_PHNODEID, TCOP_BANKID);
    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_TC")) != NULL)
        OP_PHPACKTYPE = *p;

    xmlFreeDoc(doc);

    //处理应用配置文件
    if (access((p = vstrcat("%s/conf/%d/appcfg.xml", OP_HOME, OP_BANKNODE)), 0) != 0)
        returnIfNullLoger(NULL, E_PACK_INIT, "未找到应用配置文件:%d|%d", OP_BANKNODE, TCOP_BANKID);

    doc = xmlRecoverFile(p);
    returnIfNull(doc, E_PACK_INIT);

    pNode = XMLGetNode(doc, vstrcat("//NODE[@OPNODE='%d']", OP_NODEID));
    returnIfNullLoger(pNode, E_PACK_INIT, "未找到相应平台节点的配置 OPNODE:[%d]", OP_NODEID);

    sprintf(xpath, "//NODE[@OPNODE='%d']/TRAN[@TCTCODE='%s']", OP_NODEID, reqtcode);

    if ((pNodeSet = sdpXmlSelectNodes(doc, xpath)) != NULL)
    {
        if (pNodeSet->nodeNr > 1)
            pNode = requestDispatcher(pNodeSet, reqbuf);
        else
            pNode = pNodeSet->nodeTab[0];

        returnIfNullLoger(pNode, E_PACK_INIT, "未找到平台节点下相应交易配置 XPATH:[%s]", xpath);

        if ((p = sdpXmlNodeGetAttrText(pNode, "OPTCODE")) != NULL)
            OP_OPTCODE = atoi(p);

        if ((p = sdpXmlNodeGetAttrText(pNode, "BKTCODE")) != NULL)
            OP_BKTCODE = atoi(p);

        if ((p = sdpXmlNodeGetAttrText(pNode, "APPTYPE")) != NULL)
            OP_APPTYPE = *p;

        if ((p = sdpXmlNodeGetAttrText(pNode, "PK")) != NULL)
            OP_APPPK = *p;

        if ((p = sdpXmlNodeGetAttrText(pNode, "DESC")) != NULL)
            strcpy(OP_APPNAME, ConvertOutput(p, OPPACK_ENCODEING));

        if ((p = sdpXmlNodeGetAttrText(pNode, "DOINIT")) != NULL)
            OP_DOINIT = atoi(p);

        if ((p = sdpXmlNodeGetAttrText(pNode, "EXTEND")) != NULL)
            OP_APPEXTEND = atoi(p);

        if ((p = sdpXmlNodeGetAttrText(pNode, "COMCODE")) != NULL)
            OP_COMCODE = atoi(p);

        if ((p = sdpXmlNodeGetAttrText(pNode, "ENV")) != NULL)
            putenv(p);
    }

    INFO("NODEID=%d TCCODE=%d OPCODE=%d APPTYPE=%c TCPKTYPE=%c BKPKTYPE=%c BKTCODE=%d EXT=%d INIT=%d COMTR=%d (%s)",
            OP_NODEID, OP_TCTCODE, OP_OPTCODE, OP_APPTYPE, OP_TCPACKTYPE, 
            OP_BKPACKTYPE, OP_BKTCODE, OP_APPEXTEND, OP_DOINIT, OP_COMCODE, OP_APPNAME);

    xmlFreeDoc(doc);

    return 0;
}

xmlNodePtr requestDispatcher(xmlNodeSet *ns, char *reqbuf)
{
    xmlDoc *doc = NULL;
    xmlNodePtr pNode = NULL;
    char expression[128] = {0};
    char *p = NULL;
    char *p1 = NULL;
    int i = 0;

    if ((p = sdpXmlNodeGetAttrText(ns->nodeTab[0], "DISPATCH")) == NULL)
        return NULL;

    if (*p == '/') //支持XPATH表达式
    {
        if ((doc = xmlRecoverDoc(reqbuf)) == NULL)
            return NULL;
    } else return NULL; //扩展定长报文表达式

    for (i = 0; i < ns->nodeNr; i++)
    {
        if ((p = sdpXmlNodeGetAttrText(ns->nodeTab[i], "DISPATCH")) == NULL)
            break;

        strcpy(expression, p);
        if ((p = strchr(expression, '=')) == NULL)
            break;

        *p = 0;
        if ((p1 = XMLGetNodeVal(doc, expression)) == NULL)
            break;

        if (strcmp(p1, p+1) == 0)
        {
            pNode = ns->nodeTab[i];
            INFO("根据DISPATCH=[%s]进行判断完成", expression);
            break;
        }
    }

    xmlFreeDoc(doc);

    return pNode;
}
