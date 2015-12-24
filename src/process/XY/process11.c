#include "interface.h"
#include "nt_const.h"

/*
 * 根据交易结果判断中心清算状态
 */
int IsAcctCode(char *pRet)
{
    switch(atoi(pRet))
    {
        case 0:
            return 0;
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
            return 7;       //清算状态不确定
        default:
            return 2;
    }
}

/*
   查询记帐流水
   */
int QueryAcctSerial(xmlDocPtr opDoc, char *sSerial, char *sState)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "select acctserial, result from acctjour where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            OP_REGIONID, XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Originator"),
            XMLGetNodeVal(opDoc, "//Refid"),
            XMLGetNodeVal(opDoc, "//Inoutflag"));
    ret = DBQueryStrings(sSqlStr, 2, sSerial, sState);
    return ret;
}

/*
 * 更新记帐流水表记录为已冲正
 */
int UpAcctSerial(xmlDocPtr opDoc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "update acctjour set revserial = '%s', result = '%s' where nodeid = %d and workdate = '%s' and originator = '%s' and refid = '%s' and inoutflag = '%s'",
            XMLGetNodeVal(opDoc, "//opHostSerial"),
            "2",    //已冲正
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Originator"),
            XMLGetNodeVal(opDoc, "//Refid"),
            pInOutFlag
           );
    ret = OPDBExec(sSqlStr);
    return ret;
}

/*
 * 获取行内清算账户
 */
int GetClearAcct(char *pExchgNo, char *pAcct)
{
    int ret;
    char sSqlStr[1024]={0};

    sprintf(sSqlStr, "select clearacct from organinfo where exchno = '%s'",
            pExchgNo);
    ret = DBQueryString(pAcct, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("查询[%s]清算帐号失败", pExchgNo);
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("根据同城行号[%s]未找到行内清算帐号", pExchgNo);
        return ret;
    }
    return 0;
}

/*
 * 提入业务获取行内机构号
 */
int GetBankOrgInfo(char *pExchgNo, char *pOrgId)
{
    int ret;
    char sSqlStr[1024];

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select organid from organinfo where exchno = '%s'",
            pExchgNo);
    //memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = DBQueryString(pOrgId, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("查询行内机构号失败");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("根据同城行号[%s]未找到行内机构号", pExchgNo);
        return ret;
    }
    return 0;
}

/*
 * 提出记帐查询经办柜员号
 */
int GetInputOper(char *pSerial, char *pWorkDate, int iNode, char *pOper, char *pExchgNo)
{
    int ret;
    char sSqlStr[1024];

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select acctoper from trnjour where inoutflag = '1' and refid = '%s' \
            and workdate = '%s' and nodeid = %d and originator = '%s'",
            pSerial, pWorkDate, iNode, pExchgNo);
    BKINFO("QUERY INPUTOPER SQL:%s", sSqlStr);
    ret = DBQueryString(pOper, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("查询交易经办柜员失败");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("交易不存在");
        return ret;
    }
    return ret;
}

/*
 * 组织1102转换报文
 * 提出借 (转帐支票,定期借记,3省1市汇票)
 */
int PACKCHG_1102(xmlDoc *doc, char *p)
{
    int ret;
    char sSerial[20+1], sDate[8+1], sOper[6+1];
    char sExchgNo[12+1]={0}, sClearAcct[32+1]={0};

    /*复核记帐时，记帐柜员需送经办员号，所以需查询经办柜员*/
    BKINFO("获取经办柜员号...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //提出
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    ret = GetClearAcct(sExchgNo, sClearAcct);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//Payacct", sClearAcct);

    return 0;
}

/*
 * 组织1106转换报文
 * 向行内冲正
 * pInOutFlag 1-提出业务 进行冲正 2-提入业务 进行冲正
 */
int PACKCHG_1106(xmlDoc *doc, char *pInOutFlag)
{
    int ret;
    char sSqlStr[1024]={0};
    char sExchgNo[12+1]={0}, sHnOrgId[3+1]={0};
    char sSerial[20+1]={0}, sOper[6+1]={0};

    BKINFO("提入交易获取行内机构...");
    if(pInOutFlag[0] == '2')        //提入交易冲正
        strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    else                                    //提出交易冲正
        strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    sprintf(sSqlStr, "select acctserial from acctjour where workdate = '%s' and refid = '%s' \
            and originator = '%s' and inoutflag = '%s'",
            GetWorkdate(), XMLGetNodeVal(doc, "//Refid"), XMLGetNodeVal(doc, "//Originator"), pInOutFlag);
    ret = DBQueryString(sSerial, sSqlStr);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("查询原交易记帐流水失败");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("原交易记帐流水不存在");
        return ret;
    }
    XMLSetNodeVal(doc, "//opHreserved1", sSerial);
    memcpy(sOper, sSerial, 4);
    XMLSetNodeVal(doc, "//opOperid", sOper);
    return 0;
}

/*
 * 组织1202转换报文
 * 验证支付密码
 */
int PACKCHG_1202(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];
    char sNoteNo[20+1], sNoteType[2+1], sDCFlag[1+1], sHnType[2+1], sHnNo[20+1];
    char sPasswd[20+1]={0};

    BKINFO("提入交易获取行内机构...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, "1");
    //strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //票据号码 票据种类转换
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);

    strcpy(sPasswd, XMLGetNodeVal(doc, "//Agreement")); //中心支付密码存放节点
    if(strlen(sPasswd) != 0)
        XMLSetNodeVal(doc, "//Paykey", sPasswd);        //前台验密支付密码存放节点(报文转换节点)

    return 0;
}

/*
 * 组织1621转换报文 南通暂不使用
 * 提出借(非截留票据)
 */
int PACKCHG_1621(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * 组织1622转换报文
 * 提出贷 (支票,特转,来帐代转)
 */
int PACKCHG_1622(xmlDoc *doc, char *p)
{
    int ret;
    char sSqlStr[1024];
    char sSerial[20+1], sDate[8+1], sOper[6+1], sDCFlag[1+1]={0};
    char sNoteType[2+1]={0}, sNoteNo[20+1]={0}, sHnType[2+1]={0}, sHnNo[20+1]={0};
    char sExchgNo[12+1]={0};

    /*复核记帐时，记帐柜员需送经办员号，所以需查询经办柜员*/
    BKINFO("获取经办柜员号...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //提出
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    //凭证号码为空行内凭证种类为99（其他凭证)
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    //凭证号码不为空且不为0时，凭证号码需转换
    ret = trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    if(ret)
    {
        BKINFO("凭证种类,凭证号码转换失败");
        return ret;
    }
    BKINFO("凭证种类:%s", sHnType);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    //凭证种类为2（转帐支票）且为贷记业务时 该票据相当于进帐单
    if((atoi(sNoteType) == NOTE_CHECK || atoi(sNoteType) == NOTE_REMIT) && atoi(sDCFlag) == 2)
    {
        XMLSetNodeVal(doc, "//Issueamt", "0");
        //XMLSetNodeVal(doc, "//Notetype", sHnType);
        XMLSetNodeVal(doc, "//Noteno", sHnNo);
    }
    else
    {
        XMLSetNodeVal(doc, "//Notetype", "99");
        XMLSetNodeVal(doc, "//opEXBKDrawType", "3");   //3 凭印签取款
        XMLSetNodeVal(doc, "//Noteno", sNoteNo+strlen(sNoteNo)-7);
    }
    return 0;
}

/*
 * 组织1623转换报文
 * 提入借 截留(支票,税款缴款书,定期借/贷)
 */
int PACKCHG_1623(xmlDoc *doc, char *p)
{
    int ret;
    char sNoteType[2+1], sNoteNo[20+1], sHnType[2+1], sHnNo[20+1];
    char sDCFlag[1+1], sExchgNo[12+1], sHnOrgId[3+1];

    BKINFO("提入交易获取行内机构...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    XMLSetNodeVal(doc, "//Issueamt", "0");  //限额

    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //票据号码 票据种类转换
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);
    if(atoi(sNoteType) == 2 && atoi(sDCFlag) == 1)            //行内支票需凭密  1-借
        XMLSetNodeVal(doc, "//opEXBKDrawType", "1");   //凭密
    else
        XMLSetNodeVal(doc, "//opEXBKDrawType", "3");   //凭签
    //XMLSetNodeVal(doc, "//Memo", "103");

    return 0;
}

/*
 * 组织1624转换报文
 * 提入贷 (支票,来帐代转,特转,网银)
 */
int PACKCHG_1624(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];

    BKINFO("提入交易获取行内机构...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);
    return 0;
}

/*
 * 组织1625转换报文 南通暂不用
 * 他行退我行票(提出被退票)
 */
int PACKCHG_1625(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * 组织1626转换报文 南通暂不用
 * 我行退他行票 借贷分别与1623、1624配合使用
 */
int PACKCHG_1626(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * 组织1631转换报文 南通暂不使用
 * 用于非本网点帐号进行提出借收妥记帐
 */
int PACKCHG_1631(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * 组织1632转换报文
 * 提入贷 (信电汇)
 */
int PACKCHG_1632(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1];

    BKINFO("提入交易获取行内机构...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    XMLSetNodeVal(doc, "//Noteno", "");

    return 0;
}

/*
 * 组织1637转换报文
 * 提出贷 (汇款凭证,网银)
 */
int PACKCHG_1637(xmlDoc *doc, char *p)
{
    return 0;
}

/*
 * 组织1643转换报文
 * 提入借 (截留银行本票)
 */
int PACKCHG_1643(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1], sExchgNo[12+1], sDCFlag[1+1];
    char sNoteType[2+1], sNoteNo[20+1], sHnType[2+1], sHnNo[20+1], sHnHpNo[20+1];

    BKINFO("提入交易获取行内机构...");
    memset(sExchgNo, 0, sizeof sExchgNo);
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    memset(sHnOrgId, 0, sizeof sHnOrgId);
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    memset(sNoteType, 0, sizeof sNoteType);
    strcpy(sNoteType, XMLGetNodeVal(doc, "//Notetype"));
    memset(sNoteNo, 0, sizeof sNoteNo);
    strcpy(sNoteNo, XMLGetNodeVal(doc, "//Noteno"));
    memset(sDCFlag, 0, sizeof sDCFlag);
    strcpy(sDCFlag, XMLGetNodeVal(doc, "//Dcflag"));
    memset(sHnType, 0, sizeof sHnType);
    memset(sHnNo, 0, sizeof sHnNo);
    //票据号码 票据种类转换
    trans_pzxh(sNoteNo, sNoteType, sDCFlag, sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Notetype", sHnType);
    XMLSetNodeVal(doc, "//Noteno", sHnNo);
    memset(sHnHpNo, 0, sizeof sHnHpNo);
    sprintf(sHnHpNo, "%s%s", sHnType, sHnNo);
    XMLSetNodeVal(doc, "//Reserved", sHnHpNo);

    return 0;
}

/*
 * 组织1648转换报文
 * 提出借 (截留银行本票)
 */
int PACKCHG_1648(xmlDoc *doc, char *p)
{
    int ret;
    char sSerial[20+1], sDate[8+1], sOper[6+1];
    char sExchgNo[12+1]={0};

    BKINFO("获取经办柜员号...");
    memset(sSerial, 0, sizeof sSerial);
    strcpy(sSerial, XMLGetNodeVal(doc, "//Refid"));
    memset(sDate, 0, sizeof sDate);
    //strcpy(sDate, XMLGetNodeVal(doc, "//opWorkdate"));
    strcpy(sDate, GetWorkdate());
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Originator"));
    //提出
    memset(sOper, 0, sizeof sOper);
    ret = GetInputOper(sSerial, sDate, OP_REGIONID, sOper, sExchgNo);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opOperid", sOper);

    return 0;
}

/*
   查询户名与2380交易一样
   */
int PACKCHG_3380(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1]={0}, sExchgNo[12+1]={0};

    BKINFO("提入交易获取行内机构...");
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);

    return 0;
}

/*
 * 组织6101转换报文 
 * 提入借(定期借/贷,税款缴款书)交易协议号检查
 */
int PACKCHG_6101(xmlDoc *doc, char *p)
{
    int ret;
    char sHnOrgId[3+1]={0}, sExchgNo[12+1]={0};

    BKINFO("提入交易获取行内机构...");
    strcpy(sExchgNo, XMLGetNodeVal(doc, "//Acceptor"));
    ret = GetBankOrgInfo(sExchgNo, sHnOrgId);
    if(ret)
        return ret;
    XMLSetNodeVal(doc, "//opBankno", sHnOrgId);
    return 0;
}

/*
 * 录入记帐
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF11_100001(void *doc, char *p)
{
    return 0;
}

/*
 * 复核记帐
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF11_100002(void *doc, char *p)
{
    int ret;
    int iNoteType,iTrnCode;
    xmlDoc *opDoc = (xmlDoc *)doc;
    char sNoteType[2+1]={0}, sDCFlag[1+1]={0}, sCurCode[5+1]={0};
    char sResult[8+1]={0}, sTrnCode[6+1]={0}, sNoteNo[20+1]={0};

    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//Curcode"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//Notetype"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    iNoteType = atoi(sNoteType);

    BKINFO("凭证种类:%d 借贷标志:%s 与中心通讯标志:%s", iNoteType, sDCFlag, p);
    //重新赋值凭证号码，网点送过来的是左补0的12位凭证号码
    //行内需要的是8位凭证号码
    strcpy(sNoteNo, XMLGetNodeVal(opDoc, "//Noteno"));
    if(strlen(sNoteNo) > 8)
        XMLSetNodeVal(opDoc, "//Noteno", sNoteNo+strlen(sNoteNo)-8);
    //与中心通讯后 判断中心返回的结果来处理 如果是提出贷失败 需冲正
    if(p[0] == COMMTOPH_AFTER[0])
    {
        //中心返回结果
        strcpy(sResult, XMLGetNodeVal(opDoc, "//opTCRetcode")); 
        if(atoi(sResult) != 0)
        {
            ret = IsAcctCode(sResult);
            if(ret == 7)            //清算状态不确定
                return E_SYS_COMM;
            if(sDCFlag[0] == FL_CREDIT[1] && memcmp(sCurCode, "CNY", 3) == 0)
            {
                ret = PACKCHG_1106(opDoc, "1");                   //冲正
                iTrnCode = 1106;
                goto ACCT;
            }
            return E_APP_NONEEDACCOUNT;           //行内不记帐返回 交易失败
        }
    }
    //外币不自动记帐
    if(memcmp(sCurCode, "CNY", 3) != 0)
        return 0;

    if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_DEBIT[1] 
            && p[0] == COMMTOPH_AFTER[0])                    //提出借支票中心返回
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    else if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_CREDIT[1]
            && p[0] == COMMTOPH_BEFORE[0])                   //提出贷支票发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_PROM && p[0] == COMMTOPH_AFTER[0])    //提出借本票中心返回
    {
        ret = PACKCHG_1648(opDoc, p);
        iTrnCode = 1648;
    }
    else if(iNoteType == NOTE_ZONE_DRAFT && p[0] == COMMTOPH_AFTER[0])  //提出借3省1市汇票中心返回
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    else if(iNoteType == NOTE_CORRXFER && p[0] == COMMTOPH_BEFORE[0])   //提出贷来帐代转发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_SPCXFER && p[0] == COMMTOPH_BEFORE[0])    //提出贷特转发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_NETBANK && p[0] == COMMTOPH_BEFORE[0])    //提出贷网银发送中心
    {
        ret = PACKCHG_1637(opDoc, p);
        iTrnCode = 1637;
    }
    else if(iNoteType == NOTE_REMIT && p[0] == COMMTOPH_BEFORE[0])      //提出贷汇款凭证发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_DRAFT && p[0] == COMMTOPH_BEFORE[0])      //提出贷全国银行汇票发送中心
    {
        ret = PACKCHG_1643(opDoc, p);
        iTrnCode = 1643;
    }
    else if(iNoteType == NOTE_CONPAY && p[0] == COMMTOPH_BEFORE[0])     //提出贷代理集中支付发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_COLLECTION && p[0] == COMMTOPH_BEFORE[0])   //提出贷托收凭证发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    else if(iNoteType == NOTE_PDC && p[0] == COMMTOPH_BEFORE[0] && sDCFlag[0] == FL_CREDIT[1])   //提出贷定期借/贷发送中心
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    //税费缴款书提出贷发送中心
    else if(iNoteType == NOTE_TAXPAY && p[0] == COMMTOPH_BEFORE[0] && sDCFlag[0] == FL_CREDIT[1])
    {
        ret = PACKCHG_1622(opDoc, p);
        iTrnCode = 1622;
    }
    //提出借定期借/贷(借)发送中心
    else if(iNoteType == NOTE_PDC && p[0] == COMMTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }
    /*税费缴款书提出贷发送中心
    else if(iNoteType == NOTE_TAXPAY && p[0] == COMMTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    {
        ret = PACKCHG_1102(opDoc, p);
        iTrnCode = 1102;
    }*/
    else
    {
        if(p[0] == COMMTOPH_BEFORE[0])
            BKINFO("发送中心前不记帐");
        else if(p[0] == COMMTOPH_AFTER[0])
        {
            BKINFO("发送中心后不记帐");
            return E_APP_ACCOUNTSUCC;
        }
        return 0;
    }

ACCT:
    if(ret)
    {
        BKINFO("行内报文组织失败");
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "行内冲正失败" : "行内记帐失败");
        return iTrnCode == 1106 ? E_APP_CZFAIL : E_APP_ACCOUNTFAIL;
    }

    //向行内记帐
    ret = callInterface(iTrnCode, opDoc);
    if(ret)
        return ret;
    memset(sResult, 0, sizeof sResult);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
    {
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "行内冲正失败" : "行内记帐失败");
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }
    //else
        //XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "行内冲正成功" : "行内记帐成功");

    //如果是冲正交易不记录冲正流水
    if(iTrnCode == 1106)
    {
        ret = UpAcctSerial(opDoc, "1");
        return E_APP_ACCOUNTANDCZ;
    }
    sprintf(sTrnCode, "%d", iTrnCode);
    XMLSetNodeVal(opDoc, "//opOreserved2", sTrnCode);
    XMLSetNodeVal(opDoc, "//opRetinfo", vstrcat("柜员流水%s", XMLGetNodeVal(opDoc, "//opHostSerial")));
    ret = InsertAcctjour(opDoc);
    BKINFO("记录记帐流水表%s", ret == 0 ? "成功" : "失败");

    return E_APP_ACCOUNTSUCC;
}

/*
   同步记帐
   */
int PF11_100010(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sSerial[8+1]={0}, sState[1+1]={0};

    ret = QueryAcctSerial(opDoc, sSerial, sState);
    if(ret == 0)
    {
        if(atoi(sState) == 1)
            return E_APP_ACCOUNTSUCC;
        else if(atoi(sState) == 2)
            return E_APP_CZSUCC;
        else
            return E_APP_ACCOUNTFAIL;
    }
    else if(ret == E_DB_NORECORD)
    {
        ret = PF11_100002(opDoc, p);
        return ret;
    }
    else
        return E_APP_ACCOUNTFAIL;
    //return PF11_100002(doc, p);
}

/*
 * 提入记帐
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF11_100005(void *doc, char *p)
{
    int ret;
    int iNoteType, iTrnCode;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sResult[8+1]={0}, sCurCode[5+1]={0}, sNoteNo[20+1]={0};
    char sNoteType[2+1]={0}, sTrnCode[6+1]={0}, sDCFlag[1+1]={0};
    char sAcctName[50+1]={0};
    char *AcctCheck;

    strcpy(sCurCode, XMLGetNodeVal(opDoc, "//Curcode"));
    strcpy(sNoteType, XMLGetNodeVal(opDoc, "//Notetype"));
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    iNoteType = atoi(sNoteType);

    BKINFO("凭证种类:%d 借贷标志:%s 与中心通讯标志:%s 币种:%s", iNoteType, sDCFlag, p, sCurCode);
    //外币不自动记帐
    if(memcmp(sCurCode, "CNY", 3) != 0)
    {
        if(iNoteType == NOTE_FORCHECK && sDCFlag[0] == FL_DEBIT[1]) //提入借外币转帐支票需验密
        {
            ret = PACKCHG_1202(opDoc, p);
            iTrnCode = 1202;
            goto ACCT;
        }
        return 0;
    }
    //如果提入交易的凭证长度大于8位，送行内时需截取
    strcpy(sNoteNo, XMLGetNodeVal(opDoc, "//Noteno"));
    if(strlen(sNoteNo) > 8)
        XMLSetNodeVal(opDoc, "//Noteno", sNoteNo+strlen(sNoteNo)-8);
    //与中心交易失败 如果是提入借 行内需冲正
    //if(p[0] == COMMRSPTOPH_AFTER[0] && sDCFlag[0] == FL_DEBIT[1])
    if(p != NULL && sDCFlag[0] == FL_DEBIT[1])
    {
        if(p[0] == COMMRSPTOPH_AFTER[0])
        {
            ret = PACKCHG_1106(opDoc, "2");
            iTrnCode = 1106;
            goto ACCT;
        }
    }

    /*提入贷检查帐号户名 不管有没有记帐提入贷都需成功*/
    if(sDCFlag[0] == FL_CREDIT[1])
    {
        //获取账户检查标志
        AcctCheck = XMLGetNodeVal(doc, "//AcctCheck");
        if(AcctCheck != NULL)
        {
            if(atoi(AcctCheck) == 1)
            {
                ret = PACKCHG_3380(doc, p);
                ret = callInterface(3380, doc);
                if(ret)
                    return E_APP_ACCOUNTSUCC;
                memset(sResult, 0, sizeof sResult);
                strcpy(sResult, XMLGetNodeVal(doc, "//opBKRetcode"));
                if(atoi(sResult) != 0)
                {
                    BKINFO("查询户名失败");
                    return E_APP_ACCOUNTSUCC;
                }
                strcpy(sAcctName, XMLGetNodeVal(doc, "//opOreserved2"));
                if(strcmp(sAcctName, XMLGetNodeVal(doc, "//Benename")) != 0)
                {
                    BKINFO("户名不符;接收户名:%s 行内户名:%s", XMLGetNodeVal(doc, "//Benename"), sAcctName);
                    return E_APP_ACCOUNTSUCC;
                }
            }
        }
    }

    //BKINFO("凭证种类:%d 借贷标志:%s 与中心通讯标志:%s", iNoteType, sDCFlag, p);
    if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_DEBIT[1])    //转帐支票提入借
    {
        ret = PACKCHG_1623(opDoc, p);
        iTrnCode = 1623;
    }
    else if(iNoteType == NOTE_CHECK && sDCFlag[0] == FL_CREDIT[1])  //转帐支票提入贷
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_CORRXFER)  //来账代转补充凭证提入
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_SPCXFER)  //特种转帐凭证提入
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_NETBANK)  //网银
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_DRAWBACK) //税款退还书
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_FP)       //财政国库支付凭证
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_COLLECTION)   //托收凭证
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_CONPAY)       //代理集中支付
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_PDC && sDCFlag[0] == FL_CREDIT[1])    //定期借/贷(贷)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    //定期借/贷(借) 税费缴款书
    else if((iNoteType == NOTE_PDC || iNoteType == NOTE_TAXPAY) && sDCFlag[0] == FL_DEBIT[1])
    {
        //先检查协议号
        BKINFO("向行内检查协议号...");
        ret = PACKCHG_6101(opDoc, p);
        if(ret)
            E_APP_ACCOUNTFAIL;
        ret = callInterface(6101, opDoc);
        if(ret)
            return ret;
        if(atoi(XMLGetNodeVal(opDoc, "//opBKRetcode")) != 0)
        {
            BKINFO("付款帐号与协议号不符");
            return E_APP_ACCOUNTFAIL;
        }
        ret = PACKCHG_1623(opDoc, p);
        iTrnCode = 1623;
    }
    else if(iNoteType == NOTE_DRAFT)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else if(iNoteType == NOTE_REMIT)
    {
        ret = PACKCHG_1624(opDoc, p);
        iTrnCode = 1624;
    }
    else
    {
        BKINFO("此凭证记帐方式未定");
        if(sDCFlag[0] == FL_DEBIT[1])
            return E_APP_ACCOUNTFAIL;
        else
            return 0;
    }

ACCT:
    if(ret)
    {
        BKINFO("行内报文组织失败");
        XMLSetNodeVal(opDoc, "//Desc", iTrnCode == 1106 ? "行内冲正失败" : "行内记帐失败");
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }

    ret = callInterface(iTrnCode, opDoc);
    if(ret)
        return ret;

    memset(sResult, 0, sizeof sResult);
    strcpy(sResult, XMLGetNodeVal(opDoc, "//opBKRetcode"));
    if(atoi(sResult) != 0)
    {
        BKINFO("行内%s失败", iTrnCode == 1106 ? "冲正" : "记帐");
        if(iTrnCode != 1106 && sDCFlag[0] == FL_CREDIT[1])
            return 0;
        return iTrnCode == 1106 ? E_APP_ACCOUNTNOCZ : E_APP_ACCOUNTFAIL;
    }
    else
        BKINFO("行内%s成功", iTrnCode == 1106 ? "冲正" : "记帐");

    sprintf(sTrnCode, "%d", iTrnCode);
    XMLSetNodeVal(opDoc, "//opOreserved2", sTrnCode);
    ret = InsertAcctjour(opDoc);
    BKINFO("记录记帐流水表%s", ret == 0 ? "成功" : "失败");

    return E_APP_ACCOUNTSUCC;
}

/*
   交易查询行内账务处理
   */
int PF11_100008(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sTcResult[4+1]={0}, sDCFlag[1+1]={0};
    char sOpState[1+1]={0}, sHnOrgId[3+1]={0};
    char sSerial[8+1]={0}, sResult[8+1]={0};

    strcpy(sTcResult, XMLGetNodeVal(opDoc, "//opTCRetcode"));
    ret = IsAcctCode(sTcResult);
    if(ret == 7)
        return CLRSTAT_UNKNOW;
    ret = GetBankOrgInfo(atoi(XMLGetNodeVal(opDoc, "//Inoutflag")) == 1 ? XMLGetNodeVal(opDoc, "//Originator") : XMLGetNodeVal(opDoc, "//Acceptor"), sHnOrgId);
    if(ret)
        return atoi(sTcResult) == 0 ? CLRSTAT_SETTLED : CLRSTAT_FAILED;
    XMLSetNodeVal(opDoc, "//opBankno", sHnOrgId);

    sprintf(sSqlStr, "select clearstate from trnjour where nodeid = %d and workdate = '%s' and refid = '%s' \
            and inoutflag = '%s' and originator = '%s'",
            OP_REGIONID,
            XMLGetNodeVal(opDoc, "//opWorkdate"),
            XMLGetNodeVal(opDoc, "//Refid"),
            XMLGetNodeVal(opDoc, "//Inoutflag"),
            XMLGetNodeVal(opDoc, "//Originator"));
    ret = DBQueryString(sOpState, sSqlStr);
    if(ret)
    {
        BKINFO("查询原记录清算状态失败");
        return atoi(sTcResult) == 0 ? CLRSTAT_SETTLED : CLRSTAT_FAILED;
    }
    strcpy(sDCFlag, XMLGetNodeVal(opDoc, "//Dcflag"));
    BKINFO("行内清算状态:%s 中心清算结果:%s 借贷记:%s", sOpState, sTcResult, sDCFlag);
    //行内清算成功 中心清算成功 返回清算成功
    if(sOpState[0] == CLRSTAT_SETTLED && atoi(sTcResult) == 0)
    {
        /*
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        //没有记帐流水 或 有记帐流水但不是记帐成功状态需补记
        if(ret == E_DB_NORECORD || (ret == 0 && atoi(sResult) != 1))
        {
            BKINFO("未记帐,行内交易需补记");
            ret = PF11_100002(opDoc, atoi(sDCFlag) == 2 ? COMMTOPH_BEFORE : COMMTOPH_AFTER);
            return CLRSTAT_SETTLED;
        }*/
        return CLRSTAT_SETTLED;
    }
    //行内清算失败 中心清算失败 返回清算失败
    else if(sOpState[0] == CLRSTAT_FAILED && atoi(sTcResult) != 0)
    {
        /*
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        //记帐成功 需 冲正
        if(ret == 0 && atoi(sResult) == 1)
        {
            BKINFO("已记帐,行内交易需冲正");
            ret = PACKCHG_1106(opDoc, "1");     //查询发起方冲正
            if(ret)
            {
                BKINFO("组冲正报文失败，冲正失败");
                return CLRSTAT_FAILED;
            }
            ret = callInterface(1106, opDoc);
            if(ret)
                BKINFO("冲正失败");
            return CLRSTAT_FAILED;
        }*/
        return CLRSTAT_FAILED;
    }
    //行内清算失败 中心清算成功 需补记
    else if(sOpState[0] == CLRSTAT_FAILED && atoi(sTcResult) == 0)
    {
        ret = QueryAcctSerial(opDoc, sSerial, sResult);
        if(ret == E_DB_NORECORD || (ret == 0 && atoi(sResult) != 1))
        {
            BKINFO("未记帐或记帐失败,行内交易需补记");
            ret = PF11_100002(opDoc, atoi(sDCFlag) == 2 ? COMMTOPH_BEFORE : COMMTOPH_AFTER);
            return CLRSTAT_SETTLED;
        }
        return CLRSTAT_SETTLED;
    }
    //行内清算成功 中心清算失败 需冲正
    else if(sOpState[0] == CLRSTAT_SETTLED && atoi(sTcResult) != 0)
    {
        //提出交易冲正
        BKINFO("行内交易需冲正");
        ret = PACKCHG_1106(opDoc, "1");
        if(ret)
        {
            BKINFO("组冲正报文失败，冲正失败");
            return CLRSTAT_FAILED;
        }
        ret = callInterface(1106, opDoc);
        if(ret)
            BKINFO("冲正失败");
        return CLRSTAT_FAILED;
    }
    //行内清算未知 中心清算成功 提出贷不处理
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) == 0 && atoi(sDCFlag) == 2)
        return CLRSTAT_SETTLED;
    //行内清算未知 中心清算成功 提出借需补记
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) == 0 && atoi(sDCFlag) == 1)
    {
        BKINFO("提出借行内交易需补记");
        ret = PF11_100002(opDoc, COMMTOPH_AFTER);
        return CLRSTAT_SETTLED;
    }
    //行内清算未知 中心清算失败 提出借不处理
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) != 0 && atoi(sDCFlag) == 1)
        return CLRSTAT_FAILED;
    //行内清算未知 中心清算失败 提出贷需冲正
    else if(sOpState[0] == CLRSTAT_UNKNOW && atoi(sTcResult) != 0 && atoi(sDCFlag) == 2)
    {
        BKINFO("提出贷行内交易需冲正");
        ret = PACKCHG_1106(opDoc, "1");
        if(ret)
        {
            BKINFO("组冲正报文失败，冲正失败");
            return CLRSTAT_FAILED;
        }
        ret = callInterface(1106, opDoc);
        if(ret)
            BKINFO("冲正失败");
        return CLRSTAT_FAILED;
    }
    return 0;
}

/*
   中心向行内查询交易结果
   */
int PF11_100009(void *doc, char *p)
{
    int ret;
    xmlDoc *opDoc=(xmlDoc *)doc;
    char sSqlStr[1024]={0};
    char sSerial[8+1]={0}, sState[1+1]={0};

    ret = QueryAcctSerial(opDoc, sSerial, sState);
    if(ret == 0)
    {
        if(atoi(sState) == 1)
            return E_APP_ACCOUNTSUCC;
        else if(atoi(sState) == 2)
            return E_APP_CZSUCC;
        else
            return E_APP_ACCOUNTFAIL;
    }
    else if(ret == E_DB_NORECORD)
    {
        ret = PF11_100005(opDoc, "");
        return ret;
    }
    else
        return E_APP_ACCOUNTFAIL;
}

/*
 * 票据合法性检查
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF11_100007(void *doc, char *p)
{
    int ret;
    ret = PACKCHG_1202(doc, p);
    return callInterface(1202, (xmlDoc *)doc);
}

/*
 * 户名查询
 * 输入 doc 平台报文
 *      p 保留
 * 输出 pret 函数返回值(0成功 非0失败)
 *      plen 保留
 * 返回 NULL
 */
int PF11_400001(void *doc, char *p)
{
    return callInterface(2380, (xmlDoc *)doc);
}

/*
 * 获取地区代号
 */
int HFGetRegionIdInfo(void *src, char *dst)
{
    strcpy(dst, "40");
    return 0;
}

/*
 * 获取终端代号
 */
int HFGetTermNoInfo(void *src, char *dst)
{
    switch(OP_REGIONID)
    {
        case 10:        //苏州  
            strcpy(dst, "ttysztc ");
            break;
        case 11:        //南通  
            strcpy(dst, "ttynttc ");
            break;
        case 12:        //无锡  
            strcpy(dst, "ttywxtc ");
            break;
        case 13:        //常州  
            strcpy(dst, "ttycztc ");
            break;
        default:
            strcpy(dst, "ttyjstc ");
            break;  
    }
    return 0;
}

/* 
   把前台8位凭证号变成7位,单式凭证只能输七位,并且不用十六进制存放
   本式凭证必须输8位,并且用十六进制存放 (参考conpz8to7.src文件)
 */
void NoteNo2PZXH(char *eight, char *seven)
{
    int val = atoi(eight), i = 0;
    char ch;
    char sevenBak[8];
    BKINFO("eight:%s", eight);
    while(val > 0)
    {
        ch = val % 16;
        if(ch < 10)
            ch += '0';
        else 
            ch += 'a'-10;
        sevenBak[i++] = ch;
        val /= 16;
    }
    sevenBak[i] = 0;
    BKINFO("servnBak:%s i:%d", sevenBak, i);
    for (i=0; i<7; i++)
        seven[i] = sevenBak[6-i];
    BKINFO("pzxh:%s", seven);
}
/* 根据同城凭证类型和凭证号码转成行内记账凭证类型和凭证序号 */
int trans_pzxh(char *pnoteno, char *notetype, char *dcflag, char *hntype, char *pzxh)
{
    int ret;
    char sSqlStr[1024];
    char Snotetype[3];
    char Sdsbs[3];
    char Spzdjbz[2];
    char Spzzl[3];
    int pzzl;
    char tmp[30];
    char *noteno = NULL;
    
    noteno = pnoteno + strlen(pnoteno) - 8;
    BKINFO("NOTENO:[%s]", noteno);

    memset(sSqlStr, 0, sizeof sSqlStr);
    sprintf(sSqlStr, "select banktype, reserved1, dcflag from notetypemap where tctype = '%s' \
            and nodeid = %d and dcflag = '%s'",
            notetype, OP_REGIONID, dcflag);
    memset(Spzzl, 0, sizeof Spzzl);
    memset(Sdsbs, 0, sizeof Sdsbs);
    memset(Spzdjbz, 0, sizeof Spzdjbz);
    ret = DBQueryStrings(sSqlStr, 3, Spzzl, Sdsbs, Spzdjbz);
    if(ret && ret != E_DB_NORECORD)
    {
        BKINFO("查询行内凭证种类失败");
        return ret;
    }
    else if(ret == E_DB_NORECORD)
    {
        BKINFO("没有对应的行内凭证类型");
        return ret;
    }
    pzzl = atoi(Spzzl);
    memcpy(hntype, Spzzl, 2); //转成行内凭证种类

    //memcpy(pzxh, noteno+strlen(noteno)-7, 7); 默认取后7位(具体默认凭证序号需要再决定)
    if(strlen(noteno) >7)
        memcpy(tmp, noteno+strlen(noteno)-7, 7);
    else
        memcpy(tmp, noteno, strlen(noteno));
    sprintf(pzxh, "%07d", atoi(tmp));

    BKINFO("Spzdjbz:%s pzzl:%d reserved1:%s", Spzdjbz, pzzl, Sdsbs);
    if (Spzdjbz[0] == '2' || pzzl == 60 || pzzl == 61 || pzzl == 62 || pzzl == 83 || Spzdjbz[0] == '1')
    {
        BKINFO("NOTENO LEN:%d NOTENO:%s", strlen(noteno), noteno);
        if (Sdsbs[0] == '0') //单式凭证
        {
            memset(tmp, 0, sizeof tmp);
            if(strlen(noteno) > 7)
                memcpy(tmp, noteno+strlen(noteno)-7, 7);
            else
                memcpy(tmp, noteno, strlen(noteno));
            //还有更多判断(参考postPZXH_ZG.src)
            //memcpy(pzxh, noteno+strlen(noteno)-7, 7);
            sprintf(pzxh, "%07d", atoi(tmp));
        }
        else //本式凭证
        {
            if (strlen(noteno) != 8) //起始序号必须为八位
                return -1;
            else
                NoteNo2PZXH(noteno, pzxh);
        }
    }

    return 0;
}
