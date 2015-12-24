#include "tcop.h"

static char sql[SQLBUFF_MAX] = {0};

int OPTranHandle(xmlDoc *doc);
int OPInfoHandle(xmlDoc *doc);
int OPAdminHandle(xmlDoc *doc);
int OPQryPrtHandle(xmlDoc *doc);
int OPFundHandle(xmlDoc *doc);

int OPInitOPTran(xmlDoc *doc)
{
    int ret = 0;

    INFO("��ʼƽ̨����[%d]Ԥ����...", OP_OPTCODE);

    if (*XMLGetNodeVal(doc, "//opWorkdate") == 0)
        XMLSetNodeVal(doc, "//opWorkdate", GetWorkdate());

    XMLSetNodeVal(doc, "//opNodeid", vstrcat("%d", OP_REGIONID));
    XMLSetNodeVal(doc, "//opTrcode", vstrcat("%d", OP_OPTCODE));
    XMLSetNodeVal(doc, "//opTctcode", vstrcat("%d", OP_TCTCODE));
    XMLSetNodeVal(doc, "//opTrdate", getDate(0));
    XMLSetNodeVal(doc, "//opTrtime", getTime(0));

    switch (OP_OPTCODE / 100)
    {
        case 1: //������
            ret = OPTranHandle(doc); break;
        case 2: //��Ϣ��
            ret = OPInfoHandle(doc); break;
        case 3: //������
        case 4: case 5: case 6: 
            ret = OPAdminHandle(doc); break;
        case 7: //��ѯ��ӡ�� 
            ret = OPQryPrtHandle(doc); break;
        case 9: //�ʽ���
            ret = OPFundHandle(doc); break;
    }

    INFO("ƽ̨����[%d]Ԥ�������!ret=[%d]", OP_OPTCODE, ret);

    return ret;
}

int OPAfterCommToPH(xmlDoc *doc)
{
    char *p = NULL;
    int ret = 0;

    if (OP_APPTYPE != APPTYPE_OPBANK_AUTOSVR && OP_APPTYPE != APPTYPE_OUTTRANS_SERVER)
        return 0;

    p = XMLGetNodeVal(doc, "//opTCRetcode");

    switch (OP_OPTCODE / 100)
    {
        case 1: ret = OPTranAfterCommToPH(doc, atoi(p)); break;
        case 2: ret = OPInfoAfterCommToPH(doc, atoi(p)); break;
        case 4: case 5: case 6:
                ret = OPAdmAfterCommToPH(doc, atoi(p)); break;
    }

    if (OP_APPTYPE == APPTYPE_OUTTRANS_SERVER)
        INFO("��ǰ��ͨѶ�����ƽ̨Ԥ�������![%d]", ret);
    else
        INFO("������ͨѶ�����ƽ̨Ԥ�������![%d]", ret);

    return ret;
}

int OP_DoFinish(xmlDoc *doc, int ret)
{
    char setbuf[512] = {0};

    if (doc == NULL)
        return 0;

    if (OP_OPTCODE == OPT_TRAN_OUTINPUT)
    {
        if (!isSuccess(ret))
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
    }

    if (OP_OPTCODE == OPT_TRAN_OUTCHECK || OP_OPTCODE == OPT_TRAN_OUT)
    {
        if (isSuccess(ret))
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_SETTLED);
        else if (ret != E_SYS_COMM_PH && ret != E_SYS_COMM)
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
        else
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_UNKNOW);
    }

    if (OP_OPTCODE == OPT_TRAN_IN && !isSuccess(ret))
    {
        //���뽻��Ӧ�ô����ɹ�(Ӧ�𲻳ɹ�)

        if (ret == E_SYS_COMM_PH) {
            //Ӧ��ͨѶʧ��
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_UNKNOW);
        } else if (ret < 0) {
            //Ӧ�ô����δӦ��
            sprintf(setbuf, "result=%d,clearstate='%c'", E_OTHER, CLRSTAT_UNKNOW);
        } else {
            //Ӧ�ô���ʧ��(Ӧ��ʧ��)
            sprintf(setbuf, "result=%d,clearstate='%c'", ret, CLRSTAT_FAILED);
        }
    }

    if (setbuf[0] == 0)
        return 0;

    sprintf(sql, "UPDATE trnjour SET %s WHERE %s", setbuf, GetSigleTrnjourWhere(doc));
    INFO("���׽������½���״̬ [%s]", setbuf);

    return db_exec(sql);
}

