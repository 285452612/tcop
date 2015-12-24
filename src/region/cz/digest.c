#include "region_common.h"

int MailKeyMake( xmlDocPtr doc )
{
    int ret;
    int len ;
    char DealData[2048], mac[17];
    char payingacct[33], beneacct[33],noteno[13];
    char profile[60], buff[40];
    char BankId[13];
    char AuthDevNo[21];
    char Buf[128];
    int myjtype;

    //从xml报文中读取各要素信息，组成校验字符串
    memset(DealData, 0 , sizeof(DealData));
    sprintf( DealData, "%-12.12s%08d%-32.32s%-32.32s%016.0lf%s",
            xmlGetVal(doc, "//MailInfo/Content/NoteNo", noteno ) ,
            XmlGetInteger(doc, "//MailInfo/Content/IssueDate" ),
            xmlGetVal( doc, "//MailInfo/Content/BeneAcct",beneacct ),
            xmlGetVal( doc, "//MailInfo/Content/PayingAcct",payingacct),
            100 * XmlGetFloat(doc, "//MailInfo/Content/SettlAmt" ),  
            XMLGetNodeVal( doc, "//MailInfo/Content/Purpose"));

    len = strlen(DealData);
    memset(mac, 0 , sizeof(mac));
    ret = GenerateCode(DealData, len, mac);
    if(ret)
    {
        DBUG("MailKeyMake operation fail. ret=%d.AuthDevNo=%s,DealData=%s,len=%d,mac=%s ", ret,AuthDevNo,DealData,len,mac );
        return ret;
    }
    XMLSetNodeVal(doc,"//MailInfo/Content/PayKey",mac);

    return 0;
}

int TestKeyMake_New( xmlDocPtr doc, char *AuthDevNo )
{
    int ret;
    int len ;
    char DealData[1024], mac[17];
    char payingacct[33], beneacct[33],noteno[13];
    char profile[60], buff[40];
    int myjtype;

    //从xml报文中读取各要素信息，组成校验字符串
    INFO("加押...");
    memset(DealData, 0 , sizeof(DealData));
    sprintf( DealData, "%-12.12s%08d%-32.32s%-32.32s%016.0lf",
            xmlGetVal(doc, "//NoteInfo/NoteNo", noteno ) ,
            XmlGetInteger(doc, "//NoteInfo/IssueDate" ),
            xmlGetVal( doc, "//NoteInfo/BeneAcct",beneacct ),
            xmlGetVal( doc, "//NoteInfo/PayingAcct",payingacct ),
            100 * XmlGetFloat(doc, "//NoteInfo/SettlAmt" )  );
    DBUG("加押原文:%s", DealData);

    len = strlen(DealData);
    //从xml报文中读取密押
    memset(mac, 0 , sizeof(mac));

    ret = GenerateCode(DealData, len, mac);
    if(ret)
    {
        DBUG( "TestKeyMake_New operation fail. ret=%d.AuthDevNo=%s,DealData=%s,len=%d,mac=%s ", ret,AuthDevNo,DealData,len,mac );
        return ret;
    }
    SetNoteInfo("PayKey",mac,doc);

    return 0;
}

int DWTestKeyMake( xmlDocPtr doc, char *AuthDevNo )
{
    char DealData[2048];
    char Secret[50];
    char BankId[13];
    int ret = 0;
    char noteno[13], payingacct[33], beneacct[33],agreement[21];
    char Buf[128];
    int len, myjtype;
    char profile[60], buff[40];
    

    //从xml报文中读取各要素信息，组成校验字符串
    //xmlGetVal(doc, "//NoteInfo/AuthDevId", AuthDevNo);
    INFO("加单位密码...");
    memset(Buf, 0, sizeof(Buf));
    strcpy(Buf, AuthDevNo);
    fill_char_l(Buf, 20, '0');
    fill_char_l(xmlGetVal(doc, "//NoteInfo/NoteNo", noteno), 12, '0');

    sprintf( DealData, "%s%08d%-32.32s%-32.32s%016.0lf%-20.20s%s",
            noteno,
            XmlGetInteger(doc, "//NoteInfo/IssueDate" ),
            xmlGetVal( doc, "//NoteInfo/BeneAcct", beneacct ),
            xmlGetVal( doc, "//NoteInfo/PayingAcct", payingacct ),
            100.0 * XmlGetFloat(doc, "//NoteInfo/SettlAmt" ),
            XMLGetVal( doc, "//NoteInfo/Agreement",agreement ),
            Buf);
    DBUG("原文:%s", DealData);

    len = strlen(DealData);

    ret = GenerateCode(DealData, len, Secret);
    if(ret)
    {
        DBUG( "TestKeyChk_New operation fail. ret=%d.AuthDevNo=%s,DealData=%s,len=%d,mac=%s ", ret,AuthDevNo,DealData,len,Secret );
        return ret;
    }
    DBUG("PAYKEY:%s", Secret);
    SetNoteInfo("PayKey",Secret,doc);
    SetNoteInfo("AuthDevId",AuthDevNo,doc);
    if( ret )
        return -1;
    else
        return 0;
}

int AddDigest(xmlDoc *doc, int tctcode)
{
    int ret;
    char sNoteType[2+1]={0}, sTrnCode[6+1]={0};
    char sAuthDevNo[20+1]={0};

    //strcpy(sTrnCode, XMLGetNodeVal(doc, "//TrnCode"));
    //查询查复
    if(tctcode == 8006 || tctcode == 8009)
    {
        ret = MailKeyMake(doc);
        if (ret != 0)
            return E_SYS_ADDDIGEST;
        return ret;
    }
    strcpy(sAuthDevNo, (char *)GetAuthDevId());
    strcpy(sNoteType, XMLGetNodeVal(doc, "//NoteType"));

    switch(atoi(sNoteType))
    {
        case 2:
        case 21:
        case 42:
        case 81:
        case 4:
        case 41:
        case 44:
        case 46:
        case 52:
        case 56:
        case 58:
        case 61:
        case 62:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
            ret = TestKeyMake_New(doc, sAuthDevNo);
            break;
        //定期借记
        case 53:
        //税费缴款书
        case 59:
            if(atoi(XMLGetNodeVal(doc, "//DCFlag")) == 1)
                ret = DWTestKeyMake(doc, sAuthDevNo);
            else
                ret = TestKeyMake_New(doc, sAuthDevNo);
            break;
        default:
            return E_PACK_TYPE;
    }
    if (ret != 0)
        return E_SYS_ADDDIGEST;
    return 0;
}

int MailKeyChk( xmlDocPtr doc )
{
    int ret;
    int len;
    char DealData[2048], mac[17];
    char payingacct[33], beneacct[33],noteno[13];
    char profile[60], buff[40];
    char BankId[13];
    char AuthDevNo[21];
    char Buf[128];
    int myjtype;

    //普通邮件不加核押
    if( XmlGetInteger(doc, "//MailInfo/MailType") == 0)
        return 0;

    INFO("核押...");
    //从xml报文中读取各要素信息，组成校验字符串
    memset(DealData, 0 , sizeof(DealData));
    sprintf( DealData, "%-12.12s%08d%-32.32s%-32.32s%016.0lf%s",
            xmlGetVal(doc, "//MailInfo/Content/NoteNo", noteno ) ,
            XmlGetInteger(doc, "//MailInfo/Content/IssueDate" ),
            xmlGetVal( doc, "//MailInfo/Content/BeneAcct",beneacct ),
            xmlGetVal( doc, "//MailInfo/Content/PayingAcct",payingacct),
            100 * XmlGetFloat(doc, "//MailInfo/Content/SettlAmt" ),  
            XMLGetNodeVal( doc, "//MailInfo/Content/Purpose"));
    DBUG( "DealData=[%s]", DealData);

    len = strlen(DealData);

    xmlGetVal(doc, "//MailInfo/Content/TestKey", mac);
    ret = CheckCode(DealData, len, mac);
    if(ret)
    {
        DBUG( "MailKeyChk operation fail. ret=%d.AuthDevNo=%s,DealData=%s,len=%d,mac=%s ", ret,AuthDevNo,DealData,len,mac );
        return ret;
    }
    return 0;
}

int DWTestKeyChk( xmlDocPtr doc, char *AuthDevNo )
{
    char DealData[2048];
    char Secret[50];
    char BankId[13];
    int ret = 0;
    char noteno[13], payingacct[33], beneacct[33],agreement[21];
    char Buf[128];
    int len, myjtype;
    char profile[60], buff[40];
    

    //从xml报文中读取各要素信息，组成校验字符串
    //xmlGetVal(doc, "//NoteInfo/AuthDevId", AuthDevNo);
    INFO("核单位密码...");
    memset(Buf, 0, sizeof(Buf));
    strcpy(Buf, AuthDevNo);
    fill_char_l(Buf, 20, '0');
    fill_char_l(xmlGetVal(doc, "//NoteInfo/NoteNo", noteno), 12, '0');

    sprintf( DealData, "%s%08d%-32.32s%-32.32s%016.0lf%-20.20s%s",
            noteno,
            XmlGetInteger(doc, "//NoteInfo/IssueDate" ),
            xmlGetVal( doc, "//NoteInfo/BeneAcct", beneacct ),
            xmlGetVal( doc, "//NoteInfo/PayingAcct", payingacct ),
            100.0 * XmlGetFloat(doc, "//NoteInfo/SettlAmt" ),
            XMLGetVal( doc, "//NoteInfo/Agreement",agreement ),
            Buf);
    DBUG("原文:%s", DealData);

    //从xml报文中读取密押
    xmlGetVal(doc,"//NoteInfo/PayKey", Secret);


    len = strlen(DealData);

    ret = CheckCode(DealData, len, Secret);
    if(ret)
        DBUG( "TestKeyChk_New operation fail. ret=%d.AuthDevNo=%s,DealData=%s,len=%d,mac=%s ", ret,AuthDevNo,DealData,len,Secret );
    return ret;
}

int ChkCenterTestKey( xmlDocPtr doc, char *AuthDevNo )
{
    char BankId[13];
    char Buf[128];
    int result = 0;
    char notetype[5],dcflag[5];
    char OrgTestKey[20],NewTestKey[20];

    //取出原中心押
    memset(OrgTestKey, 0 ,sizeof(OrgTestKey));
    xmlGetVal(doc,"//NoteInfo/TestKey",OrgTestKey);

    result = TestKeyMake_New( doc, AuthDevNo);
    if(result)
    {
        return -1;
    }
    else
    {
        //取新中心密押
        memset(NewTestKey, 0 ,sizeof(NewTestKey));
        xmlGetVal(doc,"//NoteInfo/TestKey",NewTestKey);

        //将原中心押放回
        SetNoteInfo( "TestKey", OrgTestKey, doc );

        //比较原中心密押和新密押是否一致
        if( strcmp(OrgTestKey, NewTestKey) != 0)
        {
            return -1;
        }
        else
            return 0;
    }
}

int CheckDigest(xmlDoc *doc, int tctcode)
{
    int ret;
    char sNoteType[2+1]={0}, sTrnCode[6+1]={0};
    char sAuthDevNo[20+1]={0};

    //strcpy(sTrnCode, XMLGetNodeVal(doc, "//TrnCode"));
    //查询查复
    if(tctcode == 8001)
    {
        ret = MailKeyChk(doc);
        if (ret != 0)
            return E_SYS_CHKDIGEST;
        return ret;
    }
    strcpy(sAuthDevNo, XMLGetNodeVal(doc, "//AuthDevId"));
    ret = ChkCenterTestKey(doc, sAuthDevNo);    //核中心押
    if(ret)
        return E_SYS_CHKDIGEST;

    /*
    strcpy(sNoteType, XMLGetNodeVal(doc, "//NoteType"));

    switch(atoi(sNoteType))
    {
        //定期借记
        case 53:
        //税费缴款书
        case 59:
            if(atoi(XMLGetNodeVal(doc, "//DCFlag")) == 1)
                ret = DWTestKeyChk(doc, sAuthDevNo);    //核单位密码
            break;
        default:
            return 0;
    }
    if (ret != 0)
    {
        DBUG("核单位密码错 notetype:[%s]devno:[%s]", sNoteType, sAuthDevNo);
        return E_SYS_CHKDIGEST;
    }*/

    return ret;
}
