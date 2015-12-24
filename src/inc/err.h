#ifndef ERR_H_
#define ERR_H_

#define ERR_BASE                0
#define ERR_SUCC                0
#define ERR_SUCCBASE            90

#define isSuccess(ret)          ((ret) == ERR_SUCC || ((ret) % 100 / ERR_SUCCBASE == 1))

#define E_OTHER                 999                         /* �������� */

#define ERR_APP                 ERR_BASE+100    //Ӧ�ô���

#define E_APP_ACCOUNTSUCC       ERR_APP+ERR_SUCCBASE+1      /* ���˳ɹ� */
#define E_APP_ACCOUNTFAIL       ERR_APP+2                   /* ����ʧ�� */
#define E_APP_CZSUCC            ERR_APP+3                   /* �����ɹ� */
#define E_APP_CZFAIL            ERR_APP+4                   /* ����ʧ�� */
#define E_APP_CANCELSUCC        ERR_APP+5                   /* ȡ���ɹ� */
#define E_APP_CANCELFAIL        ERR_APP+6                   /* ȡ��ʧ�� */
#define E_APP_ACCOUNTNOCZ       ERR_APP+7                   /* ���˳ɹ�����ʧ�� */
#define E_APP_ACCOUNTANDCZ      ERR_APP+8                   /* ���˳ɹ������ɹ� */
#define E_APP_ACCOUNTNOCANCEL   ERR_APP+9                   /* ���˳ɹ�ȡ��ʧ�� */
#define E_APP_ACCOUNTANDCANCEL  ERR_APP+10                  /* ���˳ɹ�ȡ���ɹ� */
#define E_APP_NEEDGRANT         ERR_APP+11                  /* ��Ҫ��Ȩ */
#define E_APP_GRANTSUCC         ERR_APP+ERR_SUCCBASE+2      /* ��Ȩ�ɹ� */
#define E_APP_GRANTFAIL         ERR_APP+13                  /* ��Ȩʧ�� */
#define E_APP_NONEEDACCOUNT     ERR_APP+14                  /* ����ʧ�� */

#define ERR_DB                  ERR_BASE+200    //���ݿ����

#define E_DB                    ERR_DB+1                    /* ���ݿ������ */
#define E_DB_URLCFG             ERR_DB+2                    /* ���ݿ�URL���ô� */
#define E_DB_OPEN               ERR_DB+3                    /* ���ݿ�򿪴� */
#define E_DB_SELECT             ERR_DB+4                    /* ���ݿ�select�� */
#define E_DB_INSERT             ERR_DB+5                    /* ���ݿ�Insert�� */
#define E_DB_UPDATE             ERR_DB+6                    /* ���ݿ�update�� */
#define E_DB_DELETE             ERR_DB+7                    /* ���ݿ�delete�� */
#define E_DB_NORECORD           ERR_DB+8                    /* û�м�¼ */
#define E_DB_NOTSUPPORT         ERR_DB+9                    /* ���ݿ������֧�� */

#define ERR_SYS                 ERR_BASE+300    //ϵͳ����

#define E_SYS_COMM              ERR_SYS+1                   /* ͨѶ���� */
#define E_SYS_COMM_CFG          ERR_SYS+2                   /* ͨѶ���ô� */
#define E_SYS_COMM_PH           ERR_SYS+3                   /* ��ǰ��ͨ��ʧ�� */
#define E_SYS_COMM_BANK         ERR_SYS+4                   /* ������ͨ��ʧ�� */
#define E_SYS_CALL              ERR_SYS+5                   /* ϵͳ���ô� */
#define E_SYS_NODLLFUNC         ERR_SYS+6                   /* ��̬����δ�ҵ���Ӧ�ĺ��� */
#define E_SYS_ADDDIGEST         ERR_SYS+7                   /* ��Ѻ�� */
#define E_SYS_CHKDIGEST         ERR_SYS+8                   /* ��Ѻ�� */
#define E_SYS_SJLENCRYPT        ERR_SYS+9                   /* �������ܴ� */
#define E_SYS_SJLDECRYPT        ERR_SYS+10                  /* �������ܴ� */
#define E_SYS_SYDENCRYPT        ERR_SYS+11                  /* ��Ѻ�����ܴ� */
#define E_SYS_SYDDECRYPT        ERR_SYS+12                  /* ��Ѻ�����ܴ� */
#define E_SYS_BANKENCRYPT       ERR_SYS+13                  /* ���м��ܴ� */

#define ERR_PACK                ERR_BASE+400    //���Ĵ���

#define E_PACK_INIT             ERR_PACK+1                  /* ���ĳ�ʼ���� */
#define E_PACK_GETVAL           ERR_PACK+2                  /* ����ȡֵ�� */
#define E_PACK_CONVERT          ERR_PACK+3                  /* ����ת���� */
#define E_PACK_TYPE             ERR_PACK+4                  /* �������ʹ� */
#define E_PACK_CFG              ERR_PACK+5                  /* �������ô� */

#define ERR_TRAN                ERR_BASE+500    //���״���

#define E_TRAN_EXISTSUCC        ERR_TRAN+ERR_SUCCBASE+1     /* �����ѳɹ����� */
#define E_TRAN_EXISTFAIL        ERR_TRAN+1                  /* ������ʧ�ܴ��� */
#define E_TRAN_EXISTUNKNOW      ERR_TRAN+2                  /* ����״̬δ֪���� */

#define E_TRAN_QUERYREPLIED     ERR_TRAN+20                 /* ��ѯ���ѻظ�*/

#define ERR_MNG                 ERR_BASE+600    //���������

#define E_MNG_OPER_NOTEXIST     ERR_MNG+1                   /* ��Ա������ */ 
#define E_MNG_OPER_PASSWD       ERR_MNG+2                   /* ��Ա����� */
#define E_MNG_OPER_ONLINE       ERR_MNG+3                   /* ��Ա�ѵ�¼ */
#define E_MNG_OPER_CANCEL       ERR_MNG+4                   /* ��Ա��ע�� */
#define E_MNG_OPER_RIGHTS       ERR_MNG+5                   /* ��ԱȨ�޴� */
#define E_MNG_XY_NOTEXIST       ERR_MNG+6                   /* Э�鲻���� */
#define E_MNG_XY_CANCEL         ERR_MNG+7                   /* Э����ע�� */
#define E_MNG_PAYPWD            ERR_MNG+8                   /* ֧��������� */


#endif
