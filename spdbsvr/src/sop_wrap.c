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
        err_log("��⵽��ֵ����."); \
        return ret; \
    } \
} while (0)

#define XMLFREE(a)  do {xmlFree(a); a = NULL;} while (0)
#define FREE(a)  do {free(a); a = NULL;} while (0)

/*ǰ̨����Ľ��ױ���ͷ������*/
static char PDTRCD1[5];
static char PDLDTC[5];
static char PDTRDT[9];
static char PDTRTM[5];
static char PDTLSQ[13];
static char PDERTR[3];
/*����ͷ*/
/*˰Ʊ��Ϣ��ӡ*/
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

    /* ϵͳ��Ϣͷ system_head.cfg */
    /* ���ĳ�����ϵͳ�Զ����� */
    PPutMem("sysSADDR",aczBuf,4);       /* Դ��ַ */
    PPutMem("sysDADDR",aczBuf,4);       /* Ŀ�ĵ�ַ */
    PPutMem("sysRSRVD",aczBuf,1);       /* ϵͳ����λ */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* ��Ϣ������־ */
    PPutMem("sysSEQNUM",aczBuf,2);      /* ������� */
    PPutMem("sysMACFLAG",aczBuf,1);     /* У���־ */
    PPutMem("sysMACVALUE",aczBuf,8);    /* У��ֵ */

    /* ��������ͷ cmtran_head.cfg */
    //PPutStr("PDWSNO", "1a");             /* �ն˺� */
    PPutStr("PDCTNO", "    ");           /* ���д��� */

    /* ��������ͷ tran_head.cfg */
    PPutStr("PDTRSD", "");               /* �������� */
    PPutStr("PDTRMD", "");               /* ����ģʽ */
    aczTemp[0]=0x00;
    aczTemp[1]=0x01;
    PPutMem("PDTRSQ", aczTemp,2);        /* ������� */
    aczTemp[0]=0xff;
    aczTemp[1]=0xff;
    PPutMem("PDOFF1", aczTemp,2);        /* ϵͳƫ��0xffff */
    PPutMem("PDOFF2", aczTemp,2);        /* ϵͳƫ��0xffff */
    PPutStr("PDAUUS","");       /* ��Ȩ��Ա */
    PPutStr("PDAUPS","");       /* ��Ȩ���� */  

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

    /* ϵͳ��Ϣͷ system_head.cfg */
    /* ���ĳ�����ϵͳ�Զ����� */
    PPutMem("sysSADDR",aczBuf,4);       /* Դ��ַ */
    PPutMem("sysDADDR",aczBuf,4);       /* Ŀ�ĵ�ַ */
    PPutMem("sysRSRVD",aczBuf,1);       /* ϵͳ����λ */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* ��Ϣ������־ */
    PPutMem("sysSEQNUM",aczBuf,2);      /* ������� */
    PPutMem("sysMACFLAG",aczBuf,1);     /* У���־ */
    PPutMem("sysMACVALUE",aczBuf,8);    /* У��ֵ */

    /* ��������ͷ cmtran_rcv_head.cfg */
    //PPutStr("PDTRCD","6140");           /* ���״��� */
    PPutStr("PDLDTC","");               /* ���������� */
    sprintf(aczBuf, "%08ld", current_date());
    PPutStr("PDTRDT",aczBuf);               /* �������� */
    iNow=htonl(current_time());            
    PPutMem("PDTRTM",(char *)&iNow,4);      /* ����ʱ�� */
    //PPutStr("PDTLSQ","888888880001");       /* ��Ա��ˮ�� */
    PGetMem("PDTRSQ",aczTemp,2);
    PPutMem("PDERTR",aczTemp,2);        /* ��������� */
    return;
}

void sop_retinfo(char *retcode, char *retmsg)
{
    err_log("[%s][%s]", retcode, retmsg);

    // ����sopͨ���ֶ�����
    sop_setinfo();

    //AAAAAAA����ɹ�Ӧ��
    if (strncmp(retcode, "AAAAAAA", 7) != 0)
    {
        PPutStr("TPU_Ctx1","ERR000");   // ʧ�ܶ��������̶�Ϊ��ERR000��
        PPutStr("TPU_Ctx2","-1");       // �����룬�̶�Ϊ��-1��
        PPutStr("TPU_RetMsg", retmsg);  // ������Ϣ��������82λ
    }
    PPutStr("TPU_RetCode", retcode); 

    return;
}

// ȡsop������
int get_sop_trncode(char *trncode, char *sopbuf)
{
    data_head_in data;

    *trncode = 0x00;

    // ��������ͷ
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
    /*����ͷ����*/
    PGetStr("PDTRCD",PDTRCD1,sizeof(PDTRCD1));err_log("���״���      [PDTRCD]=[%s]",PDTRCD1);//���״���
    PGetStr("PDLDTC",PDLDTC,sizeof(PDLDTC));err_log("����������      [PDLDTC]=[%s]",PDLDTC);//����������
    PGetStr("PDTRDT",PDTRDT,sizeof(PDTRDT));err_log("��������        [PDTRDT]=[%s]",PDTRDT);//��������
    PGetStr("PDTRTM",PDTRTM,sizeof(PDTRTM));err_log("����ʱ��        [PDTRTM]=[%s]",PDTRTM);//����ʱ��
    PGetStr("PDTLSQ",PDTLSQ,sizeof(PDTLSQ));err_log("ǰ̨��Ա��ˮ    [PDTLSQ]=[%s]",PDTLSQ);//ǰ̨��Ա��ˮ
    PGetStr("PDERTR",PDERTR,sizeof(PDERTR));err_log("���������    [PDERTR]=[%s]",PDERTR);//���������
    /*����ͷ����*/
    // ȡת�������ļ�
    sprintf(path, "%s/etc/cvt_sop2tc.xml", getAppDir());
    if ((cvt = xmlParseFile(path)) == NULL)
    {
        err_log("����%sʧ��!", path);
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

        /* ��POOL����ȡ������� */
        iLen = PGetMem(grid, acPkt, sizeof(acPkt));

        /* ���ñ�������� */
        iRetCode = SOP_UnpackForm('0', acPkt, iLen);
        if ( iRetCode < 0 )
        {
            err_log("SOP_UnpackForm fail, ret=[%d]", iRetCode);
            return -1;
        }

        /* ȡ���������� */
        sprintf(tmp, "%s_Rows", grid);
        PGetMem(tmp, &iRows, sizeof(int));
        err_log("%s=[%d]", tmp, iRows);
        sprintf(tmp, "_%d", 0); // ȡ��һ��
        POOL_SetGetSuff(tmp);   /* ����POOL����������׺ */
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

        // type������sop,const, �������ֶ�
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
                // ͨ�����õ�ת������ת������
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
                // ����
                strcpy(buf, value);
            }

            err_log("SET [%-30s]=[%s]", name, buf);
            if (XmlSetString(doc, name, buf) != 0)
                err_log("   SET FAIL [%-30s]=[%s]", name, buf);
        }
        // ���û������type(type==NULL), ȱʡȡ�ֶ���Ϊvalue��sop��������
        else if (type == NULL || strcmp(type, "sop") == 0)
        {
            memset(buf, 0, sizeof(buf));
            PGetStr(value, buf, sizeof(buf));
            if (sofile != NULL && func != NULL)
            {
                // ͨ�����õ�ת������ת������
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

    strcpy(tctype,XmlGetStringDup(doc,"//NoteType"));//ͬ��ƾ֤����
    strcpy(bktype,XmlGetStringDup(doc,"//PNGZZL"));  //����ƾ֤����
    strcpy(qtjyma,XmlGetStringDup(doc,"//QTJYMA"));  //ǰ̨������

    if(equal(qtjyma,"sz77")||
       equal(qtjyma,"sz78")||
       equal(qtjyma,"sz79")||
       equal(qtjyma,"sz80"))
    {
        //�������⴦��Ľ��ף���ֹ����ƾ֤ת��
    }
    else if(*tctype=='\0')
    {
        //ת��ƾ֤����:����->����
        err_log("ͬ��NoteTypeΪ��,��ʼ������ƾ֤ת��������ƾ֤");
        if(*bktype=='\0')
        {
            err_log("����ƾ֤����Ϊ��,����,����ת��");
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
        err_log("ͬ��NoteType��Ϊ�գ�����ƾ֤ת��");
    }

    strcpy(tctype,XmlGetStringDup(doc,"//NoteType"));//ͬ��ƾ֤����
    if(equal(tctype,"13")||
       equal(tctype,"14")||
       equal(tctype,"42")||
       equal(tctype,"43"))
    {
#if 0
        //˰Ʊ��Ϣ��ӡ�ļ�
        if( equal(tctype,"14") )
        {
            char tax_file[512]={0};
            FILE *tax_fp=NULL;
            char PrintBuf[20480]={0};
            int tax_len=0;

            sprintf(tax_file,"%s/consoleprint/%s.tax",getenv("HOME"),XmlGetStringDup(doc,"//XINX03"));
            if((tax_fp=fopen(tax_file, "w+"))==NULL)
            {
                err_log("�޷����ļ�[%s]",tax_file);
                return -1;
            }
            //tax_len+=sprintf(PrintBuf+tax_len,"\f");
            tax_len+=sprintf(PrintBuf+tax_len,"  �����ʺ�:%-32s   �����:%s\n",
                    XmlGetStringDup(doc,"//PayingAcct"), XmlGetStringDup(doc,"//Payer"));
            fwrite(PrintBuf,tax_len,1, tax_fp);
            fclose(tax_fp);
            err_log("˰Ʊ��ӡ�ļ�[%s]�ɹ�", tax_file);
        }
#endif
        strcpy(noteno,XmlGetStringDup(doc,"//XINX03"));//ƾ֤��
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
    }
    /*
    if(*XmlGetStringDup(doc,"//TJJXTS")!=0x00 && *XmlGetStringDup(doc,"//TJJXTS")!='0')
    {
        sprintf(noteno,"%s%s",XmlGetStringDup(doc,"//TJJXTS"),
                strlen(XmlGetStringDup(doc,"//NoteNo"))==9?XmlGetStringDup(doc,"//NoteNo")+1:XmlGetStringDup(doc,"//NoteNo"));
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
        err_log("�ϲ����ƾ֤��noteno[%s]",noteno);
    }
    */
    if(*XmlGetStringDup(doc,"//PNG1XH")!=0x00 && *XmlGetStringDup(doc,"//PNG1XH")!='0')
    {
        sprintf(noteno,"%s%s",XmlGetStringDup(doc,"//PNG1XH"),
                strlen(XmlGetStringDup(doc,"//NoteNo"))==9?XmlGetStringDup(doc,"//NoteNo")+1:XmlGetStringDup(doc,"//NoteNo"));
        XmlSetString(doc,"//NoteInfo/NoteNo",noteno);
        err_log("�ϲ����ƾ֤��noteno[%s]",noteno);
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
        POOL_SetGetSuff("");    /* ���POOL����������׺���ǳ���Ҫ������ */
        XMLFREE(grid);
    }

    // xml���
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

    // ƫ����
    if (poffset == NULL)
        offset = 0;
    else
        offset = atoi(poffset);
    // ����
    len = atoi(plen);

    if (type == NULL || strcmp(type, "xml") == 0)
    {
        // ��xml����ȡ����Ӧ������
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

    /*����ͷԭ������*/
    PPutStr("PDTRCD", PDTRCD1);
    PPutStr("PDLDTC", PDLDTC);
    PPutStr("PDTRDT", PDTRDT);
    PPutStr("PDTRTM", PDTRTM);
    PPutStr("PDTLSQ", PDTLSQ);
    PPutStr("PDERTR", PDERTR);
    /*����ͷԭ������*/
    // ����ƽ̨���ص�xml����
    if ((doc = xmlParseMemory(xmlbuf, xmllen)) == NULL)
    {
        err_log("parse (%d)[%s] fail.", xmllen, xmlbuf);
        return -1;
    }

    // ȡת�������ļ�
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
        err_log("���ҽڵ�[%s]ʧ��!", path);
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

       err_log("����[%s]��������:", trncode);
       for( l=0; l< rs->nodesetval->nodeNr; l++ )
       {
       node = rs->nodesetval->nodeTab[l];
       }
     */
    {
        // �´����ĸ�ʽ
        // ȡ��packformat
        //sprintf(pktfmt, "O%s2@R", trncode);
        if ((p = XmlNodeGetAttrText(node, "packfmt")) == NULL)
        {
            err_log("���ҽڵ�[%s]ʧ��!", path);
            xmlFreeDoc(cvt);
            xmlFreeDoc(doc);
            return -1;
        }
        strcpy(pktfmt, p);
        FREE(p);

        err_log("���״����ʽ[%s]", pktfmt);
        // �����ӽڵ�
        //err_log("����[%s]��������:", trncode);
        for( cur = node->children; cur != NULL; cur = cur->next )
        {
            if (cur->type != XML_ELEMENT_NODE)
                continue;
            if (!strcasecmp(cur->name, "grid"))
            {
                xmlNodePtr scur;
                char aczSuff[128];
                int rows = 0;

                grid = XmlNodeGetAttrText(cur, "name"); // �������
                if (grid == NULL)
                {
                    err_log("grid's name  not found.");
                    continue;
                }
                sprintf(aczSuff,"_%d", rows);       // rows�ʼ�¼
                POOL_SetPutSuff(aczSuff);           // ����POOL����������׺
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
                POOL_SetPutSuff("");                // ���POOL����������׺
                sprintf(buf, "%s_Rows", grid);      // ���ñ������� 
                PPutMem(buf, &rows, sizeof(int));

                // ���ñ��������
                memset(buf, 0, sizeof(buf));
                len = 0;
                if ((rc = SOP_PackForm(grid, '0', buf, sizeof(buf), &len)) < 0)
                    err_log("SOP_PackForm() ret = %d.", rc);

                // �ѱ���ķ���POOL��
                PPutMem(grid, buf, len);
            }
            else
            {
                if ((rc = do_item_tc2sop_rsp(doc, cur)) != 0)
                    err_log("do_item_tc2sop_rsp() ret = %d.", rc);
            }
        }
#if 0
        // ת��������
        XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
        XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
        err_log("ͬ��:result=[%s] desc=[%s]", buf, errinfo);
        err_tc2sop(nbuf, buf);
        //xmlFreeDoc(doc);

        // ���÷��ر����ֶ�
        sop_retinfo(nbuf, errinfo);
        /* ��ӡ��־ sop.log */
        POOL_TraceData();
        /* ����SOP���´����� */
        memset(aczSendMsg, 0, sizeof(aczSendMsg));
        len = ConvertPoolToPkt(aczSendMsg,sizeof(aczSendMsg), pktfmt);
        if (len < 0)
        {
            err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
            return -1;
        }
        memcpy(sopbuf+iLen, aczSendMsg, len);
        iLen+=len;
#endif
    }

    xmlFreeDoc(cvt);
    //xmlFreeDoc(doc);

#if 1 
    // ת��������
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
    err_log("ͬ��:result=[%s] desc=[%s]", buf, errinfo);
    err_tc2sop(nbuf, buf);
    xmlFreeDoc(doc);

    // ���÷��ر����ֶ�
    sop_retinfo(nbuf, errinfo);

func_handler:

    /* ��ӡ��־ sop.log */
    POOL_TraceData();
    /* ����SOP���´����� */
    memset(aczSendMsg, 0, sizeof(aczSendMsg));
    len = ConvertPoolToPkt(aczSendMsg,sizeof(aczSendMsg), pktfmt);
    if (len < 0)
    {
        err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, aczSendMsg, len);

    return len;
#endif

    //err_log("���ĳ���[%d]", iLen);
    //return iLen;
}

int init_sop_head()
{
    char aczBuf[512];
    char aczTemp[64];

    memset(aczBuf,0,sizeof(aczBuf));

    /* ϵͳ��Ϣͷ system_head.cfg */
    /* ���ĳ�����ϵͳ�Զ����� */
    PPutMem("sysSADDR",aczBuf,4);       /* Դ��ַ */
    PPutMem("sysDADDR",aczBuf,4);       /* Ŀ�ĵ�ַ */
    PPutMem("sysRSRVD",aczBuf,1);       /* ϵͳ����λ */
    aczTemp[0]=0x01;
    PPutMem("sysEFLAG",aczTemp,1);      /* ��Ϣ������־ */
    PPutMem("sysSEQNUM",aczBuf,2);      /* ������� */
    PPutMem("sysMACFLAG",aczBuf,1);     /* У���־ */
    PPutMem("sysMACVALUE",aczBuf,8);    /* У��ֵ */

    /* ��������ͷ cmtran_head.cfg */
    //PPutStr("PDWSNO", "1a");             /* �ն˺� */
    PPutStr("PDCTNO", "    ");           /* ���д��� */

    /* ��������ͷ tran_head.cfg */
    PPutStr("PDTRSD", "");               /* �������� */
    PPutStr("PDTRMD", "");               /* ����ģʽ */
    aczTemp[0]=0x00;
    aczTemp[1]=0x01;
    PPutMem("PDTRSQ", aczTemp,2);        /* ������� */
    aczTemp[0]=0xff;
    aczTemp[1]=0xff;
    PPutMem("PDOFF1", aczTemp,2);        /* ϵͳƫ��0xffff */
    PPutMem("PDOFF2", aczTemp,2);        /* ϵͳƫ��0xffff */
    PPutStr("PDAUUS","");       /* ��Ȩ��Ա */
    PPutStr("PDAUPS","");       /* ��Ȩ���� */  

    return 0;
}

int sop_trans_qry_sz43(char *sopbuf)
{
    err_log("���յ���������:���������ѯsz43,��ʼ�����ѯ����...");
    int i;
    int rc=0;
    int len=0;
    int reqcount=0;
    int nowcount=0;
    int rowcount=0;
    int acctflag=-1;//���˱�־(���)
    int cacctflag=0;//���˱�־(����)
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

    char PDWSNO[6];  PGetStr("PDWSNO",PDWSNO,sizeof(PDWSNO));err_log("�ն˺�        [PDWSNO]=[%s]",PDWSNO);//�ն˺�
    char PDSBNO[5];  PGetStr("PDSBNO",PDSBNO,sizeof(PDSBNO));err_log("��������      [PDSBNO]=[%s]",PDSBNO);//��������
    char PDUSID[9];  PGetStr("PDUSID",PDUSID,sizeof(PDUSID));err_log("���׹�Ա      [PDUSID]=[%s]",PDUSID);//���׹�Ա
    char YNGYJG[5];  PGetStr("YNGYJG",YNGYJG,sizeof(YNGYJG));err_log("Ӫҵ������    [YNGYJG]=[%s]",YNGYJG);//Ӫҵ����
    //char ZYWXYH[9];  PGetStr("ZYWXYH",ZYWXYH,sizeof(ZYWXYH));err_log("�м�ҵ��Э���[ZYWXYH]=[%s]",ZYWXYH);//�м�ҵ��
    char SHKRZH[33]; PGetStr("SHKRZH",SHKRZH,sizeof(SHKRZH));err_log("�տ����ʺ�    [SHKRZH]=[%s]",SHKRZH);//�տ�����
    char SHKRXM[63]; PGetStr("SHKRXM",SHKRXM,sizeof(SHKRXM));err_log("�տ�������    [SHKRXM]=[%s]",SHKRXM);//�տ�����
    //char ZJCYHH[13]; PGetStr("ZJCYHH",ZJCYHH,sizeof(ZJCYHH));err_log("ֱ�Ӳ����к�  [ZJCYHH]=[%s]",ZJCYHH);//ֱ�Ӳ���
    //char SBHHAO[13]; PGetStr("SBHHAO",SBHHAO,sizeof(SBHHAO));err_log("�ձ����к�    [SBHHAO]=[%s]",SBHHAO);//�ձ�����
    char FUKRZH[33]; PGetStr("FUKRZH",FUKRZH,sizeof(FUKRZH));err_log("�������ʺ�    [FUKRZH]=[%s]",FUKRZH);//��������
    char FUKRXM[63]; PGetStr("FUKRXM",FUKRXM,sizeof(FUKRXM));err_log("����������    [FUKRXM]=[%s]",FUKRXM);//��������
    //char WAIGDM[13]; PGetStr("WAIGDM",WAIGDM,sizeof(WAIGDM));err_log("��ܴ���      [WAIGDM]=[%s]",WAIGDM);//��ܴ���
    //char HUOBFH[4];  PGetStr("HUOBFH",HUOBFH,sizeof(HUOBFH));err_log("���ҷ���      [HUOBFH]=[%s]",HUOBFH);//���ҷ���
    char JIO1JE[14]; PGetStr("JIO1JE",JIO1JE,sizeof(JIO1JE));err_log("���׽��      [JIO1JE]=[%s]",JIO1JE);//���׽��
    //char PNGZZL[3];  PGetStr("PNGZZL",PNGZZL,sizeof(PNGZZL));err_log("ƾ֤����      [PNGZZL]=[%s]",PNGZZL);//ƾ֤����
    //char PNGZPH[2];  PGetStr("PNGZPH",PNGZPH,sizeof(PNGZPH));err_log("ƾ֤����      [PNGZPH]=[%s]",PNGZPH);//ƾ֤����
    char PNGZHH[14]; PGetStr("PNGZHH",PNGZHH,sizeof(PNGZHH));err_log("ƾ֤��        [PNGZHH]=[%s]",PNGZHH);//ƾ֤��  
    char JIOHLX[2];  PGetStr("JIOHLX",JIOHLX,sizeof(JIOHLX));err_log("��������      [JIOHLX]=[%s]",JIOHLX);//��������
    char JIOHRQ[9];  PGetStr("JIOHRQ",JIOHRQ,sizeof(JIOHRQ));err_log("��������      [JIOHRQ]=[%s]",JIOHRQ);//��������
    char JIOHCC[2];  PGetStr("JIOHCC",JIOHCC,sizeof(JIOHCC));err_log("��������      [JIOHCC]=[%s]",JIOHCC);//��������
    //char RUZHRQ[9];  PGetStr("RUZHRQ",RUZHRQ,sizeof(RUZHRQ));err_log("��������      [RUZHRQ]=[%s]",RUZHRQ);//��������
    //char CHUPRQ[9];  PGetStr("CHUPRQ",CHUPRQ,sizeof(CHUPRQ));err_log("��Ʊ����      [CHUPRQ]=[%s]",CHUPRQ);//��Ʊ����
    //char FUKURQ[9];  PGetStr("FUKURQ",FUKURQ,sizeof(FUKURQ));err_log("��������      [FUKURQ]=[%s]",FUKURQ);//��������
    //char WEITRQ[9];  PGetStr("WEITRQ",WEITRQ,sizeof(WEITRQ));err_log("ί������      [WEITRQ]=[%s]",WEITRQ);//ί������
    //char FUHEBZ[2];  PGetStr("FUHEBZ",FUHEBZ,sizeof(FUHEBZ));err_log("���˱�־      [FUHEBZ]=[%s]",FUHEBZ);//���˱�־
    char RUZHBZ[2];  PGetStr("RUZHBZ",RUZHBZ,sizeof(RUZHBZ));err_log("���ʱ�־      [RUZHBZ]=[%s]",RUZHBZ);//���ʱ�־
    //char HXIOZT[2];  PGetStr("HXIOZT",HXIOZT,sizeof(HXIOZT));err_log("����״̬      [HXIOZT]=[%s]",HXIOZT);//����״̬
    char TUIHBZ[2];  PGetStr("TUIHBZ",TUIHBZ,sizeof(TUIHBZ));err_log("�˻��־      [TUIHBZ]=[%s]",TUIHBZ);//�˻��־
    //char LZBWZT[2];  PGetStr("LZBWZT",LZBWZT,sizeof(LZBWZT));err_log("���ʱ���״̬  [LZBWZT]=[%s]",LZBWZT);//���ʱ���
    //char FUKUQX[11]; PGetStr("FUKUQX",FUKUQX,sizeof(FUKUQX));err_log("��������      [FUKUQX]=[%s]",FUKUQX);//��������
    //char SHULIA[11]; PGetStr("SHULIA",SHULIA,sizeof(SHULIA));err_log("����          [SHULIA]=[%s]",SHULIA);//����    
    //char QISHBS[5];  PGetStr("QISHBS",QISHBS,sizeof(QISHBS));err_log("��ʼ����      [QISHBS]=[%s]",QISHBS);//��ʼ����
    char CXUNBS[3];  PGetStr("CXUNBS",CXUNBS,sizeof(CXUNBS));err_log("��ѯ����      [CXUNBS]=[%s]",CXUNBS);//��ѯ����
    //char JYDZLX[3];  PGetStr("JYDZLX",JYDZLX,sizeof(JYDZLX));err_log("���׶�������  [JYDZLX]=[%s]",JYDZLX);//���׶���
    //char SHKHHM[63]; PGetStr("SHKHHM",SHKHHM,sizeof(SHKHHM));err_log("�տ�����      [SHKHHM]=[%s]",SHKHHM);//�տ�����
    //char FUHEGY[9];  PGetStr("FUHEGY",FUHEGY,sizeof(FUHEGY));err_log("���˹�Ա      [FUHEGY]=[%s]",FUHEGY);//���˹�Ա
    //char FHGYLS[13]; PGetStr("FHGYLS",FHGYLS,sizeof(FHGYLS));err_log("���˹�Ա��ˮ��[FHGYLS]=[%s]",FHGYLS);//���˹�Ա
    //char PZHTJH[9];  PGetStr("PZHTJH",PZHTJH,sizeof(PZHTJH));err_log("ƾ֤�ύ��    [PZHTJH]=[%s]",PZHTJH);//ƾ֤�ύ
    //char JINBRQ[9];  PGetStr("JINBRQ",JINBRQ,sizeof(JINBRQ));err_log("��������      [JINBRQ]=[%s]",JINBRQ);//��������

    reqcount=atoi(CXUNBS);
    if(reqcount<1||reqcount>20)
        reqcount=20;

    getSysPara(archivedate,"ARCHIVEDATE");
    if(strcmp(JIOHRQ,archivedate)<=0)
        strcpy(tablename, "htrnjour");

    err_log("��ʼ��ʼ��sop����ͷ...");
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

    err_log("��ʼ׼��SQL���...");
    //len+=sprintf(sqlstr+len,"set rowcount %d select * from %s where nodeid=10 and inoutflag='2' and dcflag='2'",reqcount,tablename);
    len+=sprintf(sqlstr+len,"select * from %s where nodeid=10 and inoutflag='2' and dcflag='2'",tablename);
    if(strcmp(YNGYJG,""))//������
    {
        if(db_query_str(tmpbuf,13,"select exchno from bankinfo where bankid like '%%%s%%'",YNGYJG)!=0)
        {
            err_log("���ݻ�����[%s]�����к�ʧ��!",YNGYJG);
            sop_retinfo(YNGYJG, "���ݻ����Ų����к�ʧ��!");
            goto READY_TO_SEND;
        }
        len+=sprintf(sqlstr+len," and acceptor='%s'",tmpbuf);
    }
    if(strcmp(JIOHRQ,""))//��������
        len+=sprintf(sqlstr+len," and workdate='%s'",JIOHRQ);
    if(strcmp(JIOHCC,"")&&strcmp(JIOHCC,"0"))//��������
        len+=sprintf(sqlstr+len," and exchground='%s'",JIOHCC);
    if(strcmp(SHKRZH,""))//�����˺�
        len+=sprintf(sqlstr+len," and payingacct='%s'",SHKRZH);
    if(strcmp(SHKRXM,""))//��������
        len+=sprintf(sqlstr+len," and payer like '%%%s%%'",SHKRXM);
    if(strcmp(FUKRZH,""))//�տ��˺�
        len+=sprintf(sqlstr+len," and beneacct='%s'",FUKRZH);
    if(strcmp(FUKRXM,""))//�տ�����
        len+=sprintf(sqlstr+len," and benename like '%%%s%%'",FUKRXM);
    if(strcmp(JIO1JE,"")&&strcmp(JIO1JE,"0.00"))//���׽��
        len+=sprintf(sqlstr+len," and settlamt=%s",JIO1JE);
    /*
       if(strcmp(PNGZZL,""))//ƾ֤����
       {
    //if((rc = notetype_b2c(tmpbuf,"sz37",PNGZZL,"","","")) < 0)//������Ҫ��������Ա��ͨ��
    //return -1;
    //len+=sprintf(sqlstr+len," and notetype='%s'",tmpbuf);
    len+=sprintf(sqlstr+len," and notetype='%s'",PNGZZL);
    }
     */
    if(strcmp(PNGZHH,""))//ƾ֤��
        len+=sprintf(sqlstr+len," and noteno='%s'",PNGZHH);
    /*
    if(strcmp(TUIHBZ,""))//��Ʊ��־
        len+=sprintf(sqlstr+len," and tpflag='%s'",TUIHBZ);
        */
    if(strcmp(JIOHLX,""))//��������
    {
        if( atoi(JIOHLX) == 2 ) //�������
            //len+=sprintf(sqlstr+len," and tpflag!='1'");
            len+=sprintf(sqlstr+len," and trncode in('2','0002') ");
        else if( atoi(JIOHLX) == 5 ) //��Ʊ
            len+=sprintf(sqlstr+len," and trncode in('7','0007') ");
            //len+=sprintf(sqlstr+len," and tpflag='1'");
        else
        {
            rowcount=0;
            sop_retinfo("-2", "�������ʹ���");
            goto READY_TO_SEND;
        }
    }
    if(strcmp(RUZHBZ,"1")==0)//���˱�־
        cacctflag=1;
    else
        cacctflag=0;
    //len+=sprintf(sqlstr+len," set rowcount 0");
    //�����ֶδ����Ժ����

    err_log("��ʼִ��SQL���...");
    if((rc=db_query(&rs,sqlstr))!=0)
    {
        if(rc==E_DB_NORECORD)
        {
            rowcount=0;
            sop_retinfo("-2", "û�м�¼");
            goto READY_TO_SEND;
        }
        else
            return -1;
    }

    err_log("��ʼ�齨sop�������...");
    rowcount=db_row_count(&rs);
    err_log("�ҵ���¼[%d],��ʼ���ݼ����������й���..",rowcount);
    for(i=0;i<rowcount && nowcount<reqcount;i++)
    {
        /*****��ʼ:�ж��Ƿ�������*****/
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
            err_log("���˽���[%s][%s][%s][%s]*",
                    db_cell_by_name(&rs,i,"originator"),
                    db_cell_by_name(&rs,i,"workdate"),
                    db_cell_by_name(&rs,i,"refid"),
                    db_cell_by_name(&rs,i,"notetype"));
            continue;
        }
        err_log("��������[%s][%s][%s][%s]",
                db_cell_by_name(&rs,i,"originator"),
                db_cell_by_name(&rs,i,"workdate"),
                db_cell_by_name(&rs,i,"refid"),
                db_cell_by_name(&rs,i,"notetype"));
        /*****����:�ж��Ƿ�������*****/

        // ��Ŵ�0��ʼ
        sprintf(aczSuff,"_%d",nowcount++);
        POOL_SetPutSuff(aczSuff);

        //PPutStr("SHHUDH",db_cell_by_name(&rs,i,"refid"));//ҵ����ˮ��
        PPutStr("JIOHRQ",db_cell_by_name(&rs,i,"workdate"));//��������
        PPutStr("JYDZLX","00");//���׶�������
        PPutStr("JIOHLX","2");//��������
        PPutStr("TCHTCH",db_cell(&rstmp,0,1));//ͬ�������
        PPutStr("JIO1JE",db_cell_by_name(&rs,i,"settlamt"));//���׽��
        PPutStr("CHUPRQ",db_cell_by_name(&rs,i,"issuedate"));//��Ʊ����
        PPutStr("FUKURQ","");//��ʾ��������
        PPutStr("WEITRQ","");//ί������
        PPutStr("ZYWXYH","");//�������//�м�ҵ��Э���
        PPutStr("LZBWZT","");//ֹ����־//���ʱ���״̬
        PPutStr("PZHTJH","");//ֹ�����//ƾ֤�ύ��
        PPutStr("JINBRQ","");//ֹ������//��������
        PPutStr("KHZHLX","");//��ִ��־//�ͻ��ʺ�����
        PPutStr("TNGZRQ","");//��ִ����//֪ͨ����
        PPutStr("MSGIDN","");//��ִ���//���ı�ʶ��
        PPutStr("FUKUQX","");//��ִ����//��������
        PPutStr("JIOHCC",db_cell_by_name(&rs,i,"exchground"));//��������
        PPutStr("FUHEBZ","0");//���˱�־
        if(notetype_c2b_ex(db_cell_by_name(&rs,i,"notetype"),notetype,tmpbuf,0)<0)
        {
            err_log("notetype_c2b() fail");
            db_free_result(&rs);
            db_free_result(&rstmp);
            return -1;
        }
        PPutStr("PNGZZL",notetype);//ƾ֤����
        //PPutStr("PNGZZL",db_cell_by_name(&rs,i,"notetype"));//ƾ֤����
        PPutStr("PNGZPH","");//ƾ֤����
        PPutStr("PNGZHH",db_cell_by_name(&rs,i,"noteno"));//ƾ֤��
        PPutStr("HUOBFH","");//���ҷ���
        PPutStr("GUIYLS","");//��Ա��ˮ��
        PPutStr("FHGYLS","");//���˹�Ա��ˮ��
        PPutStr("TUIHBZ",db_cell_by_name(&rs,i,"tpflag"));//��Ʊ��־
        //PPutStr("TUIPLY","");//��Ʊ����
        PPutStr("TUIPLY",db_cell_by_name(&rs,i,"purpose"));//��Ʊ����
        PPutStr("RUZHBZ",acctflag?"1":"0");//���ʱ�־
        PPutStr("RUZHRQ",db_cell(&rstmp,0,2));//��������
        PPutStr("FUKRZH",db_cell_by_name(&rs,i,"payingacct"));//�������ʺ�
        PPutStr("FUKRXM",db_cell_by_name(&rs,i,"payer"));//����������
        PPutStr("WAIGDM","");//������֧����
        PPutStr("SHKRZH",db_cell_by_name(&rs,i,"beneacct"));//�տ����ʺ�
        PPutStr("SHKRXM",db_cell_by_name(&rs,i,"benename"));//�տ�������
        PPutStr("ZJCYHH","");//�տ���֧����
        PPutStr("SBHHAO","");//������֧����
        PPutStr("SHKHHM","");//�տ�����
        PPutStr("ZHYODM","");//ժҪ����
        PPutStr("SHULIA","");//�������
        PPutStr("BEIZXX","");//��ע��Ϣ
        PPutStr("FUHEGY","");//���˹�Ա

        POOL_SetPutSuff("");

        db_free_result(&rstmp);
    }
    db_free_result(&rs);


    if(nowcount==0)
    {
        sop_retinfo("-2", "û�м�¼");
        goto READY_TO_SEND;
    }
    err_log("���շ��ؼ�¼[%d]",nowcount);

    strcpy(grid,"Fsz431");
    sprintf(tmpbuf, "%s_Rows", grid);
    PPutMem(tmpbuf, &nowcount, sizeof(int));

    len=0;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    err_log("��ʼ������...");
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
        err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, tmpbuf, len);

    return len;
}

int sop_trans_qry_sz44(char *sopbuf)
{
    err_log("���յ���������:����跽��ѯsz44,��ʼ�����ѯ����...");
    int i;
    int rc=0;
    int len=0;
    int reqcount=0;
    int nowcount=0;
    int rowcount=0;
    int acctflag=0;//���˱�־(���)
    int cacctflag=0;//���˱�־(����)
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

    char PDWSNO[6];  PGetStr("PDWSNO",PDWSNO,sizeof(PDWSNO));err_log("�ն˺�      [PDWSNO]=[%s]",PDWSNO);//�ն˺�
    char PDSBNO[5];  PGetStr("PDSBNO",PDSBNO,sizeof(PDSBNO));err_log("��������    [PDSBNO]=[%s]",PDSBNO);//��������
    char PDUSID[9];  PGetStr("PDUSID",PDUSID,sizeof(PDUSID));err_log("���׹�Ա    [PDUSID]=[%s]",PDUSID);//���׹�Ա
    char YNGYJG[5];  PGetStr("YNGYJG",YNGYJG,sizeof(YNGYJG));err_log("Ӫҵ������  [YNGYJG]=[%s]",YNGYJG);
    char JIOHLX[2];  PGetStr("JIOHLX",JIOHLX,sizeof(JIOHLX));err_log("��������    [JIOHLX]=[%s]",JIOHLX);
    char JIOHRQ[18]; PGetStr("JIOHRQ",JIOHRQ,sizeof(JIOHRQ));err_log("��������    [JIOHRQ]=[%s]",JIOHRQ);
    char TRJHCC[33]; PGetStr("TRJHCC",TRJHCC,sizeof(TRJHCC));err_log("���뽻������[TRJHCC]=[%s]",TRJHCC);
    //char PNGZZL[63]; PGetStr("PNGZZL",PNGZZL,sizeof(PNGZZL));err_log("ƾ֤����    [PNGZZL]=[%s]",PNGZZL);
    char BIAOZI[2];  PGetStr("BIAOZI",BIAOZI,sizeof(BIAOZI));err_log("������־    [BIAOZI]=[%s]",BIAOZI);
    char JIO1JE[33]; PGetStr("JIO1JE",JIO1JE,sizeof(JIO1JE));err_log("���׽��    [JIO1JE]=[%s]",JIO1JE);
    char SHKRZH[63]; PGetStr("SHKRZH",SHKRZH,sizeof(SHKRZH));err_log("�տ����ʺ�  [SHKRZH]=[%s]",SHKRZH);
    char SHKRXM[43]; PGetStr("SHKRXM",SHKRXM,sizeof(SHKRXM));err_log("�տ�������  [SHKRXM]=[%s]",SHKRXM);
    char FUKRZH[14]; PGetStr("FUKRZH",FUKRZH,sizeof(FUKRZH));err_log("�������ʺ�  [FUKRZH]=[%s]",FUKRZH);
    char FUKRXM[23]; PGetStr("FUKRXM",FUKRXM,sizeof(FUKRXM));err_log("����������  [FUKRXM]=[%s]",FUKRXM);
    char TUIHBZ[3];  PGetStr("TUIHBZ",TUIHBZ,sizeof(TUIHBZ));err_log("�˻��־    [TUIHBZ]=[%s]",TUIHBZ);
    char RUZHBZ[3];  PGetStr("RUZHBZ",RUZHBZ,sizeof(RUZHBZ));err_log("���ʱ�־    [RUZHBZ]=[%s]",RUZHBZ);
    //char QISHBS[2];  PGetStr("QISHBS",QISHBS,sizeof(QISHBS));err_log("��ʼ����    [QISHBS]=[%s]",QISHBS);
    char CXUNBS[9];  PGetStr("CXUNBS",CXUNBS,sizeof(CXUNBS));err_log("��ѯ����    [CXUNBS]=[%s]",CXUNBS);

    //��ѯ����
    reqcount=atoi(CXUNBS);
    if(reqcount<1||reqcount>20)
        reqcount=20;

    getSysPara(archivedate,"ARCHIVEDATE");
    if(strcmp(JIOHRQ,archivedate)<=0)
        strcpy(tablename, "htrnjour");

    err_log("��ʼ��ʼ��sop����ͷ...");
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

    err_log("��ʼ׼��SQL���...");
    //len+=sprintf(sqlstr+len,"set rowcount %d select * from %s where nodeid=10 and inoutflag='2' and dcflag='1'",reqcount,tablename);
    len+=sprintf(sqlstr+len,"select * from %s where nodeid=10 and inoutflag='2' and dcflag='1'",tablename);
    if(strcmp(YNGYJG,""))//������
    {
        if(db_query_str(tmpbuf,13,"select exchno from bankinfo where bankid like '%%%s%%'",YNGYJG)!=0)
        {
            err_log("���ݻ�����[%s]�����к�ʧ��!",YNGYJG);
            sop_retinfo(YNGYJG,"���ݻ����Ų����к�ʧ��!");
            goto READY_TO_SEND;
        }
        len+=sprintf(sqlstr+len," and acceptor='%s'",tmpbuf);
    }
    if(strcmp(JIOHRQ,""))//��������
        len+=sprintf(sqlstr+len," and workdate='%s'",JIOHRQ);
    if(strcmp(TRJHCC,"")&&strcmp(TRJHCC,"0"))//��������
        len+=sprintf(sqlstr+len," and exchground='%s'",TRJHCC);
    /*
       if(strcmp(PNGZZL,""))//ƾ֤����
       {
    //if((rc = notetype_b2c(tmpbuf,"sz36",PNGZZL,"","","")) < 0)//������Ҫ��������Ա��ͨ��
    //return -1;
    //len+=sprintf(sqlstr+len," and notetype='%s'",tmpbuf);
    len+=sprintf(sqlstr+len," and notetype='%s'",PNGZZL);
    }
     */
    if(strcmp(JIO1JE,"")&&strcmp(JIO1JE,"0.00"))//���׽��
        len+=sprintf(sqlstr+len," and settlamt=%s",JIO1JE);
    if(strcmp(SHKRZH,""))//�տ����˺�
        len+=sprintf(sqlstr+len," and payingacct='%s'",SHKRZH);
    if(strcmp(SHKRXM,""))//�տ�������
        len+=sprintf(sqlstr+len," and payer like '%%%s%%'",SHKRXM);
    if(strcmp(FUKRZH,""))//�������˺�
        len+=sprintf(sqlstr+len," and beneacct='%s'",FUKRZH);
    if(strcmp(FUKRXM,""))//����������
        len+=sprintf(sqlstr+len," and benename like '%%%s%%'",FUKRXM);
    /*
    if(strcmp(TUIHBZ,""))//��Ʊ��־
        len+=sprintf(sqlstr+len," and tpflag = '%s'",TUIHBZ);
        */
    if(strcmp(JIOHLX,""))//��Ʊ��־
    {
        if( atoi(JIOHLX) == 3 )//����跽
            len+=sprintf(sqlstr+len," and trncode in('1','0001')");
            //len+=sprintf(sqlstr+len," and tpflag != '1'");
        else if(atoi(JIOHLX) == 6) //��Ʊ
            len+=sprintf(sqlstr+len," and trncode in('7','0007')");
            //len+=sprintf(sqlstr+len," and tpflag = '1'");
        else
        {
            rowcount=0;
            sop_retinfo("-2", "�������ʹ���");
            goto READY_TO_SEND;
        }
    }
    if(strcmp(RUZHBZ,"1")==0)//���˱�־
        cacctflag=1;
    else
        cacctflag=0;
    if(strcmp(BIAOZI,"1")==0)//������־
        len+=sprintf(sqlstr+len," and truncflag='1'");
    else
        len+=sprintf(sqlstr+len," and truncflag!='1'");
    //len+=sprintf(sqlstr+len," set rowcount 0");

    err_log("��ʼִ��SQL���...");
    if((rc=db_query(&rs,sqlstr))!=0)
    {
        if(rc==E_DB_NORECORD)
        {
            rowcount=0;
            sop_retinfo("-2", "û�м�¼");
            goto READY_TO_SEND;
        }
        else
            return -1;
    }

    err_log("��ʼ�齨sop�������...");
    rowcount=db_row_count(&rs);
    err_log("�ҵ���¼[%d],��ʼ���ݼ����������й���...",rowcount);
    for(i=0;i<rowcount && nowcount<reqcount;i++)
    {
        acctflag=-1;
        //���ȷ�ϱ�־Ϊ1��Ϊ���˳ɹ�
        //if( atoi(db_cell_by_name(&rs,i,"chkflag")) == 1 )
        if(*db_cell_by_name(&rs,i,"chkflag") == '1' || *db_cell_by_name(&rs,i,"tpflag") == '1' )
            acctflag=1;
        else
        {

            /*****��ʼ:�ж��Ƿ�������*****/
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
            err_log("���˽���[%s][%s][%s][%s]*",
                    db_cell_by_name(&rs,i,"originator"),
                    db_cell_by_name(&rs,i,"workdate"),
                    db_cell_by_name(&rs,i,"refid"),
                    db_cell_by_name(&rs,i,"notetype"));
            continue;
        }
        err_log("��������[%s][%s][%s][%s]",
                db_cell_by_name(&rs,i,"originator"),
                db_cell_by_name(&rs,i,"workdate"),
                db_cell_by_name(&rs,i,"refid"),
                db_cell_by_name(&rs,i,"notetype"));
        /*****����:�ж��Ƿ�������*****/

        // ��Ŵ�0��ʼ
        sprintf(aczSuff,"_%d",nowcount++);
        POOL_SetPutSuff(aczSuff);

        PPutStr("BAOWLS",db_cell_by_name(&rs,i,"refid"));//ҵ����ˮ��
        PPutStr("QSZXDH","0089");//�������Ĵ���
        PPutStr("JIOHRQ",db_cell_by_name(&rs,i,"workdate"));//��������
        PPutStr("JIOHLX","3");//��������//�跽3����2
        PPutStr("FQHHAO",db_cell_by_name(&rs,i,"originator"));//����н�����
        PPutStr("QISHHH",db_cell_by_name(&rs,i,"acceptor"));//�����н�����
        PPutStr("TCHTCH",rc==E_DB_NORECORD?"":db_cell(&rstmp,0,1));//ͬ�������
        PPutStr("JIO1JE",db_cell_by_name(&rs,i,"settlamt"));//���׽��
        PPutStr("TRJHCC",db_cell_by_name(&rs,i,"exchground"));//��������
        PPutStr("FUHEBZ","0");//���˱�־
        if(notetype_c2b_ex(db_cell_by_name(&rs,i,"notetype"),notetype,tmpbuf,0)<0)
        {
            err_log("notetype_c2b() fail");
            db_free_result(&rs);
            db_free_result(&rstmp);
            return -1;
        }
        PPutStr("PNGZZL",notetype);//ƾ֤����
        //PPutStr("PNGZZL",db_cell_by_name(&rs,i,"notetype"));//ƾ֤����
        PPutStr("PNGZPH","");//ƾ֤����
        PPutStr("PNGZXH",db_cell_by_name(&rs,i,"noteno"));//ƾ֤���
        PPutStr("GUIYLS","");//��Ա��ˮ��
        PPutStr("FHGYLS","");//���˹�Ա��ˮ��
        PPutStr("TUIHBZ",db_cell_by_name(&rs,i,"tpflag"));//��Ʊ��־
        PPutStr("TUIPLY","");//��Ʊ����
        PPutStr("RUZHBZ",acctflag?"1":"0");//���ʱ�־
        PPutStr("FUKRZH",db_cell_by_name(&rs,i,"payingacct"));//�������ʺ�
        PPutStr("FUKRXM",db_cell_by_name(&rs,i,"payer"));//����������
        PPutStr("WAIGDM","");//������֧����
        PPutStr("SHKRZH",db_cell_by_name(&rs,i,"beneacct"));//�տ����ʺ�
        PPutStr("SHKRXM",db_cell_by_name(&rs,i,"benename"));//�տ�������
        PPutStr("ZJCYHH","");//�տ���֧����
        PPutStr("SBHHAO","");//������֧����
        PPutStr("SHKHHM","");//�տ�����
        PPutStr("BEIZXX","");//��ע��Ϣ

        POOL_SetPutSuff("");

        db_free_result(&rstmp);
    }
    db_free_result(&rs);

    if(nowcount==0)
    {
        sop_retinfo("-2", "û�м�¼");
        goto READY_TO_SEND;
    }
    err_log("���շ��ؼ�¼[%d]",nowcount);

    strcpy(grid,"Fsz441");
    sprintf(tmpbuf, "%s_Rows", grid);
    PPutMem(tmpbuf, &nowcount, sizeof(int));

    len=0;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    err_log("��ʼ������...");
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
        err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
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
        err_log("�޷���[%s]ģʽ���ļ�[%s]",mode,file);
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
    len+=sprintf(PrintBuf+len,"  ��˰�˱���:%-20s ��˰������:%s\n",TaxPrintBuf[TaxPayCode],TaxPrintBuf[TaxPayName]);
    len+=sprintf(PrintBuf+len,"  �������:%-20s   ��Ʊ����:%s\n", TaxPrintBuf[TreCode], TaxPrintBuf[BillDate]);
    len+=sprintf(PrintBuf+len,"  ���׽��:%s      ��д���:%s\n", TaxPrintBuf[TraAmt], amount);
#if 0 
    len+=sprintf(PrintBuf+len,"                                 ��д���:%s\n", amount);
    len+=sprintf(PrintBuf+len,"  ˰Ʊ����:%-20s   ��Ʊ����:%s\n",TaxPrintBuf[TaxVouNo],TaxPrintBuf[BillDate]);
    len+=sprintf(PrintBuf+len,"  ��ҵ����:%-20s   Ԥ������:%s �����ڱ�־:%s ��ҵע������:%s\n",
            TaxPrintBuf[CorpCode],TaxPrintBuf[BudgetType],TaxPrintBuf[TrimSign],TaxPrintBuf[CorpType]);
    len+=sprintf(PrintBuf+len,"      ��ע:%s\n\n",TaxPrintBuf[Remark]);
#endif
    len+=sprintf(PrintBuf+len,"%s\n",TaxPrintBuf[Print]);

    fwrite(PrintBuf,len,1,fp);
    fclose(fp);
    if(!strstr(mode,"+"))
        err_log("�ɹ�����˰Ʊ��ӡ�ļ�[%s]",file);
    return 0;
}

int sop_trans_qry_sz78(char *sopbuf,char *xmlbuf,int xmllen)
{
    err_log("���յ���������:˰Ʊ��Ϣ��ѯsz78");
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

    err_log("��ʼ����xml���ر���...");
    if ((doc = xmlParseMemory(xmlbuf, xmllen)) == NULL)
    {
        err_log("parse (%d) fail.", xmllen);
        save_xml("3022",xmlbuf,xmllen);
        return -1;
    }

    err_log("��ʼ��ʼ��sop����ͷ...");
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

    // ת��������
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", retcode, sizeof(retcode));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", retmsg, sizeof(retmsg));
    err_log("ͬ��:result=[%s] desc=[%s]", retcode, retmsg);
    err_tc2sop(retcode, retcode);
    if(strcmp(retcode,"AAAAAAA")!=0)
    {
        xmlFreeDoc(doc);
        sop_retinfo(retcode, retmsg);
        goto READY_TO_SEND;
    }

    rowcount=atoi(XmlGetStringDup(doc,"/UFTP/CountNum"));
    err_log("ȡ�ü�¼��[%d]",rowcount);
    if(rowcount==0)
    {
        xmlFreeDoc(doc);
        sop_retinfo("-2", "û�м�¼");
        goto READY_TO_SEND;
    }

    err_log("��ʼ�齨sop�������...");
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
        PPutStr("GUIYLS",TaxPrintBuf[TraNo]     );//������ˮ
        PPutStr("SHOJGK",TaxPrintBuf[TreCode]   );//�������
        PPutStr("JIO1JE",TaxPrintBuf[TraAmt]    );//���׽��
        //PPutStr("ZZHAOM",TaxPrintBuf[TaxVouNo]  );//˰Ʊ����
        PPutStr("XINX03",TaxPrintBuf[TaxVouNo]  );//˰Ʊ����
        PPutStr("KAIPRQ",TaxPrintBuf[BillDate]  );//��Ʊ����
        PPutStr("NASHDM",TaxPrintBuf[TaxPayCode]);//��˰�˱���
        PPutStr("FUKRXM",TaxPrintBuf[TaxPayName]);//��˰������
        PPutStr("SHOKDW",TaxPrintBuf[CorpCode]  );//��ҵ����
        PPutStr("YUSUAN",TaxPrintBuf[BudgetType]);//Ԥ������
        PPutStr("SHFOBZ",TaxPrintBuf[TrimSign]  );//�����ڱ�־
        PPutStr("MAIRBZ",TaxPrintBuf[CorpType]  );//��ҵע������
        PPutStr("BEIZXX",TaxPrintBuf[Remark]    );//��ע

        save_tax(TaxPrintBuf,"w");

        sprintf(tmpnode,"/UFTP/TaxInfo%d/TaxTypeNum",i+1);
        XmlGetString(doc, tmpnode, TaxTypeNum, sizeof(TaxTypeNum));
        if( atoi(TaxTypeNum) > 7 )
        {
            sprintf(tmpnode,"/UFTP/TaxInfo%d/PrintInfo2",i+1);
            XmlGetString(doc,tmpnode,TaxPrintBuf[Print],sizeof(TaxPrintBuf[Print]));
            if(strcmp(TaxPrintBuf[Print],""))
            {
                strcpy(TaxPrintBuf[TraAmt],"����ҳ");
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
    err_log("��ʼ������...");
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
        err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
        return -1;
    }
    memcpy(sopbuf, tmpbuf, len);

    return len;
}
