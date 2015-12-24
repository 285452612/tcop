#include "interface.h"
#include "app.h"
#include "pack.h"

char OP_TCPACKTYPE = 'U';
char OP_PHPACKTYPE = 'U';
char OP_BKPACKTYPE = 'U';

/* 获取平台交易报文doc */
xmlDoc *getOPTemplateDoc(int optcode)
{
    xmlDoc *bDoc = NULL;
    xmlDoc *hDoc = NULL;
    xmlDoc *tDoc = NULL;
    xmlDoc *eDoc = NULL;
    xmlNodePtr pNode = NULL;

    if ((bDoc = getTemplateDocBase(PACK_OPBODY, 0, optcode/100*100, 0, NULL)) == NULL)
        return NULL;

    hDoc = xmlRecoverFile(vstrcat("%s/conf/ophead.xml", OP_HOME));
    returnIfNull(hDoc, NULL);

    tDoc = xmlRecoverFile(vstrcat("%s/conf/optail.xml", OP_HOME));
    returnIfNull(tDoc, NULL);

    xmlReplaceNode(XMLGetNode(bDoc, "//HEAD"), DOCROOT(hDoc));
    xmlReplaceNode(XMLGetNode(bDoc, "//TAIL"), DOCROOT(tDoc));

    if ((pNode = sdpXmlSelectNode(bDoc, "//EXTRA")) != NULL)
    {
        eDoc = xmlRecoverFile(vstrcat("%s/conf/opext%d.xml", OP_HOME, OP_REGIONID));
        returnIfNull(eDoc, NULL);
        sdpXmlNodeAddChildren(pNode, sdpXmlSelectNodes(eDoc, vstrcat("//EXTRA/*[@ID='%d' or not (@ID)]", TCOP_BANKID)));
    }

    return bDoc;
}

xmlDoc *getTemplateDocBase(char packattr, int bktcode, int optcode, int tctcode, char *xpath)
{
    xmlDoc *doc = NULL;
    char pbase[256] = {0};
    char path1[256] = {0};
    char path2[256] = {0};
    char path3[256] = {0};
    char path4[256] = {0};
    int optbase = optcode/100*100;
    char *p = NULL;

    sprintf(pbase, "%s/conf", OP_HOME);
    switch (packattr)
    {
        case PACK_MAPOP2RSP:
        case PACK_MAPTCXML2OP:
            sprintf(path1, "%s/%d/map%d-%d.xml", pbase, OP_PHNODEID, TCOP_BANKID, optcode);  //找指定区域指定银行指定交易映射文件
            sprintf(path2, "%s/%d/map%d-%d.xml", pbase, OP_PHNODEID, TCOP_BANKID, optbase);  //找指定区域指定银行指定交易公共映射文件
            sprintf(path3, "%s/%d/map%d.xml", pbase, OP_PHNODEID, optcode); //找指定区域指定交易映射文件
            sprintf(path4, "%s/%d/map%d.xml", pbase, OP_PHNODEID, optbase); //找指定区域指定交易公共映射文件
            break;
        case PACK_MAPOP2TCXML:
            sprintf(path1, "%s/%d/map%d-%04d.xml", pbase, OP_PHNODEID, TCOP_BANKID, tctcode);  //找指定区域指定银行指定同城交易映射文件
            sprintf(path2, "%s/%d/map%04d.xml", pbase, OP_PHNODEID, tctcode); //找指定区域指定同城交易映射文件
            break;  
        case PACK_MAPOP2BANK:
        case PACK_MAPBANK2OP:
            sprintf(path1, "%s/%d/map%d-%d.xml", pbase, TCOP_BANKID, OP_REGIONID, bktcode); //找指定银行指定区域指定交易映射文件
            sprintf(path2, "%s/%d/map%d.xml", pbase, TCOP_BANKID, bktcode); //找指定银行指定交易映射文件(不区分区域)
            break;
        case PACK_OPBODY:
            sprintf(path1, "%s/op%d-%d.xml", pbase, OP_BANKNODE, optcode); //找指定银行节点指定平台交易报文
            sprintf(path2, "%s/op%d-%d.xml", pbase, OP_BANKNODE, optbase); //找指定银行节点指定平台交易公共报文
            sprintf(path3, "%s/op%d.xml", pbase, optcode); //找平台指定交易报文
            sprintf(path4, "%s/op%d.xml", pbase, optbase); //找平台指定交易公共报文
            break;
        case PACK_BANKREQ:
        case PACK_BANKRSP:
            sprintf(path1, "%s/%d/bk%d-%d.xml", pbase, TCOP_BANKID, OP_REGIONID, bktcode);  //找指定银行指定地区指定交易报文
            sprintf(path2, "%s/%d/bk%d.xml", pbase, TCOP_BANKID, bktcode);  //找指定银行指定交易报文
            break;
        case PACK_OP2RSP:
            sprintf(path1, "%s/%d/tc%d-%d.xml", pbase, OP_PHNODEID, TCOP_BANKID, optcode); //找指定区域指定银行指定交易应答报文
            sprintf(path2, "%s/%d/tc%d-%d.xml", pbase, OP_PHNODEID, TCOP_BANKID, optbase); //找指定区域指定银行指定交易公共应答报文
            sprintf(path3, "%s/%d/tc%d.xml", pbase, OP_PHNODEID, optcode);  //找指定区域指定交易应答报文
            sprintf(path4, "%s/%d/tc%d.xml", pbase, OP_PHNODEID, optbase);  //找指定区域指定交易公共应答报文
            break;
        case PACK_REQSTRUCTXML:
        case PACK_RSPSTRUCTXML:
        case PACK_TCPACK:
            sprintf(path1, "%s/%d/tc%d-%04d.xml", pbase, OP_PHNODEID, TCOP_BANKID, tctcode); //找指定节点指定同城交易报文
            sprintf(path2, "%s/%d/tc%04d.xml", pbase, OP_PHNODEID, tctcode);//找指定地区指定同城交易报文
            break;
    }

    if (path1[0] != 0 && access(path1, 0) == 0) p = path1;
    else if (path2[0] != 0 && access(path2, 0) == 0) p = path2;
    else if (path3[0] != 0 && access(path3, 0) == 0) p = path3;
    else if (path4[0] != 0 && access(path4, 0) == 0) p = path4;
    else 
        INFO("无法定位报文配置文件 [%c|%d]:P1[%s]P2[%s]P3[%s]P4[%s]", 
                packattr, bktcode, path1, path2, path3, path4);

    if(*p!=NULL)INFO("使用配置文件 [%c|%d]:path[%s]",packattr, bktcode, p);

    doc = xmlRecoverFile(p);
    returnIfNullLoger(doc, NULL, "加载报文XML文件[%s]失败", p);

    switch (packattr)
    {
        case PACK_MAPOP2RSP: 
            INFO("初始化OP报文到同城应答XML报文的转换规则:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_MAPOP2BANK: 
            INFO("初始化OP报文到接口请求报文的转换规则:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_MAPBANK2OP: 
            INFO("初始化接口应答报文到OP报文的转换规则:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_MAPOP2TCXML:
            INFO("初始化OP报文到同城报文的转换规则:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_MAPTCXML2OP: 
            INFO("初始化同城XML报文到OP报文的转换规则:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_BANKREQ: 
            INFO("加载接口交易[%d]请求报文完成:%s", bktcode, basename(p)); break;
        case PACK_BANKRSP: 
            INFO("加载接口交易[%d]应答报文完成:%s", bktcode, basename(p)); break;
        case PACK_OPBODY: 
            INFO("加载平台交易报文模板完成:%s", basename(p)); break;
        case PACK_OP2RSP:
            doc = XMLDumpNodeAsDoc(XMLGetNode(doc, "//OUTPUT/*[1]")); 
            INFO("加载同城XML应答报文完成:%s", basename(p)); break;
        case PACK_TCPACK:
            doc = XMLDumpNodeAsDoc(XMLGetNode(doc, xpath));
            INFO("加载同城XML报文完成:%s xpath:[%s]", basename(p), xpath); break;
        case PACK_REQSTRUCTXML:
        case PACK_RSPSTRUCTXML:
            doc = XMLDumpNodeAsDoc(XMLGetNode(doc, xpath)); 
            INFO("加载同城结构报文对应的XML报文完成:%s xpath:[%s]", basename(p), xpath); break;
    }

    returnIfNull(doc, NULL);

    return doc;
}

xmlDoc *ConvertREQ2OP(char *reqbuf, char *reqfile, int *plen)
{
    xmlDoc *opDoc = NULL;

    if ((opDoc = getOPDoc()) == NULL)
        return NULL;

    opDoc = ConvertTCPack2OP(opDoc, reqbuf, plen, reqfile, "//INPUT/*");
    SavePack(opDoc, PACK_REQ2OP, 0);

    return opDoc;
}

xmlDoc *ConvertREQ2TCXML(char *reqbuf, char *reqfile, int *plen)
{
    xmlDoc *doc = NULL;

    INFO("开始转换请求报文到同城XML报文");

    switch (OP_TCPACKTYPE)
    {
        case PACKTYPE_XML:
            doc = xmlRecoverDoc(reqbuf);
            break;

        case PACKTYPE_STRUCT:
            if ((doc = getTCTemplateDoc(OP_TCTCODE, "//INPUT")) == NULL)
                return NULL;
            if ((ConvertStruct2Nodes(sdpXmlSelectNodes(doc, "//INPUT/*"), reqbuf)) != 0)
                return NULL;
            SavePack(doc, PACK_REQSTRUCTXML, 0);
            break;
    }
    return doc;
}

xmlDoc *ConvertPHRsp2OP(xmlDoc *opDoc, char *rspbuf, char *rspfile)
{
    xmlDoc *doc = NULL;
    xmlDoc *mapDoc = NULL;
    unsigned char *docbuf = NULL;
    int len = 0;

    if (rspfile != NULL && rspfile[0] != 0)
        XMLSetNodeVal(opDoc, "//opFilenames", rspfile);

    INFO("开始转换前置应答报文到OP报文");

    switch (OP_PHPACKTYPE)
    {
        case PACKTYPE_XML:
            doc = xmlRecoverDoc(rspbuf);
            returnIfNull(doc, NULL);
            return ConvertTCXML2OP(opDoc, doc, "//PHRSP/*");

        case PACKTYPE_STRUCT:
            if ((doc = getTCTemplateDoc(OP_TCTCODE, "//OUTPUT/*[1]")) == NULL)
                return NULL;
            if ((ConvertStruct2Nodes(sdpXmlSelectNodes(doc, "//*"), rspbuf)) != 0)
                return NULL;
            SavePack(doc, PACK_RSPSTRUCTXML, 0);
            if (ConvertTCXML2OP(opDoc, doc, "//PHRSP/*") != NULL) {
                if (OP_TCPACKTYPE == PACKTYPE_XML) {
                    xmlDocDumpMemory(doc, &docbuf, &len);
                    memcpy(rspbuf, docbuf, len);
                }
                return opDoc;
            } 
            INFO("转换TCXML到OP失败");

    }
    return NULL;
}

char *ConvertOP2REQ(xmlDoc *opDoc, char *reqbuf, char *reqfile, int *plen)
{
    xmlDoc *doc = NULL;
    xmlDoc *mapDoc = NULL;
    xmlNodeSetPtr mapNSPtr = NULL;
    unsigned char *docbuf = NULL;
    char *p = NULL;

    /* 保留:用于process中产生要传输的文件名并设置到请求报文中
    p = XMLGetNodeVal(opDoc, "//opFilenames");
    if (p != NULL && *p != 0 && reqfile != NULL)
        strcpy(reqfile, p);
        */

    INFO("开始转换OP报文到请求报文");

    switch (OP_TCPACKTYPE)
    {
        case PACKTYPE_XML:
            mapDoc = getTemplateDoc(PACK_MAPTCXML2OP, 0, "//OP2REQ");
            returnIfNull(mapDoc, NULL);
            mapNSPtr = sdpXmlSelectNodes(mapDoc, "//OP2REQ/*");
            if (reqbuf == NULL || mapNSPtr == NULL || mapNSPtr->nodeNr == 0)
                return reqbuf;
            doc = xmlRecoverDoc(reqbuf);
            returnIfNull(doc, NULL);
            if (ConvertXML2XML(doc, opDoc, mapNSPtr, PACKMAPNODE_SRC, "//", NULL) != NULL) {
                xmlDocDumpMemory(doc, &docbuf, plen);
                memcpy(reqbuf, docbuf, *plen);
                return reqbuf;
            }
            break;

        case PACKTYPE_STRUCT:
            break;
    }

    return NULL;
}

int ConvertOP2TCStruct(char *buf, int *plen, int tctcode, xmlDoc *opDoc)
{
    xmlDoc *doc = NULL;
    xmlDoc *mapDoc = NULL;
    char mapXPath[256] = {0};
    int ret = 0;

    INFO("开始转换OP报文到同城结构报文");

    if ((doc = getTCTemplateDoc(tctcode, "/PACK/INPUT")) == NULL)
        return E_PACK_INIT;

    sprintf(mapXPath, "//INPUT/*[@CODE='%d' or not(@CODE)]", tctcode);

    if ((mapDoc = getTemplateDocBase(PACK_MAPOP2TCXML, 0, OP_OPTCODE, tctcode, mapXPath)) == NULL)
        return E_PACK_INIT;

    ConvertXML2XML(doc, opDoc, sdpXmlSelectNodes(mapDoc, mapXPath), 
            PACKMAPNODE_DST, "//INPUT//", ProcessTranNodeValue);

    SavePack(doc, PACK_MAPOP2TCXML, 0);

    if ((*plen = ConvertNodes2Struct(buf, sdpXmlSelectNodes(doc, "//INPUT/*"))) < 0)
        return E_PACK_CONVERT;

    return 0;
}

int ConvertOP2BANK(char *buf, int bktcode, xmlDoc *opDoc)
{
    xmlDoc *reqDoc = NULL;
    char *p = NULL;
    int len = 0;

    if ((reqDoc = getTemplateDoc(PACK_BANKREQ, bktcode, NULL)) == NULL)
        return -1;

    if (ConvertOP2BankXML(bktcode, reqDoc, opDoc) == NULL)
        return -2;

    SavePack(reqDoc, PACK_BANKREQ, bktcode);

    switch (OP_BKPACKTYPE) 
    {
        case PACKTYPE_STRUCT:
            return ConvertNodes2Struct(buf, sdpXmlSelectNodes(reqDoc, "/PACK/INPUT/*"));

        case PACKTYPE_XML:
            if ((p = sdpXmlNodeDump2Str(sdpXmlSelectNode(reqDoc, "/PACK/INPUT"), 1)) == NULL)
                return -3;

            sdpStringTrim(p);
            len = strlen(p) - strlen("<INPUT>") - strlen("</INPUT>");

            return sprintf(buf, "%s%*.*s", sdpXmlDocGetDeclare(reqDoc), len, len, p + strlen("<INPUT>"));
            // added by chenjie
        default:
            {
                int packlen;
                callLibraryFunction(&packlen, "ConvertToBankPack", sdpXmlSelectNodes(reqDoc, "/PACK/INPUT/*"), buf); 
                if (packlen <= 0)
                    INFO("ConvertToBankPack fail.");
                return packlen;
            }
            // end.
    }
}

xmlDoc *ConvertBANK2OP(xmlDoc *opDoc, int bktcode, char *buf, int responseFlag)
{
    xmlDoc *rspDoc = NULL;
    char xpath[128] = {0};
    int ret = 0;

    sprintf(xpath, "//RESPONSE[@FLAG='%d']/*", responseFlag);

    switch (OP_BKPACKTYPE) 
    {
        case PACKTYPE_STRUCT:
            if ((rspDoc = getTemplateDoc(PACK_BANKRSP, bktcode, NULL)) == NULL)
                return NULL;
            if ((ret = ConvertStruct2Nodes(sdpXmlSelectNodes(rspDoc, xpath), buf)) != 0)
            {
                INFO("转换结构到XML失败!ret=[%d]", ret);
                return NULL;
            }
            SavePack(rspDoc, PACK_BANKRSP, bktcode);
            break;
        case PACKTYPE_XML:
            if ((rspDoc = xmlRecoverDoc(buf)) == NULL)
                return NULL;
            SavePack(rspDoc, PACK_BANKXMLRSP, bktcode);
            break;
        case 'D':
            if ((rspDoc = xmlRecoverDoc(buf)) == NULL)
                return NULL;
            xmlReplaceNode(DOCROOT(opDoc), DOCROOT(rspDoc));
            xmlSaveFile("final.xml", opDoc);
            return opDoc;
            // added by chenjie
        default:
            if ((rspDoc = getTemplateDoc(PACK_BANKRSP, bktcode, NULL)) == NULL)
                return NULL;
            callLibraryFunction(&ret, "ConvertToOPPack", sdpXmlSelectNodes(rspDoc, xpath), buf); 
            if (ret != 0)
            {
                INFO("ConvertToOPPack() fail.");
                return NULL;
            }
            SavePack(rspDoc, PACK_BANKRSP, bktcode);
            break;
            // end.
    }
    INFO("转换接口交易[%d]应答到XML报文完成!xpath:[%s]", bktcode, xpath);

    return ConvertBankXML2OP(opDoc, bktcode, rspDoc);
}

int ConvertOP2RSP(xmlDoc *opDoc, char *reqbuf, int reqlen, char *rspbuf, char *rspfile)
{
    xmlDoc *rspDoc = NULL;
    int rspLen = 0;
    unsigned char *docbuf = NULL;
    const char *p = NULL;

    if (opDoc != NULL)
        p = XMLGetNodeVal(opDoc, "//opFilenames");
    if (p != NULL && p[0] != 0)
        strcpy(rspfile, p);

    switch (OP_TCPACKTYPE)
    {
        case PACKTYPE_XML:
            if (OP_APPTYPE == APPTYPE_OPBANK_SERVER || 
                    OP_APPTYPE == APPTYPE_LOCAL_SERVER ||
                    OP_APPTYPE == APPTYPE_OPBANK_AUTOSVR)
                rspDoc = getTemplateDoc(PACK_OP2RSP, 0, NULL);
            else if (OP_APPTYPE == APPTYPE_OUTTRANS_SERVER)
            {
                if (rspbuf[0] == 0)
                    rspDoc = getTemplateDoc(PACK_OP2RSP, 0, NULL);
                else
                    rspDoc = xmlRecoverDoc(rspbuf);
            }
            returnIfNull(rspDoc, -1);
            break;

        case PACKTYPE_STRUCT:
            memcpy(rspbuf, reqbuf, reqlen);
            rspDoc = getTemplateDoc(PACK_RSPSTRUCTXML, 0, "//OUTPUT");
            returnIfNull(rspDoc, -1);
            break;
    }

    if (opDoc != NULL) {
        rspDoc = ConvertOP2TcRspXML(rspDoc, opDoc);
        returnIfNull(rspDoc, -1);
    }
    SavePack(rspDoc, PACK_OP2RSP, 0);

    switch (OP_TCPACKTYPE)
    {
        case PACKTYPE_XML:
            xmlDocDumpMemory(rspDoc, &docbuf, &rspLen);
            memcpy(rspbuf, docbuf, rspLen);
            break;

        case PACKTYPE_STRUCT:
            rspLen = ConvertNodes2Struct(rspbuf, sdpXmlSelectNodes(rspDoc, "//OUTPUT/*"));
            break;
    }

    INFO("[%d][%s]", rspLen, rspbuf);
    return rspLen;
}

xmlDoc *ConvertOP2TcRspXML(xmlDoc *tcDoc, xmlDoc *opDoc)
{
    xmlDoc *mapDoc = NULL;
    char mapXPath[256] = "//OUTPUT/*";

    if ((mapDoc = getTemplateDoc(PACK_MAPOP2RSP, 0, mapXPath)) == NULL)
        return NULL;

    return ConvertXML2XML(tcDoc, opDoc, sdpXmlSelectNodes(mapDoc, mapXPath), PACKMAPNODE_SRC, "//", NULL);
}

xmlDoc *ConvertTCPack2OP(xmlDoc *opDoc, char *tcbuf, int *plen, char *tcfile, char *mapNSXPath)
{
    xmlDoc *tcDoc = NULL;

    if (tcfile != NULL && tcfile[0] != 0)
        XMLSetNodeVal(opDoc, "//opFilenames", tcfile);

    switch (OP_TCPACKTYPE)
    {
        case PACKTYPE_XML:
            tcDoc = xmlRecoverDoc(tcbuf);
            returnIfNull(tcDoc, NULL);
            break;

        case PACKTYPE_STRUCT:
            //初始化同城doc
            if ((tcDoc = getTemplateDoc(PACK_REQSTRUCTXML, 0, "//INPUT")) == NULL)
                return NULL;

            //给同城doc中节点赋值
            if ((ConvertStruct2Nodes(sdpXmlSelectNodes(tcDoc, "//INPUT/*"), tcbuf)) != 0)
                return NULL;
            SavePack(tcDoc, PACK_REQSTRUCTXML, 0);
            break;

        default:
            INFO("未找到对应的报文类型!OP_TCPACKTYPE=[%c]", OP_TCPACKTYPE);
            return NULL;
    }

    return ConvertTCXML2OP(opDoc, tcDoc, mapNSXPath);
}

xmlDoc *ConvertOP2BankXML(int bktcode, xmlDoc *bkDoc, xmlDoc *opDoc)
{
    xmlDoc *mapDoc = NULL;
    char mapXPath[256] = {0};

    sprintf(mapXPath, "//INPUT/*[@CODE='%d' or not(@CODE)]", OP_OPTCODE);

    if ((mapDoc = getTemplateDoc(PACK_MAPOP2BANK, bktcode, mapXPath)) == NULL)
        return NULL;

    return ConvertXML2XML(bkDoc, opDoc, sdpXmlSelectNodes(mapDoc, mapXPath), 
            PACKMAPNODE_DST, "//INPUT//", ProcessTranNodeValue);
}

xmlDoc *ConvertBankXML2OP(xmlDoc *opDoc, int bktcode, xmlDoc *bkDoc)
{
    xmlDoc *mapDoc = NULL;
    char mapXPath[256] = {0};

    sprintf(mapXPath, "//OUTPUT/*[@CODE='%d' or not(@CODE)]", OP_OPTCODE);

    if ((mapDoc = getTemplateDoc(PACK_MAPBANK2OP, bktcode, mapXPath)) == NULL)
        return NULL;

    return ConvertXML2XML(opDoc, bkDoc, sdpXmlSelectNodes(mapDoc, mapXPath), 
            PACKMAPNODE_DST, "//", ProcessTranNodeValue);
}

xmlDoc *ConvertTCXML2OP(xmlDoc *opDoc, xmlDoc *tcDoc, char *mapNSXPath)
{
    xmlNodeSetPtr mapNSPtr = NULL;
    static xmlDoc *mapDoc = NULL;

    if (mapDoc == NULL && ((mapDoc = getTemplateDoc(PACK_MAPTCXML2OP, 0, mapNSXPath)) == NULL))
        return NULL;

    mapNSPtr = sdpXmlSelectNodes(mapDoc, mapNSXPath);

    return ConvertXML2XML(opDoc, tcDoc, mapNSPtr, PACKMAPNODE_DST, "//", ProcessTranNodeValue);
}

xmlDoc *ConvertXML2XML(xmlDoc *dstDoc, xmlDoc *srcDoc, xmlNodeSetPtr mapNSPtr, 
        char mapWay, char *ppath, FPProcessNodeValue fp)
{
    xmlNodePtr pNode = NULL;
    xmlNodePtr tmpNodePtr = NULL;
    char tmp[128] = {0};
    char xpath[128] = {0};
    char *prule = NULL;
    char *pvalue = NULL;
    char *ptmp = NULL;
    char *ptype = NULL;
    char *srcPath = NULL, *dstPath = NULL;
    int i = 0;

    INFO("开始转换XML报文到XML报文:mapWay=[%c]ppath=[%s]", mapWay, ppath);

    if (mapNSPtr == NULL) //无需转换
        return dstDoc;

    for (i = 0; i < mapNSPtr->nodeNr; ++i) 
    {
        pNode = mapNSPtr->nodeTab[i];
        prule = sdpXmlNodeGetAttrText(pNode, "RULE");
        ptype = sdpXmlNodeGetAttrText(pNode, "TYPE");

        if (prule[0] == 0)
            continue;
        sprintf(xpath, "%s%s", ppath, pNode->name);

        srcPath = (mapWay == PACKMAPNODE_DST ? prule : xpath);
        dstPath = (mapWay == PACKMAPNODE_DST ? xpath : prule);

        //对于源报文中找不到映射节点的字段忽略到目标节点值的映射
        if (ptype != NULL && *ptype == DB_COLTYPE_XML)
        {
            DBUG("map=[%20s->%-20s] IS XML FIELD!", srcPath, dstPath);
            tmpNodePtr = XMLGetNode(srcDoc, srcPath);
            if ((pNode = sdpXmlSelectNode(srcDoc, vstrcat("%s/*[1]", xmlGetNodePath(tmpNodePtr)))) != NULL)
                xmlReplaceNode(XMLGetNode(dstDoc, dstPath), pNode);
            continue;
        } else if ((pvalue = sdpXmlSelectNodeText(srcDoc, srcPath)) == NULL)
            continue;

        //pvalue = sdpStringTrim(pvalue); //modify by zxq 0608
        if (ptype == NULL && fp != NULL)
        {   
            ptmp = fp(pNode, pvalue);
            returnIfNullLoger(ptmp, NULL, "节点[%s]值[%s]处理失败", pNode->name, pvalue);
            DBUG("map=[%20s->%-20s]value=[%s->%s]", srcPath, dstPath, pvalue, ptmp);
            pvalue = ptmp;
        } else {
            DBUG("map=[%20s]value=[%s]", dstPath, pvalue);
        }

        if (dstPath[0] =='/' && dstPath[1] != '/')
            sdpXmlNodeNew(dstDoc, dstPath, pvalue);
        else
            sdpXmlNodeSetText(dstDoc, dstPath, pvalue);
    }

    return dstDoc;
}

int ConvertStruct2Nodes(xmlNodeSetPtr nsPtr, const char *buf)
{
    int pos = 0;
    int i = 0;

    returnIfNull(nsPtr, E_PACK_CONVERT);
    INFO("开始转换定长报文到XML节点集[%d]...", nsPtr->nodeNr);

    if (initDictDoc() == NULL)
        return  E_PACK_INIT;

    for (i = 0; i < nsPtr->nodeNr; ++i) 
    {
        if ((pos += SetNodeValueFromBuff(nsPtr->nodeTab[i], (char *)(buf+pos))) < pos)
            return E_PACK_CONVERT;
    }
    return 0;
}

int ConvertNodes2Struct(char *buf, xmlNodeSetPtr nsPtr)
{
    char *p = NULL;
    int pos = 0;
    int len = 0;
    int i = 0;

    INFO("开始转换XML节点集到定长报文...");

    returnIfNull(nsPtr, -1);
    if (initDictDoc() == NULL)
        return -2;

    for (i = 0; i < nsPtr->nodeNr; ++i)
    {
        p = ConvertOutput(xmlNodeGetContent(nsPtr->nodeTab[i]), OPPACK_ENCODEING);
        if ((p = ProcessDictNodeValue(nsPtr->nodeTab[i]->name, p, &len)) == NULL)
            return -3;
        memcpy(buf + pos, p, len ? len : strlen(p));
        pos += (len ? len : strlen(p));
    }
    return pos;
}

void SavePack(xmlDoc *doc, char savepackType, int bktcode)
{
    char path[256] = {0};
    char tmp[128] = {0};
    unsigned char *pbuf = NULL;
    xmlDoc *tmpDoc = NULL;
    FILE *fp = NULL;
    int len = 0;

    switch (savepackType)
    {
        case PACK_REQ2OP:
            sprintf(tmp, "req2op%d", OP_OPTCODE);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 0);
            break;
        case PACK_BANKREQ:
            sprintf(tmp, "bk%d", bktcode);
            tmpDoc = NEWROOTDOC();
            xmlReplaceNode(DOCROOT(tmpDoc), xmlCopyNode(XMLGetNode(doc, "//INPUT"), 1));
            xmlDocDumpFormatMemory(tmpDoc, &pbuf, &len, 1);
            break;
        case PACK_BANKRSP:
            sprintf(tmp, "bk%d", bktcode);
            tmpDoc = NEWROOTDOC();
            xmlReplaceNode(DOCROOT(tmpDoc), xmlCopyNode(XMLGetNode(doc, "//OUTPUT"), 1));
            xmlDocDumpFormatMemory(tmpDoc, &pbuf, &len, 1);
            break;
        case PACK_BANKXMLRSP:
            sprintf(tmp, "bk%d", bktcode);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 1);
            break;
        case PACK_OP2RSP:
            sprintf(tmp, "oprsp%d", OP_OPTCODE);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 0);
            break;
        case PACK_RSP2OP:
            sprintf(tmp, "rsp2op%d", OP_OPTCODE);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 0);
            break;
        case PACK_REQSTRUCTXML:
            sprintf(tmp, "structreq%d", OP_OPTCODE);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 0);
            break;
        case PACK_RSPSTRUCTXML:
            sprintf(tmp, "structrsp%d", OP_OPTCODE);
            xmlDocDumpFormatMemory(doc, &pbuf, &len, 0);
            break;
        case PACK_MAPOP2TCXML:
            sprintf(tmp, "op2tc%d", OP_TCTCODE);
            pbuf = sdpXmlNodeDump2Str(DOCROOT(doc), 0);
            break;
    }

    if (pbuf == NULL)
    {
        INFO("保存报文时转换doc到memory失败!");
        return ;
    }   

    sprintf(path, "%s/data/%s-%s.xml", OP_HOME, tmp, getDate(0));

    if ((fp = fopen(path, "a")) == NULL)
    {
        INFO("保存报文[%s]失败!%s", path, OSERRMSG);
        return;
    }
    fprintf(fp, "%s %ld------------------------------------------------------\n%s", getTime(0), getpid(), pbuf);
    fclose(fp);

    INFO("保存报文[%s]成功", path);
}
