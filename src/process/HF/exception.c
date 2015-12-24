#include "interface.h"

int SetException_10(xmlDoc *opDoc, char *rspbuf, int *rsplen, int operr)
{
    xmlDoc *doc = NULL;
    unsigned char *pbuf = NULL;
    char responseInfo[256] = "���׳ɹ�";
    char responseCode[8] = "0000";
    const char *opBKRetcode = NULL, *opBKRetinfo = NULL, *opTCRetcode = NULL, *opTCRetinfo = NULL;
    char tmp[256] = {0};
    int ret = 0;

    if ((doc = xmlRecoverDoc(rspbuf)) == NULL)
    {
        INFO("�ع���Ӧ����DOCʧ��! [%s]", rspbuf);
        return E_PACK_INIT; 
    }

    if (opDoc != NULL)
    {
        opBKRetcode = XMLGetNodeVal(opDoc, "//opBKRetcode"); opBKRetinfo = XMLGetNodeVal(opDoc, "//opBKRetinfo");
        opTCRetcode = XMLGetNodeVal(opDoc, "//opTCRetcode");
        opTCRetinfo = XMLGetNodeVal(opDoc, "//opTCRetinfo");
    }

    DBUG("BKcode:%s BKinfo:%s TCcode:%s TCinfo:%s operr:%d", SAFE_PTR(opBKRetcode), 
            SAFE_PTR(opBKRetinfo), SAFE_PTR(opTCRetcode), SAFE_PTR(opTCRetinfo), operr);

    if (operr == E_DB_OPEN) {
        strcpy(responseCode, "8999");
        strcpy(responseInfo, "���ݿ�򿪴�");
        goto EXIT;
    }

    //���뽻�������ڳ����������ڴ�����ӳ���ͬ�Ǵ�����Ϊ׼����,���޴�ӳ������ƽ̨������ӳ���ͬ�Ǵ�����Ϊ׼����
    if (isInTran())
    {
        if (opBKRetcode != NULL && atoi(opBKRetcode) != 0)
        {
            /*
            //�������ڴ��������Ӧǰ�õĴ�����
            ret = db_query_strs(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%s' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        TCOP_BANKID, OP_NODEID, opBKRetcode, OP_NODEID), responseCode, responseInfo);
                        */
            strcpy(responseCode, "8999");
            if (opBKRetinfo == NULL)
                strcpy(responseInfo, "δ֪����");
            else
                strcpy(responseInfo, opBKRetinfo);
            goto EXIT;
        } 
        if (operr) 
        {
            //����ƽ̨���������Ӧǰ�õĴ�����
            ret = db_query_strs(vstrcat("SELECT mapcode, errinfo FROM errmap a, errinfo b WHERE "
                        "a.nodeid=%d AND a.mapnode=%d AND a.errcode='%d' AND b.nodeid=%d AND b.errcode=a.mapcode",
                        OP_OPNODE, OP_NODEID, operr, OP_NODEID), responseCode, responseInfo);
            if (ret != 0)
            {
                strcpy(responseCode, "8999");
                if (opBKRetinfo == NULL)
                    strcpy(responseInfo, "δ֪����");
                else
                    strcpy(responseInfo, opBKRetinfo);
                INFO("δ�ҵ�ӳ��%s�������[%d],Ĭ��[%s]!", atoi(opBKRetcode) != 0 ? "����" : "ƽ̨", 
                        atoi(opBKRetcode) != 0 ? atoi(opBKRetcode) : operr, responseCode);
            }
        }
    }

    if (isOutTran())
    {
        //ƽ̨δ����ʱͨ������ֱ��ת����Ӧ�����Ӧ����Ϣ�Ľ��ײ�������
        if (operr == 0 && (XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Result")[0] != 0 ||
                    XMLGetNodeVal(doc, "/UFTP/MsgHdrRs/Desc")[0] != 0))
            goto EXIT2;

        //���Ĵ���
        if (opTCRetcode != NULL && opTCRetcode[0] != 0)
        {
            if (atoi(opTCRetcode) == 0)
                strcpy(tmp, "���ĳɹ�");
            else
            {
                strcpy(responseCode, opTCRetcode);
                ret = db_query_str(tmp, sizeof(tmp),
                        "SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%s'", OP_PHNODEID, opTCRetcode);
                if (ret)
                {
                    INFO("δ�ҵ��ڵ�[%d]�Ĵ�����[%s]��Ϣ", OP_PHNODEID, opTCRetcode);
                    strcpy(tmp, "������������");
                } 
            } 
        }

        //ƽ̨����
        if (operr)
        {
            ret = db_query_str(responseInfo, sizeof(responseInfo),
                    "SELECT errinfo FROM errinfo WHERE nodeid=%d AND errcode='%d'", OP_OPNODE, operr);
            if (ret)
            {
                INFO("δ�ҵ��ڵ�[%d]�Ĵ�����[%d]��Ϣ", OP_OPNODE, operr);
                strcpy(responseInfo, "ƽ̨��������");
            }
            sprintf(tmp + strlen(tmp), "%s%s", tmp[0] == 0 ? "" : "|", responseInfo);

            //���ڴ�����Ϣ
            if (opBKRetcode != NULL && opBKRetinfo[0] != 0)
                sprintf(tmp + strlen(tmp), "|%s", opBKRetinfo);

            //���ĳɹ���Ҫ�������Ӧ�����е�ƽ̨ӳ��Ĵ��������ֱ��ʹ��������Ӧ�����еĴ����뷵��
            if (opTCRetcode == NULL || atoi(opTCRetcode) == 0)
            {
                //ʹ��ƽ̨�Ĵ�����ӳ���ͬ�Ǵ����뷵��
                ret = db_query_str(responseCode, sizeof(responseCode), 
                        "SELECT mapcode FROM errmap WHERE nodeid=%d AND errcode='%d' AND mapnode=%d", 
                        OP_OPNODE, operr, OP_PHNODEID);
                if (ret) 
                    strcpy(responseCode, "9999");
            }
        }

        if (strlen(tmp))
            strcpy(responseInfo, tmp);
    }

EXIT:

    INFO("Result:[%s] Desc:[%s] operr:[%d]", responseCode, responseInfo, operr);

    //����������뽻�׵�Ӧ�������ʹ�����Ϣ
    XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Result", responseCode);
    XMLSetNodeVal(doc, "/UFTP/MsgHdrRs/Desc", responseInfo);

EXIT2:
    xmlDocDumpFormatMemory(doc, &pbuf, rsplen, 0);
    memcpy(rspbuf, pbuf, *rsplen);
    rspbuf[*rsplen] = 0;

    DBUG("RSP:[%s]", rspbuf);

    return 0;
}
