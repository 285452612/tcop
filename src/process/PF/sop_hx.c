#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include "utils.h"
#include "sop.h"
#include "udb.h"
#include "sop_st.h"
#include "prefs.h"
//#include "trncode.h"

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

/*
void map_errcode(char *soperr, char *tcoperr)
{
    strcpy(soperr, tcoperr);
    return;
}
*/

int init_sop_hx()
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

static void sop_setinfo()
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

static void sop_retinfo(char *retcode, char *retmsg)
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

int tcop2sop_req(char *trncode, xmlDocPtr doc, char *sopbuf)
{
    int count=0;
    xmlDocPtr cvt = NULL;
    xmlXPathObjectPtr result;
    xmlNodePtr node, cur;
    char *grid, pktfmt[10];
    char aczSendMsg[4096];
    char buf[1024], nbuf[1024], errinfo[83];
    char *p, path[128];
    char *name, *plen, *type, *value, *poffset, *sofile, *func, *expr;
    int len, offset, rc = 0;

    // ȡת�������ļ�
    //sprintf(path, "%s/etc/cvt.xml", getAppDir());
    sprintf(path, "%s/etc/cvt_op2sop.xml", getAppDir());
    if ((cvt = xmlParseFile(path)) == NULL)
    {
        err_log("xmlParseFile(%s) fail", path);
        return -1;
    }

    err_log("��ʼ���ҽڵ�[%s]", path);
    sprintf(path, "/Root/Transaction[@code='%s']/Request", trncode);
    if ((node = XmlLocateNode(cvt, path)) == NULL)
    {
        err_log("���ҽڵ�[%s]ʧ��!", path);
        xmlFreeDoc(cvt);
        return -1;
    }

    // ���ͱ��ĸ�ʽ
    //sprintf(pktfmt, "O%s1@S", trncode);
    if ((p = XmlNodeGetAttrText(node, "packfmt")) == NULL)
    {
        err_log("���ҽڵ�[%s]ʧ��!", path);
        xmlFreeDoc(cvt);
        return -1;
    }
    err_log("���ҽڵ�ɹ�!");
    sprintf(pktfmt, "%s", p);
    FREE(p);

    // �����ӽڵ�
    cur = node->children;
    err_log("����[%s]��������:", trncode);
    while( cur != NULL )
    {
        if (cur->type != XML_ELEMENT_NODE)
        {
            cur = cur->next;
            continue;
        }
        /*�����*/
        if (!strcasecmp(cur->name, "grid"))
        {
            xmlNodePtr scur;
            char aczSuff[128];
            int rows = 1;

            grid = XmlNodeGetAttrText(cur, "name"); // �������
            if (grid == NULL)
            {
                err_log("grid's name  not found.");
                continue;
            }

            scur = cur->children;
            sprintf(aczSuff,"_%d", 0);          // ֻ����һ������
            POOL_SetPutSuff(aczSuff);           // ����POOL����������׺
            while( scur != NULL )
            {
                if (scur->type != XML_ELEMENT_NODE)
                {
                    scur = scur->next;
                    continue;
                }
                name = XmlNodeGetAttrText(scur, "name");
                plen = XmlNodeGetAttrText(scur, "length");
                type = XmlNodeGetAttrText(scur, "type");
                value = XmlNodeGetAttrText(scur, "value");
                poffset = XmlNodeGetAttrText(scur, "offset");
                sofile = XmlNodeGetAttrText(scur, "dll");
                func = XmlNodeGetAttrText(scur, "func");
                expr = XmlNodeGetAttrText(scur, "expr");
                if (name == NULL || plen == NULL)
                {
                    rc = -1;
                    break;
                }
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
                    memset(buf, 0, sizeof(buf));
                    XmlGetString(doc, value, buf, sizeof(buf));
                    *(buf+offset+len) = '\0';
                    rtrim(buf+offset);
                    if (sofile != NULL && func != NULL)
                    {
                        memset(nbuf, 0, sizeof(nbuf));
                        rc = callConvertFunc(sofile, func, expr, buf+offset, nbuf);
                        if (rc != 0)
                            break;
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
                            break;
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
                    rc = -1;
                    break;
                }

                XMLFREE(name);
                XMLFREE(plen);
                XMLFREE(type);
                XMLFREE(value);
                XMLFREE(poffset);
                XMLFREE(sofile);
                XMLFREE(func);
                XMLFREE(expr);
                scur = scur->next;
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
            name = XmlNodeGetAttrText(cur, "name");
            plen = XmlNodeGetAttrText(cur, "length");
            type = XmlNodeGetAttrText(cur, "type");
            value = XmlNodeGetAttrText(cur, "value");
            poffset = XmlNodeGetAttrText(cur, "offset");
            sofile = XmlNodeGetAttrText(cur, "dll");
            func = XmlNodeGetAttrText(cur, "func");
            expr = XmlNodeGetAttrText(cur, "expr");
            if (name == NULL || plen == NULL)
            {
                rc = -1;
                break;
            }
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
                memset(buf, 0, sizeof(buf));
                XmlGetString(doc, value, buf, sizeof(buf));
                *(buf+offset+len) = '\0';
                rtrim(buf+offset);
                if (sofile != NULL && func != NULL)
                {
                    memset(nbuf, 0, sizeof(nbuf));
                    rc = callConvertFunc(sofile, func, expr, buf+offset, nbuf);
                    if (rc != 0)
                        break;
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
                        break;
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
                rc = -1;
                break;
            }

            XMLFREE(name);
            XMLFREE(plen);
            XMLFREE(type);
            XMLFREE(value);
            XMLFREE(poffset);
            XMLFREE(sofile);
            XMLFREE(func);
            XMLFREE(expr);
        }
        cur = cur->next;
    }
    if (rc != 0)
    {
        XMLFREE(name);
        XMLFREE(plen);
        XMLFREE(type);
        XMLFREE(value);
        XMLFREE(poffset);
        XMLFREE(sofile);
        XMLFREE(func);
        XMLFREE(expr);
    }
    xmlFreeDoc(cvt);

    /*
    // ת��������
    XmlGetString(doc, "/UFTP/MsgHdrRs/Result", buf, sizeof(buf));
    XmlGetString(doc, "/UFTP/MsgHdrRs/Desc", errinfo, sizeof(errinfo));
    map_errcode(buf, nbuf);

    // ���÷��ر����ֶ�
    sop_retinfo(nbuf, errinfo);
     */

    /* ��ӡ��־ sop.log */
    POOL_TraceData();
    /* ����SOP�����ͱ��� */
    memset(aczSendMsg, 0, sizeof(aczSendMsg));
    len = ConvertPoolToPkt(aczSendMsg,sizeof(aczSendMsg), pktfmt);
    if (len < 0)
    {
        err_log("����SOP(%s)���ϴ�����ʧ��(%d)", pktfmt, len);
        return -1;
    }

    memcpy(sopbuf, aczSendMsg, len);

    return len;
}

int sop2tcop_rsp(char *trncode, xmlDocPtr doc, char *sopbuf, int soplen)
{
    xmlDocPtr cvt = NULL;
    xmlNodePtr node, cur;
    xmlXPathObjectPtr rs = NULL;
    char aczRetCode[8], aczRetMsg[1024];
    xmlChar *xbuf;
    char pktfmt[10];
    char buf[256], nbuf[256];
    char path[128];
    char *name, *type, *value, *grid, *sofile, *func, *expr;
    int len, rc = 0;
    char *p;
    char tmp[128]={0};
    int l=0;

    // ȡת�������ļ�
    //sprintf(path, "%s/etc/cvt.xml", getAppDir());
    sprintf(path, "%s/etc/cvt_op2sop.xml", getAppDir());
    if ((cvt = xmlParseFile(path)) == NULL)
    {
        err_log("����%sʧ��!", path);
        return -1;
    }

    sprintf(path, "/Root/Transaction[@code='%s']/Response", trncode);
    rs = getnodeset(cvt, path);
    if (rs == NULL || rs->nodesetval->nodeNr <= 0)
    {
        err_log("locate xml fail,[%s]!", path);
        xmlXPathFreeObject(rs);
        XMLFreeDoc( cvt );
        return -1;
    }

    /*
       if ((node = XmlLocateNode(cvt, path)) == NULL)
       {
       err_log("locate xml fail, [%s]", path);
       return -1;
       }
     */
    //���Ӧ���ĸ�ʽ
    for( l=0; l< rs->nodesetval->nodeNr; l++ )
    {
        node = rs->nodesetval->nodeTab[l];

        if ((p = XmlNodeGetAttrText(node, "packfmt")) == NULL)
        {
            err_log("���ҽڵ�[%s]ʧ��!", path);
            xmlFreeDoc(cvt);
            return -1;
        }
        sprintf(pktfmt, "%s", p);
        FREE(p);


        // ���SOP���´�����
        rc = ConvertPktToPool(sopbuf, soplen, pktfmt);
        if ( rc<0 )
        {
            err_log("���SOP���´�����(%s)ʧ��(%d)", pktfmt, rc);
            return -1;
        }
        rc = 0;

        // ��ӡ��־ sop.log 
        POOL_TraceData();

        memset(aczRetCode, 0, sizeof(aczRetCode));
        PGetStr("TPU_RETCODE", aczRetCode, sizeof(aczRetCode));
        err_log("***RetCode=[%s]***",aczRetCode);

        memset(tmp, 0, sizeof(tmp));
        PGetStr("GUIYLS", tmp, sizeof(tmp));
        err_log("***GUIYLS=[%s]***",tmp);

        memset(tmp, 0, sizeof(tmp));
        PGetStr("PTCWDH", tmp, sizeof(tmp));
        err_log("***PTCWDH=[%s]***",tmp);
        /*
           if (strcmp(trncode, "9960") == 0)
           {
           PGetStr("TIOZLX", aczRetCode, sizeof(aczRetCode));
           PGetStr("TPU_RETMSG", aczRetMsg, sizeof(aczRetMsg));
           err_log("���ں���ϵͳ���� %s: %s",aczRetCode, aczRetMsg);
           if (aczRetCode[0] == '1')
           {
           XmlSetVal(doc, "//opBKRetcode", "8999");
           XmlSetVal(doc, "//opBKRetinfo", "������˰����ʧ��");
           goto err_handler;
           }
           }
           else
           {
         */
        if (strcmp(aczRetCode, "AAAAAAA") != 0)
        {
            memset(aczRetMsg, 0, sizeof(aczRetMsg));
            PGetStr("TPU_RETMSG", aczRetMsg, sizeof(aczRetMsg));
            err_log("���ں���ϵͳ���� %s: %s",aczRetCode, aczRetMsg);

            // ���Ϊ����, ����EGG0862ʱ��Ϊ�ɹ�
            if (strncmp(pktfmt+1, "7704", 4) == 0 
                    && (strncmp(aczRetCode, "EGG0862", 7) == 0 
                        || strncmp(aczRetCode, "EGG0681", 7) == 0))
            {
                err_log("���ڷ����ѳ���.");
            }
            else
            {
                XmlSetVal(doc, "//opBKRetcode", aczRetCode);
                XmlSetVal(doc, "//opBKRetinfo", aczRetMsg);
                //break;
                goto err_handler;
            }
        }
        else
        {
            err_log("����ϵͳ�ɹ�����!");
        }
        //}

        err_log( "********�����ʽ[%s]********", pktfmt );
        if( strcmp(pktfmt, "O95793@R") != 0 )
        {
            err_log( "************���������ʽ*****************");

            if ((grid = XmlNodeGetAttrText(node, "grid")) != NULL)
            {
                err_log("׼�������������");
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
            else
            {
                err_log("��ʼ����sop���ر�������");
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
                    err_log("xml fail.");
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
                            break;
                        strcpy(buf, nbuf);
                    }
                    else
                    {
                        // ����
                        strcpy(buf, value);
                    }

                    err_log("SET [%-30s]=[%s]", name, buf);
                    XmlSetVal(doc, name, buf);
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
                            break;
                        memset(buf, 0, sizeof(buf));
                        strcpy(buf, nbuf);
                    }

                    err_log("SET [%-30s]=[%s]", name, buf);
                    XmlSetVal(doc, name, buf);
                }

                XMLFREE(name);
                XMLFREE(type);
                XMLFREE(value);
                XMLFREE(sofile);
                XMLFREE(func); 
                XMLFREE(expr);
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
            }

            if (grid != NULL)
            {
                POOL_SetGetSuff("");    /* ���POOL����������׺���ǳ���Ҫ������ */
                XMLFREE(grid);
            }
        }
        else
        {
            err_log( "************9579�����ʽ*****************");
            rc = UnPack_95793( doc, pktfmt);
        }
    }

err_handler:

    xmlFreeDoc(cvt);
    return rc;
}
/*95793���*/
int UnPack_95793( xmlDocPtr doc, char *pktfmt )
{
    int iLen=0, i=0, j=0;
    char acPkt[2048]={0};
    int  ch, ch1;
    char *p=NULL;
    char buf[512]={0};

    err_log( "************��ʼ9579���*****************");
    memset(acPkt,0x0,sizeof(acPkt));
    iLen=PGetMem("O95793",acPkt,sizeof(acPkt));

    //ȡO95793��������
    ch1=acPkt[3];      //�Թ�ǰ���
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opJIGOMC", buf );
    err_log( "SET opJIGOMC=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opGUIYLS", buf );
    err_log( "SET opGUIYLS=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opNIANNN", buf );
    err_log( "SET opNIANNN=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opYUEFEN", buf );
    err_log( "SET opYUEFEN=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opRIIIII", buf );
    err_log( "SET opRIIIII=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opZHANGH", buf );
    err_log( "SET opZHANGH=[%s]    ����[%d]", buf, ch1 );


    j=j+i;
    ch1=acPkt[j+3];
    ch=ch1;
    j=j+3;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opKEHZWM", buf );
    err_log( "SET opKEHZWM=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j+18];
    ch=ch1;
    j=j+18;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opSHMUMC", buf+3 );
    err_log( "SET opSHMUMC=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[j];
    ch=ch1;
    j=j;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opHUOBMC", buf );
    err_log( "SET opHUOBMC=[%s]    ����[%d]", buf, ch1 );

    j=j+i;
    ch1=acPkt[iLen-9];
    ch=ch1;
    //j=iLen-9;
    j=iLen-10;

    memset(buf, 0x0, sizeof(buf));
    for(i=1; i<=ch; i++)
        sprintf( buf, "%s%c", buf, acPkt[i+j]);
    buf[i] = '\0';
    XmlSetVal( doc, "//opJIO1GY", buf );
    err_log( "SET opJIO1GY=[%s]    ����[%d]", buf, ch1 );

    XmlSetVal( doc, "//opYJLXZE", "0.00");

    return 0;
}
