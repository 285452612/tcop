#include "interface.h"
#include "chinese.h"
#include "comm.h"

static int ret = 0;

char *GetStrElement(char *abuf, char *path, char *val, int size)
{
    xmlDocPtr       doc         = NULL;
    char            buf[4096]   = {0};

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
    xmlDoc          *doc            = NULL;
    xmlDoc          *opDoc          = NULL;
    unsigned char   *docbuf         = NULL;
    char            encTrack[1024]  = {0};  //磁条密文(base64)
    char            decTrack[512]   = {0};   //磁条明文
    char            track2[128]     = {0};     //磁条2明文
    char            track3[128]     = {0};     //磁条3明文
    int             len             = 0, dcflag=0, notetype=0, classid=0;
    char            pwd[40]         = {0};
    char            tmp[256]        = {0};
    char            tmp1[256]       = {0};
    char            *p              = NULL;
    char            *q              = NULL;
    char            *q1             = NULL;
    char            *ptmp           = NULL;
    char            sql[1024]       = {0};
    char            payacct[33]     = {0}, payer[81]={0}, beneacct[32]={0},benename[81]={0};
    char            settlamt[20]    = {0}, noteno[20]={0}, snotetype[3]={0}, issuedate[9]={0};
    char            tblname[16]     = "trnjour", workdate[9]={0}, acceptor[9]={0};

    doc = xmlRecoverDoc(req); 
    returnIfNullLoger(doc, E_PACK_INIT, "预处理提出交易报文初始化错");

    dcflag = atoi(XMLGetNodeVal(doc, "//DCFlag"));
    notetype = atoi(XMLGetNodeVal(doc, "//NoteType"));
    classid = atoi(XMLGetNodeVal(doc, "//SvcClass"));

    if (isInTran()) {
        if (classid == 2) {
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
                    memcpy(track3, decTrack+116, 104);
                    XmlSetString(doc, "/UFTP/MsgHdrRq/Track2", track2);
                    XmlSetString(doc, "/UFTP/MsgHdrRq/Track3", track3);
                    XMLSetNodeVal(doc, "//TrackInfo", decTrack);
                }
            }
        }
    }

    if (isOutTran()) 
    {
#if 0
        p = XMLGetNodeVal(doc, "//AcctOper");
        if( strlen(p) > 6 )
        {
            XMLSetNodeVal(doc, "//AcctOper", p+2);
            INFO("操作员截断[%s]->[%s]", p, XMLGetNodeVal(doc, "//AcctOper") );
        }
#endif 

        p = XMLGetNodeVal(doc, "//TrnCode");
        XMLSetNodeVal(doc, "//MsgHdrRq/WorkDate", GetWorkdate());
        XMLSetNodeVal(doc, "//ExchgRound",GetExchground());
        //凭证号码为空自动设置为流水号
        ptmp = XMLGetNodeVal(doc, "//NoteNo");
        if( ptmp != NULL )
        {
            if( strlen(ptmp) == 0 )
                XMLSetNodeVal(doc,"//NoteNo", XMLGetNodeVal(doc, "//RefId"));
        }
        /*退票交易根据流水去查找原票据信息*/
        //if( atoi(p) == 7 && dcflag == 1 )
        if( atoi(p) == 7 )
        {
            ptmp = XMLGetNodeVal(doc, "//Agreement");
            memset( workdate, 0, sizeof(workdate) );
            memcpy( workdate, ptmp, 8 );

            if (strcmp(workdate, GetArchivedate()) <= 0)
                strcpy(tblname, "htrnjour");

            sprintf( sql, "select beneacct, benename, payingacct, payer, settlamt, notetype,"
                    " noteno, issuedate, acceptor from %s "
                    " where workdate='%8.8s' and refid='%s' and originator='%s' and inoutflag='2'",
                    tblname, ptmp,  ptmp+9, XMLGetNodeVal(doc, "//Acceptor") );

            ret = db_query_strs(sql, beneacct, benename, payacct, payer, settlamt, snotetype, noteno, issuedate, acceptor );

            if( dcflag == 1 )
            {
                /*查询营业机构 9328交易*/
                q=XMLGetNodeVal(doc, "//TermId");
                //转换同城报文到平台报文
                opDoc = getOPDoc();
                returnIfNull(opDoc, E_PACK_INIT);
                XMLSetNodeVal(opDoc, "//opWorkdate", XMLGetNodeVal(doc, "//WorkDate"));
                XMLSetNodeVal(opDoc, "//opOperid", XMLGetNodeVal(doc, "//AcctOper"));
                XMLSetNodeVal(opDoc, "//opPDWSNO", XMLGetNodeVal(doc, "//PDWSNO"));
                //XMLSetNodeVal(opDoc, "//opInnerBank", XMLGetNodeVal(doc, "//TermId"));
                XMLSetNodeVal(opDoc, "//opInnerBank", q);

                if( ret = callInterface( 9328, opDoc) )
                {
                    BKINFO("查询营业机构失败[%d]...", ret);
                    return ret;
                }

                p=XMLGetNodeVal(opDoc, "//opHostSerial");
                if(strncmp( p,"89",2 ))
                {
                    BKINFO("集中业务虚拟柜员[%s]行内机构[%s]...", XMLGetNodeVal(doc, "//AcctOper"), p);
                    memset(tmp, 0, sizeof(tmp));
                    ret = db_query_str(tmp, sizeof(tmp), "SELECT parent FROM bankinfo where nodeid=%d and exchno='%s'", 
                            OP_REGIONID, XMLGetNodeVal(doc, "//Originator"));
                    if( ret )
                    {
                        BKINFO("行内机构找不到送[%s]...", "8901");
                        XMLSetNodeVal(doc, "//TermId", "8901");
                    }
                    else
                    {
                        BKINFO("原交易接收行[%s],送[%s]退票行内记账...", XMLGetNodeVal(doc, "//Originator"), tmp);
                        XMLSetNodeVal(doc, "//TermId", tmp);
                    }

                }
                else
                    XMLSetNodeVal(doc, "//TermId", p);

                memset(tmp1, 0, sizeof(tmp1));
                ret = db_query_str(tmp1, sizeof(tmp1), "SELECT exchno FROM bankinfo where nodeid=%d and bankid='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//TermId"));
                XMLSetNodeVal(doc, "//Originator", tmp1);

                BKINFO("退票交易行内记账机构[%s],发送人行机构[%s]...", XMLGetNodeVal(doc, "//TermId"), tmp1);
            }

            //信电汇
            if( atoi(snotetype) == 17 || atoi(snotetype) == 18 )
            {
                XMLSetNodeVal(doc, "//OppBank", XMLGetNodeVal(doc, "//Acceptor"));
                XMLSetNodeVal(doc, "//OppBankName", XMLGetNodeVal(doc, "//Acceptor"));
                XMLSetNodeVal(doc, "//OppCustAddr", XMLGetNodeVal(doc, "//Acceptor"));
            }
            XMLSetNodeVal(doc, "//BeneAcct", payacct);
            XMLSetNodeVal(doc, "//BeneName", payer);
            XMLSetNodeVal(doc, "//PayingAcct", beneacct);
            XMLSetNodeVal(doc, "//Payer", benename);
            XMLSetNodeVal(doc, "//NoteType", snotetype);
            XMLSetNodeVal(doc, "//NoteNo", noteno);
            XMLSetNodeVal(doc, "//SettlAmt", settlamt);
            XMLSetNodeVal(doc, "//IssueDate", issuedate);
            //行内退票使用
            XMLSetNodeVal(doc, "//SHKRZH", beneacct);
            XMLSetNodeVal(doc, "//SHKRXM", benename);
            XMLSetNodeVal(doc, "//FUKRZH", payacct);
            XMLSetNodeVal(doc, "//FUKRXM", payer);
        }
        if (classid == 2) 
        { //个人业务
            XMLSetNodeVal(doc, "//TrnCode", dcflag == 1 ? "0001" : "0002");
            //if (notetype == 72 || notetype == 74) //现金或转账通兑
            if(notetype==71)
            {
                XMLSetNodeVal(doc,"//Payer","现金");
            }else if(notetype==72)
            {
                XMLSetNodeVal(doc,"//BeneName","现金");
            }
            if ((p = XMLGetNodeVal(doc, "//TrackInfo")) != NULL && *p != 0x00)
            {
                strcpy(decTrack, p);
                INFO("磁道信息加密前:[%s]", decTrack);
                if ((ret = Data_Encrypt_Soft10(XMLGetNodeVal(doc, "//WorkDate"),
                                XMLGetNodeVal(doc, "//RefId"),
                                decTrack, strlen(decTrack), encTrack, &len)) != 0) {
                    INFO("磁道信息加密失败:%d[%s]", ret, decTrack);
                    return E_SYS_SYDENCRYPT;
                }
                encTrack[len] = 0;
                INFO("磁道信息加密后:%d[%s]", len, encTrack);
                XMLSetNodeVal(doc, "//TrackInfo", encTrack);
            }
        } 
        if (dcflag == 2) 
        {
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
        //操作员截断
        p = XMLGetNodeVal(doc, "//AcctOper");
        if( strlen(p) > 6 )
        {
            XMLSetNodeVal(doc, "//AcctOper", p+2);
            INFO("操作员截断[%s]->[%s]", p, XMLGetNodeVal(doc, "//AcctOper") );
        }
    }

    xmlDocDumpMemory(doc, &docbuf, plen);
    memcpy(req, docbuf, *plen);

    return 0;
}

//申请工作密钥
int PF_1631(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *tmpDoc         = NULL;
    char            bankno[13]      = {0};
    char            *p              = NULL;
    char            *pik            = NULL, *mac = NULL;

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
    if ((ret = WriteKey( pik, mac )) != 0)
        return ret;

    return 0;
}

//系统状态查询
int PF_1601(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *tmpDoc         = NULL;
    char            *p              = NULL;

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
    char        tmp[12]         = {0};
    char        tmp1[12]         = {0};

    if (strcmp(GetRound(), XMLGetNodeVal(req, "//SysStatus/WorkRound")) != 0) 
    {
        if (ret = UpdPreRound(GetRound()))
            return ret;
        if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
            return ret;
        if ((ret = UpdExchground(XMLGetNodeVal(req, "//SysStatus/ExchgRound"))) != 0)
            return ret;
        //交换日期
        if (strcmp(GetExchgdate(), XMLGetNodeVal(req, "//SysStatus/ExchgDate")) != 0)
        {
            UpdPreExchgdate(GetExchgdate());
            if ((ret = UpdExchgdate(XMLGetNodeVal(req, "//SysStatus/ExchgDate"))) != 0)
                return ret;
        }
        //清算日期
        if (strcmp(GetCleardate(), XMLGetNodeVal(req, "//SysStatus/ClearDate")) != 0)
        {
            strcpy(tmp1, GetClearround());
            sprintf(tmp, "%s-%s", GetCleardate(), tmp1);
            BKINFO("TMP[%s] GETCLEARDATE[%s], GETCLEARROUND[%s]", tmp, GetCleardate(), tmp1 );
            UpdPreCleardate(tmp);
            if ((ret = UpdCleardate(XMLGetNodeVal(req, "//SysStatus/ClearDate"))) != 0)
                return ret;
        }
        ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));
    }

    return ret;
}

//日切通知
int PF_1603(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char        tmp[12]         = {0};
    char        tmp2[12]        = {0};
    char        tmp1[12]        = {0};

    if ((ret = UpdPreWorkdate(GetWorkdate())) != 0)
        return ret;
    if ((ret = UpdWorkdate(XMLGetNodeVal(req, "//SysStatus/WorkDate"))) != 0)
        return ret;
    if (ret = UpdPreRound(GetRound())) 
        return ret;
    if ((ret = UpdRound(XMLGetNodeVal(req, "//SysStatus/WorkRound"))) != 0)
        return ret;

    strcpy(tmp, GetCleardate());
    strcpy(tmp1, XMLGetNodeVal(req, "//SysStatus/ClearDate"));
    strcpy(tmp2, GetPreCleardate());
    //if (memcmp(tmp, tmp2, 8) != 0 || tmp[9] == '0')
    BKINFO( "人行清算日期:[%s], 前一清算日期[%s], 平台清算日期[%s]",
            tmp1, tmp2, tmp );

    if (memcmp(tmp, tmp1, 8) != 0 )
    {
        sprintf(tmp2, "%s-%s", tmp, GetClearround());
        BKINFO( "前一清算日期[%s]", tmp2 );
        if ((ret = UpdPreCleardate(tmp2)) != 0) 
            return ret;
    }

    if ((ret = UpdCleardate(XMLGetNodeVal(req, "//SysStatus/ClearDate"))) != 0)
        return ret;
    ret = UpdClearround(XMLGetNodeVal(req, "//SysStatus/ClearRound"));

    if (strcmp(GetExchgdate(), XMLGetNodeVal(req, "//SysStatus/ExchgDate")) != 0)
    {
        UpdPreExchgdate(GetExchgdate());
        if ((ret = UpdExchgdate(XMLGetNodeVal(req, "//SysStatus/ExchgDate"))) != 0)
            return ret;
        ret = UpdExchground(XMLGetNodeVal(req, "//SysStatus/ExchgRound"));
    }
    return ret;
}

//对账通知
int PF_1604(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char        *p          = NULL;
    char        value[20]   = {0};
    int         round       = 0;
    xmlDoc      *dzReq      = NULL;
    xmlDoc      *dzRsp      = NULL;

    XMLSetNodeVal(*rsp, "//SysStatus/WorkDate", XMLGetNodeVal(req, "//SysStatus/WorkDate"));

    round = atoi(XMLGetNodeVal(req, "//WorkRound"));
    sprintf(value, "%s-%d", XMLGetNodeVal(req, "//SysStatus/WorkDate"), round);

    if ((ret = UpdSettlmsgDateround(value)) != 0) 
        return ret;

#if 0 
    //如果前面有一场自动取对账失败则后面所有都不自动取对账
    if (round != (atoi(GetSettledDateround()+9) + 1)) {
        INFO("前场[%s]自动取对账失败,本场[%d]不自动取对账", GetSettledDateround()+9, round);
        return 0;
    }
#endif

    //自动对账
    dzReq = getTCTemplateDoc(1605, "//INPUT/*");
    returnIfNull(dzReq, E_PACK_INIT);
    dzRsp = getTCTemplateDoc(1605, "//OUTPUT/*");
    returnIfNull(dzRsp, E_PACK_INIT);

    XMLSetNodeVal(dzReq, "//Originator", GetCBankno());
    XMLSetNodeVal(dzReq, "//MsgHdrRq/WorkDate", XMLGetNodeVal(req, "//SysStatus/WorkDate"));
    XMLSetNodeVal(dzReq, "//SysStatus/WorkDate", XMLGetNodeVal(req, "//SysStatus/WorkDate"));
    XMLSetNodeVal(dzReq, "//WorkRound", XMLGetNodeVal(req, "//WorkRound"));

    if (PF_1605(dzReq, &dzRsp, NULL))
        return ret;

    if (*XMLGetNodeVal(dzRsp, "//Desc") != 0) 
        return E_OTHER;

    return 0;
}

//下载对账数据
int PF_1605(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *tmpDoc             = NULL;
    xmlDoc          *dzRsp              = NULL;
    char            *pworkdate          = NULL, *pworkround = NULL;
    char            svcclassList[12]    = {0};
    char            filename[1024]      = {0}; 
    char            *p                  = NULL;
    char            tmp[20]             = {0};
    char            svcclass[2]         = {0};
    char            settledDate[12]     = {0}; //已取对账日期场次
    char            settledMsgDate[12]  = {0}; //对账通知日期场次
    int             allRoundFlag        = 0;
    char            j                   = 0;
    int             i                   = 0;

    //检查机构是否有权限取对账
    if (rspfile != NULL && strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "机构无此权限");
        return 0;
    }

    pworkdate = XMLGetNodeVal(req, "//SysStatus/WorkDate"); 
    pworkround = XMLGetNodeVal(req, "//WorkRound"); 
    strcpy(settledDate, GetSettledDateround());
    strcpy(settledMsgDate, GetSettlmsgDateround());

#if 0
    //必需取最后对账通知日期的对账(否则通过管理平台修改系统参数)
    if (memcmp(pworkdate, settledMsgDate, 8) != 0) {
        XMLSetNodeVal(*rsp, "//Desc", vstrcat("取对账日期[%s]不等于通知日期[%8.8s]", pworkdate, settledMsgDate));
        return 0;
    }

    BKINFO("pWorkDate[%s],pWorkRound[%s],SettledDate[%s],settledMsgDate[%s]", 
            pworkdate,pworkround,settledDate,settledMsgDate);

    //检查是否可取对账
    if (settledMsgDate[9] == '0' || pworkround[0] > settledMsgDate[9]) {
        XMLSetNodeVal(*rsp, "//Desc", "对账尚未完成");
        return 0;
    }

    //取全部场次
    if (pworkround[0] == '0') {
        allRoundFlag = 1;
        if (settledDate[9] == settledMsgDate[9]) {
            XMLSetNodeVal(*rsp, "//Desc", "系统已全部对账");
            return 0;
        }
    } else {
        if (pworkround[0] <=  settledDate[9]) {
            XMLSetNodeVal(*rsp, "//Desc", "系统已对账");
            return 0;
        }
        /*
           if (pworkround[0] != settledDate[9]+1) {
           XMLSetNodeVal(*rsp, "//Desc", vstrcat("取对账场次[%s]不正确,该场之前有第[%c]场未对账", 
           pworkround, settledDate[9]+1));
           return 0;
           }
         */
    }
#endif
    if (pworkround[0] == '0') 
        allRoundFlag = 1;
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

    //sprintf(tmp, "%s-%s", pworkdate, pworkround);
    sprintf(tmp, "%s-%c", pworkdate, settledMsgDate[9]);
    UpdSettledDateround(tmp);

    dzRsp = getTCTemplateDoc(1606, "//INPUT/*");
    returnIfNull(dzRsp, E_PACK_INIT);
    XMLSetNodeVal(dzRsp, "//WorkDate", pworkdate);
    XMLSetNodeVal(dzRsp, "//WorkRound", pworkround);
    XMLSetNodeVal(dzRsp, "//Originator", XMLGetNodeVal(req, "//Originator"));

    if (PF_1606(dzRsp, rsp, filename))
    {
        INFO("场次[%s]自动回执失败", pworkround);
        return 0;
    }
    INFO("回执结果[%s]", XMLGetNodeVal(*rsp, "//Desc"));

    if (rspfile == NULL && strcmp(XMLGetNodeVal(*rsp, "//Desc"), "回执成功") == 0)
        XMLSetNodeVal(*rsp, "//Desc", "");
    else
        XMLSetNodeVal(*rsp, "//Desc", vstrcat("对账成功,自动回执[%s]", XMLGetNodeVal(*rsp, "//Desc")));


    INFO("下载对账处理结果[%s]", XMLGetNodeVal(*rsp, "//Desc"));

    return 0;
}

//手工发送对账回执
int PF_1606(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *tmpDoc             = NULL;
    char            svcclassList[12]    = {0};
    char            filename[1024]      = {0}; 
    char            *p                  = NULL;
    char            tmp[20]             = {0};
    char            svcclass[2]         = {0};
    char            *pworkdate          = NULL;
    char            *pworkround         = NULL;
    char            settledDate[12]     = {0};
    int             i                   = 0;

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
    char            *fields[100]            = {0};


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
    char            *fields[100]            = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 16)
        return -1;

    return db_exec("INSERT INTO noteinfo VALUES(%d, '%s', '%s', %s, '%s', '%s', '%s', %s, %s, '%s', "
            "'%s', '%s', '%s', '', '', '%s', '', '', '')",
            OP_REGIONID, fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6],
            fields[7], fields[8], fields[9], fields[10], fields[11], fields[13]);
}

static int HandleDownFileLineCodetype(char *line, char *reserved)
{
    char            *fields[100]            = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 2)
        return -1;

    return db_exec("INSERT INTO codetype VALUES(%d, '%s', '%s')",
            OP_REGIONID, fields[0], fields[1]);
}

static int HandleDownFileLineGeneralcode(char *line, char *reserved)
{
    char            *fields[100]            = {0};

    if (sdpStringSplit(line, fields, 100, '|') != 3)
        return -1;

    return db_exec("INSERT INTO generalcode VALUES(%d, '%s', '%s', '%s')",
            OP_REGIONID, fields[0], fields[1], fields[2]);
}

//从中心下载参数
int PF_1619(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *tmpDoc             = NULL;
    char            filename[1024]      = {0};
    char            file[256]           = {0};

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
    char            sql[1024]           = {0};
    char            *p                  = NULL;
    char            settledate[9]       = {0};
    char            lastarchdate[9]     = {0};
    char            *archdate           = NULL;
    char            tmp[12]             = {0};
    char            precleardate[12]    = {0};

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
    /*以后跨工作日期的时候修改*/
#if 0 
    if (strcmp(getDate(0), precleardate) != 0)
    {
        INFO("机器日期与前一清算日期不同,不归档!");
        return 0;
    }
#endif

    // 归档日期后一天
    if ((archdate = daysafter(lastarchdate, "%Y%m%d", 1)) == NULL)
    {
        INFO("取归档日期出错[%s]", lastarchdate);
        return 0;
    }
    INFO("开始日终归档[%s]...", archdate);
    /*
       if ((ret = db_query_str(tmp, sizeof(tmp), "SELECT min(workdate) FROM trnjour")) != 0)
       return ret;
     */
    if (strcmp(archdate, precleardate) <= 0)
        sprintf(sql, "DELETE FROM htrnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    else
    {
        INFO("归档日期[%s]大于上一清算日期[%s],不需要归档,则直接返回成功", archdate, precleardate);
        //更新归档日期(以归档日期归档)
        //UpdArchivedate(archdate);
        return 0;
        //goto EXIT;
        //sprintf(sql, "DELETE FROM htrnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, precleardate, archdate);
    }
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    if (strcmp(archdate, precleardate) <= 0)
    {
        sprintf(sql, "INSERT INTO htrnjour SELECT * FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", 
                OP_REGIONID, archdate, precleardate);
    }
    else 
    {
        sprintf(sql, "INSERT INTO htrnjour SELECT * FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", 
                OP_REGIONID, precleardate, archdate);
    }
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    sprintf(tmp, "%s-0", GetWorkdate());
    INFO("更新对账通知工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettlmsgDateround(tmp)) != 0)
        return ret;

    INFO("更新已取对账工作日期和场次信息[%s]", tmp);
    if ((ret = UpdSettledDateround(tmp)) != 0)
        return ret;

EXIT:
    //更新归档日期
    UpdArchivedate(precleardate);

    // 最后才能删除当日库数据
    if (strcmp(archdate, precleardate) <= 0)
        sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    else
        sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, precleardate, archdate);
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
    xmlDoc              *opDoc          = NULL;
    xmlDoc              *tmpDoc         = NULL;
    char                setbuf[1024]    = {0};
    char                where[1024]     = {0};
    char                *p              = NULL;
    char                result[5]       = {0}, inoutflag[2]={0}, txflag[2]={0};
    int                 clearstate;

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
    xmlDoc          *opDoc              = NULL;
    char            sql[SQLBUFF_MAX]    = {0};
    char            where[512]          = {0};
    char            result[2]           = {0};

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'", 
            OP_REGIONID,
            XMLGetNodeVal(req, "//TrnCtl/WorkDate"),
            XMLGetNodeVal(req, "//TrnCtl/RefId"),
            XMLGetNodeVal(req, "//TrnCtl/Originator"),
            OP_INTRAN_FLAG);
    sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    ret = QueryTableByID(*rsp, "trnjour", 109, sql);
    if (ret != 0 && ret != E_DB_NORECORD)
        XMLSetNodeVal(*rsp, "//Content//Result", "8999");
    else if (ret == E_DB_NORECORD)
        XMLSetNodeVal(*rsp, "//Content//Result", "8059");
    else {
#if 0
        opDoc = getOPDoc();
        returnIfNull(opDoc, E_PACK_INIT);

        if (QueryTableByID(opDoc, "trnjour", 106, sql) != 0)
        {
            INFO("提入交易结果查询查本地提入记录失败");
            return E_DB;
        }
        //行内处理库需要定义该交易对应的平台交易处理程序
        ret = callProcess(opDoc, COMMTOPH_AFTER); 
#endif
        ret = db_query_str(result, sizeof(result), "select result from acctjour where %s", where);
        if (ret != 0 && ret != E_DB_NORECORD)
            XMLSetNodeVal(*rsp, "//Content//Result", "8999");
        else if (ret == E_DB_NORECORD)
            XMLSetNodeVal(*rsp, "//Content//Result", "8056");
        else if (atoi(result) == 1 || atoi(result) == 5)//5-挂账也作成功处理
        {
            XMLSetNodeVal(*rsp, "//Content//Result", "0000");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',cleardate='%s',result=%d WHERE %s",
                    CLRSTAT_SETTLED, GetRound(), XMLGetNodeVal(req, "//WorkDate"), ret, where);
        } else {
            XMLSetNodeVal(*rsp, "//Content//Result", "8999");
            XMLSetNodeVal(*rsp, "//Content//Desc", "行内记账失败");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',result=%d WHERE %s", 
                    CLRSTAT_FAILED, GetRound(), ret, where);
        }
        db_exec(sql);
    }

    return 0;
}

//提入交易状态同步(针对提出行进行同步)
int PF_0009(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    xmlDoc          *opDoc              = NULL;
    char            sql[SQLBUFF_MAX]    = {0};
    char            where[512]          = {0};
    char            *p                  = NULL;

    sprintf(where, "nodeid=%d AND workdate='%s' AND refid='%s' AND originator='%s' AND inoutflag='%s'",
            OP_REGIONID,
            XMLGetNodeVal(req, "//WorkDate"),
            XMLGetNodeVal(req, "//RefId"),
            XMLGetNodeVal(req, "//Originator"),
            OP_OUTTRAN_FLAG);
    sprintf(sql, "SELECT * FROM trnjour WHERE %s", where);

    opDoc = getOPDoc();
    returnIfNull(opDoc, E_PACK_INIT);

    if (QueryTableByID(opDoc, "trnjour", 106, sql) != 0)
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
    db_exec(sql);

    return 0;
}

//手工调账置流水
int PF_1609(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char            sSqlStr[1024]       ={0};
    char            sRefId[16+1]        ={0}, sOriginator[12+1]={0}, sInOutFlag[1+1]={0}, sSerial[8+1]={0};
    char            sOrgId[3+1]         ={0}, sDealOper[4+1]={0}, sOper[4+1]={0}, sAcctType[1+1]={0};
    char            sResult[1+1]        ={0};
    int             iAcctType;

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
    char            result[5]           = {0};

    switch (OP_TCTCODE) {
        case 3111: ret = PrintDiffNote(req, rspfile);       break; //资金清算差额
        case 3122: ret = PrintSettleResult(req, rspfile);   break; //对账结果报告单
        case 3105: ret = PrintInNoteList(req, rspfile);     break; //提入数据明细表
        case 3104: ret = PrintInJJQD(req, rspfile);         break; //提入交接清单
        case 3103: ret = PrintOutNoteList(req, rspfile);    break; //提出数据明细表
        case 3100: ret = PrintInNoteAdd(req, rspfile);      break; //提入补充凭证查询打印
        case 3130: ret = PrintInNoteAdd_COP(req, rspfile);  break; //提入补充凭证查询打印(cop)
        case 3131: ret = PrintClearTotal_PF(req, rspfile);     break; //清算业务量统计表
        case 3132: ret = PrintNoteTotal_PF(req, rspfile);      break; //清算业务种类统计表
        case 3133: ret = PrintInNoteAdd_KS(req, rspfile);      break; //来账扣税凭证打印
        case 3120: ret = PrintOutNoteTotal(req, rspfile);   break;
        case 3121: ret = PrintInNoteTotal(req, rspfile);    break;
        case 3101: ret = PrintOutJHD(req, rspfile);         break; //提出交换汇总单
        case 3102: ret = PrintOutJJQD(req, rspfile);        break; //提出交接清单
        case 8015: ret = MailQuery(req, rspfile);           break; //邮件查询
        case 3001: ret = QryOutNote(req, rspfile);          break; //提出凭证查询
        case 3002: ret = QryInNote(req, rspfile);           break; //提入凭证查询
        case 8204: ret = QryAgreementList(req, rspfile);    break; //协议信息汇总单
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

int PF_8204(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
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

int PF_3130(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3131(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3132(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3133(xmlDoc *req, xmlDoc **rsp, char *rspfile)
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

int PF_3122(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return PF_PRTQRY(req, rsp, rspfile);
}

int PF_3300(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    ST_CHINESE      mydcflag[]          = {
        { PART_DEBIT,      "借:" },
        { PART_CREDIT,     "贷:" },
        { PART_ZS,         "指示:" },
        { -1, NULL },
    };
    char            originator[13]      = {0};
    char            status[81]          = {0};
    char            traninfo[128]       = {0};
    char            settlinfo[81]       = {0};
    char            tmp[40]             = {0};
    char            *workdate           = NULL, *workround = NULL;
    result_set      rs;
    int             i, rc;

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
    if (rc = db_query_nolog(&rs, "select dcflag,count(*) as incount from trnjour "
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
//新增本地协议
int PF_1011(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    int ret=0;
    char sqlwhere[256]={0};

    sprintf(sqlwhere, "nodeid = %d and payingacct = '%s' and svcid = '%s'", OP_REGIONID, XMLGetNodeVal(req, "//AcctId"),  XMLGetNodeVal(req, "//SVCId"));
    if (db_hasrecord("agreement", sqlwhere) == TRUE)
    {
        XMLSetNodeVal(*rsp, "//Desc", "协议已存在");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else
    {
        ret = db_exec("insert into agreement values(%d, '', '', '', '%s', '', '', '%s', '%s', '', '', '', '%s', '', '', '%s', '', '%s', 0, '', '', '', '1', '', '')", 
                OP_REGIONID, 
                XMLGetNodeVal(req, "//SVCId"), 
                XMLGetNodeVal(req, "//AcctId"), 
                XMLGetNodeVal(req, "//Name"), 
                XMLGetNodeVal(req, "//Address"), 
                XMLGetNodeVal(req, "//Phone"), 
                XMLGetNodeVal(req, "//AgreementId")
                );
        if(ret)
        {
            XMLSetNodeVal(*rsp, "//Desc", "新增协议失败");
            XMLSetNodeVal(*rsp, "//Result", "8999");
        }
        else
        {
            XMLSetNodeVal(*rsp, "//Desc", "新增协议成功");
            XMLSetNodeVal(*rsp, "//Result", "0000");
        }
    }
    return 0;
}
//注销本地协议
int PF_1012(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    int ret=0;
    char sqlwhere[256]={0};

    sprintf(sqlwhere, "nodeid = %d and payingacct = '%s' and agreementid = '%s' and svcid = '%s'", 
            OP_REGIONID, 
            XMLGetNodeVal(req, "//AcctId"), 
            XMLGetNodeVal(req, "//AgreementId"), 
            XMLGetNodeVal(req, "//SVCId"));
    if (db_hasrecord("agreement", sqlwhere) != TRUE)
    {
        XMLSetNodeVal(*rsp, "//Desc", "协议不存在");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else
    {
        ret = db_exec("delete from agreement where %s", sqlwhere);
        if(ret)
        {
            XMLSetNodeVal(*rsp, "//Desc", "注销协议失败");
            XMLSetNodeVal(*rsp, "//Result", "8999");
        }
        else
        {
            XMLSetNodeVal(*rsp, "//Desc", "注销协议成功");
            XMLSetNodeVal(*rsp, "//Result", "0000");
        }
    }
    return 0;
}
//查询本地协议
int PF_1014(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    int ret=0;
    char sqlwhere[256]={0};
    char sAgreement[64]={0}, sName[81]={0};
    char sAddr[81]={0}, sPhone[21]={0};

    sprintf(sqlwhere, "select agreementid, payer, addr, phone1 from agreement where nodeid = %d and payingacct = '%s' and svcid = '%s'", 
            OP_REGIONID, 
            XMLGetNodeVal(req, "//AcctId"), 
            XMLGetNodeVal(req, "//SVCId"));
    ret = db_query_strs(sqlwhere, sAgreement, sName, sAddr, sPhone);
    if( ret == E_DB_NORECORD )
    {
        XMLSetNodeVal(*rsp, "//Desc", "无此账户的协议");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else if( ret )
    {
        XMLSetNodeVal(*rsp, "//Desc", "查询协议失败");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else
    {
        XmlSetString(*rsp, "/UFTP/AcctDetail/Name", sName);
        XmlSetString(*rsp, "/UFTP/AcctDetail/AgreementId", sAgreement);
        XmlSetString(*rsp, "/UFTP/AcctDetail/Phone", sPhone);
        XmlSetString(*rsp, "/UFTP/AcctDetail/Address", sAddr);
        XMLSetNodeVal(*rsp, "//Desc", "查询成功");
        XMLSetNodeVal(*rsp, "//Result", "0000");
    }
    return 0;
}
