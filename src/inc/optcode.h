#ifndef OPTCODE_H_
#define OPTCODE_H_

/** ƽ̨�����붨�� */

#define OPT_TRAN_BASE       100      //������

#define OPT_TRAN_OUTINPUT   OPT_TRAN_BASE+1      //���¼��
#define OPT_TRAN_OUTCHECK   OPT_TRAN_BASE+2      //�������
#define OPT_TRAN_OUTMODIFY  OPT_TRAN_BASE+3      //����޸�
#define OPT_TRAN_IN         OPT_TRAN_BASE+4      //���뽻��
#define OPT_TRAN_QRYSIGLE   OPT_TRAN_BASE+5      //���ʼ�¼��ѯ
#define OPT_TRAN_ACCOUNT    OPT_TRAN_BASE+6      //�ֹ���������
#define OPT_TRAN_CZ         OPT_TRAN_BASE+7      //�ֹ����ʳ�����ȡ��
#define OPT_TRAN_OUT        OPT_TRAN_BASE+8      //ֱ���������
#define OPT_TRAN_QUERY      OPT_TRAN_BASE+20     //��ʼ�¼��ѯ

#define OPT_INFO_BASE       200      //��Ϣ��

#define OPT_INFO_REQSEND    OPT_INFO_BASE+1      //��ѯ��¼��                        
#define OPT_INFO_RSPSEND    OPT_INFO_BASE+2      //�鸴��¼��                        
#define OPT_INFO_MSGSEND    OPT_INFO_BASE+3      //���ɸ�ʽ����                     
#define OPT_INFO_QRYSINGLE  OPT_INFO_BASE+4      //��ѯ�鸴�鵥�ʲ�ѯ                        
#define OPT_INFO_INREQBOOK  OPT_INFO_BASE+11     //�����ѯ��
#define OPT_INFO_INRSPBOOK  OPT_INFO_BASE+12     //����鸴��
#define OPT_INFO_INMSG      OPT_INFO_BASE+13     //�������ɸ�ʽ

#define OPT_ADM_COMMON      300     //ͨ�ù�����

#define OPT_ADM_DELTRAN     OPT_ADM_COMMON+1     //�������ɾ��

#define OPT_ADM_ACCT        400     //�˻�������

#define OPT_ACCT_BANKQRY    OPT_ADM_ACCT+1       //�����˻���ѯ
#define OPT_ACCT_REGISTER   OPT_ADM_ACCT+2       //����
#define OPT_ACCT_CANCEL     OPT_ADM_ACCT+3       //����
#define OPT_ACCT_MODIFY     OPT_ADM_ACCT+4       //�˻��޸�
#define OPT_ACCT_PBCQRY     OPT_ADM_ACCT+5       //�����˻���Ϣ��ѯ

#define OPT_ADM_OPER        500     //��Ա������

#define OPT_OPER_LOGIN      OPT_ADM_OPER+1       //��Ա��¼
#define OPT_OPER_LOGOUT     OPT_ADM_OPER+2       //��Ա�˳�
#define OPT_OPER_MODPWD     OPT_ADM_OPER+3       //�޸�����
#define OPT_OPER_CANCEL     OPT_ADM_OPER+4       //��Աע��
#define OPT_OPER_CLRPWD     OPT_ADM_OPER+5       //�������
#define OPT_OPER_RESET      OPT_ADM_OPER+6       //ǿ��ǩ��
#define OPT_OPER_AUTHORIZE  OPT_ADM_OPER+7       //��Ա��Ȩ
#define OPT_OPER_GETRIGHTS  OPT_ADM_OPER+8       //��ȡ��ԱȨ��

#define OPT_ADM_SYS         600     //ϵͳ������

#define OPT_SYS_DOWNFILE    OPT_ADM_SYS+1        //��������
#define OPT_SYS_QRYARG      OPT_ADM_SYS+2        //ƽ̨������ѯ
#define OPT_SYS_XYHREG      OPT_ADM_SYS+3        //Э���ע��
#define OPT_SYS_XYHCANCEL   OPT_ADM_SYS+4        //Э���ע��
#define OPT_SYS_XYHQRY      OPT_ADM_SYS+5        //Э��Ų�ѯ

#define OPT_QRYPRT_BASE     700     //��ѯ��ӡ��

#define OPT_QRYPRT_NOTES    OPT_QRYPRT_BASE+1    //�������ƾ֤��ѯ
#define OPT_QRYPRT_OPER     OPT_QRYPRT_BASE+2    //��Ա��Ϣ��ѯ

#define OPT_FUND_BASE       900     //�ʽ���

#define OPT_FUND_DOWNDZ     OPT_FUND_BASE+1      //���ض�������

#endif
