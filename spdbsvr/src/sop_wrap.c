#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include "utils.h"
#include "util.h"
#include "sop.h"
#include "udb.h"
#include "sop_st.h"
#include "prefs.h"
//#include "trncode.h"
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

/*前台发起的交易报文头的内容*/
static char PDTRCD1[5];
static char PDLDTC[5];
static char PDTRDT[9];
static char PDTRTM[5];
static char PDTLSQ[13];
static char PDERTR[3];
/*报文头*/
/*税票信息打印*/
/*
enum Tax
{
    TraNo=0,
    TaxPayCode,
    TaxPayName,
    TreCode,
    TraAmt,
    TaxVouNo,
    BillDate,
    CorpCode,
    BudgetType,
    TrimSign,
    CorpType,
    Remark,
    Payingbank,
    Payingacct,
    Payer,
    Print,
    TaxSize1,
    TaxSize2=1024
};
char TaxPrintBuf[TaxSize1][TaxSize2];
*/

int equal(char *s1,char *s2)
{
    return strcmp(s1,s2)?0:1;
}

void memrev(char *s,size_t n)
{
    char *s1=s;
    char *s2=(char*)malloc(n);
    for(s2+=n;n>0;n--)*--s2=*s1++;
    memcpy(s,s2,s1-s);
    free(s2);
}

char *getSysPara(char *paraval,char *paraname)
{
    if(db_query_str(paraval, 128, "select paraval from syspara where nodeid=10 and paraname='%s'",paraname)!=0)
        *paraval=0x00;
    return paraval;
}

char *map_trncode(char *trncode, char *rs)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr node;
    char path[256];
    char *p;

    sprintf(path, "%s/etc/njcb.xml", getAppDir());
    returnIfNull((doc = xmlParseFile(path)), NULL);

    sprintf(path, "/Root/Transaction[@code='%s']/%s", trncode, rs);
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        xmlFreeDoc(doc);
        return NULL;
    }
    p = XmlNodeGetAttrText(node, "packfmt");
    xmlFreeDoc(doc);
    return p;
}

void map_errcode(char *soperr, char *tcoperr)
{
    strcpy(soperr, tcoperr);
    return;
}
void err_tc2sop(char *soperr, char *tcoperr)
{
     if (strcmp(tcoperr, "0000") == 0)
         strcpy(soperr, "AAAAAAA");
     else
         strcpy(soperr, tcoperr);
}

int init_sop()
{
    char aczBuf[512];
    char aczTemp[64];

    if (InitConEnv(USE_COP) < 0)
        return -1;

    memset(aczBuf,0,sizeof(aczBuf));

    /* 系统信息头 system_head.cfg */
    /* 报文长度由系统自动加入 */
    PPutMem("sysSADDR",aczBuf,4);       /* 源地址 */
    PPutMem("sysDADDR",aczBuf,4);       /* 目的地址 */
    PPutMem("sysRSRVD",aczBuf,1);       /* 系统保留位 */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* 信息结束标志 */
    PPutMem("sysSEQNUM",aczBuf,2);      /* 报文序号 */
    PPutMem("sysMACFLAG",aczBuf,1);     /* 校验标志 */
    PPutMem("sysMACVALUE",aczBuf,8);    /* 校验值 */

    /* 公共交易头 cmtran_head.cfg */
    //PPutStr("PDWSNO", "1a");             /* 终端号 */
    PPutStr("PDCTNO", "    ");           /* 城市代码 */

    /* 交易数据头 tran_head.cfg */
    PPutStr("PDTRSD", "");               /* 交易子码 */
    PPutStr("PDTRMD", "");               /* 交易模式 */
    aczTemp[0]=0x00;
    aczTemp[1]=0x01;
    PPutMem("PDTRSQ", aczTemp,2);        /* 交易序号 */
    aczTemp[0]=0xff;
    aczTemp[1]=0xff;
    PPutMem("PDOFF1", aczTemp,2);        /* 系统偏移0xffff */
    PPutMem("PDOFF2", aczTemp,2);        /* 系统偏移0xffff */
    PPutStr("PDAUUS","");       /* 授权柜员 */
    PPutStr("PDAUPS","");       /* 授权密码 */  

    return 0;
}

void free_sop()
{
    return DestoryConEnv();
}

void sop_setinfo()
{
    char aczBuf[128];
    char aczTemp[64];
    int  iNow;

    memset(aczBuf,0,sizeof(aczBuf));

    /* 系统信息头 system_head.cfg */
    /* 报文长度由系统自动加入 */
    PPutMem("sysSADDR",aczBuf,4);       /* 源地址 */
    PPutMem("sysDADDR",aczBuf,4);       /* 目的地址 */
    PPutMem("sysRSRVD",aczBuf,1);       /* 系统保留位 */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* 信息结束标志 */
    PPutMem("sysSEQNUM",aczBuf,2);      /* 报文序号 */
    PPutMem("sysMACFLAG",aczBuf,1);     /* 校验标志 */
    PPutMem("sysMACVALUE",aczBuf,8);    /* 校验值 */

    /* 公共交易头 cmtran_rcv_head.cfg */
    //PPutStr("PDTRCD","6140");           /* 交易代码 */
    PPutStr("PDLDTC","");               /* 联动交易码 */
    sprintf(aczBuf, "%08ld", current_date());
    PPutStr("PDTRDT",aczBuf);               /* 交易日期 */
    iNow=htonl(current_time());            
    PPutMem("PDTRTM",(char *)&iNow,4);      /* 交易时间 */
    //PPutStr("PDTLSQ","888888880001");       /* 柜员流水号 */
    PGetMem("PDTRSQ",aczTemp,2);
    PPutMem("PDERTR",aczTemp,2);        /* 出错交易序号 */
    return;
}

void sop_retinfo(char *retcode, char *retmsg)
{
    err_log("[%s][%s]", retcode, retmsg);

    // 设置sop通用字段内容
    sop_setinfo();

    //AAAAAAA代表成功应答
    if (strncmp(retcode, "AAAAAAA", 7) != 0)
    {
        PPutStr("TPU_Ctx1","ERR000");   // 失败对象名，固定为”ERR000”
        PPutStr("TPU_Ctx2","-1");       // 错误码，固定为”-1”
        PPutStr("TPU_RetMsg", retmsg);  // 返回信息，不超过82位
    }
    PPutStr("TPU_RetCode", retcode); 

    return;
}

// 取sop交易码
int get_sop_trncode(char *trncode, char *sopbuf)
{
    data_head_in data;

    *trncode = 0x00;

    // 解析数据头
    memset(&data, 0, sizeof(data));
    memcpy(&data, sopbuf+sizeof(msg_head_in)+sizeof(pub_head_in), sizeof(data));
    strncpy(trncode, data.JIAOYM, sizeof(data.JIAOYM));
    trncode[sizeof(data.JIAOYM)] = 0x00;
    return 0;
}

int sop2tc_req(char *opptrcode, char *xmlbuf, char *sopbuf, int soplen)
{
    xmlDocPtr cvt = NULL;
    xmlDocPtr doc = NULL;
    xmlNodePtr node, cur;
    xmlChar *xbuf;
    char pktfmt[10];
    char buf[256], nbuf[256];
    char path[128];
    char *name, *type, *value, *grid, *sofile, *func, *expr;
    char bktype[4]={0}, tctype[4]={0}, qtjyma[8]={0}, noteno[32]={0};
    int len, rc = 0;
    /*报文头数据*/
    PGetStr("PDTRCD",PDTRCD1,sizeof(PDTRCD1));err_log("交易代码      [PDTRCD]=[%s]",PDTRCD1);//交易代码
    PGetStr("PDLDTC",PDLDTC,sizeof(PDLDTC));err_log("联动交易码      [PDLDTC]=[%s]",PDLDTC);//联动交易码
    PGetStr("PDTRDT",PDTRDT,sizeof(PDTRDT));err_log("交易日期        [PDTRDT]=[%s]",PDTRDT);//交易日期
    PGetStr("PDTRTM",PDTRTM,sizeof(PDTRTM));err_log("交易时间        [PDTRTM]=[%s]",PDTRTM);//交易时间
    PGetStr("PDTLSQ",PDTLSQ,sizeof(PDTLSQ));err_log("前台柜员流水    [PDTLSQ]=[%s]",PDTLSQ);//前台柜员流水
    PGetStr("PDERTR",PDERTR,sizeof(PDERTR));err_log("出错交易序号    [PDERTR]=[%s]",PDERTR);//出错交易序号
    /*报文头数据*/
    // 取转换配置文件
    sprintf(path, "%s/etc/cvt_sop2tc.xml", getAppDir());
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

    if ((grid = XmlNodeGetAttrText(node, "grid")) != NULL)
    {
        char acPkt[2048];
        char tmp[64];
        int iLen, iRetCode, iRows;

        /* 从POOL池中取出表格报文 */
        iLen = PGetMem(grid, acPkt, sizeof(acPkt));

        /* 调用表格拆包函数 */
        iRetCode = SOP_UnpackForm('0', acPkt, iLen);
        if ( iRetCode < 0 )
        {
            err_log("SOP_UnpackForm fail, ret=[%d]", iRetCode);
            return -1;
        }

        /* 取出表格的行数 */
        sprintf(tmp, "%s_Rows", grid);
        PGetMem(tmp, &iRows, sizeof(int));
        err_log("%s=[%d]", tmp, iRows);
        sprintf(tmp, "_%d", 0); // 取第一行
        POOL_SetGetSuff(tmp);   /* 设置POOL池数据名后缀 */
    }

    if ((doc = XmlNewDocEnc("UFTP", "GB18030")) == NULL)
    {
        XMLFREE(grid);
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
        value = XmlNodeGetAttrText(cur, "value");
        sofile = XmlNodeGetAttrText(cur, "dll");
        func = XmlNodeGetAttrText(cur, "func");
        expr = XmlNodeGetAttrText(cur, "expr");
        if (name == NULL)
        {
            err_log("cvt_sop2tc.xml: item's name not found.");
            rc = -1;
            break;
        }

        // type不等于sop,const, 跳过该字段
        if (type != NULL && strcmp(type, "sop") != 0 && strcmp(type, "const") != 0)
        {
            XMLFREE(name); 
            XMLFREE(type);
            XMLFREE(value);
            XMLFREE(sofile); 
            XMLFREE(func); 
            XMLFREE(expr);
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
        // 如果没有配置type(type==NULL), 缺省取字段名为value的sop报文内容
        else if (type == NULL || strcmp(type, "sop") == 0)
        {
            memset(buf, 0, sizeof(buf));
            PGetStr(value, buf, sizeof(buf));
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
        cur = cur->next;
    }

    strcpy(tctype,XmlGetStringDup(doc,"//NoteType"));//同城凭证类型
    strcpy(bktype,XmlGetStringDup(doc,"//PNGZZL"));  //行内凭证类型
    strcpy(qtjyma,XmlGetStringDup(doc,"//QTJYMA"));  //前台交易码

    if(equal(qtjyma,"sz77")||
       equal(qtjyma,"sz78")||
       equal(qtjyma,"sz79")||
       equal(qtjyma,"sz80"))
    {
        //跳过特殊处理的交易，禁止进行凭证转换
    }
    else if(*tctype=='\0')
    {
        //转换凭证类型:行内->人行
        err_log("同城NoteType为空,开始从行内凭证转换至人行凭证");
        if(*bktype=='\0')
        {
            err_log("行内凭证类型为空,跳过,无需转换");
        }
        else
        {
            rc=notetype_b2c(tctype,
                            XmlGetStringDup(doc,"//QTJYMA"),
                            XmlGetStringDup(doc,"//PNGZZL"),
                            XmlGetStringDup(doc,"//PNG1ZL"),
                            XmlGetStringDup(doc,"//PNGZZZ"),
                            XmlGetStringDup(doc,"//BEIZXX"),
                            XmlGetStringDup(doc,"//TJRZFS"),
                            XmlGetStringDup(doc,"//BIAOZI")
                            );
            if(rc==0)
            {
                if(strcmp(XmlGetStringDup(doc,"//QTJYMA"),"sz79")==0)
                    XmlSetString(doc,"//QueryInfo/NoteType",tctype);
                else
                    XmlSetString(doc,"//NoteInfo/NoteType",tctype);
            }
        }
    }
    else
    {
        err_log("同城NoteType不为空，无需凭证转换");
    }

    strcpy(tctype,XmlGetStringDup(doc,"//NoteType"));//同城凭证类型
    if(equal(tctype,"13")||
       equal(tctype,"14")||
       equal(tctype,"42")||
       equal(tctype,"43"))
    {
#if 0
        //税票信息打印文件
        if( equal(tctype,"14") )
        {
            char tax_file[512]={0};
            FILE *tax_fp=NULL;
            char PrintBuf[20480]={0};
            int tax_len=0;

            sprintf(tax_file,"%s/consoleprint/%s.tax",getenv("HOME"),XmlGetStringDup(doc,"//XINX03"));
            if((tax_fp=fopen(tax_file, "w+"))==NULL)
            {
                err_log("无法打开文件[%s]",tax_file);
                return -1;
            }
            //tax_len+=sprintf(PrintBuf+tax_len,"\f");
            tax_len+=sprintf(PrintBuf+tax_len,"  付款帐号:%-32s   付款户名:%s\n",
                    XmlGetStringDup(doc,"//PayingAcct"), XmlGetStringDup(doc,"//Payer"));
            fwrite(PrintBuf,tax_len,1, tax_fp);
            fclose(tax_fp);
            err_log("税票打印文件[%s]成功", tax_file);
        }
#endif
        strcpy(noteno,XmlGetStringDup(doc,"//XINX03"));//凭证号
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
    }
    /*
    if(*XmlGetStringDup(doc,"//TJJXTS")!=0x00 && *XmlGetStringDup(doc,"//TJJXTS")!='0')
    {
        sprintf(noteno,"%s%s",XmlGetStringDup(doc,"//TJJXTS"),
                strlen(XmlGetStringDup(doc,"//NoteNo"))==9?XmlGetStringDup(doc,"//NoteNo")+1:XmlGetStringDup(doc,"//NoteNo"));
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
        err_log("合并后的凭证号noteno[%s]",noteno);
    }
    */
    if(*XmlGetStringDup(doc,"//PNG1XH")!=0x00 && *XmlGetStringDup(doc,"//PNG1XH")!='0')
    {
        sprintf(noteno,"%s%s",XmlGetStringDup(doc,"//PNG1XH"),
                strlen(XmlGetStringDup(doc,"//NoteNo"))==9?XmlGetStringDup(doc,"//NoteNo")+1:XmlGetStringDup(doc,"//NoteNo"));
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
        err_log("合并后的凭证号noteno[%s]",noteno);
    }

    if (rc != 0)
    {
        XMLFREE(name);
        XMLFREE(type);
        XMLFREE(value);
        XMLFREE(sofile);
        XMLFREE(func); 
        XMLFREE(expr);
    }

    if (grid != NULL)
    {
        POOL_SetGetSuff("");    /* 清除POOL池数据名后缀，非常重要，出错 */
        XMLFREE(grid);
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
    xmlFreeDoc(doc);
    xmlFreeDoc(cvt);
    return rc;
}

int do_item_tc2sop_rsp(xmlDocPtr doc, xmlNodePtr cur)
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
        *(buf+offset+len) = '\0';
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

int tc2sop_rsp(char *trncode, char *sopbuf, char *xmlbuf, int xmllen)
{
    xmlDocPtr cvt = NULL;
    xmlDocPtr doc = NULL;
    xmlXPathObjectPtr result;
    xmlXPathObjectPtr rs = NULL;
    xmlNodePtr node, cur;
    char aczSendMsg[4096];
    char *grid, pktfmt[10];
    char buf[1024], nbuf[1024], errinfo[83];
    char *p, path[128];
    //char *name, *plen, *type, *value, *poffset, *sofile, *func, *expr;
    int len, offset, rc = 0;
    int l=0;
    int iLen=0;

    /*报文头原样返回*/
    PPutStr("PDTRCD", PDTRCD1);
    PPutStr("PDLDTC", PDLDTC);
    PPutStr("PDTRDT", PDTRDT);
    PPutStr("PDTRTM", PDTRTM);
    PPutStr("PDTLSQ", PDTLSQ);
    PPutStr("PDERTR", PDERTR);
    /*报文头原样返回*/
    // 解析平台返回的xml报文
    if ((doc = xmlParseMemory(xmlbuf, xmllen)) == NULL)
    {
        err_log("parse (%d)[%s] fail.", xmllen, xmlbuf);
        return -1;
    }

    // 取转换配置文件
    sprintf(path, "%s/etc/cvt_sop2tc.xml", getAppDir());
    if ((cvt = xmlParseFile(path)) == NULL)
    {
        err_log("parse %s fail.", path);
        xmlFreeDoc(doc);
        return -1;
    }

    sprintf(path, "/Root/Transaction[@code='%s']/Response", trncode);
    if ((node = XmlLocateNode(cvt, path)) == NULL)
    {
        err_log("查找节点[%s]失败!", path);
        xmlFreeDoc(cvt);
        xmlFreeDoc(doc);
        return -1;
    }
    /*
       rs = getnodeset(cvt, path);
       if (rs == NULL || rs->nodesetval->nodeNr <= 0)
       {
       err_log("locate xml fail,[%s]!", path);
       xmlXPathFreeObject(rs);
       xmlFreeDoc( cvt );
       return -1;
       }

       err_log("交易[%s]返回内容:", trncode);
       for( l=0; l< rs->nodesetval->nodeNr; l++ )
       {
       node = rs->nodesetval->nodeTab[l];
       }
     */
    {
        // 下传报文格式
        // 取得packformat
        //sprintf(pktfmt, "O%s2@R", trncode);
        if ((p = XmlNodeGetAttrText(node, "packfmt")) == NULL)
        {
            err_log("查找节点[%s]失败!", path);
            xmlFreeDoc(cvt);
            xmlFreeDoc(doc);
            return -1;
        }
        strcpy(pktfmt, p);
        FREE(p);

        err_log("交易打包格式[%s]", pktfmt);
        // 遍历子节点
        //err_log("交易[%s]返回内容:", trncode);
        for( cur = node->children; cur != NULL; cur = cur->next )
        {
            if (cur->type != XML_ELEMENT_NODE)
                continue;
            if (!strcasecmp(cur->name, "grid"))
            {
                xmlNodePtr scur;
                char aczSuff[128];
                int rows = 0;

                grid = XmlNodeGetAttrText(cur, "name"); // 表格名称
                if (grid == NULL)
                {
                    err_log("grid's name  not found.");
                    continue;
                }
                sprintf(aczSuff,"_%d", rows);       // rows笔记录
                POOL_SetPutSuff(aczSuff);           // 设置POOL池数据名后缀
                for (scur = cur->children; scur != NULL; scur = scur->next)
                {
                    if (scur->type != XML_ELEMENT_NODE)
                        continue;
                    if ((rc = do_item_tc2sop_rsp(doc, scur)) != 0)
                    {
                        err_log("do_item_tc2sop_rsp() ret = %d.", rc);
                        POOL_SetPutSuff("");
                        continue;
                    }
                }
                POOL_SetPutSuff("");                // 清除POOL池数据名后缀
                sprintf(buf, "%s_Rows", grid);      // 设置表格的行数 
                PPutMem(buf, &rows, sizeof(int));

                // 调用表格打包函数
                memset(buf, 0, sizeof(buf));
                len = 0;
                if ((rc = SOP_PackForm(grid, '0', buf, sizeof(buf), &len)) < 0)
                    err_log("SOP_PackForm() ret = %d.", rc);

                // 把表格报文放入POOL池
                PPutMem(grid, buf, len);
            }
            else
            {
                if ((rc = do_item_tc2sop_rsp(doc, cur)) != 0)
                    err_log("do_item_tc2sop_rsp() ret = %d.", rc);
            }
        }
#if 0
        // 转换错误码
        XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
        XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
        err_log("同城:result=[%s] desc=[%s]", buf, errinfo);
        err_tc2sop(nbuf, buf);
        //xmlFreeDoc(doc);

        // 设置返回报文字段
        sop_retinfo(nbuf, errinfo);
        /* 打印日志 sop.log */
        POOL_TraceData();
        /* 生成SOP的下传报文 */
        memset(aczSendMsg, 0, sizeof(aczSendMsg));
        len = ConvertPoolToPkt(aczSendMsg,sizeof(aczSendMsg), pktfmt);
        if (len < 0)
        {
            err_log("生成SOP(%s)的上传报文失败(%d)", pktfmt, len);
            return -1;
        }
        memcpy(sopbuf+iLen, aczSendMsg, len);
        iLen+=len;
#endif
    }

    xmlFreeDoc(cvt);
    //xmlFreeDoc(doc);

#if 1 
    // 转换错误码
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
    err_log("同城:result=[%s] desc=[%s]", buf, errinfo);
    err_tc2sop(nbuf, buf);
    xmlFreeDoc(doc);

    // 设置返回报文字段
    sop_retinfo(nbuf, errinfo);

func_handler:

    /* 打印日志 sop.log */
    POOL_TraceData();
    /* 生成SOP的下传报文 */
    memset(aczSendMsg, 0, sizeof(aczSendMsg));
    len = ConvertPoolToPkt(aczSendMsg,sizeof(aczSendMsg), pktfmt);
    if (len < 0)
    {
        err_log("生成SOP(%s)的上传报文失败(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, aczSendMsg, len);

    return len;
#endif

    //err_log("报文长度[%d]", iLen);
    //return iLen;
}

int init_sop_head()
{
    char aczBuf[512];
    char aczTemp[64];

    memset(aczBuf,0,sizeof(aczBuf));

    /* 系统信息头 system_head.cfg */
    /* 报文长度由系统自动加入 */
    PPutMem("sysSADDR",aczBuf,4);       /* 源地址 */
    PPutMem("sysDADDR",aczBuf,4);       /* 目的地址 */
    PPutMem("sysRSRVD",aczBuf,1);       /* 系统保留位 */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* 信息结束标志 */
    PPutMem("sysSEQNUM",aczBuf,2);      /* 报文序号 */
    PPutMem("sysMACFLAG",aczBuf,1);     /* 校验标志 */
    PPutMem("sysMACVALUE",aczBuf,8);    /* 校验值 */

    /* 公共交易头 cmtran_head.cfg */
    //PPutStr("PDWSNO", "1a");             /* 终端号 */
    PPutStr("PDCTNO", "    ");           /* 城市代码 */

    /* 交易数据头 tran_head.cfg */
    PPutStr("PDTRSD", "");               /* 交易子码 */
    PPutStr("PDTRMD", "");               /* 交易模式 */
    aczTemp[0]=0x00;
    aczTemp[1]=0x01;
    PPutMem("PDTRSQ", aczTemp,2);        /* 交易序号 */
    aczTemp[0]=0xff;
    aczTemp[1]=0xff;
    PPutMem("PDOFF1", aczTemp,2);        /* 系统偏移0xffff */
    PPutMem("PDOFF2", aczTemp,2);        /* 系统偏移0xffff */
    PPutStr("PDAUUS","");       /* 授权柜员 */
    PPutStr("PDAUPS","");       /* 授权密码 */  

    return 0;
}

int sop_trans_qry_sz43(char *sopbuf)
{
    err_log("接收到交易请求:提入贷方查询sz43,开始读入查询条件...");
    int i;
    int rc=0;
    int len=0;
    int reqcount=0;
    int nowcount=0;
    int rowcount=0;
    int acctflag=-1;//入账标志(结果)
    int cacctflag=0;//入账标志(条件)
    char aczSuff[128]={0};
    char pktfmt[16]={0};
    char grid[8]={0};
    char sqlstr[1024]={0};
    char tmpbuf[10240]={0};
    int icurtime=0;
    char archivedate[9]={0};
    char tablename[128]="trnjour";
    char notetype[8]={0};
    result_set rs,rstmp;

    char PDWSNO[6];  PGetStr("PDWSNO",PDWSNO,sizeof(PDWSNO));err_log("终端号        [PDWSNO]=[%s]",PDWSNO);//终端号
    char PDSBNO[5];  PGetStr("PDSBNO",PDSBNO,sizeof(PDSBNO));err_log("机构代码      [PDSBNO]=[%s]",PDSBNO);//机构代码
    char PDUSID[9];  PGetStr("PDUSID",PDUSID,sizeof(PDUSID));err_log("交易柜员      [PDUSID]=[%s]",PDUSID);//交易柜员
    char YNGYJG[5];  PGetStr("YNGYJG",YNGYJG,sizeof(YNGYJG));err_log("营业机构号    [YNGYJG]=[%s]",YNGYJG);//营业机构
    //char ZYWXYH[9];  PGetStr("ZYWXYH",ZYWXYH,sizeof(ZYWXYH));err_log("中间业务协议号[ZYWXYH]=[%s]",ZYWXYH);//中间业务
    char SHKRZH[33]; PGetStr("SHKRZH",SHKRZH,sizeof(SHKRZH));err_log("收款人帐号    [SHKRZH]=[%s]",SHKRZH);//收款人帐
    char SHKRXM[63]; PGetStr("SHKRXM",SHKRXM,sizeof(SHKRXM));err_log("收款人姓名    [SHKRXM]=[%s]",SHKRXM);//收款人姓
    //char ZJCYHH[13]; PGetStr("ZJCYHH",ZJCYHH,sizeof(ZJCYHH));err_log("直接参与行号  [ZJCYHH]=[%s]",ZJCYHH);//直接参与
    //char SBHHAO[13]; PGetStr("SBHHAO",SBHHAO,sizeof(SBHHAO));err_log("收报行行号    [SBHHAO]=[%s]",SBHHAO);//收报行行
    char FUKRZH[33]; PGetStr("FUKRZH",FUKRZH,sizeof(FUKRZH));err_log("付款人帐号    [FUKRZH]=[%s]",FUKRZH);//付款人帐
    char FUKRXM[63]; PGetStr("FUKRXM",FUKRXM,sizeof(FUKRXM));err_log("付款人姓名    [FUKRXM]=[%s]",FUKRXM);//付款人姓
    //char WAIGDM[13]; PGetStr("WAIGDM",WAIGDM,sizeof(WAIGDM));err_log("外管代码      [WAIGDM]=[%s]",WAIGDM);//外管代码
    //char HUOBFH[4];  PGetStr("HUOBFH",HUOBFH,sizeof(HUOBFH));err_log("货币符号      [HUOBFH]=[%s]",HUOBFH);//货币符号
    char JIO1JE[14]; PGetStr("JIO1JE",JIO1JE,sizeof(JIO1JE));err_log("交易金额      [JIO1JE]=[%s]",JIO1JE);//交易金额
    //char PNGZZL[3];  PGetStr("PNGZZL",PNGZZL,sizeof(PNGZZL));err_log("凭证种类      [PNGZZL]=[%s]",PNGZZL);//凭证种类
    //char PNGZPH[2];  PGetStr("PNGZPH",PNGZPH,sizeof(PNGZPH));err_log("凭证批号      [PNGZPH]=[%s]",PNGZPH);//凭证批号
    char PNGZHH[14]; PGetStr("PNGZHH",PNGZHH,sizeof(PNGZHH));err_log("凭证号        [PNGZHH]=[%s]",PNGZHH);//凭证号  
    char JIOHLX[2];  PGetStr("JIOHLX",JIOHLX,sizeof(JIOHLX));err_log("交换类型      [JIOHLX]=[%s]",JIOHLX);//交换类型
    char JIOHRQ[9];  PGetStr("JIOHRQ",JIOHRQ,sizeof(JIOHRQ));err_log("交换日期      [JIOHRQ]=[%s]",JIOHRQ);//交换日期
    char JIOHCC[2];  PGetStr("JIOHCC",JIOHCC,sizeof(JIOHCC));err_log("交换场次      [JIOHCC]=[%s]",JIOHCC);//交换场次
    //char RUZHRQ[9];  PGetStr("RUZHRQ",RUZHRQ,sizeof(RUZHRQ));err_log("入帐日期      [RUZHRQ]=[%s]",RUZHRQ);//入帐日期
    //char CHUPRQ[9];  PGetStr("CHUPRQ",CHUPRQ,sizeof(CHUPRQ));err_log("出票日期      [CHUPRQ]=[%s]",CHUPRQ);//出票日期
    //char FUKURQ[9];  PGetStr("FUKURQ",FUKURQ,sizeof(FUKURQ));err_log("付款日期      [FUKURQ]=[%s]",FUKURQ);//付款日期
    //char WEITRQ[9];  PGetStr("WEITRQ",WEITRQ,sizeof(WEITRQ));err_log("委托日期      [WEITRQ]=[%s]",WEITRQ);//委托日期
    //char FUHEBZ[2];  PGetStr("FUHEBZ",FUHEBZ,sizeof(FUHEBZ));err_log("复核标志      [FUHEBZ]=[%s]",FUHEBZ);//复核标志
    char RUZHBZ[2];  PGetStr("RUZHBZ",RUZHBZ,sizeof(RUZHBZ));err_log("入帐标志      [RUZHBZ]=[%s]",RUZHBZ);//入帐标志
    //char HXIOZT[2];  PGetStr("HXIOZT",HXIOZT,sizeof(HXIOZT));err_log("核销状态      [HXIOZT]=[%s]",HXIOZT);//核销状态
    char TUIHBZ[2];  PGetStr("TUIHBZ",TUIHBZ,sizeof(TUIHBZ));err_log("退汇标志      [TUIHBZ]=[%s]",TUIHBZ);//退汇标志
    //char LZBWZT[2];  PGetStr("LZBWZT",LZBWZT,sizeof(LZBWZT));err_log("来帐报文状态  [LZBWZT]=[%s]",LZBWZT);//来帐报文
    //char FUKUQX[11]; PGetStr("FUKUQX",FUKUQX,sizeof(FUKUQX));err_log("付款期限      [FUKUQX]=[%s]",FUKUQX);//付款期限
    //char SHULIA[11]; PGetStr("SHULIA",SHULIA,sizeof(SHULIA));err_log("数量          [SHULIA]=[%s]",SHULIA);//数量    
    //char QISHBS[5];  PGetStr("QISHBS",QISHBS,sizeof(QISHBS));err_log("起始笔数      [QISHBS]=[%s]",QISHBS);//起始笔数
    char CXUNBS[3];  PGetStr("CXUNBS",CXUNBS,sizeof(CXUNBS));err_log("查询笔数      [CXUNBS]=[%s]",CXUNBS);//查询笔数
    //char JYDZLX[3];  PGetStr("JYDZLX",JYDZLX,sizeof(JYDZLX));err_log("交易对帐类型  [JYDZLX]=[%s]",JYDZLX);//交易对帐
    //char SHKHHM[63]; PGetStr("SHKHHM",SHKHHM,sizeof(SHKHHM));err_log("收款行名      [SHKHHM]=[%s]",SHKHHM);//收款行名
    //char FUHEGY[9];  PGetStr("FUHEGY",FUHEGY,sizeof(FUHEGY));err_log("复核柜员      [FUHEGY]=[%s]",FUHEGY);//复核柜员
    //char FHGYLS[13]; PGetStr("FHGYLS",FHGYLS,sizeof(FHGYLS));err_log("复核柜员流水号[FHGYLS]=[%s]",FHGYLS);//复核柜员
    //char PZHTJH[9];  PGetStr("PZHTJH",PZHTJH,sizeof(PZHTJH));err_log("凭证提交号    [PZHTJH]=[%s]",PZHTJH);//凭证提交
    //char JINBRQ[9];  PGetStr("JINBRQ",JINBRQ,sizeof(JINBRQ));err_log("经办日期      [JINBRQ]=[%s]",JINBRQ);//经办日期

    reqcount=atoi(CXUNBS);
    if(reqcount<1||reqcount>20)
        reqcount=20;

    getSysPara(archivedate,"ARCHIVEDATE");
    if(strcmp(JIOHRQ,archivedate)<=0)
        strcpy(tablename, "htrnjour");

    err_log("开始初始化sop报文头...");
    init_sop_head();
    PPutStr("PDTRCD","sz43");
    PPutStr("PDWSNO",PDWSNO);
    PPutStr("PDSBNO",PDSBNO);
    PPutStr("PDUSID",PDUSID);
    PPutStr("PDQTDT",getDate(0));
    PPutStr("PDTRDT",getDate(0));
    icurtime=atoi(getTime(0));
    memrev((char*)&icurtime,sizeof(int));
    PPutMem("PDTRTM",&icurtime,sizeof(int));

    err_log("开始准备SQL语句...");
    //len+=sprintf(sqlstr+len,"set rowcount %d select * from %s where nodeid=10 and inoutflag='2' and dcflag='2'",reqcount,tablename);
    len+=sprintf(sqlstr+len,"select * from %s where nodeid=10 and inoutflag='2' and dcflag='2'",tablename);
    if(strcmp(YNGYJG,""))//机构号
    {
        if(db_query_str(tmpbuf,13,"select exchno from bankinfo where bankid like '%%%s%%'",YNGYJG)!=0)
        {
            err_log("根据机构号[%s]查找行号失败!",YNGYJG);
            sop_retinfo(YNGYJG, "根据机构号查找行号失败!");
            goto READY_TO_SEND;
        }
        len+=sprintf(sqlstr+len," and acceptor='%s'",tmpbuf);
    }
    if(strcmp(JIOHRQ,""))//交换日期
        len+=sprintf(sqlstr+len," and workdate='%s'",JIOHRQ);
    if(strcmp(JIOHCC,"")&&strcmp(JIOHCC,"0"))//交换场次
        len+=sprintf(sqlstr+len," and exchground='%s'",JIOHCC);
    if(strcmp(SHKRZH,""))//付款账号
        len+=sprintf(sqlstr+len," and payingacct='%s'",SHKRZH);
    if(strcmp(SHKRXM,""))//付款姓名
        len+=sprintf(sqlstr+len," and payer like '%%%s%%'",SHKRXM);
    if(strcmp(FUKRZH,""))//收款账号
        len+=sprintf(sqlstr+len," and beneacct='%s'",FUKRZH);
    if(strcmp(FUKRXM,""))//收款姓名
        len+=sprintf(sqlstr+len," and benename like '%%%s%%'",FUKRXM);
    if(strcmp(JIO1JE,"")&&strcmp(JIO1JE,"0.00"))//交易金额
        len+=sprintf(sqlstr+len," and settlamt=%s",JIO1JE);
    /*
       if(strcmp(PNGZZL,""))//凭证种类
       {
    //if((rc = notetype_b2c(tmpbuf,"sz37",PNGZZL,"","","")) < 0)//这里需要和行内人员沟通下
    //return -1;
    //len+=sprintf(sqlstr+len," and notetype='%s'",tmpbuf);
    len+=sprintf(sqlstr+len," and notetype='%s'",PNGZZL);
    }
     */
    if(strcmp(PNGZHH,""))//凭证号
        len+=sprintf(sqlstr+len," and noteno='%s'",PNGZHH);
    /*
    if(strcmp(TUIHBZ,""))//退票标志
        len+=sprintf(sqlstr+len," and tpflag='%s'",TUIHBZ);
        */
    if(strcmp(JIOHLX,""))//交换类型
    {
        if( atoi(JIOHLX) == 2 ) //提入贷方
            //len+=sprintf(sqlstr+len," and tpflag!='1'");
            len+=sprintf(sqlstr+len," and trncode in('2','0002') ");
        else if( atoi(JIOHLX) == 5 ) //退票
            len+=sprintf(sqlstr+len," and trncode in('7','0007') ");
            //len+=sprintf(sqlstr+len," and tpflag='1'");
        else
        {
            rowcount=0;
            sop_retinfo("-2", "交换类型错误");
            goto READY_TO_SEND;
        }
    }
    if(strcmp(RUZHBZ,"1")==0)//入账标志
        cacctflag=1;
    else
        cacctflag=0;
    //len+=sprintf(sqlstr+len," set rowcount 0");
    //其他字段待测试后加入

    err_log("开始执行SQL语句...");
    if((rc=db_query(&rs,sqlstr))!=0)
    {
        if(rc==E_DB_NORECORD)
        {
            rowcount=0;
            sop_retinfo("-2", "没有记录");
            goto READY_TO_SEND;
        }
        else
            return -1;
    }

    err_log("开始组建sop表格数据...");
    rowcount=db_row_count(&rs);
    err_log("找到记录[%d],开始根据记账条件进行过滤..",rowcount);
    for(i=0;i<rowcount && nowcount<reqcount;i++)
    {
        /*****开始:判断是否已入账*****/
        if(*db_cell_by_name(&rs,i,"chkflag") == '1' || *db_cell_by_name(&rs,i,"tpflag") == '1' )
        {
            acctflag=1;
        }
        else
        {
            rc = db_query(&rstmp,"select result,revserial,workdate from acctjour where nodeid=10 and inoutflag='2'"
                    " and originator='%s' and workdate='%s' and refid='%s' and trncode='8248' ",
                    db_cell_by_name(&rs,i,"originator"), db_cell_by_name(&rs,i,"workdate"), db_cell_by_name(&rs,i,"refid"));
            acctflag=-1;
            if(rc!=0)
            {
                if(rc!=E_DB_NORECORD)
                {
                    db_free_result(&rs);
                    return -1;
                }
            }
            else if(*db_cell(&rstmp,0,0)=='1')
            {
                acctflag=1;
            }
            else if(*db_cell(&rstmp,0,0)=='5')
            {
                acctflag=0;
            }
        }

        if(acctflag!=cacctflag)
        {
            err_log("过滤交易[%s][%s][%s][%s]*",
                    db_cell_by_name(&rs,i,"originator"),
                    db_cell_by_name(&rs,i,"workdate"),
                    db_cell_by_name(&rs,i,"refid"),
                    db_cell_by_name(&rs,i,"notetype"));
            continue;
        }
        err_log("保留交易[%s][%s][%s][%s]",
                db_cell_by_name(&rs,i,"originator"),
                db_cell_by_name(&rs,i,"workdate"),
                db_cell_by_name(&rs,i,"refid"),
                db_cell_by_name(&rs,i,"notetype"));
        /*****结束:判断是否已入账*****/

        // 序号从0开始
        sprintf(aczSuff,"_%d",nowcount++);
        POOL_SetPutSuff(aczSuff);

        //PPutStr("SHHUDH",db_cell_by_name(&rs,i,"refid"));//业务流水号
        PPutStr("JIOHRQ",db_cell_by_name(&rs,i,"workdate"));//交换日期
        PPutStr("JYDZLX","00");//交易对帐类型
        PPutStr("JIOHLX","2");//交换类型
        PPutStr("TCHTCH",db_cell(&rstmp,0,1));//同城提出号
        PPutStr("JIO1JE",db_cell_by_name(&rs,i,"settlamt"));//交易金额
        PPutStr("CHUPRQ",db_cell_by_name(&rs,i,"issuedate"));//出票日期
        PPutStr("FUKURQ","");//提示付款日期
        PPutStr("WEITRQ","");//委托日期
        PPutStr("ZYWXYH","");//交易序号//中间业务协议号
        PPutStr("LZBWZT","");//止付标志//来帐报文状态
        PPutStr("PZHTJH","");//止付序号//凭证提交号
        PPutStr("JINBRQ","");//止付日期//经办日期
        PPutStr("KHZHLX","");//回执标志//客户帐号类型
        PPutStr("TNGZRQ","");//回执日期//通知日期
        PPutStr("MSGIDN","");//回执序号//报文标识号
        PPutStr("FUKUQX","");//回执期限//付款期限
        PPutStr("JIOHCC",db_cell_by_name(&rs,i,"exchground"));//交换场次
        PPutStr("FUHEBZ","0");//复核标志
        if(notetype_c2b_ex(db_cell_by_name(&rs,i,"notetype"),notetype,tmpbuf,0)<0)
        {
            err_log("notetype_c2b() fail");
            db_free_result(&rs);
            db_free_result(&rstmp);
            return -1;
        }
        PPutStr("PNGZZL",notetype);//凭证种类
        //PPutStr("PNGZZL",db_cell_by_name(&rs,i,"notetype"));//凭证种类
        PPutStr("PNGZPH","");//凭证批号
        PPutStr("PNGZHH",db_cell_by_name(&rs,i,"noteno"));//凭证号
        PPutStr("HUOBFH","");//货币符号
        PPutStr("GUIYLS","");//柜员流水号
        PPutStr("FHGYLS","");//复核柜员流水号
        PPutStr("TUIHBZ",db_cell_by_name(&rs,i,"tpflag"));//退票标志
        //PPutStr("TUIPLY","");//退票理由
        PPutStr("TUIPLY",db_cell_by_name(&rs,i,"purpose"));//退票理由
        PPutStr("RUZHBZ",acctflag?"1":"0");//入帐标志
        PPutStr("RUZHRQ",db_cell(&rstmp,0,2));//入帐日期
        PPutStr("FUKRZH",db_cell_by_name(&rs,i,"payingacct"));//付款人帐号
        PPutStr("FUKRXM",db_cell_by_name(&rs,i,"payer"));//付款人名称
        PPutStr("WAIGDM","");//付款行支付号
        PPutStr("SHKRZH",db_cell_by_name(&rs,i,"beneacct"));//收款人帐号
        PPutStr("SHKRXM",db_cell_by_name(&rs,i,"benename"));//收款人名称
        PPutStr("ZJCYHH","");//收款行支付号
        PPutStr("SBHHAO","");//接收行支付号
        PPutStr("SHKHHM","");//收款行名
        PPutStr("ZHYODM","");//摘要代码
        PPutStr("SHULIA","");//背书次数
        PPutStr("BEIZXX","");//备注信息
        PPutStr("FUHEGY","");//复核柜员

        POOL_SetPutSuff("");

        db_free_result(&rstmp);
    }
    db_free_result(&rs);


    if(nowcount==0)
    {
        sop_retinfo("-2", "没有记录");
        goto READY_TO_SEND;
    }
    err_log("最终返回记录[%d]",nowcount);

    strcpy(grid,"Fsz431");
    sprintf(tmpbuf, "%s_Rows", grid);
    PPutMem(tmpbuf, &nowcount, sizeof(int));

    len=0;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    err_log("开始打包表格...");
    if((rc = SOP_PackForm(grid, '0', tmpbuf, sizeof(tmpbuf), &len)) < 0)
    {
        err_log("SOP_PackForm() ret = %d.", rc);
        return -1;
    }

    PPutMem(grid, tmpbuf, len);
    save_pack(grid, tmpbuf, len);
    PPutStr("TPU_RetCode", "AAAAAAA");

READY_TO_SEND:
    sprintf(pktfmt, "Osz432@R");
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if((len = ConvertPoolToPkt(tmpbuf, sizeof(tmpbuf), pktfmt)) < 0)
    {
        err_log("生成SOP(%s)的上传报文失败(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, tmpbuf, len);

    return len;
}

int sop_trans_qry_sz44(char *sopbuf)
{
    err_log("接收到交易请求:提入借方查询sz44,开始读入查询条件...");
    int i;
    int rc=0;
    int len=0;
    int reqcount=0;
    int nowcount=0;
    int rowcount=0;
    int acctflag=0;//入账标志(结果)
    int cacctflag=0;//入账标志(条件)
    char aczSuff[128]={0};
    char pktfmt[16]={0};
    char grid[8]={0};
    char sqlstr[1024]={0};
    char tmpbuf[10240]={0};
    char notetype[8]={0};
    int icurtime=0;
    char archivedate[9]={0};
    char tablename[128]="trnjour";
    result_set rs,rstmp;

    char PDWSNO[6];  PGetStr("PDWSNO",PDWSNO,sizeof(PDWSNO));err_log("终端号      [PDWSNO]=[%s]",PDWSNO);//终端号
    char PDSBNO[5];  PGetStr("PDSBNO",PDSBNO,sizeof(PDSBNO));err_log("机构代码    [PDSBNO]=[%s]",PDSBNO);//机构代码
    char PDUSID[9];  PGetStr("PDUSID",PDUSID,sizeof(PDUSID));err_log("交易柜员    [PDUSID]=[%s]",PDUSID);//交易柜员
    char YNGYJG[5];  PGetStr("YNGYJG",YNGYJG,sizeof(YNGYJG));err_log("营业机构号  [YNGYJG]=[%s]",YNGYJG);
    char JIOHLX[2];  PGetStr("JIOHLX",JIOHLX,sizeof(JIOHLX));err_log("交换类型    [JIOHLX]=[%s]",JIOHLX);
    char JIOHRQ[18]; PGetStr("JIOHRQ",JIOHRQ,sizeof(JIOHRQ));err_log("交换日期    [JIOHRQ]=[%s]",JIOHRQ);
    char TRJHCC[33]; PGetStr("TRJHCC",TRJHCC,sizeof(TRJHCC));err_log("提入交换场次[TRJHCC]=[%s]",TRJHCC);
    //char PNGZZL[63]; PGetStr("PNGZZL",PNGZZL,sizeof(PNGZZL));err_log("凭证种类    [PNGZZL]=[%s]",PNGZZL);
    char BIAOZI[2];  PGetStr("BIAOZI",BIAOZI,sizeof(BIAOZI));err_log("截留标志    [BIAOZI]=[%s]",BIAOZI);
    char JIO1JE[33]; PGetStr("JIO1JE",JIO1JE,sizeof(JIO1JE));err_log("交易金额    [JIO1JE]=[%s]",JIO1JE);
    char SHKRZH[63]; PGetStr("SHKRZH",SHKRZH,sizeof(SHKRZH));err_log("收款人帐号  [SHKRZH]=[%s]",SHKRZH);
    char SHKRXM[43]; PGetStr("SHKRXM",SHKRXM,sizeof(SHKRXM));err_log("收款人姓名  [SHKRXM]=[%s]",SHKRXM);
    char FUKRZH[14]; PGetStr("FUKRZH",FUKRZH,sizeof(FUKRZH));err_log("付款人帐号  [FUKRZH]=[%s]",FUKRZH);
    char FUKRXM[23]; PGetStr("FUKRXM",FUKRXM,sizeof(FUKRXM));err_log("付款人姓名  [FUKRXM]=[%s]",FUKRXM);
    char TUIHBZ[3];  PGetStr("TUIHBZ",TUIHBZ,sizeof(TUIHBZ));err_log("退汇标志    [TUIHBZ]=[%s]",TUIHBZ);
    char RUZHBZ[3];  PGetStr("RUZHBZ",RUZHBZ,sizeof(RUZHBZ));err_log("入帐标志    [RUZHBZ]=[%s]",RUZHBZ);
    //char QISHBS[2];  PGetStr("QISHBS",QISHBS,sizeof(QISHBS));err_log("起始笔数    [QISHBS]=[%s]",QISHBS);
    char CXUNBS[9];  PGetStr("CXUNBS",CXUNBS,sizeof(CXUNBS));err_log("查询笔数    [CXUNBS]=[%s]",CXUNBS);

    //查询笔数
    reqcount=atoi(CXUNBS);
    if(reqcount<1||reqcount>20)
        reqcount=20;

    getSysPara(archivedate,"ARCHIVEDATE");
    if(strcmp(JIOHRQ,archivedate)<=0)
        strcpy(tablename, "htrnjour");

    err_log("开始初始化sop报文头...");
    init_sop_head();
    PPutStr("PDTRCD","sz44");
    PPutStr("PDWSNO",PDWSNO);
    PPutStr("PDSBNO",PDSBNO);
    PPutStr("PDUSID",PDUSID);
    PPutStr("PDQTDT",getDate(0));
    PPutStr("PDTRDT",getDate(0));
    icurtime=atoi(getTime(0));
    memrev((char*)&icurtime,sizeof(int));
    PPutMem("PDTRTM",&icurtime,sizeof(int));

    err_log("开始准备SQL语句...");
    //len+=sprintf(sqlstr+len,"set rowcount %d select * from %s where nodeid=10 and inoutflag='2' and dcflag='1'",reqcount,tablename);
    len+=sprintf(sqlstr+len,"select * from %s where nodeid=10 and inoutflag='2' and dcflag='1'",tablename);
    if(strcmp(YNGYJG,""))//机构号
    {
        if(db_query_str(tmpbuf,13,"select exchno from bankinfo where bankid like '%%%s%%'",YNGYJG)!=0)
        {
            err_log("根据机构号[%s]查找行号失败!",YNGYJG);
            sop_retinfo(YNGYJG,"根据机构号查找行号失败!");
            goto READY_TO_SEND;
        }
        len+=sprintf(sqlstr+len," and acceptor='%s'",tmpbuf);
    }
    if(strcmp(JIOHRQ,""))//交换日期
        len+=sprintf(sqlstr+len," and workdate='%s'",JIOHRQ);
    if(strcmp(TRJHCC,"")&&strcmp(TRJHCC,"0"))//交换场次
        len+=sprintf(sqlstr+len," and exchground='%s'",TRJHCC);
    /*
       if(strcmp(PNGZZL,""))//凭证种类
       {
    //if((rc = notetype_b2c(tmpbuf,"sz36",PNGZZL,"","","")) < 0)//这里需要和行内人员沟通下
    //return -1;
    //len+=sprintf(sqlstr+len," and notetype='%s'",tmpbuf);
    len+=sprintf(sqlstr+len," and notetype='%s'",PNGZZL);
    }
     */
    if(strcmp(JIO1JE,"")&&strcmp(JIO1JE,"0.00"))//交易金额
        len+=sprintf(sqlstr+len," and settlamt=%s",JIO1JE);
    if(strcmp(SHKRZH,""))//收款人账号
        len+=sprintf(sqlstr+len," and payingacct='%s'",SHKRZH);
    if(strcmp(SHKRXM,""))//收款人姓名
        len+=sprintf(sqlstr+len," and payer like '%%%s%%'",SHKRXM);
    if(strcmp(FUKRZH,""))//付款人账号
        len+=sprintf(sqlstr+len," and beneacct='%s'",FUKRZH);
    if(strcmp(FUKRXM,""))//付款人姓名
        len+=sprintf(sqlstr+len," and benename like '%%%s%%'",FUKRXM);
    /*
    if(strcmp(TUIHBZ,""))//退票标志
        len+=sprintf(sqlstr+len," and tpflag = '%s'",TUIHBZ);
        */
    if(strcmp(JIOHLX,""))//退票标志
    {
        if( atoi(JIOHLX) == 3 )//提入借方
            len+=sprintf(sqlstr+len," and trncode in('1','0001')");
            //len+=sprintf(sqlstr+len," and tpflag != '1'");
        else if(atoi(JIOHLX) == 6) //退票
            len+=sprintf(sqlstr+len," and trncode in('7','0007')");
            //len+=sprintf(sqlstr+len," and tpflag = '1'");
        else
        {
            rowcount=0;
            sop_retinfo("-2", "交换类型错误");
            goto READY_TO_SEND;
        }
    }
    if(strcmp(RUZHBZ,"1")==0)//入账标志
        cacctflag=1;
    else
        cacctflag=0;
    if(strcmp(BIAOZI,"1")==0)//截留标志
        len+=sprintf(sqlstr+len," and truncflag='1'");
    else
        len+=sprintf(sqlstr+len," and truncflag!='1'");
    //len+=sprintf(sqlstr+len," set rowcount 0");

    err_log("开始执行SQL语句...");
    if((rc=db_query(&rs,sqlstr))!=0)
    {
        if(rc==E_DB_NORECORD)
        {
            rowcount=0;
            sop_retinfo("-2", "没有记录");
            goto READY_TO_SEND;
        }
        else
            return -1;
    }

    err_log("开始组建sop表格数据...");
    rowcount=db_row_count(&rs);
    err_log("找到记录[%d],开始根据记账条件进行过滤...",rowcount);
    for(i=0;i<rowcount && nowcount<reqcount;i++)
    {
        acctflag=-1;
        //如果确认标志为1则为入账成功
        //if( atoi(db_cell_by_name(&rs,i,"chkflag")) == 1 )
        if(*db_cell_by_name(&rs,i,"chkflag") == '1' || *db_cell_by_name(&rs,i,"tpflag") == '1' )
            acctflag=1;
        else
        {

            /*****开始:判断是否已入账*****/
            rc = db_query(&rstmp,"select result,revserial,workdate from acctjour where nodeid=10 and inoutflag='2'"
                    " and originator='%s' and workdate='%s' and refid='%s'",
                    db_cell_by_name(&rs,i,"originator"), db_cell_by_name(&rs,i,"workdate"), db_cell_by_name(&rs,i,"refid"));
            if(rc!=0)
            {
                if(rc==E_DB_NORECORD)
                {
                    acctflag=0;
                }
                else
                {
                    db_free_result(&rs);
                    return -1;
                }
            }
            else if(*db_cell(&rstmp,0,0)=='1')
            {
                acctflag=1;
            }
            else if(*db_cell(&rstmp,0,0)=='9')
            {
                acctflag=0;
            }
        }

        if(acctflag!=cacctflag)
        {
            err_log("过滤交易[%s][%s][%s][%s]*",
                    db_cell_by_name(&rs,i,"originator"),
                    db_cell_by_name(&rs,i,"workdate"),
                    db_cell_by_name(&rs,i,"refid"),
                    db_cell_by_name(&rs,i,"notetype"));
            continue;
        }
        err_log("保留交易[%s][%s][%s][%s]",
                db_cell_by_name(&rs,i,"originator"),
                db_cell_by_name(&rs,i,"workdate"),
                db_cell_by_name(&rs,i,"refid"),
                db_cell_by_name(&rs,i,"notetype"));
        /*****结束:判断是否已入账*****/

        // 序号从0开始
        sprintf(aczSuff,"_%d",nowcount++);
        POOL_SetPutSuff(aczSuff);

        PPutStr("BAOWLS",db_cell_by_name(&rs,i,"refid"));//业务流水号
        PPutStr("QSZXDH","0089");//清算中心代号
        PPutStr("JIOHRQ",db_cell_by_name(&rs,i,"workdate"));//交换日期
        PPutStr("JIOHLX","3");//交换类型//借方3贷方2
        PPutStr("FQHHAO",db_cell_by_name(&rs,i,"originator"));//提出行交换号
        PPutStr("QISHHH",db_cell_by_name(&rs,i,"acceptor"));//提入行交换号
        PPutStr("TCHTCH",rc==E_DB_NORECORD?"":db_cell(&rstmp,0,1));//同城提出号
        PPutStr("JIO1JE",db_cell_by_name(&rs,i,"settlamt"));//交易金额
        PPutStr("TRJHCC",db_cell_by_name(&rs,i,"exchground"));//交换场次
        PPutStr("FUHEBZ","0");//复核标志
        if(notetype_c2b_ex(db_cell_by_name(&rs,i,"notetype"),notetype,tmpbuf,0)<0)
        {
            err_log("notetype_c2b() fail");
            db_free_result(&rs);
            db_free_result(&rstmp);
            return -1;
        }
        PPutStr("PNGZZL",notetype);//凭证种类
        //PPutStr("PNGZZL",db_cell_by_name(&rs,i,"notetype"));//凭证种类
        PPutStr("PNGZPH","");//凭证批号
        PPutStr("PNGZXH",db_cell_by_name(&rs,i,"noteno"));//凭证序号
        PPutStr("GUIYLS","");//柜员流水号
        PPutStr("FHGYLS","");//复核柜员流水号
        PPutStr("TUIHBZ",db_cell_by_name(&rs,i,"tpflag"));//退票标志
        PPutStr("TUIPLY","");//退票理由
        PPutStr("RUZHBZ",acctflag?"1":"0");//入帐标志
        PPutStr("FUKRZH",db_cell_by_name(&rs,i,"payingacct"));//付款人帐号
        PPutStr("FUKRXM",db_cell_by_name(&rs,i,"payer"));//付款人姓名
        PPutStr("WAIGDM","");//付款行支付号
        PPutStr("SHKRZH",db_cell_by_name(&rs,i,"beneacct"));//收款人帐号
        PPutStr("SHKRXM",db_cell_by_name(&rs,i,"benename"));//收款人姓名
        PPutStr("ZJCYHH","");//收款行支付号
        PPutStr("SBHHAO","");//接收行支付号
        PPutStr("SHKHHM","");//收款行名
        PPutStr("BEIZXX","");//备注信息

        POOL_SetPutSuff("");

        db_free_result(&rstmp);
    }
    db_free_result(&rs);

    if(nowcount==0)
    {
        sop_retinfo("-2", "没有记录");
        goto READY_TO_SEND;
    }
    err_log("最终返回记录[%d]",nowcount);

    strcpy(grid,"Fsz441");
    sprintf(tmpbuf, "%s_Rows", grid);
    PPutMem(tmpbuf, &nowcount, sizeof(int));

    len=0;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    err_log("开始打包表格...");
    if((rc = SOP_PackForm(grid, '0', tmpbuf, sizeof(tmpbuf), &len)) < 0)
    {
        err_log("SOP_PackForm() ret = %d.", rc);
        return -1;
    }

    PPutMem(grid, tmpbuf, len);
    save_pack(grid, tmpbuf, len);
    PPutStr("TPU_RetCode", "AAAAAAA");

READY_TO_SEND:
    sprintf(pktfmt, "Osz442@R");
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if((len = ConvertPoolToPkt(tmpbuf, sizeof(tmpbuf), pktfmt)) < 0)
    {
        err_log("生成SOP(%s)的上传报文失败(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, tmpbuf, len);

    return len;
}
enum Tax
{
    TraNo=0,
    TaxPayCode,
    TaxPayName,
    TreCode,
    TraAmt,
    TaxVouNo,
    BillDate,
    CorpCode,
    BudgetType,
    TrimSign,
    CorpType,
    Remark,
    Print,
    TaxSize1,
    TaxSize2=1024
};

int save_tax(char TaxPrintBuf[][TaxSize2],char *mode)
{
    int i,len=0;
    FILE *fp=NULL;
    char file[256]={0};
    char PrintBuf[20480]={0};
    char amount[101]={0};

    sprintf(file,"%s/consoleprint/%s.tax",getenv("HOME"),TaxPrintBuf[TaxVouNo]);
    if((fp=fopen(file,mode))==NULL)
    {
        err_log("无法以[%s]模式打开文件[%s]",mode,file);
        return -1;
    }
    if(strstr(mode,"+"))
    {
        len+=sprintf(PrintBuf+len,"\f");
    }
    else
    {
        //len+=sprintf(PrintBuf+len,"\n\n\n\n\n\n");
        len+=sprintf(PrintBuf+len,"%380s\n"," ");
    }
    MoneyToChinese(TaxPrintBuf[TraAmt], amount);
    len+=sprintf(PrintBuf+len,"  纳税人编码:%-20s 纳税人名称:%s\n",TaxPrintBuf[TaxPayCode],TaxPrintBuf[TaxPayName]);
    len+=sprintf(PrintBuf+len,"  国库代码:%-20s   开票日期:%s\n", TaxPrintBuf[TreCode], TaxPrintBuf[BillDate]);
    len+=sprintf(PrintBuf+len,"  交易金额:%s      大写金额:%s\n", TaxPrintBuf[TraAmt], amount);
#if 0 
    len+=sprintf(PrintBuf+len,"                                 大写金额:%s\n", amount);
    len+=sprintf(PrintBuf+len,"  税票号码:%-20s   开票日期:%s\n",TaxPrintBuf[TaxVouNo],TaxPrintBuf[BillDate]);
    len+=sprintf(PrintBuf+len,"  企业代码:%-20s   预算种类:%s 整理期标志:%s 企业注册类型:%s\n",
            TaxPrintBuf[CorpCode],TaxPrintBuf[BudgetType],TaxPrintBuf[TrimSign],TaxPrintBuf[CorpType]);
    len+=sprintf(PrintBuf+len,"      备注:%s\n\n",TaxPrintBuf[Remark]);
#endif
    len+=sprintf(PrintBuf+len,"%s\n",TaxPrintBuf[Print]);

    fwrite(PrintBuf,len,1,fp);
    fclose(fp);
    if(!strstr(mode,"+"))
        err_log("成功保存税票打印文件[%s]",file);
    return 0;
}

int sop_trans_qry_sz78(char *sopbuf,char *xmlbuf,int xmllen)
{
    err_log("接收到交易请求:税票信息查询sz78");
    int i;
    int rc=0;
    int len=0;
    int rowcount=0;
    char tmpnode[1024]={0};
    char aczSuff[128]={0};
    char pktfmt[16]={0};
    char grid[8]={0};
    char retcode[1024]={0};
    char retmsg[1024]={0};
    char tmpbuf[524288]={0};//1024*512
    int icurtime=0;
    xmlDocPtr doc = NULL;
    char TaxPrintBuf[TaxSize1][TaxSize2];
    int  iTaxTypeNum=0;
    char TaxTypeNum[3]={0};

    err_log("开始解析xml返回报文...");
    if ((doc = xmlParseMemory(xmlbuf, xmllen)) == NULL)
    {
        err_log("parse (%d) fail.", xmllen);
        save_xml("3022",xmlbuf,xmllen);
        return -1;
    }

    err_log("开始初始化sop报文头...");
    init_sop_head();
    PPutStr("PDTRCD","sz78");
    PPutStr("PDWSNO",XmlGetStringDup(doc,"//PDWSNO"));
    PPutStr("PDSBNO",XmlGetStringDup(doc,"//PDSBNO"));
    PPutStr("PDUSID",XmlGetStringDup(doc,"//PDUSID"));
    PPutStr("PDQTDT",getDate(0));
    PPutStr("PDTRDT",getDate(0));
    icurtime=atoi(getTime(0));
    memrev((char*)&icurtime,sizeof(int));
    PPutMem("PDTRTM",&icurtime,sizeof(int));

    // 转换错误码
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", retcode, sizeof(retcode));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", retmsg, sizeof(retmsg));
    err_log("同城:result=[%s] desc=[%s]", retcode, retmsg);
    err_tc2sop(retcode, retcode);
    if(strcmp(retcode,"AAAAAAA")!=0)
    {
        xmlFreeDoc(doc);
        sop_retinfo(retcode, retmsg);
        goto READY_TO_SEND;
    }

    rowcount=atoi(XmlGetStringDup(doc,"/UFTP/CountNum"));
    err_log("取得记录数[%d]",rowcount);
    if(rowcount==0)
    {
        xmlFreeDoc(doc);
        sop_retinfo("-2", "没有记录");
        goto READY_TO_SEND;
    }

    err_log("开始组建sop表格数据...");
    for(i=0;i<rowcount;i++)
    {
        sprintf(aczSuff,"_%d",i);
        POOL_SetPutSuff(aczSuff);

        sprintf(tmpnode,"/UFTP/TaxInfo%d/TraNo",i+1);      XmlGetString(doc,tmpnode,TaxPrintBuf[TraNo]     ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TreCode",i+1);    XmlGetString(doc,tmpnode,TaxPrintBuf[TreCode]   ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TraAmt",i+1);     XmlGetString(doc,tmpnode,TaxPrintBuf[TraAmt]    ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TaxVouNo",i+1);   XmlGetString(doc,tmpnode,TaxPrintBuf[TaxVouNo]  ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/BillDate",i+1);   XmlGetString(doc,tmpnode,TaxPrintBuf[BillDate]  ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TaxPayCode",i+1); XmlGetString(doc,tmpnode,TaxPrintBuf[TaxPayCode],TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TaxPayName",i+1); XmlGetString(doc,tmpnode,TaxPrintBuf[TaxPayName],TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/CorpCode",i+1);   XmlGetString(doc,tmpnode,TaxPrintBuf[CorpCode]  ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/BudgetType",i+1); XmlGetString(doc,tmpnode,TaxPrintBuf[BudgetType],TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/TrimSign",i+1);   XmlGetString(doc,tmpnode,TaxPrintBuf[TrimSign]  ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/CorpType",i+1);   XmlGetString(doc,tmpnode,TaxPrintBuf[CorpType]  ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/Remark",i+1);     XmlGetString(doc,tmpnode,TaxPrintBuf[Remark]    ,TaxSize2);
        sprintf(tmpnode,"/UFTP/TaxInfo%d/PrintInfo1",i+1); XmlGetString(doc,tmpnode,TaxPrintBuf[Print] ,TaxSize2);
        PPutStr("GUIYLS",TaxPrintBuf[TraNo]     );//交易流水
        PPutStr("SHOJGK",TaxPrintBuf[TreCode]   );//国库代码
        PPutStr("JIO1JE",TaxPrintBuf[TraAmt]    );//交易金额
        //PPutStr("ZZHAOM",TaxPrintBuf[TaxVouNo]  );//税票号码
        PPutStr("XINX03",TaxPrintBuf[TaxVouNo]  );//税票号码
        PPutStr("KAIPRQ",TaxPrintBuf[BillDate]  );//开票日期
        PPutStr("NASHDM",TaxPrintBuf[TaxPayCode]);//纳税人编码
        PPutStr("FUKRXM",TaxPrintBuf[TaxPayName]);//纳税人名称
        PPutStr("SHOKDW",TaxPrintBuf[CorpCode]  );//企业代码
        PPutStr("YUSUAN",TaxPrintBuf[BudgetType]);//预算种类
        PPutStr("SHFOBZ",TaxPrintBuf[TrimSign]  );//整理期标志
        PPutStr("MAIRBZ",TaxPrintBuf[CorpType]  );//企业注册类型
        PPutStr("BEIZXX",TaxPrintBuf[Remark]    );//备注

        save_tax(TaxPrintBuf,"w");

        sprintf(tmpnode,"/UFTP/TaxInfo%d/TaxTypeNum",i+1);
        XmlGetString(doc, tmpnode, TaxTypeNum, sizeof(TaxTypeNum));
        if( atoi(TaxTypeNum) > 7 )
        {
            sprintf(tmpnode,"/UFTP/TaxInfo%d/PrintInfo2",i+1);
            XmlGetString(doc,tmpnode,TaxPrintBuf[Print],sizeof(TaxPrintBuf[Print]));
            if(strcmp(TaxPrintBuf[Print],""))
            {
                strcpy(TaxPrintBuf[TraAmt],"见上页");
                save_tax(TaxPrintBuf,"a+");
            }
        }

        POOL_SetPutSuff("");
    }
    xmlFreeDoc(doc);

    strcpy(grid,"Fsz781");
    sprintf(tmpbuf, "%s_Rows", grid);
    PPutMem(tmpbuf, &rowcount, sizeof(int));

    len=0;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    err_log("开始打包表格...");
    if((rc = SOP_PackForm(grid, '0', tmpbuf, sizeof(tmpbuf), &len)) < 0)
    {
        err_log("SOP_PackForm() ret = %d.", rc);
        return -1;
    }

    PPutMem(grid, tmpbuf, len);
    save_pack(grid, tmpbuf, len);
    sop_retinfo(retcode, retmsg);

READY_TO_SEND:
    sprintf(pktfmt, "Osz782@R");
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if((len = ConvertPoolToPkt(tmpbuf, sizeof(tmpbuf), pktfmt)) < 0)
    {
        err_log("生成SOP(%s)的上传报文失败(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, tmpbuf, len);

    return len;
}
