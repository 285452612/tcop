#ifndef _REMOTE_ST_H
#define _REMOTE_ST_H

// ��������
typedef struct _data_remote_in_0002
{
    char           JIAOYM[4];     // ���״���    
    char           BWAIBBZ[1];    // ����ұ�־
    char           BIZHO[3];      // ����   
    char           TCHULS[10];    // �����ˮ��
    char           TCHUHH[6];     // ����к�
    char           TRUHHH[6];     // �����к�   
    char           PIOJUZL[2];    // Ʊ������
    char           PZHHAO[20];    // ƾ֤��
    char           QIANFRQ[10];   // ǩ������
    char           SHKRZHH[32];   // �տ����˺�
    char           SHKRHUM[80];   // �տ��˻���
    char           SHKRKHHH[6];   // �տ��˿����к�
    char           FUKRZHH[32];   // �������˺�
    char           FUKRHUM[80];   // �����˻���
    char           FUKRKHHH[6];    // �����˿����к�
    char           DUIFHHH[6];     // �Է����к�
    char           DUIFHHM[60];    // �Է�������
    char           JINE[15];   // ���
    char           XIANE[15];   // �޶�
    char           ZHFMM[20];   // ֧������
    char           TCHENGMM[16];   // ͬ����Ѻ
    char           SHIYYT[60];   // ������;
    char           BEIYONG[200];   // ����
    char           WLIP[15];   // ����ip��ַ
    char           ZHJIP[15];   // ����ip��ַ
    char           ZHONDHAO[10];   // �ն˺�
    char           TCHHCZYH[4];   // ����в���Ա��
    char           CHULJG[2];   // ������
    char           TOUZHJGBZ[1];   // ͸֧�����־
    char           TITBZ[20];   // ������־
} data_remote_in_0002;

// ��������
typedef struct _data_remote_in_0043
{
    char           JIAOYM[4];     // ���״���    
    char           OPENBANK[6];   // �����к�
    char           EXCHANGENO[6]; // ������
    char           ACCTNO[32];    // �˺�
    char           ACCTNAME[80];  // ����
    char           FILENAME[64];  // �ļ���
    char           STARTDATE[10]; // ��ʼ����
    char           FINALDATE[10]; // ��������
    char           FLAG[1];       // ��־
    char           NETIP[15];     // ����IP
    char           HOSTIP[15];    // ����IP
    char           TERMID[10];    // �ն˺�
    char           OPERNO[4];     // ����Ա��
    char           RESULT[2];     // ������
} data_remote_in_0043;
#endif
