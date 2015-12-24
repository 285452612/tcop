/**
 * ���ݵ����������ת������
 */

#include "cz_const.h"

int transHandle(int commcode, char *reqbuf)
{
    char tmpNoteno[11] ={0};;
    char tmpAmt[13] ={0};
    char tmpNotetype[3]={0};

    switch (commcode) {
        case 1:
        case 2:
            //�����־ת��
            if (commcode == 1)
                reqbuf[190] = '0'; 
            else 
                reqbuf[190] = '1';

            memcpy(tmpAmt, reqbuf+192, 13); //���
            sprintf(reqbuf+192, "%12.2lf", atof(tmpAmt)); 
            memcpy(tmpNoteno, reqbuf+205, 11 ); //ƾ֤����
            sprintf(reqbuf+205, "%010d", atoi(tmpNoteno)); 
            /*ҵ������ת��*/
            memcpy(tmpNotetype, reqbuf+4, 3);//ƾ֤����
            if( (atoi(tmpNotetype)==11)||(atoi(tmpNotetype)==33)||(atoi(tmpNotetype)==44) )
                reqbuf[2]='2';
            break;
    }

    return 0;
}

int svrMain(char *commcode, char *reqbuf, char *rspbuf, int *plen)
{
    int ret = 0;
    char errmsg[256] = {0};
    char tmpbuf[4096] = {0};
    int i = 0, j = 0;;

    if ((ret = initSysConfig(commcode, 1)) != 0) {
        *plen = sprintf(rspbuf, "%06d%-200s", E_PACK_INIT, "��ʼ������ϵͳ���ó���");
        return 0;
    }

    memset(tmpbuf, 0, sizeof(tmpbuf));
    memset(errmsg, 0, sizeof(errmsg));
    memcpy(tmpbuf, reqbuf, *plen);

    for (i = 0; i < sizeof(G_TransInfos)/sizeof(TransInfo); i++)
    {
        if (G_TransInfos[i].commcode != atoi(commcode))
            continue;
        for (j = 0; j < sizeof(G_ExchgInfos)/sizeof(ExchgInfo); j++) {
            if (memcmp(G_ExchgInfos[j].exchgNo, tmpbuf+G_TransInfos[i].exchgnoPos, strlen(G_ExchgInfos[j].exchgNo)) == 0) {
                INFO("��ȡ������Ϣ:%s->%s:%d", G_ExchgInfos[j].exchgNo, G_ExchgInfos[j].prehostAddr, G_ExchgInfos[j].prehostPort);
                break;
            }
        }
        break;
    }

    if (transHandle(atoi(commcode), tmpbuf)) {
        *plen = sprintf(rspbuf, "%06d%-200s", E_SYS_CALL, "���״����");
        return 0;
    }

    ret = SwitchRecord(G_ExchgInfos[j].prehostAddr, G_ExchgInfos[j].prehostPort, G_TransInfos[i].tctcode, tmpbuf,
            *plen, plen, errmsg);

    *plen = sprintf(rspbuf, "%06d%-200s", ret, errmsg);

    return 0;
}

int main()
{
    TAPIHEAD head;
    char reqbuf[4096+1] = {0};
    char rspbuf[4096+1] = {0};
    char tmp[4096] = {0};
    char *pfile = NULL;
    int i = 0;
    int len = 0;
    int ret = 0;

    strcpy(OP_HOME, getenv("HOME"));

    memset(&head, 0, sizeof(head));
    memset(reqbuf, 0, sizeof(reqbuf));
    memset(rspbuf, 0, sizeof(rspbuf));

    if ((ret = svr_rcv(&head, reqbuf, G_REQFILE, 0)) < 0)
    {
        INFO("�����������ʧ��!node=[%s]ret=[%d]", head.NodeId, ret);
        goto EXIT;
    }

    len = head.Sleng;
    {
        memcpy(tmp, reqbuf, len);
        for (i = 0; i < len - 1; i++)
            if (tmp[i] == 0) tmp[i] = ' ';
        DBUG("���յ�������:%d\n%s]", len, tmp);
    }
    INFO("REQ:[%s-%s]----------------------------------", head.NodeId, head.TrType);
    ret = svrMain(head.TrType, reqbuf, rspbuf, &len);
    INFO("RSP:[%s-%s]----------------------------------[%d]", head.NodeId, head.TrType, ret);
    if (ret != 0)
        goto EXIT;

    {
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, rspbuf, len);
        for (i = 0; i < len - 1; i++)
            if (tmp[i] == 0) tmp[i] = ' ';
        DBUG("����Ӧ����:\n%s]", tmp);
    }

    memset(&head, 0, sizeof(head));
    head.Sleng = len;
    head.PackInfo |= htonl(PI_DCOMPRESS);
    if (strlen(G_RSPFILE) > 0)
    {
        head.Ftype = 'B';
        head.PackInfo |= htonl(PI_FCOMPRESS);
        pfile = G_RSPFILE;
    }

    ret = svr_snd(&head, rspbuf, pfile, 0);

    INFO("������Ӧ���ݳ���[%d]�ļ�[%s]%s!ret=[%d]", len, 
            pfile == NULL ? "" : pfile, (ret == 0 ? "�ɹ�" : "ʧ��"), ret);

EXIT:

    INFOLINE();
    tapi_svrend();

    return 0;
}
