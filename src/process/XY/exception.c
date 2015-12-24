#include "interface.h"

int SetException(char *rspbuf, int *rsplen, int operr)
{
    xmlDoc *doc = NULL;
    unsigned char *pbuf = NULL;
    const char *p = NULL;
    const char *pResult = NULL;
    char tcErrinfo[256] = "交易成功";
    char tcErrcode[8] = "0000";
    char tmp[256] = {0};
    int ret = 0;

    if ((doc = xmlRecoverDoc(rspbuf)) == NULL)
    {
        INFO("重构响应报文DOC失败! [%s]", rspbuf);
        return E_PACK_INIT; 
    }

    //提入交易若行内出错则以行内错误码映射的同城错误码为准返回,若无此映射则以平台错误码映射的同城错误码为准返回
    if (isInTran())
    {
        //"//Reserve"中存放行内错误码
        p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Reserve");

        if (isSuccess(operr)) 
        {
            //用于提入贷记直接返回成功的判断
        } 
        else if (p != NULL && p[0] != 0)
        {
            //查找行内错误码对相应前置的错误码
            ret = DBQueryStrings(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%s' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        TCOP_BANKID, OP_NODEID, p, OP_NODEID), 2, tcErrcode, tcErrinfo);
        } 
        else if (operr) 
        {
            //查找平台错误码对相应前置的错误码
            ret = DBQueryStrings(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%d' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        OP_OPNODE, OP_NODEID, operr, OP_NODEID), 2, tcErrcode, tcErrinfo);
        }

        if (ret != 0)
        {
            //通过配置支持可忽略的行内/平台错误
            if (ret == E_DB_NORECORD)
            {
                strcpy(tcErrcode, "8999");
                strcpy(tcErrinfo, "未知错误");
                INFO("未映射%s处理错误[%d],默认[%s]!", 
                        atoi(p) != 0 ? "行内" : "平台", 
                        atoi(p) != 0 ? atoi(p) : operr, tcErrcode);
            } else
                return ret;
        }

        XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", tcErrcode);
        XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc", tcErrinfo);
    }

    if (isOutTran())
    {
        //中心错误
        pResult = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result");
        if (pResult[0] != 0)
        {
            if (atoi(pResult) == 0)
                strcpy(tmp, "中心成功");
            else
            {
                ret = DBQueryString(tcErrinfo, 
                        vstrcat("SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%s'", OP_PHNODEID, pResult));
                if (ret)
                {
                    if (ret != E_DB_NORECORD)
                        return ret;
                    INFO("未找到节点[%d]的错误码[%s]信息", OP_PHNODEID, pResult);
                    strcpy(tmp, "中心其它错误");
                } else  
                    strcpy(tmp, tcErrinfo);
            } 
        } 

        //平台错误
        if (operr)
        {
            ret = DBQueryString(tcErrinfo, 
                    vstrcat("SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%d'", OP_OPNODE, operr));
            if (ret)
            {
                if (ret != E_DB_NORECORD)
                    return ret;
                INFO("未找到节点[%d]的错误码[%d]信息", OP_OPNODE, operr);
                strcpy(tmp, "平台其它错误");
            }
            sprintf(tmp + strlen(tmp), "%s%s", tmp[0] == 0 ? "" : "|", tcErrinfo);

            //行内错误信息
            p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc");
            if (p[0] != 0)
                sprintf(tmp + strlen(tmp), "|%s", p);

            //中心成功则要设置提出应答报文中的错误码否则直接使用中心响应报文中的错误码返回
            if (pResult[0] == 0 || atoi(pResult) == 0)
            {
                //使用平台的错误码映射的同城错误码返回
                ret = DBQueryString(tcErrcode, vstrcat("SELECT mapcode FROM errmap WHERE nodeid=%d AND errcode='%d' AND mapnode=%d",
                            OP_OPNODE, operr, OP_PHNODEID));
                if (ret) 
                {
                    if (ret != E_DB_NORECORD)
                        return ret;
                    strcpy(tcErrcode, "9999");
                }
                XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", tcErrcode);
            }
        }

        p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result");
        if (p != NULL && p[0] == 0)
            XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Reserve"));
        if (strlen(tmp))
            XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc",  tmp);
    }

    INFO("Result:[%s] Desc:[%s] operr:[%d]", XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result"), 
            XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc"), operr);

    xmlDocDumpFormatMemory(doc, &pbuf, rsplen, 0);
    memcpy(rspbuf, pbuf, *rsplen);
    rspbuf[*rsplen] = 0;

    DBUG("RSP:[%s]", rspbuf);

    return 0;
}
