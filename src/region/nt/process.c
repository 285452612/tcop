#include "interface.h"
#include "comm.h"

int MailQuery(xmlDocPtr, char *);
char *getSendBoxUniqueWhere(xmlDoc *doc);
int SettleSvr(char *dzRspFile, xmlDoc *doc, char *workdate, char *workround, char *svcclass);

//交易预处理
int OP_DoInit(char *req, int *plen)
{
    xmlDoc *doc = NULL;
    unsigned char *docbuf = NULL;
    char sDCFlag[2]={0}, sNoteType[2+1]={0};
    int ret = 0;

    doc = xmlRecoverDoc(req); 
    returnIfNullLoger(doc, E_PACK_INIT, "预处理提出交易报文初始化(覆盖工作日期)错");

    XMLSetNodeVal(doc, "//MsgHdrRq/WorkDate", GetWorkdate());

    //提出贷交易对账户检查标志赋值
    if (isOutTran())
    {
        strcpy(sDCFlag, XMLGetNodeVal(doc, "//DCFlag"));
        if(atoi(sDCFlag) == 2)
        {
            strcpy(sNoteType, XMLGetNodeVal(doc, "//NoteType"));
            switch(atoi(sNoteType))
            {
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
                    XMLSetNodeVal(doc, "//AcctCheck", "1");
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

//切场通知
int PF_1602(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret = 0;

    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    return ret;
}

//日切通知
int PF_1603(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret = 0;
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
    int ret = 0;

    XMLSetNodeVal(*rsp, "//SysStatus/WorkDate", XMLGetNodeVal(req, "//SysStatus/WorkDate"));

    round = atoi(XMLGetNodeVal(req, "//WorkRound"));
    sprintf(value, "%s-%d", XMLGetNodeVal(req, "//SysStatus/WorkDate"), round);

    if ((ret = UpdSettlmsgDateround(value)) != 0) 
        return ret;

    //‘0’不结束。本场未决交易可计入下一场 ‘1’结束。本场未决交易作为失败处理
    p = (char *)GetRoundTrnend();
    sprintf(value, "%*.*s%s", round-1, round-1, p, XMLGetNodeVal(req, "//TrnEnd"));
    /*
    if (p != NULL)
        sprintf(value, "%s%s", p, XMLGetNodeVal(req, "//TrnEnd"));
    else
        sprintf(value, "0%s", p, XMLGetNodeVal(req, "//TrnEnd"));
        */

    if ((ret = UpdRoundTrnend(value)) != 0)
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
    int ret = 0;

    //检查机构是否有权限取对账
    if (strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "机构无此权限");
        return 0;
    }

    //XMLSetNodeVal(req, "//SysStatus/WorkDate", "20091222"); //for debug

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
        XMLSetNodeVal(req, "//SvcClass", svcclass);

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

    dzRsp = getTemplateDoc(PACK_TCPACK, 0, "//INPUT/*");
    returnIfNull(dzRsp, E_PACK_INIT);
    XMLSetNodeVal(dzRsp, "//WorkDate", pworkdate);
    XMLSetNodeVal(dzRsp, "//Reserve", pworkround);
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
    int i = 0;
    int ret = 0;

    //XMLSetNodeVal(req, "//WorkDate", "20091222"); //for debug

    //检查机构是否有权限发回执
    if (strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "机构无此权限");
        return 0;
    }

    pworkdate = XMLGetNodeVal(req, "//WorkDate"); 
    pworkround = XMLGetNodeVal(req, "//Reserve"); 
    sprintf(tmp, "%s-%s", pworkdate, pworkround);

    //检查是否可发回执
    if (strcmp(tmp, GetSettledDateround()) > 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "对账尚未完成");
        return 0;
    }

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

//日终归档
int PF_9999(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char sql[1024] = {0};
    char *p = NULL;
    char settledate[9] = {0};
    char tmp[12] = {0};
    char precleardate[12] = {0};
    int ret = 0;

    p = GetSettledDateround();
    strncpy(settledate, p, 8); 
    p = GetArchivedate();

    if (memcmp(p, settledate, 8) == 0)
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
    if (strcmp(getDate(0), precleardate) != 0)
    {
        INFO("机器日期与前一清算日期不同,不归档!");
        return 0;
    }

    INFO("开始日终归档...");
    
    if ((ret = DBQueryNumber("SELECT count(*) FROM trnjour")) == ERR_DBQUERYNUMBER)
        return ret;

    if (ret > 0) 
    {
        if ((ret = DBQueryString(tmp, "SELECT min(workdate) FROM trnjour")) != 0)
            return ret;

        sprintf(sql, "DELETE FROM htrnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, tmp, precleardate);
        if ((ret = DBExecFunc(sql, 0)) != 0 && ret != E_DB_NORECORD)
            return ret;

        sprintf(sql, "INSERT INTO htrnjour SELECT * FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", 
                OP_REGIONID, tmp, precleardate);
        if ((ret = OPDBExec(sql)) != 0 && ret != E_DB_NORECORD)
            return ret;

        sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, tmp, precleardate);
        if ((ret = DBExecFunc(sql, 0)) != 0 && ret != E_DB_NORECORD)
            return ret;
    }

    sprintf(tmp, "%s-0", GetWorkdate());
    INFO("更新对账通知工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettlmsgDateround(tmp)) != 0)
        return ret;

    INFO("更新已取对账工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettledDateround(tmp)) != 0)
        return ret;

    UpdRoundTrnend("");
    //更新归档日期
    UpdArchivedate(precleardate);

    INFO("日终归档成功");

    return 0;
}

int PF_8001(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *tmpDoc = NULL;
    const char *p = NULL;
    int ret = 0;

    XMLSetNodeVal(req, "//Reserve", vstrcat("%d", OP_REGIONID));

    if (isOutTran()) 
    {
        XMLSetNodeVal(req, "//SendDate", GetWorkdate());
        if ((ret = InsertTableByID(req, "sendbox", 600001)) != 0)
            return ret;
        tmpDoc = CommDocToPHNoFile(&ret, 8001, req);
        returnIfNull(tmpDoc, ret);
        *rsp = tmpDoc;
    }
    if (isInTran())
    {
        p = XMLGetNodeVal(req, "//MailType");
        if (*p == '0') //邮件
            ret = InsertTableByID(req, "recvbox", 600003);
        else //查询查复
            ret = InsertTableByID(req, "recvbox", 600004);
    }
    return ret;
}

int PF_8014(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char sql[1024] = {0};
    int ret = 0;

    sprintf(sql, "SELECT * FROM sendbox WHERE %s", getSendBoxUniqueWhere(req));

    ret = QueryTableByID(*rsp, "sendbox", 0, sql);

    if (ret == 0)
        ret = E_DB_NORECORD;
    else if (ret != 1)
        ret = E_DB_SELECT;
    else
        ret = 0;

    return ret;
}

/*
   邮件查询
 */
int PF_8015(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = MailQuery(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   查询书录入
 */
int PF_8004(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    XMLSetNodeVal(req, "//Reserve", vstrcat("%d", OP_REGIONID));
    XMLSetNodeVal(req, "//SendDate", GetWorkdate());

    return InsertTableByID(req, "sendbox", 0);
}

/*
   查复书录入
 */
int PF_8007(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    XMLSetNodeVal(req, "//Reserve", vstrcat("%d", OP_REGIONID));
    XMLSetNodeVal(req, "//SendDate", GetWorkdate());

    return InsertTableByID(req, "sendbox", 0);
}

/*
   查询书复核
 */
int PF_8006(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;
    xmlDoc *tmpDoc = NULL;

    tmpDoc = CommDocToPHNoFile(&ret, 8006, req);
    returnIfNull(tmpDoc, ret);
    *rsp = tmpDoc;
    OPDBExec(vstrcat("update sendbox set sended = '1' where %s", getSendBoxUniqueWhere(req)));

    return 0;
}

/*
   查复书复核
 */
int PF_8009(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;
    xmlDoc *tmpDoc = NULL;

    tmpDoc = CommDocToPHNoFile(&ret, 8009, req);
    returnIfNull(tmpDoc, ret);
    *rsp = tmpDoc;
    OPDBExec(vstrcat("update sendbox set sended = '1' where %s", getSendBoxUniqueWhere(req))); 

    return 0;
}

/*
   查询书修改
 */
int PF_8005(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return UpdateTableByID(req, "sendbox", 0, getSendBoxUniqueWhere(req));
}

/*
   查复书修改
 */
int PF_8008(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return UpdateTableByID(req, "sendbox", 0, getSendBoxUniqueWhere(req));
}

char *getSendBoxUniqueWhere(xmlDoc *req)
{
    static char sql[1024] = {0};

    sprintf(sql, "nodeid=%d and senddate='%s' and mailid=%d and sender='%s'",
            OP_REGIONID, GetWorkdate(), 
            atoi(XMLGetNodeVal(req, "//MailId")), 
            XMLGetNodeVal(req, "//Originator"));

    return sql;
}

/*
   提出凭证查询
 */
int PF_3001(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = QryOutNote(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   提入凭证查询
 */
int PF_3002(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = QryInNote(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   查询提出查询查复书
 */
int PF_8012(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = QryOutQuery(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   查询提入查询查复书
 */
int PF_8013(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = QryInQuery(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   打印提入查询查复书
 */
int PF_8011(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = PrintInQuery(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}

/*
   对账单打印
 */
int PF_1618(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = PrintAcctList(req, rspfile);

    if(!ret)
    {
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
        XMLSetNodeVal(*rsp, "//Result", "0000");
    }
    else
        rspfile[0] = 0;

    return ret;
}

//提出交易结果查询
int PF_1006(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    xmlDoc *tmpDoc = NULL;
    char setbuf[1024] = {0};
    char where[1024] = {0};
    char *p = NULL;
    int clearstate = 'U';
    int ret;

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

    XMLSetNodeVal(opDoc, "//Inoutflag", OP_OUTTRAN_FLAG);
    XMLSetNodeVal(opDoc, "//opNodeid", vstrcat("%d", OP_REGIONID));

    if (ConvertTCXML2OP(opDoc, tmpDoc, "//INPUT/*") == NULL)
        return E_PACK_CONVERT;

    SavePack(opDoc, PACK_RSP2OP, 0); 

    //调用相应账务处理(返回清算结果)
    clearstate = callProcess(opDoc, NULL);

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'",
            OP_REGIONID,
            XMLGetNodeVal(tmpDoc, "//Content//WorkDate"),
            XMLGetNodeVal(tmpDoc, "//Content//RefId"),
            XMLGetNodeVal(tmpDoc, "//Content//Originator"),
            OP_OUTTRAN_FLAG);

    INFO("提出交易结果查询账务处理结果:[%c]", (char)clearstate);

    switch (clearstate)
    {
        case CLRSTAT_SETTLED:   //清算成功
            sprintf(setbuf, "result=0,clearstate='%c',cleardate='%s',clearround='%s'",
                    CLRSTAT_SETTLED, 
                    XMLGetNodeVal(tmpDoc, "//Content//ClearDate"),
                    XMLGetNodeVal(tmpDoc, "//Content//ClearRound"));
            break;

        case CLRSTAT_UNKNOW:   //清算未知
            sprintf(setbuf, "result=%d,clearstate='%c'", 
                    atoi(XMLGetNodeVal(tmpDoc, "//Content//Result")), 
                    CLRSTAT_UNKNOW);
            break;

        case CLRSTAT_FAILED:   //中心返回清算失败
            sprintf(setbuf, "result=%d,clearstate='%c'",
                    atoi(XMLGetNodeVal(tmpDoc, "//Content//Result")), 
                    CLRSTAT_FAILED);
            break;

        default:
            INFO("提出交易同步调用账务处理返回同城清算状态未知[%c]", (char)clearstate);
            break;
    }

    if (setbuf[0] != 0);
    {
        INFO("提出交易结果查询后更新交易状态 [%s]", setbuf);
        ret = OPDBExec(vstrcat("UPDATE trnjour SET %s WHERE %s", setbuf, where));
    }

    return ret;
}

//提入交易结果查询(针对提入行查询)
int PF_0006(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    char sql[SQLBUFFLEN] = {0};
    char where[512] = {0};
    int ret = 0;

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'", 
            OP_REGIONID,
            XMLGetNodeVal(req, "//TrnCtl/WorkDate"),
            XMLGetNodeVal(req, "//TrnCtl/RefId"),
            XMLGetNodeVal(req, "//TrnCtl/Originator"),
            OP_INTRAN_FLAG);
    sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    ret = QueryTableByID(*rsp, "trnjour", 100009, sql);
    if (ret == 0) //无此记录
        XMLSetNodeVal(*rsp, "//Content//Result", "8059");
    else if (ret != 1) //记录不唯一?
        XMLSetNodeVal(*rsp, "//Content//Result", "8301");
    else {
        opDoc = getOPDoc();
        returnIfNull(opDoc, E_PACK_INIT);

        if (QueryTableByID(opDoc, "trnjour", 100006, sql) != 1)
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
        OPDBExec(sql);
    }

    return 0;
}

//提入交易状态同步(针对提出行进行同步)
int PF_0009(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc *opDoc = NULL;
    char sql[SQLBUFFLEN] = {0};
    char where[512] = {0};
    char *p = NULL;
    int ret = 0;

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'",
            OP_REGIONID,
            XMLGetNodeVal(req, "//WorkDate"),
            XMLGetNodeVal(req, "//RefId"),
            XMLGetNodeVal(req, "//Originator"),
            OP_OUTTRAN_FLAG);
    sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    opDoc = getOPDoc();
    returnIfNull(opDoc, E_PACK_INIT);

    if (QueryTableByID(opDoc, "trnjour", 100006, sql) != 1)
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
            XMLGetNodeVal(req, "//ClearState"),
            XMLGetNodeVal(req, "//WorkRound"), 
            XMLGetNodeVal(req, "//WorkDate"), 
            ret, where);
    OPDBExec(sql);

    return 0;
}

//手工调账置流水
int PF_1609(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret=0;
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
    OPDBExec(sSqlStr);

    XMLSetNodeVal(*rsp, "//Desc", "交易成功");

    return 0;
}
/*
   提入补充凭证查询打印
 */
int PF_3100(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    int ret;

    ret = PrintInNoteAdd(req, rspfile);

    if(!ret)
        XMLSetNodeVal(*rsp, "//Reserve", rspfile);
    else
        rspfile[0] = 0;

    return ret;
}
