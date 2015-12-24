#include "interface.h"
#include "chinese.h"
#include "comm.h"

static int ret = 0;

char *GetStrElement(char *abuf, char *path, char *val, int size)
{
    xmlDocPtr doc = NULL;
    char buf[4096];

    *val = 0;
    snprintf(buf, sizeof(buf), 
            "<?xml version='1.0' encoding='GB18030'?>%s", abuf);
    if ((doc = xmlParseMemory(buf, strlen(buf))) == NULL)
        return val;

    XmlGetString(doc, path, val, size);
    xmlFreeDoc(doc);

    return val;
}

//交易预处理
int OP_DoInit(char *req, int *plen)
{
    xmlDoc *doc = NULL;
    unsigned char *docbuf = NULL;
    char encTrack[1024] = {0};  //磁条密文(base64)
    char decTrack[512] = {0};   //磁条明文
    char track2[128] = {0};     //磁条2明文
    int len = 0;
    int dcflag = 0;
    int notetype = 0;
    int classid = 0;
    char pwd[40] = {0};
    char tmp[256] = {0};
    char *p;

    doc = xmlRecoverDoc(req); 
    returnIfNullLoger(doc, E_PACK_INIT, "预处理提出交易报文初始化错");

    dcflag = atoi(XMLGetNodeVal(doc, "//DCFlag"));
    notetype = atoi(XMLGetNodeVal(doc, "//NoteType"));
    classid = atoi(XMLGetNodeVal(doc, "//SvcClass"));

    if (isInTran()) {
        if (classid == 2) {
            //if (notetype == 71 || notetype == 73) { //现金或转账通存
            memset(decTrack, 0, sizeof(decTrack));
            if ((p = XMLGetNodeVal(doc, "//TrackInfo")) != NULL && *p != 0x00)
            {
                strcpy(encTrack, p);
                if ((ret = Data_Decrypt_Soft10(XMLGetNodeVal(doc, "//WorkDate"), 
                                XMLGetNodeVal(doc, "//RefId"), 
                                encTrack, strlen(encTrack), decTrack, &len)) != 0) {
                    INFO("磁道信息解密失败:%d|%d|%s|%s[%s]", ret, strlen(encTrack),
                            XMLGetNodeVal(doc, "//WorkDate"),
                            XMLGetNodeVal(doc, "//RefId"), encTrack);
                    XMLSetNodeVal(doc, "//MAC", vstrcat("%d", E_SYS_SYDDECRYPT));
                } else {
                    decTrack[len] = 0;
                    INFO("track=[%s]", decTrack);
                    memcpy(track2, decTrack+79, 37);
                    //XMLSetNodeVal(doc, "//Reserve", track2); //存放二磁道信息
                    XmlSetString(doc, "/UFTP/MsgHdrRq/Track2", track2);
                    XMLSetNodeVal(doc, "//TrackInfo", decTrack);
                }
            }
            //}
        }
    }

    if (isOutTran()) {
        XMLSetNodeVal(doc, "//MsgHdrRq/WorkDate", GetWorkdate());

        if (classid == 2) { //个人业务
            XMLSetNodeVal(doc, "//TrnCode", dcflag == 1 ? "0001" : "0002");
            //if (notetype == 72 || notetype == 74) { //现金或转账通兑
            strcpy(decTrack, XMLGetNodeVal(doc, "//TrackInfo"));
            memcpy(track2, decTrack+79, 37);
            sdpStringTrim(track2);
            if ((ret = Data_Encrypt_Soft10(XMLGetNodeVal(doc, "//WorkDate"),
                            XMLGetNodeVal(doc, "//RefId"),
                            decTrack, strlen(decTrack), encTrack, &len)) != 0) {
                INFO("磁道信息加密失败:%d[%s]", ret, decTrack);
                return E_SYS_SYDENCRYPT;
            }
            encTrack[len] = 0;
            DBUG("磁道信息加密后:%d[%s]", len, encTrack);
            XmlSetString(doc, "/UFTP/MsgHdrRq/Track2", track2);
            XMLSetNodeVal(doc, "//TrackInfo", encTrack);
            //}
        } 
        if (notetype == 14) //查询缴税
            XMLSetNodeVal(doc, "//TrnCode", "0002");
        if (dcflag == 2) {
            switch(notetype) {
                case 2:     //转帐支票
                case 41:    //全国银行汇票
                case 44:    //来账代转补充凭证
                case 61:    //特种转账凭证
                case 62:    //网银
                case 71:    //个人现金通存通兑
                case 72:    //个人结算账户转账
                case 81:    //外币转账支票
                case 84:    //外币个人转账
                case 86:    //外币网银
                case 87:    //外币特种转账凭证
                    //XMLSetNodeVal(doc, "//AcctCheck", "1");
                    break;
                default:
                    break;
            }
        }
    }

    xmlDocDumpMemory(doc, &docbuf, plen);
    memcpy(req, docbuf, *plen);

    return 0;
}

//申请工作密钥
int PF_1631(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    char bankno[13];
    char *p = NULL;
    char *pik, *mac;

    tmpDoc = CommDocToPH(&ret, 1631, req, NULL);
    returnIfNull(tmpDoc, ret);
    *rsp = tmpDoc;

    p = XMLGetNodeVal(tmpDoc, "//Result");
    if (atoi(p) != 0)
        return 0;
    pik = XMLGetNodeVal(tmpDoc, "//PIK");
    mac = XMLGetNodeVal(tmpDoc, "//MAC");

    sprintf(bankno, "20%s", GetCBankno());
    DBUG("bankno=[%s][%s][%s]", bankno, pik, mac);
    if ((ret = WritePIK(bankno, pik, mac)) != 0)
        return ret;

    return 0;
}

//系统状态查询
int PF_1601(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    char *p = NULL;

    tmpDoc = CommDocToPH(&ret, 1601, req, NULL);
    returnIfNull(tmpDoc, ret);
    *rsp = tmpDoc;

    p = XMLGetNodeVal(tmpDoc, "//Result");
    if (atoi(p) != 0)
        return 0;

    if (strcmp(GetWorkdate(), XMLGetNodeVal(tmpDoc, "//SysStatus/WorkDate")) != 0)
    {
        if ((ret = UpdPreWorkdate(GetWorkdate())) != 0)
            return ret;
        if ((ret = UpdWorkdate(XMLGetNodeVal(tmpDoc, "//SysStatus/WorkDate"))) != 0)
            return ret;
    }

    if (strcmp(GetRound(), XMLGetNodeVal(tmpDoc, "//SysStatus/WorkRound")) != 0)
    {
        if ((ret = UpdRound(XMLGetNodeVal(tmpDoc, "//SysStatus/WorkRound"))) != 0)
            return ret;
    }

    if (strcmp(GetCleardate(), XMLGetNodeVal(tmpDoc, "//SysStatus/ClearDate")) != 0)
    {
        if ((ret = UpdPreCleardate(GetCleardate())) != 0)
            return ret;
        if ((ret = UpdCleardate(XMLGetNodeVal(tmpDoc, "//SysStatus/ClearDate"))) != 0)
            return ret;
        ret = UpdClearround(XMLGetNodeVal(tmpDoc, "//SysStatus/ClearRound"));
    }

    if (strcmp(GetExchgdate(), XMLGetNodeVal(tmpDoc, "//SysStatus/ExchgDate")) != 0)
    {
        if ((ret = UpdExchgdate(XMLGetNodeVal(tmpDoc, "//SysStatus/ExchgDate"))) != 0)
            return ret;
        ret = UpdExchground(XMLGetNodeVal(tmpDoc, "//SysStatus/ExchgRound"));
    }

    return ret;
}

//切场通知
int PF_1602(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    return ret;
}

//日切通知
int PF_1603(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char tmp[12] = {0};
    char tmp2[12] = {0};

    if ((ret = UpdPreWorkdate(GetWorkdate())) != 0)
        return ret;
    if ((ret = UpdWorkdate(XMLGetNodeVal(req, "//SysStatus/WorkDate"))) != 0)
        return ret;
    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;

    strcpy(tmp, GetCleardate());
    strcpy(tmp2, GetPreCleardate());
    if (memcmp(tmp, tmp2, 8) != 0 || tmp[9] == '0')
    {
        sprintf(tmp2, "%s-1", tmp);
        if ((ret = UpdPreCleardate(tmp2)) != 0) 
            return ret;
    }

    if ((ret = UpdCleardate(XMLGetNodeVal(req, "//SysStatus/ClearDate"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    return ret;
}

//对账通知
int PF_1604(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char *p = NULL;
    char value[20] = {0};
    int round = 0;

    XMLSetNodeVal(*rsp, "//SysStatus/WorkDate", XMLGetNodeVal(req, "//SysStatus/WorkDate"));

    round = atoi(XMLGetNodeVal(req, "//WorkRound"));
    sprintf(value, "%s-%d", XMLGetNodeVal(req, "//SysStatus/WorkDate"), round);

    if ((ret = UpdSettlmsgDateround(value)) != 0) 
        return ret;

    return 0;
}

//下载对账数据
int PF_1605(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    xmlDoc *dzRsp = NULL;
    char *pworkdate = NULL, *pworkround = NULL;
    char svcclassList[12] = {0};
    char filename[1024] = {0}; 
    char *p = NULL;
    char tmp[20] = {0};
    char svcclass[2] = {0};
    int i = 0;

    //检查机构是否有权限取对账
    if (strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "机构无此权限");
        return 0;
    }

    pworkdate = XMLGetNodeVal(req, "//SysStatus/WorkDate"); 
    pworkround = XMLGetNodeVal(req, "//WorkRound"); 

    sprintf(tmp, "%s-%s", pworkdate, pworkround);

    //检查是否已对账
    if (strcmp(tmp, GetSettledDateround()) == 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "系统已对账");
        return 0;
    }

    //检查是否可取对账
    if (strcmp(tmp, GetSettlmsgDateround()) > 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "对账尚未完成");
        return 0;
    }

    XMLSetNodeVal(req, "//Reserve", "0");   //总是取对账明细及差额数据
    XMLSetNodeVal(req, "//Sender", "999");  //接口取对账

    if ((p = GetClasslist()) == NULL)
    {
        XMLSetNodeVal(*rsp, "//Desc", "取业务列表出错");
        return 0;
    }
    strcpy(svcclassList, p);

    for (i = 0; i < strlen(svcclassList); i++)
    {
        if (svcclassList[i] != '1')
            continue;

        sprintf(svcclass, "%d", i+1);
        XmlSetString(req, "/UFTP/MsgHdrRq/SvcClass", svcclass);

        INFO("开始取业务[%s]场次[%s]对账数据...", svcclass, pworkround);
        memset(filename, 0, sizeof(filename));
        tmpDoc = CommDocToPH(&ret, 1605, req, filename);
        returnIfNull(tmpDoc, ret);

        INFO("[%s]:日期[%s]场次[%s]业务[%s]", 
                XMLGetNodeVal(tmpDoc, "//Desc"), pworkdate, pworkround, svcclass);
        if ((ret = atoi(XMLGetNodeVal(tmpDoc, "//Result"))))
        {
            if (ret == 3902) 
                continue;
            XMLSetNodeVal(*rsp, "//Desc", "取对账中心处理失败");
            return 0;
        }

        INFO("开始业务[%s]场次[%s]对账处理...", svcclass, pworkround);
        if ((ret = SettleSvr(filename, tmpDoc, pworkdate, pworkround, svcclass)) != 0)
        {
            INFO("对账处理失败 ret=[%d]", ret);
            XMLSetNodeVal(*rsp, "//Desc", "对账处理失败");
            return 0;
        }
        INFO("对账处理成功");
    }

    UpdSettledDateround(tmp);

    dzRsp = getTCTemplateDoc(1606, "//INPUT/*");
    returnIfNull(dzRsp, E_PACK_INIT);
    XMLSetNodeVal(dzRsp, "//WorkDate", pworkdate);
    XMLSetNodeVal(dzRsp, "//WorkRound", pworkround);
    XMLSetNodeVal(dzRsp, "//Originator", XMLGetNodeVal(req, "//Originator"));

    if (PF_1606(dzRsp, rsp, filename))
        return 0;

    XMLSetNodeVal(*rsp, "//Desc", vstrcat("对账成功,自动回执[%s]", XMLGetNodeVal(*rsp, "//Desc")));

    return 0;
}

//手工发送对账回执
int PF_1606(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    char svcclassList[12] = {0};
    char filename[1024] = {0}; 
    char *p = NULL;
    char tmp[20] = {0};
    char svcclass[2] = {0};
    char *pworkdate, *pworkround;
    char settledDate[12] = {0};
    int i = 0;

    //检查机构是否有权限发回执
    if (strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0) {
        XMLSetNodeVal(*rsp, "//Desc", "机构无此权限");
        return 0;
    }

    pworkround = XMLGetNodeVal(req, "//WorkRound"); 
    strcpy(settledDate, GetSettledDateround());
    settledDate[8] = 0;
    pworkdate = settledDate;

    //检查是否可发回执
    if (strcmp(pworkround, settledDate+9) > 0) {
        XMLSetNodeVal(*rsp, "//Desc", "对账尚未完成");
        return 0;
    }

    if ((p = GetClasslist()) == NULL) {
        XMLSetNodeVal(*rsp, "//Desc", "取业务列表出错");
        return 0;
    }
    strcpy(svcclassList, p);

    for (i = 0; i < strlen(svcclassList); i++)
    {
        if (svcclassList[i] != '1')
            continue;

        sprintf(svcclass, "%d", i+1);

        XMLSetNodeVal(req, "//SvcClass", svcclass);

        INFO("生成对账回执文件...");
        memset(filename, 0, sizeof(filename));
        if (ret = CenterAndBankSettle(filename, atoi(svcclass), pworkdate, atoi(pworkround)))
        {
            INFO("生成对账回执文件出错,ret=%d", ret);
            XMLSetNodeVal(*rsp, "//Desc", "生成对账回执出错");
            return 0;
        }
        INFO("开始发送对账回执:日期[%s]场次[%s]业务[%s]文件[%s]", pworkdate, pworkround, svcclass, filename);
        if (!strlen(filename))
        {
            INFO("无本场次交易数据不发送回执");
            continue;
        }

        tmpDoc = CommDocToPH(&ret, 1606, req, filename);
        returnIfNull(tmpDoc, ret);

        p = XMLGetNodeVal(tmpDoc, "//Result");
        if (atoi(p) != 0)
        {
            INFO("对账回执应答出错!result=[%s]desc=[%s]", p, XMLGetNodeVal(tmpDoc, "//Desc"));
            XMLSetNodeVal(*rsp, "//Desc", vstrcat("对账回执发送后中心应答错[%s]", XMLGetNodeVal(tmpDoc, "//Desc")));
            return 0;
        }
    }

    XMLSetNodeVal(*rsp, "//Desc", "回执成功");

    return 0;
}

static int HandleDownFileLineOrganinfo(char *line, char *reserved)
{
    char *fields[100] = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 19)
        return -1;

    return db_exec("INSERT INTO organinfo VALUES(%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
            "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '', '')",
            OP_REGIONID, fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7],
            fields[8], fields[9], fields[10], fields[11], fields[12], fields[13], fields[14], fields[15],
            fields[16], fields[17], fields[18]);
}

static int HandleDownFileLineNoteinfo(char *line, char *reserved)
{
    char *fields[100] = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 16)
        return -1;

    return db_exec("INSERT INTO noteinfo VALUES(%d, '%s', '%s', %s, '%s', '%s', '%s', %s, %s, '%s', "
            "'%s', '%s', '%s', '', '', '%s', '', '', '')",
            OP_REGIONID, fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6],
            fields[7], fields[8], fields[9], fields[10], fields[11], fields[13]);
}

static int HandleDownFileLineCodetype(char *line, char *reserved)
{
    char *fields[100] = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 2)
        return -1;

    return db_exec("INSERT INTO codetype VALUES(%d, '%s', '%s')",
            OP_REGIONID, fields[0], fields[1]);
}

static int HandleDownFileLineGeneralcode(char *line, char *reserved)
{
    char *fields[100] = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 3)
        return -1;

    return db_exec("INSERT INTO generalcode VALUES(%d, '%s', '%s', '%s')",
            OP_REGIONID, fields[0], fields[1], fields[2]);
}

//从中心下载参数
int PF_1619(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    char filename[1024] = {0};
    char file[256] = {0};

    XMLSetNodeVal(req, "//TrnCode", "1612");
    XmlSetString(req, "/UFTP/MsgHdrRq/Reserve", "organinfo+noteinfo+codetype+generalcode");
    tmpDoc = CommDocToPH(&ret, 1612, req, filename);
    returnIfNull(tmpDoc, ret);

    if (ret == 0 && atoi(XMLGetNodeVal(tmpDoc, "//Result")) == 0)
    {
        if ((ret = db_exec("DELETE FROM organinfo")) != 0)
            return ret;
        sprintf(file, "%s/organinfo", getenv("FILES_DIR"));
        if (sdpFileLinesForeach(file, 1, HandleDownFileLineOrganinfo, NULL) != 0)
            return E_DB;

        if ((ret = db_exec("DELETE FROM noteinfo")) != 0)
            return ret;
        sprintf(file, "%s/noteinfo", getenv("FILES_DIR"));
        if (sdpFileLinesForeach(file, 1, HandleDownFileLineNoteinfo, NULL) != 0)
            return E_DB;

        if ((ret = db_exec("DELETE FROM codetype")) != 0)
            return ret;
        sprintf(file, "%s/codetype", getenv("FILES_DIR"));
        if (sdpFileLinesForeach(file, 1, HandleDownFileLineCodetype, NULL) != 0)
            return E_DB;

        if ((ret = db_exec("DELETE FROM generalcode")) != 0)
            return ret;
        sprintf(file, "%s/generalcode", getenv("FILES_DIR"));
        if (sdpFileLinesForeach(file, 1, HandleDownFileLineGeneralcode, NULL) != 0)
            return E_DB;
    }

    return 0;
}

//参数更新通知
int PF_1608(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return 0;
}

//日终归档
int PF_9999(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char sql[1024] = {0};
    char *p = NULL;
    char settledate[9] = {0};
    char lastarchdate[9] = {0};
    char *archdate = NULL;
    char tmp[12] = {0};
    char precleardate[12] = {0};

    p = GetSettledDateround();
    memset(settledate, 0, sizeof(settledate));
    strncpy(settledate, p, 8); 
    p = GetArchivedate();
    memset(lastarchdate, 0, sizeof(lastarchdate));
    strncpy(lastarchdate, p, 8); 

    if (memcmp(lastarchdate, settledate, 8) == 0)
    {
        INFO("[%s]已归档(已对账日期与当前归档日期一致)", p);
        return 0;
    }

    //注:当场结束也不应该归档(由定时进程发起时间控制)
    strcpy(tmp, GetSettledDateround());
    if (strcmp(GetSettlmsgDateround(), tmp) != 0)
    {
        INFO("尚未对账结束!");
        return 0;
    }

    memset(precleardate, 0, sizeof(precleardate));
    memcpy(precleardate, GetPreCleardate(), 8);
    if (DiffDate(getDate(0), precleardate) < 0)
    {
        INFO("机器日期小于前一清算日期不同,不归档!");
        return 0;
    }

    // 归档日期后一天
    if ((archdate = daysafter(lastarchdate, "%Y%m%d", 1)) == NULL)
    {
        INFO("取归档日期出错!");
        return 9999;
    }
    INFO("开始日终归档[%s]...", archdate);
    /*
       if ((ret = db_query_str(tmp, sizeof(tmp), "SELECT min(workdate) FROM trnjour")) != 0)
       return ret;
     */
    sprintf(sql, "DELETE FROM htrnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    sprintf(sql, "INSERT INTO htrnjour SELECT * FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", 
            OP_REGIONID, archdate, precleardate);
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    sprintf(tmp, "%s-0", GetWorkdate());
    INFO("更新对账通知工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettlmsgDateround(tmp)) != 0)
        return ret;

    INFO("更新已取对账工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettledDateround(tmp)) != 0)
        return ret;

    //更新归档日期
    UpdArchivedate(precleardate);

    // 最后才能删除当日库数据
    sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    INFO("日终归档成功");

    return 0;
}


/*
 * 根据交易结果判断中心清算状态
 */
static int IsClearState(char *pRet, int clearstate)
{
    switch(atoi(pRet))
    {
        case 0:
            return clearstate;
            //交易结果不明确
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
        case 207:
        case 3002:
        case 3003:
        case 8043:
        case 8048:
        case 8202:
        case 8203:
        case 8204:
        case 8205:
            return CLRSTAT_UNKNOW;       //清算状态不确定
        default:
            return CLRSTAT_FAILED;
    }
}

//交易结果查询
int PF_0066(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    xmlDoc *tmpDoc = NULL;
    char setbuf[1024] = {0};
    char where[1024] = {0};
    char *p = NULL;
    char result[5];
    char inoutflag[2];
    char txflag[2];
    int clearstate;

    XmlGetString(req, "//InOutFlag", inoutflag, sizeof(inoutflag));
    XMLSetNodeVal(req, "//TrnCode", "0006"); 
    tmpDoc = CommDocToPHNoFile(&ret, 6, req);
    returnIfNull(tmpDoc, ret);
    *rsp = tmpDoc;

    p = XMLGetNodeVal(tmpDoc, "/UFTP/MsgHdrRs/Result");
    if (atoi(p) != 0)
        return 0;

    //转换同城报文到平台报文
    opDoc = getOPDoc();
    returnIfNull(opDoc, E_PACK_INIT);

    XMLSetNodeVal(opDoc, "//opInoutflag", inoutflag);
    XMLSetNodeVal(opDoc, "//opNodeid", vstrcat("%d", OP_REGIONID));

    if (ConvertTCXML2OP(opDoc, tmpDoc, "//INPUT/*") == NULL)
        return E_PACK_CONVERT;

    SavePack(opDoc, PACK_RSP2OP, 0); 

    clearstate = *XmlGetStringDup(opDoc, "//opClearstate");
    XmlGetString(opDoc, "//opTCRetcode", result, sizeof(result));
    INFO("中心返回交易结果:[%s], 清算状态:[%c].", result, clearstate);
    clearstate = IsClearState(result, clearstate);
    switch (clearstate)
    {
        case CLRSTAT_CHECKED:   //已对账
        case CLRSTAT_SETTLED:   //清算成功
            sprintf(setbuf, "result=0,clearstate='%c',cleardate='%s',clearround='%s'",
                    clearstate, XMLGetNodeVal(tmpDoc, "//Content//ClearDate"),
                    XMLGetNodeVal(tmpDoc, "//Content//ClearRound"));
            break;

        case CLRSTAT_UNKNOW:   //清算未知
        case CLRSTAT_FAILED:   //中心返回清算失败
            sprintf(setbuf, "result=%d,clearstate='%c'", 
                    XmlGetInteger(tmpDoc, "//Content//Result"), clearstate);
            break;
        default:
            break;
    }
    if (setbuf[0] != 0)
    {
        INFO("交易结果查询后更新交易状态 [%s]", setbuf);
        ret = db_exec(vstrcat("UPDATE trnjour SET %s WHERE nodeid=%d "
                    "AND workdate='%s' AND refid='%s' AND originator='%s' "
                    "AND inoutflag='%s'", setbuf, OP_REGIONID,
                    XMLGetNodeVal(tmpDoc, "//Content//WorkDate"),
                    XMLGetNodeVal(tmpDoc, "//Content//RefId"),
                    XMLGetNodeVal(tmpDoc, "//Content//Originator"),
                    inoutflag));
    }

    //调用相应账务处理(返回行内交易结果)
    ret = callProcess(opDoc, NULL);
    if (ret == 0)
    {
        sprintf(setbuf, "%s-%s 行内流水:%s", 
                XmlGetStringDup(opDoc, "//opTreserved1"), 
                XmlGetStringDup(opDoc, "//opBKRetinfo"), 
                XmlGetStringDup(opDoc, "//opTreserved2"));
    }
    else if (ret == E_DB_NORECORD)
        sprintf(setbuf, "查询失败: 无记录");
    else
        sprintf(setbuf, "查询失败: %d", ret);

    INFO("行内交易结果:[%s]", setbuf);

    XmlSetString(tmpDoc, "/UFTP/MsgHdrRs/Reserve", setbuf);

    return 0;
}

//提入交易结果查询(针对提入行查询)
int PF_0006(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    char sql[SQLBUFF_MAX] = {0};
    char where[512] = {0};

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'", 
            OP_REGIONID,
            XMLGetNodeVal(req, "//TrnCtl/WorkDate"),
            XMLGetNodeVal(req, "//TrnCtl/RefId"),
            XMLGetNodeVal(req, "//TrnCtl/Originator"),
            OP_INTRAN_FLAG);
    sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    ret = QueryTableByID(*rsp, "trnjour", 109, where);
    if (ret != 0 && ret != E_DB_NORECORD)
        XMLSetNodeVal(*rsp, "//Content//Result", "8999");
    else if (ret == E_DB_NORECORD)
        XMLSetNodeVal(*rsp, "//Content//Result", "8059");
    else {
        opDoc = getOPDoc();
        returnIfNull(opDoc, E_PACK_INIT);

        if (QueryTableByID(opDoc, "trnjour", 106, where) != 0)
        {
            INFO("提入交易结果查询查本地提入记录失败");
            return E_DB;
        }
        //行内处理库需要定义该交易对应的平台交易处理程序
        ret = callProcess(opDoc, COMMTOPH_AFTER); 
        if (!isSuccess(ret))
        {
            XMLSetNodeVal(*rsp, "//Content//Result", "8999");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',result=%d WHERE %s", 
                    CLRSTAT_FAILED, GetRound(), ret, where);
        } else {
            XMLSetNodeVal(*rsp, "//Content//Result", "0000");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',cleardate='%s',result=%d WHERE %s",
                    CLRSTAT_SETTLED, GetRound(), XMLGetNodeVal(req, "//WorkDate"), ret, where);
        }
        db_exec(sql);
    }

    return 0;
}

//提入交易状态同步(针对提出行进行同步)
int PF_0009(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    char sql[SQLBUFF_MAX] = {0};
    char where[512] = {0};
    char *p = NULL;

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'",
            OP_REGIONID,
            XMLGetNodeVal(req, "//MsgHdrRq/WorkDate"),
            XMLGetNodeVal(req, "//MsgHdrRq/RefId"),
            XMLGetNodeVal(req, "//MsgHdrRq/Originator"),
            OP_OUTTRAN_FLAG);
    //sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    opDoc = getOPDoc();
    returnIfNull(opDoc, E_PACK_INIT);

    if (QueryTableByID(opDoc, "trnjour", 106, where) != 0)
    {
        INFO("提入交易状态同步查本地提出记录失败");
        return E_DB;
    }

    ret = atoi(XMLGetNodeVal(req, "//Result"));
    //if (XMLGetNodeVal(opDoc, "//Clearstate")[0] != CLRSTAT_SETTLED) //本地不是清算成功状态
    {
        p = XMLGetNodeVal(req, "//ClearState");
        if (p != NULL && p[0] == CLRSTAT_SETTLED) //中心清算成功
        {
            p = XMLGetNodeVal(req, "//Result");
            if (p != NULL && strcmp("0000", p) == 0) //中心结果成功
            {
                //行内处理库需要定义该交易对应的平台交易处理程序
                ret = callProcess(opDoc, COMMTOPH_AFTER); 
            }
        }
    }
    sprintf(sql, "UPDATE trnjour SET clearstate='%s',clearround='%s',cleardate='%s',result=%d WHERE %s", 
            (ret == 0 ? "1" : "9"),
            XMLGetNodeVal(req, "//SettlInfo/WorkRound"), 
            XMLGetNodeVal(req, "//SettlInfo/ClearDate"), 
            ret, where);
    db_exec(sql);

    return 0;
}

//手工调账置流水
int PF_1609(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char sSqlStr[1024]={0};
    char sRefId[16+1]={0}, sOriginator[12+1]={0}, sInOutFlag[1+1]={0}, sSerial[8+1]={0};
    char sOrgId[3+1]={0}, sDealOper[4+1]={0}, sOper[4+1]={0}, sAcctType[1+1]={0};
    char sResult[1+1]={0};
    int iAcctType;

    strcpy(sRefId, XMLGetNodeVal(req, "//RefId"));
    strcpy(sOriginator, XMLGetNodeVal(req, "//Originator"));
    strcpy(sInOutFlag, XMLGetNodeVal(req, "//Reserve"));
    strcpy(sSerial, XMLGetNodeVal(req, "//TermId"));
    strcpy(sOrgId, XMLGetNodeVal(req, "//Acceptor"));
    strcpy(sDealOper, XMLGetNodeVal(req, "//Auditor"));
    strcpy(sOper, XMLGetNodeVal(req, "//AcctOper"));
    strcpy(sAcctType, XMLGetNodeVal(req, "//TermType"));
    iAcctType = atoi(sAcctType);

    sprintf(sSqlStr, "select result from acctjour where workdate = '%08ld' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            current_date(), sOriginator, sRefId, sInOutFlag);
    ret = DBQueryString(sResult, sSqlStr);
    if(ret != 0 && ret != E_DB_NORECORD)
    {
        XMLSetNodeVal(*rsp, "//Desc", "查询记帐流水失败");
        return 0;
    }
    if(ret == 0)
    {
        if(iAcctType == 1 && atoi(sResult) == 1)    //补记 已记帐
        {
            XMLSetNodeVal(*rsp, "//Desc", "记帐流水已存在，无需补记帐");
            return 0;
        }
        else if(iAcctType == 2 && atoi(sResult) == 2)   //冲正 已冲正
        {
            XMLSetNodeVal(*rsp, "//Desc", "该交易已冲正，无需冲正");
            return 0;
        }
    }
    else if(iAcctType == 2)
    {
        XMLSetNodeVal(*rsp, "//Desc", "记帐流水不存在，无需冲正");
        return 0;
    }

    memset(sSqlStr, 0, sizeof sSqlStr);
    if(iAcctType == 1 && ret == E_DB_NORECORD)  //补记
    {
        sprintf(sSqlStr, "insert into acctjour values(%d, '%08ld', '%s', '%s', '%s', '%s', \
            '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
                OP_REGIONID, current_date(), sOriginator, sRefId, sInOutFlag, "1609", sSerial, "",
                sOrgId, sDealOper, "", sAcctType, "0", "", "", "", sOper);
    }
    else
    {
        sprintf(sSqlStr, "update acctjour set result = '%s', %s = '%s' where workdate = '%08ld' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
                sAcctType, iAcctType == 1 ? "acctserial" : "revserial", sSerial, current_date(), sOriginator, sRefId, sInOutFlag);
    }
    db_exec(sSqlStr);

    XMLSetNodeVal(*rsp, "//Desc", "交易成功");

    return 0;
}

int PF_PRTQRY(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char result[5] = {0};

    switch (OP_TCTCODE) {
        case 3111: ret = PrintDiffNote(req, rspfile);       break; //资金清算差额
        case 3105: ret = PrintInNoteList(req, rspfile);     break; //提入数据明细表
        case 3104: ret = PrintInJJQD(req, rspfile);         break; //提入交接清单
        case 3103: ret = PrintOutNoteList(req, rspfile);    break; //提出数据明细表
        case 3100: ret = PrintInNoteAdd(req, rspfile);      break; //提入补充凭证查询打印
        case 3120: ret = PrintOutNoteTotal(req, rspfile);   break;
        case 3121: ret = PrintInNoteTotal(req, rspfile);    break;
        case 3101: ret = PrintOutJHD(req, rspfile);         break; //提出交换汇总单
        case 3102: ret = PrintOutJJQD(req, rspfile);        break; //提出交接清单
        case 8015: ret = MailQuery(req, rspfile);           break; //邮件查询
        case 3001: ret = QryOutNote(req, rspfile);          break; //提出凭证查询
        case 3002: ret = QryInNote(req, rspfile);           break; //提入凭证查询
        case 8012: ret = QryOutQuery(req, rspfile);         break; //查询提出查询查复书
        case 8013: ret = QryInQuery(req, rspfile);          break; //查询提入查询查复书
        case 8011: ret = PrintInQuery(req, rspfile);        break; //打印提入查询查复书
        case 8010: ret = PrintOutQuery(req, rspfile);       break; //打印提出查询查复书
        default: ret = E_SYS_CALL;                          break;
    }

    if (!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    sprintf(result, "%04d", ret);
    XMLSetNodeVal(*rsp, "//Result", result);

    return ret;
}

int PF_8015(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3001(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3002(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_8012(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_8013(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_8011(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_8010(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3100(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3120(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3121(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3101(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3102(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3103(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3104(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3105(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3111(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3300(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    ST_CHINESE mydcflag[] = {
        { PART_DEBIT,      "借:" },
        { PART_CREDIT,     "贷:" },
        { PART_ZS,         "指示:" },
        { -1, NULL },
    };
    char originator[13];
    char status[81];
    char traninfo[128];
    char settlinfo[81];
    char tmp[40];
    char *workdate, *workround;
    result_set rs;
    int i, rc;

    workround = GetSysPara("CURROUND");
    //sprintf(settlinfo, "工作日期:%s 场次:%s", workdate, workround);
    workdate = GetWorkdate();
    XmlGetString(req, "//Originator", tmp, sizeof(tmp));
    rc = db_query_str(originator, sizeof(originator), 
            "select exchno from bankinfo where bankid='%s'", tmp);
    if (rc != 0)
        return E_DB;
    sprintf(settlinfo, "工作日:%02ld/%02ld ", atol(workdate)%10000/100,
            atol(workdate)%100);

    XmlSetString(*rsp, "/UFTP/MsgHdrRs/WorkDate", workdate);
    XmlSetString(*rsp, "/UFTP/SysStatus/WorkRound", workround);

    memset(traninfo, 0, sizeof(traninfo));
    if (rc = db_query_nolog(&rs, "select dcflag,count(*) as incount "
                "from trnjour "
                "where inoutflag='2' and workdate='%s' and acceptor='%s' "
                "and truncflag='1' and printnum=0 group by dcflag "
                "order by dcflag", workdate, originator))
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        for (i = 0; i < db_row_count(&rs); i++)
        {
            if (db_cell_i(&rs, i, 1) > 0)
            {
                if (traninfo[0] == 0x00)
                    strcpy(traninfo, "提入未打印(");
                strcat(traninfo, GetChineseName(mydcflag, db_cell_i(&rs, i, 0)));
                strcat(traninfo, db_cell(&rs, i, 1));
            }
        }
        if (traninfo[0] != 0x00)
            strcat(traninfo, ")");
        db_free_result(&rs);
    }

    if (rc = db_query_nolog(&rs, "select count(*) as incount from trnjour "
                "where inoutflag='1' and workdate='%s' and originator='%s' "
                "and clearstate='%c' and trncode in('506','501', '503')", 
                workdate, originator, CLRSTAT_UNSETTLED))
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        if (db_cell_i(&rs, 0, 0) > 0)
        {
            strcat(traninfo, "未复核:");
            strcat(traninfo, db_cell(&rs, 0, 0));
        }
        db_free_result(&rs);
    }

    if (rc = db_query_nolog(&rs, "select count(*) as incount from queryinfo "
                "where inoutflag='2' and readflag='0' and acceptor='%s'",
                originator) != 0)
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        if (db_cell_i(&rs, 0, 0) > 0)
        {
            strcat(traninfo, "查询:");
            strcat(traninfo, db_cell(&rs, 0, 0));
        }
        db_free_result(&rs);
    }

    if (rc = db_query_nolog(&rs, "select count(*) as incount from queryinfo "
                "where inoutflag='1' and state='1' and originator='%s'", 
                originator) != 0)
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        if (db_cell_i(&rs, 0, 0) > 0)
        {
            strcat(traninfo, "查复:");
            strcat(traninfo, db_cell(&rs, 0, 0));
        }
        db_free_result(&rs);
    }

    if (rc = db_query_nolog(&rs, "select count(*) as incount from freemsg "
                "where inoutflag='2' and readflag='0' and acceptor='%s'", 
                originator) != 0)
    {
        if (rc != E_DB_NORECORD)
            return -1;
        rc = 0;
    }
    else
    {
        if (db_cell_i(&rs, 0, 0) > 0)
        {
            strcat(traninfo, "邮件:");
            strcat(traninfo, db_cell(&rs, 0, 0));
        }
        db_free_result(&rs);
    }

    memset(status, 0, sizeof(status));
    strcpy(status, settlinfo);
    memset(status + strlen(settlinfo), ' ', 79 - strlen(settlinfo));
    traninfo[78 - strlen(settlinfo)] = 0;
    strncpy(status + 79 - strlen(traninfo), traninfo, strlen(traninfo));
    XmlSetString(*rsp, "//MsgHdrRs/Reserve", status);

    return 0;
}

