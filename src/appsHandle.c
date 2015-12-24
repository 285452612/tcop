#include "tcop.h"
#include "pack.h"
#include "comm.h"

int callProcess(xmlDoc *opDoc, const char *reserved)
{
    char funcname[128] = {0};
    int ret = 0;

    sprintf(funcname, "PF%d_%d", OP_REGIONID, OP_OPTCODE);

    callLibraryFunction(&ret, funcname, opDoc, reserved); 

    //动态库中未定义此处理函数默认返回成功(表示无需特殊业务处理)
    if (ret == E_SYS_NODLLFUNC)
        return 0;

    return ret;
}

int callInterface(int bktcode, xmlDoc *opDoc)
{
    BankCommData commdata = {0};
    int ret = 0;
    char workdate[9] = {0}; //for debug
    xmlChar *buf = NULL;

    strcpy(workdate, XMLGetNodeVal(opDoc, "//opWorkdate"));
    memset(&commdata, 0, sizeof(commdata));
    switch(OP_BKPACKTYPE)
    {
        case PACKTYPE_STRUCT:
        case PACKTYPE_XML:
            strcpy(workdate, XMLGetNodeVal(opDoc, "//opWorkdate"));
            //test
            //XMLSetNodeVal(opDoc, "//opWorkdate", "20110311");

            if ((commdata.bodylen = ConvertOP2BANK(commdata.body, bktcode, opDoc)) < 0)
            {
                INFO("转换OP报文到接口交易通讯请求报文失败!ret=[%d]", commdata.bodylen);
                return E_PACK_CONVERT;
            }
            break;
        default: // 'D'
            sprintf(commdata.head, "%d", bktcode);
            xmlDocDumpMemory(opDoc, &buf, &commdata.bodylen);
            if (commdata.bodylen <= 0 || buf == NULL)
            {
                INFO("DUMP OP报文出错");
                return E_PACK_CONVERT;
            }
            //INFO("commdata.bodylen[%d], commdata.body[%d]", commdata.bodylen, sizeof(commdata.body));
            if (commdata.bodylen > sizeof(commdata.body))
            {
                INFO("commdata.body 内存溢出");
                return E_PACK_CONVERT;
            }
            memset(commdata.body, 0, sizeof(commdata.body));
            memcpy(commdata.body, buf, commdata.bodylen);
            break;
    }
    DBUG("发送行内请求交易[%d]报文=>%d[%*.*s]", bktcode,
            commdata.bodylen, commdata.bodylen, commdata.bodylen, commdata.body);

    //加载通讯协议
    callLibraryFunction(&ret, "CommProtocol", &commdata); 
    if (ret < 0)
        return E_SYS_COMM_BANK;

    commdata.body[commdata.bodylen] = 0;

    XMLSetNodeVal(opDoc, "//opWorkdate", workdate);

    if (ConvertBANK2OP(opDoc, bktcode, commdata.body, ret) == NULL)
        INFO("转换接口交易通讯应答报文到OP报文失败,但当成功处理!ret=[%d]", E_PACK_CONVERT);

    return 0;
}

int outAppMain(xmlDoc *opDoc, char *reqbuf, char *rspbuf, int *plen)
{
    int ret = 0;

    if (OP_APPEXTEND != 0) {
        ret = callProcess(opDoc, COMMTOPH_BEFORE);
        if (!isSuccess(ret))
            return ret;
    }

    if (OP_PHPACKTYPE == PACKTYPE_STRUCT)
    {
        if ((ret = ConvertOP2TCStruct(reqbuf, plen, OP_TCTCODE, opDoc)) != 0)
            return ret;
    } else if (OP_PHPACKTYPE == PACKTYPE_XML) 
    {
        if (ConvertOP2REQ(opDoc, reqbuf, G_REQFILE, plen) == NULL)
            return E_PACK_CONVERT;
    }

    if ((ret = TransferToPH(reqbuf, G_REQFILE, rspbuf, G_RSPFILE, plen)) != 0)
        XMLSetNodeVal(opDoc, "//opTCRetcode", vstrcat("%d", E_SYS_COMM_PH));

    if (ret == 0 && ConvertPHRsp2OP(opDoc, rspbuf, G_RSPFILE) == NULL)
        return E_PACK_CONVERT;

    if ((ret = OPAfterCommToPH(opDoc)) != 0)
        return ret;

    if (OP_APPEXTEND != 0)
        ret = callProcess(opDoc, COMMTOPH_AFTER);

    return ret;
}

int opAppMain(xmlDoc *opDoc)
{
    int ret = 0;
    const char *p = NULL;

    //平台应用直接调用相应的银行相关的处理程序
    if ((ret = callProcess(opDoc, p)) != 0)
        return ret;

    return ret;
}

int opRegionTrans(char *reqbuf, char *rspbuf, int *plen)
{
    xmlDoc *reqDoc = NULL;
    xmlDoc *rspDoc = NULL;
    char funcname[128] = {0};
    unsigned char *docbuf = NULL;
    int ret = 0, i = 0;

    reqDoc = ConvertREQ2TCXML(reqbuf, G_REQFILE, plen);
    returnIfNull(reqDoc, E_PACK_INIT);

    rspDoc = getTemplateDoc(PACK_OP2RSP, 0, NULL);
    returnIfNull(rspDoc, E_PACK_INIT);

    sprintf(funcname, "PF_%04d", OP_TCTCODE);
    /*
       for (i = 0; i < 100; i++) {
       if (G_RPROCESS[i].tctcode == 0 || G_RPROCESS[i].pfunc == NULL)
       break;
       else if (G_RPROCESS[i].tctcode == OP_TCTCODE && G_RPROCESS[i].pfunc != NULL)
       strcpy(funcname, G_RPROCESS[i].pfunc);
       }
     */
    callRegionLibraryFunction(&ret, funcname, reqDoc, &rspDoc, G_RSPFILE);

    xmlDocDumpMemory(rspDoc, &docbuf, plen);
    memcpy(rspbuf, docbuf, *plen);

    return ret;
}

int opUndefInTrans(char *reqbuf, char *rspbuf, int *plen)
{
    xmlDoc *rspDoc = NULL;
    unsigned char *docbuf = NULL;

    rspDoc = getTemplateDoc(PACK_TCPACK, 0, "//OUTPUT/*");
    returnIfNull(rspDoc, E_PACK_INIT);

    xmlDocDumpMemory(rspDoc, &docbuf, plen);
    memcpy(rspbuf, docbuf, *plen);

    return 0;
}

int opException(xmlDoc *opDoc, char *rspbuf, int *rsplen, int operr)
{
    int ret = 0;
    char tmp[20] = {0};
    sprintf(tmp, "SetException_%d", OP_REGIONID);
    callLibraryFunction(&ret, tmp, opDoc, rspbuf, rsplen, operr); 
    return ret;
}

int OP_DoInit(char *reqbuf, int *reqlen)
{
    int ret = 0;
    callRegionLibraryFunction(&ret, "OP_DoInit", reqbuf, reqlen);
    return ret;
}

int DigestHandle(char *reqbuf, int *plen)
{
    unsigned char *docbuf = NULL;
    xmlDoc *doc = NULL;
    int ret = 0;

    if (OP_APPPK == APPPK_CHECK)
    {
        doc = xmlRecoverDoc(reqbuf);
        returnIfNull(doc, E_PACK_INIT);

        if (isOutTran()) {
            callRegionLibraryFunction(&ret, "AddDigest", doc, OP_TCTCODE);
            xmlDocDumpMemory(doc, &docbuf, plen);
            memcpy(reqbuf, docbuf, *plen);
        } else if (isInTran()) {
            callRegionLibraryFunction(&ret, "CheckDigest", doc, OP_TCTCODE);
        }
    }

    return ret;
}
