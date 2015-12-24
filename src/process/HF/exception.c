#include "interface.h"

int SetException_10(xmlDoc *opDoc, char *rspbuf, int *rsplen, int operr)
{
    xmlDoc *doc = NULL;
    unsigned char *pbuf = NULL;
    char responseInfo[256] = "交易成功";
    char responseCode[8] = "0000";
    const char *opBKRetcode = NULL, *opBKRetinfo = NULL, *opTCRetcode = NULL, *opTCRetinfo = NULL;
    char tmp[256] = {0};
    int ret = 0;

    if ((doc = xmlRecoverDoc(rspbuf)) == NULL)
    {
        INFO("重构响应报文DOC失败! [%s]", rspbuf);
        return E_PACK_INIT; 
    }

    if (opDoc != NULL)
    {
        opBKRetcode = XMLGetNodeVal(opDoc, "//opBKRetcode"); opBKRetinfo = XMLGetNodeVal(opDoc, "//opBKRetinfo");
        opTCRetcode = XMLGetNodeVal(opDoc, "//opTCRetcode");
        opTCRetinfo = XMLGetNodeVal(opDoc, "//opTCRetinfo");
    }

    DBUG("BKcode:%s BKinfo:%s TCcode:%s TCinfo:%s operr:%d", SAFE_PTR(opBKRetcode), 
            SAFE_PTR(opBKRetinfo), SAFE_PTR(opTCRetcode), SAFE_PTR(opTCRetinfo), operr);

    if (operr == E_DB_OPEN) {
        strcpy(responseCode, "8999");
        strcpy(responseInfo, "数据库打开错");
        goto EXIT;
    }

    //提入交易若行内出错则以行内错误码映射的同城错误码为准返回,若无此映射则以平台错误码映射的同城错误码为准返回
    if (isInTran())
    {
        if (opBKRetcode != NULL && atoi(opBKRetcode) != 0)
        {
            /*
            //查找行内错误码对相应前置的错误码
            ret = db_query_strs(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%s' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        TCOP_BANKID, OP_NODEID, opBKRetcode, OP_NODEID), responseCode, responseInfo);
                        */
            strcpy(responseCode, "8999");
            if (opBKRetinfo == NULL)
                strcpy(responseInfo, "未知错误");
            else
                strcpy(responseInfo, opBKRetinfo);
            goto EXIT;
        } 
        if (operr) 
        {
            //查找平台错误码对相应前置的错误码
            ret = db_query_strs(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%d' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        OP_OPNODE, OP_NODEID, operr, OP_NODEID), responseCode, responseInfo);
            if (ret != 0)
            {
                strcpy(responseCode, "8999");
                if (opBKRetinfo == NULL)
                    strcpy(responseInfo, "未知错误");
                else
                    strcpy(responseInfo, opBKRetinfo);
                INFO("未找到映射%s处理错误[%d],默认[%s]!", atoi(opBKRetcode) != 0 ? "行内" : "平台", 
                        atoi(opBKRetcode) != 0 ? atoi(opBKRetcode) : operr, responseCode);
            }
        }
    }

    if (isOutTran())
    {
        //平台未出错时通过报文直接转换出应答码和应答信息的交易不做处理
        if (operr == 0 && (XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result")[0] != 0 ||
                    XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc")[0] != 0))
            goto EXIT2;

        //中心错误
        if (opTCRetcode != NULL && opTCRetcode[0] != 0)
        {
            if (atoi(opTCRetcode) == 0)
                strcpy(tmp, "中心成功");
            else
            {
                strcpy(responseCode, opTCRetcode);
                ret = db_query_str(tmp, sizeof(tmp),
                        "SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%s'", OP_PHNODEID, opTCRetcode);
                if (ret)
                {
                    INFO("未找到节点[%d]的错误码[%s]信息", OP_PHNODEID, opTCRetcode);
                    strcpy(tmp, "中心其它错误");
                } 
            } 
        }

        //平台错误
        if (operr)
        {
            ret = db_query_str(responseInfo, sizeof(responseInfo),
                    "SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%d'", OP_OPNODE, operr);
            if (ret)
            {
                INFO("未找到节点[%d]的错误码[%d]信息", OP_OPNODE, operr);
                strcpy(responseInfo, "平台其它错误");
            }
            sprintf(tmp + strlen(tmp), "%s%s", tmp[0] == 0 ? "" : "|", responseInfo);

            //行内错误信息
            if (opBKRetcode != NULL && opBKRetinfo[0] != 0)
                sprintf(tmp + strlen(tmp), "|%s", opBKRetinfo);

            //中心成功则要设置提出应答报文中的平台映射的错误码否则直接使用中心响应报文中的错误码返回
            if (opTCRetcode == NULL || atoi(opTCRetcode) == 0)
            {
                //使用平台的错误码映射的同城错误码返回
                ret = db_query_str(responseCode, sizeof(responseCode), 
                        "SELECT mapcode FROM errmap WHERE nodeid=%d AND errcode='%d' AND mapnode=%d", 
                        OP_OPNODE, operr, OP_PHNODEID);
                if (ret) 
                    strcpy(responseCode, "9999");
            }
        }

        if (strlen(tmp))
            strcpy(responseInfo, tmp);
    }

EXIT:

    INFO("Result:[%s] Desc:[%s] operr:[%d]", responseCode, responseInfo, operr);

    //设置提出提入交易的应答错误码和错误信息
    XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", responseCode);
    XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc", responseInfo);

EXIT2:
    xmlDocDumpFormatMemory(doc, &pbuf, rsplen, 0);
    memcpy(rspbuf, pbuf, *rsplen);
    rspbuf[*rsplen] = 0;

    DBUG("RSP:[%s]", rspbuf);

    return 0;
}
