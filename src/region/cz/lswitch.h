/*--------------------------------------*/
/*      �����������н����ӿ�    */
/*--------------------------------------*/

#ifndef LSWITCH_H_
#define LSWITCH_H_

/*����������ݽӿ�*/
typedef struct 
{
    char    bzlx[2];        /*��������*/
    char    yw[2];          /*ҵ������*/
    char    pzlx[3];        /*ҵ������*/
    char    tczh[37];       /*����ʺ�*/
    char    tchh[5];        /*����к�*/
    char    tkmc[37];       /*����ͻ���*/
    char    trzh[37];       /*�Է��ʺ�*/
    char    trhh[5];        /*�Է��к�*/
    char    rkmc[37];       /*�Է��ͻ���*/
    char    hrhm[25];       /*�����к�*/
    char    jdbz[2];        /*�����־*/
    char    je[13];         /*ƾ֤���*/
    char    pzbh[11];       /*ƾ֤���*/
    char    pzsy[29];       /*ƾ֤����*/
    char    my[13];         /*�ط���Ѻ*/
    char    mm[17];         /*֧������*/
    char    qfrq[9];        /*ǩ������*/
    char    zje[12];        /*��Ʊǩ�����*/
    char    ye[12];         /*��Ʊ������*/
    char    trplsh[6];      /*ƾ֤���*/
    char    mmxy[6];        /*֧������У��ֵ*/
    char    tcplsh[6];      /*���ϴ���������*/
    char    hchm[25];       /*����к�*/
    char    fbhh[13];       /*�����к�*/
    char    sbhh[13];       /*�ձ��к�*/
    char    pztjh[11];      /*ƾ֤�ύ��*/
}TRTC;

/*�������ݽӿ�*/
typedef struct
{
    char    bzlx[2];        /*��������*/
    char    jhhh[5];        /*�����к�*/
    char    sj[7];          /*���ʲ���ʱ��*/
    char    bz[2];          /*�ֳ���־*/
    char    pzbz[2];        /*����ƾ֤��־*/
    char    dzpc[3];        /*��������*/
    char    rq[7];          /*����*/
    char    blsh[6];        /*�ʰ���ˮ��*/

    char    tjfbs[6];       /*������������跽����*/
    char    tdfbs[6];       /*�������������������*/
    char    tjfje[15];      /*������������跽���*/
    char    tdfje[15];      /*������������������*/

    char    rjfbs[6];       /*������������跽����*/
    char    rdfbs[6];       /*�������������������*/
    char    rjfje[15];      /*������������跽���*/
    char    rdfje[15];      /*������������������*/

    char    ztjfbs[6];      /*����������跽����*/
    char    ztdfbs[6];      /*�����������������*/
    char    ztjfje[15];     /*����������跽���*/
    char    ztdfje[15];     /*����������������*/

    char    zrjfbs[6];      /*����������跽����*/
    char    zrdfbs[6];      /*�����������������*/
    char    zrjfje[15];     /*����������跽���*/
    char    zrdfje[15];     /*����������������*/
}DZSJ;

/*������ϸ�ӿ�*/
typedef struct
{
    char    bzlx[2];        /*��������*/
    char    sj[7];          /*����ʱ��*/
    char    qshh[5];        /*�����к�*/
    char    ejqshh[5];      /*�������㵥λ�к�*/
    char    jhhh[5];        /*���㵥λ*/
    char    rq[7];          /*��������*/
    char    ysje[17];       /*Ӧ�ս��*/
    char    yfje[17];       /*Ӧ�����*/
}QSMX;

/*����跽ƾ֤��ִ*/
typedef struct
{
    char    trhh[5];        /*�����к�*/
    char    trplsh[6];      /*����ƾ֤��ˮ��*/
    char    jdbz[2];        /*�����־*/
    char    hzdm[3];        /*��ִ����*/
    char    mmxy[6];        /*����У��ֵ*/
}TRHZ;

/*���ƾ֤��ִ*/
typedef struct
{
    char    tchh[5];        /*����к�*/
    char    fslsh[6];       /*ԭ������*/
    char    jdbz[2];        /*�����־*/
    char    hzdm[3];        /*��ִ����*/
    char    mmxy[6];        /*����У��ֵ*/
    char    tcplsh[6];      /*����������*/
}TCHZ;

/*��������ʺŽӿ�*/
typedef struct
{
    char    zhh[37];        /*����ʺ�*/
    char    khmc[37];       /*����ʺ�����*/
    char    zhtjdm[7];      /*�ʺ�ͳ����*/
    char    czbz[2];        /*������*/
    char    zhzt[2];        /*״̬��*/
    char    jhhh[5];        /*�����к�*/
    char    sfbz[2];        /*�շѱ�־*/
}TRTCZH;

/*�����Чƾ֤���*/
typedef struct
{
    char    tchh[5];        /*����к�*/
    char    fslsh[6];       /*ԭ������*/
    char    hzdm[3];        /*��Чԭ�����*/
}TCWX;

/*���ڽ�Ǻ�ͬ�ӿ� add by zxg 2006-05-04*/
typedef struct
{
    char    dwdm[6];        /*��λ����*/
    char    bz[2];          /*ά�����ͣ�0����������£�1��ע��*/
    char    hth[45];        /*��ͬ��*/
    char    tchh[5];        /*�տ����к�*/
    char    tczh[37];       /*�տ��ʺ�*/
    char    tckh[61];       /*����ͻ�*/
    char    trhh[5];        /*�������к�*/
    char    trzh[37];       /*�������ʺ�*/
    char    trkh[61];       /*����������*/
    char    lsh[6];         /*��ˮ��*/
}TCHT;

#endif
