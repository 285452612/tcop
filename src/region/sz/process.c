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

//����Ԥ����
int OP_DoInit(char *req, int *plen)
{
    xmlDoc          *doc            = NULL;
    xmlDoc          *opDoc          = NULL;
    unsigned char   *docbuf         = NULL;
    char            encTrack[1024]  = {0};  //��������(base64)
    char            decTrack[512]   = {0};   //��������
    char            track2[128]     = {0};     //����2����
    char            track3[128]     = {0};     //����3����
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
    returnIfNullLoger(doc, E_PACK_INIT, "Ԥ����������ױ��ĳ�ʼ����");

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
                    INFO("�ŵ���Ϣ����ʧ��:%d|%d|%s|%s[%s]", ret, strlen(encTrack),
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
            INFO("����Ա�ض�[%s]->[%s]", p, XMLGetNodeVal(doc, "//AcctOper") );
        }
#endif 

        p = XMLGetNodeVal(doc, "//TrnCode");
        XMLSetNodeVal(doc, "//MsgHdrRq/WorkDate", GetWorkdate());
        XMLSetNodeVal(doc, "//ExchgRound",GetExchground());
        //ƾ֤����Ϊ���Զ�����Ϊ��ˮ��
        ptmp = XMLGetNodeVal(doc, "//NoteNo");
        if( ptmp != NULL )
        {
            if( strlen(ptmp) == 0 )
                XMLSetNodeVal(doc,"//NoteNo", XMLGetNodeVal(doc, "//RefId"));
        }
        /*��Ʊ���׸�����ˮȥ����ԭƱ����Ϣ*/
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
                /*��ѯӪҵ���� 9328����*/
                q=XMLGetNodeVal(doc, "//TermId");
                //ת��ͬ�Ǳ��ĵ�ƽ̨����
                opDoc = getOPDoc();
                returnIfNull(opDoc, E_PACK_INIT);
                XMLSetNodeVal(opDoc, "//opWorkdate", XMLGetNodeVal(doc, "//WorkDate"));
                XMLSetNodeVal(opDoc, "//opOperid", XMLGetNodeVal(doc, "//AcctOper"));
                XMLSetNodeVal(opDoc, "//opPDWSNO", XMLGetNodeVal(doc, "//PDWSNO"));
                //XMLSetNodeVal(opDoc, "//opInnerBank", XMLGetNodeVal(doc, "//TermId"));
                XMLSetNodeVal(opDoc, "//opInnerBank", q);

                if( ret = callInterface( 9328, opDoc) )
                {
                    BKINFO("��ѯӪҵ����ʧ��[%d]...", ret);
                    return ret;
                }

                p=XMLGetNodeVal(opDoc, "//opHostSerial");
                if(strncmp( p,"89",2 ))
                {
                    BKINFO("����ҵ�������Ա[%s]���ڻ���[%s]...", XMLGetNodeVal(doc, "//AcctOper"), p);
                    memset(tmp, 0, sizeof(tmp));
                    ret = db_query_str(tmp, sizeof(tmp), "SELECT parent FROM bankinfo where nodeid=%d and exchno='%s'", 
                            OP_REGIONID, XMLGetNodeVal(doc, "//Originator"));
                    if( ret )
                    {
                        BKINFO("���ڻ����Ҳ�����[%s]...", "8901");
                        XMLSetNodeVal(doc, "//TermId", "8901");
                    }
                    else
                    {
                        BKINFO("ԭ���׽�����[%s],��[%s]��Ʊ���ڼ���...", XMLGetNodeVal(doc, "//Originator"), tmp);
                        XMLSetNodeVal(doc, "//TermId", tmp);
                    }

                }
                else
                    XMLSetNodeVal(doc, "//TermId", p);

                memset(tmp1, 0, sizeof(tmp1));
                ret = db_query_str(tmp1, sizeof(tmp1), "SELECT exchno FROM bankinfo where nodeid=%d and bankid='%s'", 
                        OP_REGIONID, XMLGetNodeVal(doc, "//TermId"));
                XMLSetNodeVal(doc, "//Originator", tmp1);

                BKINFO("��Ʊ�������ڼ��˻���[%s],�������л���[%s]...", XMLGetNodeVal(doc, "//TermId"), tmp1);
            }

            //�ŵ��
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
            //������Ʊʹ��
            XMLSetNodeVal(doc, "//SHKRZH", beneacct);
            XMLSetNodeVal(doc, "//SHKRXM", benename);
            XMLSetNodeVal(doc, "//FUKRZH", payacct);
            XMLSetNodeVal(doc, "//FUKRXM", payer);
        }
        if (classid == 2) 
        { //����ҵ��
            XMLSetNodeVal(doc, "//TrnCode", dcflag == 1 ? "0001" : "0002");
            //if (notetype == 72 || notetype == 74) //�ֽ��ת��ͨ��
            if(notetype==71)
            {
                XMLSetNodeVal(doc,"//Payer","�ֽ�");
            }else if(notetype==72)
            {
                XMLSetNodeVal(doc,"//BeneName","�ֽ�");
            }
            if ((p = XMLGetNodeVal(doc, "//TrackInfo")) != NULL && *p != 0x00)
            {
                strcpy(decTrack, p);
                INFO("�ŵ���Ϣ����ǰ:[%s]", decTrack);
                if ((ret = Data_Encrypt_Soft10(XMLGetNodeVal(doc, "//WorkDate"),
                                XMLGetNodeVal(doc, "//RefId"),
                                decTrack, strlen(decTrack), encTrack, &len)) != 0) {
                    INFO("�ŵ���Ϣ����ʧ��:%d[%s]", ret, decTrack);
                    return E_SYS_SYDENCRYPT;
                }
                encTrack[len] = 0;
                INFO("�ŵ���Ϣ���ܺ�:%d[%s]", len, encTrack);
                XMLSetNodeVal(doc, "//TrackInfo", encTrack);
            }
        } 
        if (dcflag == 2) 
        {
            switch(notetype) {
                case 2:     //ת��֧Ʊ
                case 41:    //ȫ�����л�Ʊ
                case 44:    //���˴�ת����ƾ֤
                case 61:    //����ת��ƾ֤
                case 62:    //����
                case 71:    //�����ֽ�ͨ��ͨ��
                case 72:    //���˽����˻�ת��
                case 81:    //���ת��֧Ʊ
                case 84:    //��Ҹ���ת��
                case 86:    //�������
                case 87:    //�������ת��ƾ֤
                    //XMLSetNodeVal(doc, "//AcctCheck", "1");
                    break;
                default:
                    break;
            }
        }
        //����Ա�ض�
        p = XMLGetNodeVal(doc, "//AcctOper");
        if( strlen(p) > 6 )
        {
            XMLSetNodeVal(doc, "//AcctOper", p+2);
            INFO("����Ա�ض�[%s]->[%s]", p, XMLGetNodeVal(doc, "//AcctOper") );
        }
    }

    xmlDocDumpMemory(doc, &docbuf, plen);
    memcpy(req, docbuf, *plen);

    return 0;
}

//���빤����Կ
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

//ϵͳ״̬��ѯ
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

//�г�֪ͨ
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
        //��������
        if (strcmp(GetExchgdate(), XMLGetNodeVal(req, "//SysStatus/ExchgDate")) != 0)
        {
            UpdPreExchgdate(GetExchgdate());
            if ((ret = UpdExchgdate(XMLGetNodeVal(req, "//SysStatus/ExchgDate"))) != 0)
                return ret;
        }
        //��������
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

//����֪ͨ
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
    BKINFO( "������������:[%s], ǰһ��������[%s], ƽ̨��������[%s]",
            tmp1, tmp2, tmp );

    if (memcmp(tmp, tmp1, 8) != 0 )
    {
        sprintf(tmp2, "%s-%s", tmp, GetClearround());
        BKINFO( "ǰһ��������[%s]", tmp2 );
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

//����֪ͨ
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
    //���ǰ����һ���Զ�ȡ����ʧ����������ж����Զ�ȡ����
    if (round != (atoi(GetSettledDateround()+9) + 1)) {
        INFO("ǰ��[%s]�Զ�ȡ����ʧ��,����[%d]���Զ�ȡ����", GetSettledDateround()+9, round);
        return 0;
    }
#endif

    //�Զ�����
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

//���ض�������
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
    char            settledDate[12]     = {0}; //��ȡ�������ڳ���
    char            settledMsgDate[12]  = {0}; //����֪ͨ���ڳ���
    int             allRoundFlag        = 0;
    char            j                   = 0;
    int             i                   = 0;

    //�������Ƿ���Ȩ��ȡ����
    if (rspfile != NULL && strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "�����޴�Ȩ��");
        return 0;
    }

    pworkdate = XMLGetNodeVal(req, "//SysStatus/WorkDate"); 
    pworkround = XMLGetNodeVal(req, "//WorkRound"); 
    strcpy(settledDate, GetSettledDateround());
    strcpy(settledMsgDate, GetSettlmsgDateround());

#if 0
    //����ȡ������֪ͨ���ڵĶ���(����ͨ������ƽ̨�޸�ϵͳ����)
    if (memcmp(pworkdate, settledMsgDate, 8) != 0) {
        XMLSetNodeVal(*rsp, "//Desc", vstrcat("ȡ��������[%s]������֪ͨ����[%8.8s]", pworkdate, settledMsgDate));
        return 0;
    }

    BKINFO("pWorkDate[%s],pWorkRound[%s],SettledDate[%s],settledMsgDate[%s]", 
            pworkdate,pworkround,settledDate,settledMsgDate);

    //����Ƿ��ȡ����
    if (settledMsgDate[9] == '0' || pworkround[0] > settledMsgDate[9]) {
        XMLSetNodeVal(*rsp, "//Desc", "������δ���");
        return 0;
    }

    //ȡȫ������
    if (pworkround[0] == '0') {
        allRoundFlag = 1;
        if (settledDate[9] == settledMsgDate[9]) {
            XMLSetNodeVal(*rsp, "//Desc", "ϵͳ��ȫ������");
            return 0;
        }
    } else {
        if (pworkround[0] <=  settledDate[9]) {
            XMLSetNodeVal(*rsp, "//Desc", "ϵͳ�Ѷ���");
            return 0;
        }
        /*
           if (pworkround[0] != settledDate[9]+1) {
           XMLSetNodeVal(*rsp, "//Desc", vstrcat("ȡ���˳���[%s]����ȷ,�ó�֮ǰ�е�[%c]��δ����", 
           pworkround, settledDate[9]+1));
           return 0;
           }
         */
    }
#endif
    if (pworkround[0] == '0') 
        allRoundFlag = 1;
    sprintf(tmp, "%s-%s", pworkdate, pworkround);
    //����Ƿ��Ѷ���
    if (strcmp(tmp, GetSettledDateround()) == 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "ϵͳ�Ѷ���");
        return 0;
    }

    //����Ƿ��ȡ����
    if (strcmp(tmp, GetSettlmsgDateround()) > 0)
    {
        XMLSetNodeVal(*rsp, "//Desc", "������δ���");
        return 0;
    }

    XMLSetNodeVal(req, "//Reserve", "0");   //����ȡ������ϸ���������
    XMLSetNodeVal(req, "//Sender", "999");  //�ӿ�ȡ����

    if ((p = GetClasslist()) == NULL)
    {
        XMLSetNodeVal(*rsp, "//Desc", "ȡҵ���б�����");
        return 0;
    }
    strcpy(svcclassList, p);

    for (i = 0; i < strlen(svcclassList); i++)
    {
        if (svcclassList[i] != '1')
            continue;

        sprintf(svcclass, "%d", i+1);
        XmlSetString(req, "/UFTP/MsgHdrRq/SvcClass", svcclass);

        INFO("��ʼȡҵ��[%s]����[%s]��������...", svcclass, pworkround);
        memset(filename, 0, sizeof(filename));
        tmpDoc = CommDocToPH(&ret, 1605, req, filename);
        returnIfNull(tmpDoc, ret);

        INFO("[%s]:����[%s]����[%s]ҵ��[%s]", 
                XMLGetNodeVal(tmpDoc, "//Desc"), pworkdate, pworkround, svcclass);
        if ((ret = atoi(XMLGetNodeVal(tmpDoc, "//Result"))))
        {
            if (ret == 3902) 
                continue;
            XMLSetNodeVal(*rsp, "//Desc", "ȡ�������Ĵ���ʧ��");
            return 0;
        }

        INFO("��ʼҵ��[%s]����[%s]���˴���...", svcclass, pworkround);
        if ((ret = SettleSvr(filename, tmpDoc, pworkdate, pworkround, svcclass)) != 0)
        {
            INFO("���˴���ʧ�� ret=[%d]", ret);
            XMLSetNodeVal(*rsp, "//Desc", "���˴���ʧ��");
            return 0;
        }
        INFO("���˴����ɹ�");
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
        INFO("����[%s]�Զ���ִʧ��", pworkround);
        return 0;
    }
    INFO("��ִ���[%s]", XMLGetNodeVal(*rsp, "//Desc"));

    if (rspfile == NULL && strcmp(XMLGetNodeVal(*rsp, "//Desc"), "��ִ�ɹ�") == 0)
        XMLSetNodeVal(*rsp, "//Desc", "");
    else
        XMLSetNodeVal(*rsp, "//Desc", vstrcat("���˳ɹ�,�Զ���ִ[%s]", XMLGetNodeVal(*rsp, "//Desc")));


    INFO("���ض��˴������[%s]", XMLGetNodeVal(*rsp, "//Desc"));

    return 0;
}

//�ֹ����Ͷ��˻�ִ
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

    //�������Ƿ���Ȩ�޷���ִ
    if (strcmp(XMLGetNodeVal(req, "//Originator"), GetCBankno()) != 0) {
        XMLSetNodeVal(*rsp, "//Desc", "�����޴�Ȩ��");
        return 0;
    }

    pworkround = XMLGetNodeVal(req, "//WorkRound"); 
    strcpy(settledDate, GetSettledDateround());
    settledDate[8] = 0;
    pworkdate = settledDate;

    //����Ƿ�ɷ���ִ
    if (strcmp(pworkround, settledDate+9) > 0) {
        XMLSetNodeVal(*rsp, "//Desc", "������δ���");
        return 0;
    }

    if ((p = GetClasslist()) == NULL) {
        XMLSetNodeVal(*rsp, "//Desc", "ȡҵ���б�����");
        return 0;
    }
    strcpy(svcclassList, p);

    for (i = 0; i < strlen(svcclassList); i++)
    {
        if (svcclassList[i] != '1')
            continue;

        sprintf(svcclass, "%d", i+1);

        XMLSetNodeVal(req, "//SvcClass", svcclass);

        INFO("���ɶ��˻�ִ�ļ�...");
        memset(filename, 0, sizeof(filename));
        if (ret = CenterAndBankSettle(filename, atoi(svcclass), pworkdate, atoi(pworkround)))
        {
            INFO("���ɶ��˻�ִ�ļ�����,ret=%d", ret);
            XMLSetNodeVal(*rsp, "//Desc", "���ɶ��˻�ִ����");
            return 0;
        }
        INFO("��ʼ���Ͷ��˻�ִ:����[%s]����[%s]ҵ��[%s]�ļ�[%s]", pworkdate, pworkround, svcclass, filename);
        if (!strlen(filename))
        {
            INFO("�ޱ����ν������ݲ����ͻ�ִ");
            continue;
        }

        tmpDoc = CommDocToPH(&ret, 1606, req, filename);
        returnIfNull(tmpDoc, ret);

        p = XMLGetNodeVal(tmpDoc, "//Result");
        if (atoi(p) != 0)
        {
            INFO("���˻�ִӦ�����!result=[%s]desc=[%s]", p, XMLGetNodeVal(tmpDoc, "//Desc"));
            XMLSetNodeVal(*rsp, "//Desc", vstrcat("���˻�ִ���ͺ�����Ӧ���[%s]", XMLGetNodeVal(tmpDoc, "//Desc")));
            return 0;
        }
    }

    XMLSetNodeVal(*rsp, "//Desc", "��ִ�ɹ�");

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

//���������ز���
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

//��������֪ͨ
int PF_1608(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    return 0;
}

//���չ鵵
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
        INFO("[%s]�ѹ鵵(�Ѷ��������뵱ǰ�鵵����һ��)", p);
        return 0;
    }

    //ע:��������Ҳ��Ӧ�ù鵵(�ɶ�ʱ���̷���ʱ�����)
    strcpy(tmp, GetSettledDateround());
    if (strcmp(GetSettlmsgDateround(), tmp) != 0)
    {
        INFO("��δ���˽���!");
        return 0;
    }

    memset(precleardate, 0, sizeof(precleardate));
    memcpy(precleardate, GetPreCleardate(), 8);
    /*�Ժ�繤�����ڵ�ʱ���޸�*/
#if 0 
    if (strcmp(getDate(0), precleardate) != 0)
    {
        INFO("����������ǰһ�������ڲ�ͬ,���鵵!");
        return 0;
    }
#endif

    // �鵵���ں�һ��
    if ((archdate = daysafter(lastarchdate, "%Y%m%d", 1)) == NULL)
    {
        INFO("ȡ�鵵���ڳ���[%s]", lastarchdate);
        return 0;
    }
    INFO("��ʼ���չ鵵[%s]...", archdate);
    /*
       if ((ret = db_query_str(tmp, sizeof(tmp), "SELECT min(workdate) FROM trnjour")) != 0)
       return ret;
     */
    if (strcmp(archdate, precleardate) <= 0)
        sprintf(sql, "DELETE FROM htrnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    else
    {
        INFO("�鵵����[%s]������һ��������[%s],����Ҫ�鵵,��ֱ�ӷ��سɹ�", archdate, precleardate);
        //���¹鵵����(�Թ鵵���ڹ鵵)
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
    INFO("���¶���֪ͨ�������ںͳ�����Ϣ[%s]", tmp);
    if ((ret = UpdSettlmsgDateround(tmp)) != 0)
        return ret;

    INFO("������ȡ���˹������ںͳ�����Ϣ[%s]", tmp);
    if ((ret = UpdSettledDateround(tmp)) != 0)
        return ret;

EXIT:
    //���¹鵵����
    UpdArchivedate(precleardate);

    // ������ɾ�����տ�����
    if (strcmp(archdate, precleardate) <= 0)
        sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, archdate, precleardate);
    else
        sprintf(sql, "DELETE FROM trnjour WHERE nodeid=%d and workdate BETWEEN '%s' AND '%s'", OP_REGIONID, precleardate, archdate);
    if ((ret = db_exec(sql)) != 0 && ret != E_DB_NORECORD)
        return ret;

    INFO("���չ鵵�ɹ�");

    return 0;
}

/*
 * ���ݽ��׽���ж���������״̬
 */
static int IsClearState(char *pRet, int clearstate)
{
    switch(atoi(pRet))
    {
        case 0:
            return clearstate;
            //���׽������ȷ
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
            return CLRSTAT_UNKNOW;       //����״̬��ȷ��
        default:
            return CLRSTAT_FAILED;
    }
}

//���׽����ѯ
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

    //ת��ͬ�Ǳ��ĵ�ƽ̨����
    opDoc = getOPDoc();
    returnIfNull(opDoc, E_PACK_INIT);

    XMLSetNodeVal(opDoc, "//opInoutflag", inoutflag);
    XMLSetNodeVal(opDoc, "//opNodeid", vstrcat("%d", OP_REGIONID));

    if (ConvertTCXML2OP(opDoc, tmpDoc, "//INPUT/*") == NULL)
        return E_PACK_CONVERT;

    SavePack(opDoc, PACK_RSP2OP, 0); 

    clearstate = *XmlGetStringDup(opDoc, "//opClearstate");
    XmlGetString(opDoc, "//opTCRetcode", result, sizeof(result));
    INFO("���ķ��ؽ��׽��:[%s], ����״̬:[%c].", result, clearstate);
    clearstate = IsClearState(result, clearstate);
    switch (clearstate)
    {
        case CLRSTAT_CHECKED:   //�Ѷ���
        case CLRSTAT_SETTLED:   //����ɹ�
            sprintf(setbuf, "result=0,clearstate='%c',cleardate='%s',clearround='%s'",
                    clearstate, XMLGetNodeVal(tmpDoc, "//Content//ClearDate"),
                    XMLGetNodeVal(tmpDoc, "//Content//ClearRound"));
            break;

        case CLRSTAT_UNKNOW:   //����δ֪
        case CLRSTAT_FAILED:   //���ķ�������ʧ��
            sprintf(setbuf, "result=%d,clearstate='%c'", 
                    XmlGetInteger(tmpDoc, "//Content//Result"), clearstate);
            break;
        default:
            break;
    }
    if (setbuf[0] != 0)
    {
        INFO("���׽����ѯ����½���״̬ [%s]", setbuf);
        ret = db_exec(vstrcat("UPDATE trnjour SET %s WHERE nodeid=%d "
                    "AND workdate='%s' AND refid='%s' AND originator='%s' "
                    "AND inoutflag='%s'", setbuf, OP_REGIONID,
                    XMLGetNodeVal(tmpDoc, "//Content//WorkDate"),
                    XMLGetNodeVal(tmpDoc, "//Content//RefId"),
                    XMLGetNodeVal(tmpDoc, "//Content//Originator"),
                    inoutflag));
    }

    //������Ӧ������(�������ڽ��׽��)
    ret = callProcess(opDoc, NULL);
    if (ret == 0)
    {
        sprintf(setbuf, "%s-%s ������ˮ:%s", 
                XmlGetStringDup(opDoc, "//opTreserved1"), 
                XmlGetStringDup(opDoc, "//opBKRetinfo"), 
                XmlGetStringDup(opDoc, "//opTreserved2"));
    }
    else if (ret == E_DB_NORECORD)
        sprintf(setbuf, "��ѯʧ��: �޼�¼");
    else
        sprintf(setbuf, "��ѯʧ��: %d", ret);

    INFO("���ڽ��׽��:[%s]", setbuf);

    XmlSetString(tmpDoc, "/UFTP/MsgHdrRs/Reserve", setbuf);

    return 0;
}

//���뽻�׽����ѯ(��������в�ѯ)
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
            INFO("���뽻�׽����ѯ�鱾�������¼ʧ��");
            return E_DB;
        }
        //���ڴ�������Ҫ����ý��׶�Ӧ��ƽ̨���״�������
        ret = callProcess(opDoc, COMMTOPH_AFTER); 
#endif
        ret = db_query_str(result, sizeof(result), "select result from acctjour where %s", where);
        if (ret != 0 && ret != E_DB_NORECORD)
            XMLSetNodeVal(*rsp, "//Content//Result", "8999");
        else if (ret == E_DB_NORECORD)
            XMLSetNodeVal(*rsp, "//Content//Result", "8056");
        else if (atoi(result) == 1 || atoi(result) == 5)//5-����Ҳ���ɹ�����
        {
            XMLSetNodeVal(*rsp, "//Content//Result", "0000");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',cleardate='%s',result=%d WHERE %s",
                    CLRSTAT_SETTLED, GetRound(), XMLGetNodeVal(req, "//WorkDate"), ret, where);
        } else {
            XMLSetNodeVal(*rsp, "//Content//Result", "8999");
            XMLSetNodeVal(*rsp, "//Content//Desc", "���ڼ���ʧ��");
            sprintf(sql, "UPDATE trnjour SET clearstate='%c',clearround='%s',result=%d WHERE %s", 
                    CLRSTAT_FAILED, GetRound(), ret, where);
        }
        db_exec(sql);
    }

    return 0;
}

//���뽻��״̬ͬ��(�������н���ͬ��)
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
        INFO("���뽻��״̬ͬ���鱾�������¼ʧ��");
        return E_DB;
    }

    ret = atoi(XMLGetNodeVal(req, "//Result"));
    //if (XMLGetNodeVal(opDoc, "//Clearstate")[0] != CLRSTAT_SETTLED) //���ز�������ɹ�״̬
    {
        p = XMLGetNodeVal(req, "//ClearState");
        if (p != NULL && p[0] == CLRSTAT_SETTLED) //��������ɹ�
        {
            p = XMLGetNodeVal(req, "//Result");
            if (p != NULL && strcmp("0000", p) == 0) //���Ľ���ɹ�
            {
                //���ڴ�������Ҫ����ý��׶�Ӧ��ƽ̨���״�������
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

//�ֹ���������ˮ
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
        XMLSetNodeVal(*rsp, "//Desc", "��ѯ������ˮʧ��");
        return 0;
    }
    if(ret == 0)
    {
        if(iAcctType == 1 && atoi(sResult) == 1)    //���� �Ѽ���
        {
            XMLSetNodeVal(*rsp, "//Desc", "������ˮ�Ѵ��ڣ����貹����");
            return 0;
        }
        else if(iAcctType == 2 && atoi(sResult) == 2)   //���� �ѳ���
        {
            XMLSetNodeVal(*rsp, "//Desc", "�ý����ѳ������������");
            return 0;
        }
    }
    else if(iAcctType == 2)
    {
        XMLSetNodeVal(*rsp, "//Desc", "������ˮ�����ڣ��������");
        return 0;
    }

    memset(sSqlStr, 0, sizeof sSqlStr);
    if(iAcctType == 1 && ret == E_DB_NORECORD)  //����
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

    XMLSetNodeVal(*rsp, "//Desc", "���׳ɹ�");

    return 0;
}

int PF_PRTQRY(xmlDoc *req, xmlDoc **rsp, char *rspfile)
{
    char            result[5]           = {0};

    switch (OP_TCTCODE) {
        case 3111: ret = PrintDiffNote(req, rspfile);       break; //�ʽ�������
        case 3122: ret = PrintSettleResult(req, rspfile);   break; //���˽�����浥
        case 3105: ret = PrintInNoteList(req, rspfile);     break; //����������ϸ��
        case 3104: ret = PrintInJJQD(req, rspfile);         break; //���뽻���嵥
        case 3103: ret = PrintOutNoteList(req, rspfile);    break; //���������ϸ��
        case 3100: ret = PrintInNoteAdd(req, rspfile);      break; //���벹��ƾ֤��ѯ��ӡ
        case 3130: ret = PrintInNoteAdd_COP(req, rspfile);  break; //���벹��ƾ֤��ѯ��ӡ(cop)
        case 3131: ret = PrintClearTotal_PF(req, rspfile);     break; //����ҵ����ͳ�Ʊ�
        case 3132: ret = PrintNoteTotal_PF(req, rspfile);      break; //����ҵ������ͳ�Ʊ�
        case 3133: ret = PrintInNoteAdd_KS(req, rspfile);      break; //���˿�˰ƾ֤��ӡ
        case 3120: ret = PrintOutNoteTotal(req, rspfile);   break;
        case 3121: ret = PrintInNoteTotal(req, rspfile);    break;
        case 3101: ret = PrintOutJHD(req, rspfile);         break; //����������ܵ�
        case 3102: ret = PrintOutJJQD(req, rspfile);        break; //��������嵥
        case 8015: ret = MailQuery(req, rspfile);           break; //�ʼ���ѯ
        case 3001: ret = QryOutNote(req, rspfile);          break; //���ƾ֤��ѯ
        case 3002: ret = QryInNote(req, rspfile);           break; //����ƾ֤��ѯ
        case 8204: ret = QryAgreementList(req, rspfile);    break; //Э����Ϣ���ܵ�
        case 8012: ret = QryOutQuery(req, rspfile);         break; //��ѯ�����ѯ�鸴��
        case 8013: ret = QryInQuery(req, rspfile);          break; //��ѯ�����ѯ�鸴��
        case 8011: ret = PrintInQuery(req, rspfile);        break; //��ӡ�����ѯ�鸴��
        case 8010: ret = PrintOutQuery(req, rspfile);       break; //��ӡ�����ѯ�鸴��
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
        { PART_DEBIT,      "��:" },
        { PART_CREDIT,     "��:" },
        { PART_ZS,         "ָʾ:" },
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
    //sprintf(settlinfo, "��������:%s ����:%s", workdate, workround);
    workdate = GetWorkdate();
    XmlGetString(req, "//Originator", tmp, sizeof(tmp));
    rc = db_query_str(originator, sizeof(originator), 
            "select exchno from bankinfo where bankid='%s'", tmp);
    if (rc != 0)
        return E_DB;
    sprintf(settlinfo, "������:%02ld/%02ld ", atol(workdate)%10000/100,
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
                    strcpy(traninfo, "����δ��ӡ(");
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
            strcat(traninfo, "δ����:");
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
            strcat(traninfo, "��ѯ:");
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
            strcat(traninfo, "�鸴:");
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
            strcat(traninfo, "�ʼ�:");
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
//��������Э��
int PF_1011(xmlDocPtr req, xmlDocPtr *rsp, char *rspfile)
{
    int ret=0;
    char sqlwhere[256]={0};

    sprintf(sqlwhere, "nodeid = %d and payingacct = '%s' and svcid = '%s'", OP_REGIONID, XMLGetNodeVal(req, "//AcctId"),  XMLGetNodeVal(req, "//SVCId"));
    if (db_hasrecord("agreement", sqlwhere) == TRUE)
    {
        XMLSetNodeVal(*rsp, "//Desc", "Э���Ѵ���");
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
            XMLSetNodeVal(*rsp, "//Desc", "����Э��ʧ��");
            XMLSetNodeVal(*rsp, "//Result", "8999");
        }
        else
        {
            XMLSetNodeVal(*rsp, "//Desc", "����Э��ɹ�");
            XMLSetNodeVal(*rsp, "//Result", "0000");
        }
    }
    return 0;
}
//ע������Э��
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
        XMLSetNodeVal(*rsp, "//Desc", "Э�鲻����");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else
    {
        ret = db_exec("delete from agreement where %s", sqlwhere);
        if(ret)
        {
            XMLSetNodeVal(*rsp, "//Desc", "ע��Э��ʧ��");
            XMLSetNodeVal(*rsp, "//Result", "8999");
        }
        else
        {
            XMLSetNodeVal(*rsp, "//Desc", "ע��Э��ɹ�");
            XMLSetNodeVal(*rsp, "//Result", "0000");
        }
    }
    return 0;
}
//��ѯ����Э��
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
        XMLSetNodeVal(*rsp, "//Desc", "�޴��˻���Э��");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else if( ret )
    {
        XMLSetNodeVal(*rsp, "//Desc", "��ѯЭ��ʧ��");
        XMLSetNodeVal(*rsp, "//Result", "8999");
    }
    else
    {
        XmlSetString(*rsp, "/UFTP/AcctDetail/Name", sName);
        XmlSetString(*rsp, "/UFTP/AcctDetail/AgreementId", sAgreement);
        XmlSetString(*rsp, "/UFTP/AcctDetail/Phone", sPhone);
        XmlSetString(*rsp, "/UFTP/AcctDetail/Address", sAddr);
        XMLSetNodeVal(*rsp, "//Desc", "��ѯ�ɹ�");
        XMLSetNodeVal(*rsp, "//Result", "0000");
    }
    return 0;
}