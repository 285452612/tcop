#include "cz_const.h"
#include "sdp.h"
#include "SDKxml.h"

ExchgInfo G_ExchgInfos[10] = {0};
TransInfo G_TransInfos[40] = {0};

int initSysConfig(char *commcode, int inoutflag)
{
    xmlDoc *doc = NULL;
    xmlNodeSetPtr nodeSet = NULL; 
    xmlNodePtr nodePtr = NULL;
    char path[256] = {0};
    const char *p = NULL;
    int i = 0;
    
    sprintf(path, "%s/etc/cztc.xml", getenv("HOME"));
    if ((doc = xmlRecoverFile(path)) == NULL) {
        INFO("加载地区系统配置文件[%s]失败", path);
        return -1;
    }

    memset(&G_ExchgInfos, 0, sizeof(G_ExchgInfos));
    memset(&G_TransInfos, 0, sizeof(G_TransInfos));

    if ((nodeSet = XMLGetNodeSet(doc, "//ExchgNo")) == NULL)
        return -2;
    for (i = 0; i < nodeSet->nodeNr; i++) {
        nodePtr = nodeSet->nodeTab[i];
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "ID")) == NULL)
            return -2;
        strcpy(G_ExchgInfos[i].exchgNo, p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "ADDRESS")) == NULL)
            return -3;
        strcpy(G_ExchgInfos[i].prehostAddr, p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "PORT")) == NULL)
            return -4;
        G_ExchgInfos[i].prehostPort = atoi(p);
    }

    if (inoutflag == 1) {
        sprintf(path, "//Tran[@COMMCODE='%d' and @IOFLAG='1']", atoi(commcode));
        if ((nodePtr = XMLGetNode(doc, path)) == NULL)
            return -5;
        G_TransInfos[0].commcode = atoi(commcode);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "TCTCODE")) == NULL)
            return -6;
        strcpy(G_TransInfos[0].tctcode, p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "ExchgNoPos")) == NULL)
            return -7;
        G_TransInfos[0].exchgnoPos = atoi(p);
        return 0;
    }

    if ((nodeSet = XMLGetNodeSet(doc, "//Tran[@IOFLAG='2']")) == NULL)
        return -8;
    for (i = 0; i < nodeSet->nodeNr; i++) {
        nodePtr = nodeSet->nodeTab[i];
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "TCTCODE")) == NULL)
            return -9;
        strcpy(G_TransInfos[i].tctcode, p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "TimePeriod")) == NULL)
            return -10;
        G_TransInfos[0].timePeriod = atoi(p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "ExchgNoPos")) != NULL)
            G_TransInfos[0].exchgnoPos = atoi(p);
        if ((p = sdpXmlNodeGetAttrText(nodePtr, "COMMCODE")) == NULL)
            return -12;
        G_TransInfos[i].commcode = atoi(p);
    }

    return 0;
}
