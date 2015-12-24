#include "interface.h"

int SetException(char *rspbuf, int *rsplen, int operr)
{
    xmlDoc *doc = NULL;
    unsigned char *pbuf = NULL;
    const char *p = NULL;
    const char *pResult = NULL;
    char tcErrinfo[256] = "���׳ɹ�";
    char tcErrcode[8] = "0000";
    char tmp[256] = {0};
    int ret = 0;

    if ((doc = xmlRecoverDoc(rspbuf)) == NULL)
    {
        INFO("�ع���Ӧ����DOCʧ��! [%s]", rspbuf);
        return E_PACK_INIT; 
    }

    //���뽻�������ڳ����������ڴ�����ӳ���ͬ�Ǵ�����Ϊ׼����,���޴�ӳ������ƽ̨������ӳ���ͬ�Ǵ�����Ϊ׼����
    if (isInTran())
    {
        //"//Reserve"�д�����ڴ�����
        p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Reserve");

        if (isSuccess(operr)) 
        {
            //�����������ֱ�ӷ��سɹ����ж�
        } 
        else if (p != NULL && p[0] != 0)
        {
            //�������ڴ��������Ӧǰ�õĴ�����
            ret = DBQueryStrings(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%s' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        TCOP_BANKID, OP_NODEID, p, OP_NODEID), 2, tcErrcode, tcErrinfo);
        } 
        else if (operr) 
        {
            //����ƽ̨���������Ӧǰ�õĴ�����
            ret = DBQueryStrings(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%d' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        OP_OPNODE, OP_NODEID, operr, OP_NODEID), 2, tcErrcode, tcErrinfo);
        }

        if (ret != 0)
        {
            //ͨ������֧�ֿɺ��Ե�����/ƽ̨����
            if (ret == E_DB_NORECORD)
            {
                strcpy(tcErrcode, "8999");
                strcpy(tcErrinfo, "δ֪����");
                INFO("δӳ��%s�������[%d],Ĭ��[%s]!", 
                        atoi(p) != 0 ? "����" : "ƽ̨", 
                        atoi(p) != 0 ? atoi(p) : operr, tcErrcode);
            } else
                return ret;
        }

        XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", tcErrcode);
        XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc", tcErrinfo);
    }

    if (isOutTran())
    {
        //���Ĵ���
        pResult = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result");
        if (pResult[0] != 0)
        {
            if (atoi(pResult) == 0)
                strcpy(tmp, "���ĳɹ�");
            else
            {
                ret = DBQueryString(tcErrinfo, 
                        vstrcat("SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%s'", OP_PHNODEID, pResult));
                if (ret)
                {
                    if (ret != E_DB_NORECORD)
                        return ret;
                    INFO("δ�ҵ��ڵ�[%d]�Ĵ�����[%s]��Ϣ", OP_PHNODEID, pResult);
                    strcpy(tmp, "������������");
                } else  
                    strcpy(tmp, tcErrinfo);
            } 
        } 

        //ƽ̨����
        if (operr)
        {
            ret = DBQueryString(tcErrinfo, 
                    vstrcat("SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%d'", OP_OPNODE, operr));
            if (ret)
            {
                if (ret != E_DB_NORECORD)
                    return ret;
                INFO("δ�ҵ��ڵ�[%d]�Ĵ�����[%d]��Ϣ", OP_OPNODE, operr);
                strcpy(tmp, "ƽ̨��������");
            }
            sprintf(tmp + strlen(tmp), "%s%s", tmp[0] == 0 ? "" : "|", tcErrinfo);

            //���ڴ�����Ϣ
            p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc");
            if (p[0] != 0)
                sprintf(tmp + strlen(tmp), "|%s", p);

            //���ĳɹ���Ҫ�������Ӧ�����еĴ��������ֱ��ʹ��������Ӧ�����еĴ����뷵��
            if (pResult[0] == 0 || atoi(pResult) == 0)
            {
                //ʹ��ƽ̨�Ĵ�����ӳ���ͬ�Ǵ����뷵��
                ret = DBQueryString(tcErrcode, vstrcat("SELECT mapcode FROM errmap WHERE nodeid=%d AND errcode='%d' AND mapnode=%d",
                            OP_OPNODE, operr, OP_PHNODEID));
                if (ret) 
                {
                    if (ret != E_DB_NORECORD)
                        return ret;
                    strcpy(tcErrcode, "9999");
                }
                XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", tcErrcode);
            }
        }

        p = XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result");
        if (p != NULL && p[0] == 0)
            XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Reserve"));
        if (strlen(tmp))
            XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc",  tmp);
    }

    INFO("Result:[%s] Desc:[%s] operr:[%d]", XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result"), 
            XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc"), operr);

    xmlDocDumpFormatMemory(doc, &pbuf, rsplen, 0);
    memcpy(rspbuf, pbuf, *rsplen);
    rspbuf[*rsplen] = 0;

    DBUG("RSP:[%s]", rspbuf);

    return 0;
}
