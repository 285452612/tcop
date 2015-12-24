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
        //����ӦOPƽ̨Ӧ�����͵��������Ĭ��Ϊת������
        INFO("��ʼת������[%d]...", OP_TCTCODE);
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
                INFO("����Ӧ�����͵Ľ���[%d]����ʧ��,ret[%d]", OP_TCTCODE, ret);
            if (ret < 0)
                ret = E_OTHER;
            INFO("region_server ret=[%d].", ret);
            return ret;
        default:
            INFO("Ĭ�ϲ����������Ӧ��[%d]", OP_TCTCODE);
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

    //��Ѻ��Ѻ����
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
            //����Ӧ��Ϊƽ̨Ԥ����Ĵ������(��:¼��ʱ�������ڵ����)
            break;
    }

EXIT:
    if ((*plen = ConvertOP2RSP(opDoc, reqbuf, reqlen, rspbuf, G_RSPFILE)) < 0)
        return E_PACK_CONVERT;

    return ret;
}

//��ȡǰ��ͨѶ�ڵ�(��Ҫ����tcp.iniת��)
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

        //ע:���ƽ̨��ǰ��ͨѶ�ڵ�(4λ)�����������������ĵ�ǰ��ͨѶ�ڵ�(3λ)���ú���
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

    //���ݷ��ͽڵ��TCOP_BANKID���Ҷ�ӦOPNODE
    if (access(p = vstrcat("%s/conf/main.xml", OP_HOME), 0) != 0)
        returnIfNullLoger(NULL, E_PACK_INIT, "δ�ҵ�Ӧ�������ļ�:%s", p);

    doc = xmlRecoverFile(p);
    returnIfNull(doc, E_PACK_INIT);

    pNode = XMLGetNode(doc, vstrcat("//BANK[@ID='%d']/NODE[@COMMNODE='%s']", TCOP_BANKID, reqnode));
    returnIfNullLoger(pNode, E_PACK_INIT, "δ�ҵ���ӦͨѶ�ڵ������ reqnode:[%s]BANK id=[%d]", reqnode, TCOP_BANKID);

    if ((p = sdpXmlNodeGetAttrText(pNode, "OPNODE")) != NULL)
        OP_NODEID = atoi(p);

    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_TC")) != NULL)
        OP_TCPACKTYPE = *p;
    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_BK")) != NULL)
        OP_BKPACKTYPE = *p;

    pNode = XMLGetNode(doc, vstrcat("//BANK[@ID='%d']/NODE[@OPNODE='%d']", TCOP_BANKID, OP_PHNODEID));
    returnIfNullLoger(pNode, E_PACK_INIT, "δ�ҵ���ӦͨѶ�ڵ������ PHNODE:[%d]BANK id=[%d]", OP_PHNODEID, TCOP_BANKID);
    if ((p = sdpXmlNodeGetAttrText(pNode, "PACKTYPE_TC")) != NULL)
        OP_PHPACKTYPE = *p;

    xmlFreeDoc(doc);

    //����Ӧ�������ļ�
    if (access((p = vstrcat("%s/conf/%d/appcfg.xml", OP_HOME, OP_BANKNODE)), 0) != 0)
        returnIfNullLoger(NULL, E_PACK_INIT, "δ�ҵ�Ӧ�������ļ�:%d|%d", OP_BANKNODE, TCOP_BANKID);

    doc = xmlRecoverFile(p);
    returnIfNull(doc, E_PACK_INIT);

    pNode = XMLGetNode(doc, vstrcat("//NODE[@OPNODE='%d']", OP_NODEID));
    returnIfNullLoger(pNode, E_PACK_INIT, "δ�ҵ���Ӧƽ̨�ڵ������ OPNODE:[%d]", OP_NODEID);

    sprintf(xpath, "//NODE[@OPNODE='%d']/TRAN[@TCTCODE='%s']", OP_NODEID, reqtcode);

    if ((pNodeSet = sdpXmlSelectNodes(doc, xpath)) != NULL)
    {
        if (pNodeSet->nodeNr > 1)
            pNode = requestDispatcher(pNodeSet, reqbuf);
        else
            pNode = pNodeSet->nodeTab[0];

        returnIfNullLoger(pNode, E_PACK_INIT, "δ�ҵ�ƽ̨�ڵ�����Ӧ�������� XPATH:[%s]", xpath);

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

    if (*p == '/') //֧��XPATH���ʽ
    {
        if ((doc = xmlRecoverDoc(reqbuf)) == NULL)
            return NULL;
    } else return NULL; //��չ�������ı��ʽ

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
            INFO("����DISPATCH=[%s]�����ж����", expression);
            break;
        }
    }

    xmlFreeDoc(doc);

    return pNode;
}
