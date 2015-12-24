#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include "utils.h"
#include "remote_st.h"
#include "udb.h"
#include "prefs.h"
#include "err.h"

#define returnIfNull(p, ret) \
    do \
{ \
    if ((p) == NULL) \
    { \
        err_log("检测到空值返回."); \
        return ret; \
    } \
} while (0)

#define XMLFREE(a)  do {xmlFree(a); a = NULL;} while (0)
#define FREE(a)  do {free(a); a = NULL;} while (0)

/*
#define wrap(A,a) (((A *)dataBuff)->a)
#define SGETSTR(A,a,b,c) memcpy( c, wrap(A,a,b), sizeof(wrap(A,a,b)) )
#define SSETSTR(A,a,b,c)  memcpy( wrap(A,a,b), c, sizeof(wrap(A,a,b)) )
*/

void SGETSTR( char *sbuf, int set, char *buf)
{
    char trncode[5]={0};
    get_struct_trncode(trncode,sbuf);
    if(strcmp(trncode,"0002")==0)
    {
        data_remote_in_0002 *sdata = (data_remote_in_0002 *)sbuf;
        switch( set )
        {
            case 1: //交易代码
                memcpy( buf, sdata->JIAOYM, 4); 
                break;
            case 2://本外币标志
                memcpy( buf, sdata->BWAIBBZ, 1);
                break;
            case 3://币种
                memcpy( buf, sdata->BIZHO, 3);
                break;
            case 4://流水
                memcpy( buf, sdata->TCHULS, 10);
                break;
            case 5://提出行号
                memcpy( buf, sdata->TCHUHH, 6);
                break;
            case 6://提入行号
                memcpy( buf, sdata->TRUHHH, 6);
                break;
            case 7://票据种类
                memcpy( buf, sdata->PIOJUZL, 2);
                break;
            case 8://凭证号
                memcpy( buf, sdata->PZHHAO, 20);
                break;
            case 9://签发日期
                memcpy( buf, sdata->QIANFRQ, 10);
                break;
            case 10://收款人帐号
                memcpy( buf, sdata->SHKRZHH, 32);
                break;
            case 11://收款人户名
                memcpy( buf, sdata->SHKRHUM, 80);
                break;
            case 12://收款人行号
                memcpy( buf, sdata->SHKRKHHH, 6);
                break;
            case 13://付款人帐号
                memcpy( buf, sdata->FUKRZHH, 32);
                break;
            case 14://付款人户名
                memcpy( buf, sdata->FUKRHUM, 80);
                break;
            case 15://付款人行号
                memcpy( buf, sdata->FUKRKHHH, 6);
                break;
            case 16://对方行号
                memcpy( buf, sdata->DUIFHHH, 6);
                break;
            case 17:// 对方行行名
                memcpy( buf, sdata->DUIFHHM, 60);
                break;
            case 18:// 金额
                memcpy( buf, sdata->JINE, 15);
                break;
            case 19:// 限额
                memcpy( buf, sdata->XIANE, 15);
                break;
            case 20:// 支付密码
                memcpy( buf, sdata->ZHFMM, 20);
                break;
            case 21:// 同城密押
                memcpy( buf, sdata->TCHENGMM, 16);
                break;
            case 22:// 事由用途
                memcpy( buf, sdata->SHIYYT, 60);
                break;
            case 23: // 备用
                memcpy( buf, sdata->BEIYONG, 200);
                break;
            case 24:// 网络ip地址
                memcpy( buf, sdata->WLIP, 15);
                break;
            case 25:// 主机ip地址
                memcpy( buf, sdata->ZHJIP, 15);
                break;
            case 26:// 终端号
                memcpy( buf, sdata->ZHONDHAO, 10);
                break;
            case 27:// 提出行操作员号
                memcpy( buf, sdata->TCHHCZYH, 4);
                break;
            case 28://处理结果
                memcpy( buf, sdata->CHULJG, 2);
                break;
            case 29://透支警告标志
                memcpy( buf, sdata->TOUZHJGBZ, 1);
                break;
            case 30://其他标志
                memcpy( buf, sdata->TITBZ, 20);
                break;
            default:
                break;
        }
    }
    else if(strcmp(trncode,"0043")==0)
    {
        data_remote_in_0043 *sdata = (data_remote_in_0043 *)sbuf;
        switch( set )
        {
            case 1://交易代码
                memcpy( buf, sdata->JIAOYM, 4); 
                break;
            case 2://开户行号
                memcpy( buf, sdata->OPENBANK, 6);
                break;
            case 3://交换号
                memcpy( buf, sdata->EXCHANGENO, 6);
                break;
            case 4://账号
                memcpy( buf, sdata->ACCTNO, 32);
                break;
            case 5://户名
                memcpy( buf, sdata->ACCTNAME, 80);
                break;
            case 6://文件名
                memcpy( buf, sdata->FILENAME, 64);
                break;
            case 7://开始日期
                memcpy( buf, sdata->STARTDATE, 10);
                break;
            case 8://结束日期
                memcpy( buf, sdata->FINALDATE, 10);
                break;
            case 9://标志位
                memcpy( buf, sdata->FLAG, 1);
                break;
            case 10://网络IP地址
                memcpy( buf, sdata->NETIP, 15);
                break;
            case 11://主机IP地址
                memcpy( buf, sdata->HOSTIP, 15);
                break;
            case 12://终端号
                memcpy( buf, sdata->TERMID, 10);
                break;
            case 13://操作员号
                memcpy( buf, sdata->OPERNO, 4);
                break;
            case 14://处理结果
                memcpy( buf, sdata->RESULT, 2);
                break;
            default:
                break;
        }
    }
    return;
}

void err_tc2struct(char *serr, char *tcoperr)
{
    if (strcmp(tcoperr, "0000") == 0)
        strcpy(serr, "00");
    else
        strcpy(serr, tcoperr);
}

void struct_retinfo(char *retcode, char *retmsg)
{
    int errcode=0;
    err_log("[%s][%s]", retcode, retmsg);

    errcode=atoi(retcode);
    switch(errcode)
    {
        case    0:errcode=0;break;
        case  101:errcode=44;break;
        case  102:errcode=44;break;
        case  103:errcode=44;break;
        case  104:errcode=44;break;
        case  105:errcode=40;break;
        case  106:errcode=40;break;
        case  110:errcode=44;break;
        case  201:errcode=21;break;
        case  202:errcode=21;break;
        case  203:errcode=22;break;
        case  204:errcode=21;break;
        case  205:errcode=21;break;
        case  207:errcode=21;break;
        case  301:errcode=31;break;
        case  302:errcode=31;break;
        case  303:errcode=24;break;
        case  304:errcode=24;break;
        case  305:errcode=31;break;
        case  306:errcode=31;break;
        case  307:errcode=31;break;
        case  308:errcode=42;break;
        case 1001:errcode=49;break;
        case 1002:errcode=32;break;
        case 1003:errcode=33;break;
        case 1004:errcode=49;break;
        case 1005:errcode=49;break;
        case 1006:errcode=31;break;
        case 1007:errcode=32;break;
        case 1008:errcode=33;break;
        case 1009:errcode=49;break;
        case 1010:errcode=49;break;
        case 1013:errcode=40;break;
        case 1014:errcode=41;break;
        case 1019:errcode=40;break;
        case 1020:errcode=41;break;
        case 1023:errcode=30;break;
        case 1024:errcode=30;break;
        case 1025:errcode=30;break;
        case 1026:errcode=30;break;
        case 2001:errcode=45;break;
        case 2002:errcode=34;break;
        case 2003:errcode=45;break;
        case 2004:errcode=47;break;
        case 2005:errcode=46;break;
        case 2006:errcode=35;break;
        case 2007:errcode=46;break;
        case 2015:errcode=45;break;
        case 2016:errcode=34;break;
        case 2017:errcode=45;break;
        case 2019:errcode=45;break;
        case 2020:errcode=34;break;
        case 2021:errcode=45;break;
        case 2025:errcode=45;break;
        case 2026:errcode=34;break;
        case 2027:errcode=45;break;
        case 2029:errcode=45;break;
        case 2030:errcode=34;break;
        case 2031:errcode=45;break;
        case 3001:errcode=30;break;
        case 3002:errcode=37;break;
        case 3003:errcode=43;break;
        case 3004:errcode=38;break;
        case 3005:errcode=38;break;
        case 3006:errcode=48;break;
        case 3007:errcode=60;break;
        case 3008:errcode=39;break;
        case 3010:errcode=39;break;
        case 3012:errcode=61;break;
        case 3017:errcode=31;break;
        case 3021:errcode=22;break;
        case 3101:errcode=36;break;
        case 3103:errcode=52;break;
        case 3105:errcode=63;break;
        case 3201:errcode=99;break;
        case 3901:errcode=39;break;
        case 3902:errcode=39;break;
        case 9999:errcode=30;break;
        /*modify by litao 2013-7-18
         其他错误映射为99-交易失败*/
        //default:errcode=0;
        default:errcode=99;
       /*end litao 2013-7-18*/
    }
    if(errcode == 0 )
        sprintf(retcode,"%s","00");
    else
        sprintf(retcode,"%d",errcode);
    retcode[2]=0x00;

    return;
}


// 取struct交易码
int get_struct_trncode(char *trncode, char *sbuf)
{
    memcpy(trncode, sbuf, 4);
    trncode[4] = 0x00;
    return 0;
}

int struct2tc_req(char *opptrcode, char *xmlbuf, char *sbuf, int slen)
{
    xmlDocPtr cvt = NULL;
    xmlDocPtr doc = NULL;
    xmlNodePtr node, cur;
    xmlChar *xbuf;
    char pktfmt[10];
    char buf[256], nbuf[256];
    char path[128];
    char *name, *type, *value, *grid, *sofile, *func, *expr, *set;
    char bktype[4]={0}, tctype[4]={0}, qtjyma[8]={0}, noteno[32]={0};
    int len, rc = 0;

    // 取转换配置文件
    sprintf(path, "%s/etc/cvt_struct2tc.xml", getAppDir());
    if ((cvt = xmlParseFile(path)) == NULL)
    {
        err_log("解析%s失败!", path);
        return -1;
    }

    if (*opptrcode != 0x00)
    {
        sprintf(path, "/Root/Transaction[@code='%s']/Request", opptrcode);
        if ((node = XmlLocateNode(cvt, path)) == NULL)
        {
            err_log("locate xml fail, [%s]", path);
            return -1;
        }
        if ((xbuf = XmlNodeGetAttrText(node, "opptrcode")) == NULL)
        {
            err_log("opptrcode not found, in [%s].", path);
            return -1;
        }
        strcpy(opptrcode, xbuf);
        XMLFREE(xbuf);
    }

    if ((doc = XmlNewDocEnc("UFTP", "GB18030")) == NULL)
    {
        return -1;
    }

    cur = node->children;
    while(cur != NULL)
    {
        if (cur->type != XML_ELEMENT_NODE)
        {
            cur = cur->next;
            continue;
        }
        name = XmlNodeGetAttrText(cur, "name");
        type = XmlNodeGetAttrText(cur, "type");
        set = XmlNodeGetAttrText(cur, "set");
        value = XmlNodeGetAttrText(cur, "value");
        sofile = XmlNodeGetAttrText(cur, "dll");
        func = XmlNodeGetAttrText(cur, "func");
        expr = XmlNodeGetAttrText(cur, "expr");
        if (name == NULL)
        {
            err_log("cvt_struct2tc.xml: item's name not found.");
            rc = -1;
            break;
        }

        // type不等于sop,const, 跳过该字段
        if (type != NULL && strcmp(type, "struct") != 0 && strcmp(type, "const") != 0)
        {
            XMLFREE(name); 
            XMLFREE(type);
            XMLFREE(value);
            XMLFREE(sofile); 
            XMLFREE(func); 
            XMLFREE(expr);
            XMLFREE(set);
            cur = cur->next;
            continue;
        }
        else if (type != NULL && strcmp(type, "const") == 0)
        {
            memset(buf, 0, sizeof(buf));
            if (sofile != NULL && func != NULL)
            {
                // 通过配置的转换函数转换内容
                memset(nbuf, 0, sizeof(nbuf));
                rc = callConvertFunc(sofile, func, expr, value, nbuf);
                if (rc != 0)
                {
                    err_log("callConvertFunc(%s) fail.", func);
                    break;
                }
                strcpy(buf, nbuf);
            }
            else
            {
                // 常量
                strcpy(buf, value);
            }

            err_log("SET [%-30s]=[%s]", name, buf);
            if (XmlSetString(doc, name, buf) != 0)
                err_log("   SET FAIL [%-30s]=[%s]", name, buf);
        }
        // 如果没有配置type(type==NULL), 缺省取字段名为value的struct报文内容
        else if (type == NULL || strcmp(type, "struct") == 0)
        {
            memset(buf, 0, sizeof(buf));
            SGETSTR( sbuf, atoi(set), buf);
            rtrim(buf);
            if (sofile != NULL && func != NULL)
            {
                // 通过配置的转换函数转换内容
                memset(nbuf, 0, sizeof(nbuf));
                rc = callConvertFunc(sofile, func, expr, buf, nbuf);
                if (rc != 0)
                {
                    err_log("callConvertFunc(%s) fail.", func);
                    break;
                }
                memset(buf, 0, sizeof(buf));
                strcpy(buf, nbuf);
            }

            err_log("SET [%-30s]=[%s]", name, buf);
            if (XmlSetString(doc, name, buf) != 0)
                err_log("   SET FAIL [%-30s]=[%s]", name, buf);
        }

        XMLFREE(name);
        XMLFREE(type);
        XMLFREE(value);
        XMLFREE(sofile);
        XMLFREE(func); 
        XMLFREE(expr);
        XMLFREE(set);
        cur = cur->next;
    }
    if (rc != 0)
    {
        XMLFREE(name);
        XMLFREE(type);
        XMLFREE(value);
        XMLFREE(sofile);
        XMLFREE(func); 
        XMLFREE(expr);
        XMLFREE(set);
    }
    // xml打包
    if (rc == 0)
    {
        xmlDocDumpMemory(doc, &xbuf, &rc);
        memcpy(xmlbuf, xbuf, rc);
        err_log("[%d][%s].", rc, xmlbuf);
    }

    if (*opptrcode == 0x00)
    {
        XmlGetString(doc, "/UFTP/MsgHdrRq/TrnCode", buf, sizeof(buf));
        strcpy(opptrcode, buf);
    }

    /*老同城报文标志*/
    XmlSetString(doc, "/UFTP/MsgHdrRq/Reserve", "1");
    xmlFreeDoc(doc);
    xmlFreeDoc(cvt);
    return rc;
}
/*
   int do_item_tc2struct_rsp(char *sbuf, xmlDocPtr doc, xmlNodePtr cur)
   {
   char buf[1024], nbuf[1024];
   char *name, *plen, *type, *value, *poffset, *sofile, *func, *expr;
   int len, offset, rc = 0;

   returnIfNull(name = XmlNodeGetAttrText(cur, "name"), -1);
   returnIfNull(plen = XmlNodeGetAttrText(cur, "length"), -1);
   type = XmlNodeGetAttrText(cur, "type");
   value = XmlNodeGetAttrText(cur, "value");
   poffset = XmlNodeGetAttrText(cur, "offset");
   sofile = XmlNodeGetAttrText(cur, "dll");
   func = XmlNodeGetAttrText(cur, "func");
   expr = XmlNodeGetAttrText(cur, "expr");

// 偏移量
if (poffset == NULL)
offset = 0;
else
offset = atoi(poffset);
// 长度
len = atoi(plen);

if (type == NULL || strcmp(type, "xml") == 0)
{
// 从xml报文取出相应域填入
XmlGetString(doc, value, buf, sizeof(buf));
 *(sbuf+offset+len) = '\0';
 rtrim(buf+offset);
 if (sofile != NULL && func != NULL)
 {
 memset(nbuf, 0, sizeof(nbuf));
 rc = callConvertFunc(sofile, func, expr, buf+offset, nbuf);
 if (rc != 0)
 goto func_handler;
 lrtrim(nbuf);
 err_log("PUT [%s]=[%s]", name, nbuf);
 PPutStr(name, nbuf);
 }
 else
 {
 err_log("PUT [%s]=[%s]", name, buf+offset);
 PPutStr(name, buf+offset);
 }
 }
 else if (strcmp(type, "const") == 0)
 {
 if (sofile != NULL && func != NULL)
 {
 memset(nbuf, 0, sizeof(nbuf));
 rc = callConvertFunc(sofile, func, expr, (void *)doc, nbuf);
 if (rc != 0)
 goto func_handler;
 lrtrim(nbuf);
 err_log("PUT [%s]=[%s]", name, nbuf);
 PPutStr(name, nbuf);
 }
 else
 {
 err_log("PUT [%s]=[%s]", name, value);
 PPutStr(name, value);
 }
 }
 else
 {
 err_log("Wrong type: %s.", type);
 }

func_handler:
XMLFREE(name);
XMLFREE(plen);
XMLFREE(type);
XMLFREE(value);
XMLFREE(poffset);
XMLFREE(sofile);
XMLFREE(func);
XMLFREE(expr);

return rc;
}
    */
int tc2struct_rsp(char *trncode, char *sbuf, char *xmlbuf, int xmllen)
{
    xmlDocPtr doc = NULL;
    xmlXPathObjectPtr result;
    char buf[1024], nbuf[1024], errinfo[83], tcd[5];

    // 解析平台返回的xml报文
    if ((doc = xmlParseMemory(xmlbuf, xmllen)) == NULL)
    {
        err_log("parse (%d)[%s] fail.", xmllen, xmlbuf);
        return -1;
    }
    // 转换错误码
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
    err_log("同城:result=[%s] desc=[%s]", buf, errinfo);
    err_tc2struct(nbuf, buf);
    struct_retinfo(nbuf, errinfo);

    get_struct_trncode(tcd,sbuf);
    if(strcmp(tcd,"0002")==0)
    {
        data_remote_in_0002 *sdata = (data_remote_in_0002 *)sbuf;
        // 修改交易结果
        memcpy( sdata->CHULJG, nbuf, 2 );
    }
    else if(strcmp(tcd,"0043")==0)
    {
        data_remote_in_0043 *sdata = (data_remote_in_0043 *)sbuf;
        // 修改交易结果
        memcpy( sdata->RESULT, nbuf, 2 );
        if( atoi(nbuf) == 0 )
        {
            // 修改其他字段
            XmlGetString(doc, "/UFTP/AcctDetail/OpenBank", buf, sizeof(buf));
            memcpy( sdata->EXCHANGENO, buf, 6<strlen(buf)?6:strlen(buf));
            XmlGetString(doc, "/UFTP/AcctDetail/Name", buf, sizeof(buf));
            memcpy( sdata->ACCTNAME, buf, 80<strlen(buf)?80:strlen(buf));
        }
    }

    xmlFreeDoc(doc);
    return strlen(sbuf);
}
//8位日期转10位
char *Date8To10(char *DateBuff)
{
    char tmp[11] = {0};

    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%4.4s/%2.2s/%2.2s", DateBuff, DateBuff+4,DateBuff+6);
    memcpy(DateBuff, tmp, 10);
    return DateBuff;
}

//10位日期转8位
char *Date10To8(char *DateBuff)
{
    char tmp[9] = {0};

    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%4.4s%2.2s%2.2s", DateBuff, DateBuff+5,DateBuff+8);
    memcpy(DateBuff, tmp, 8);
    return DateBuff;
}
