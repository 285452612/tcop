
#pragma pack(1)

// ϵͳ��Ϣͷ�ĸ�ʽ���£���55bytes��  ����ͷ�ܳ�140B 
typedef struct _msg_head_in
{
    unsigned short  SHJBCD;     // 2B ���ݰ�����, ת���������ʽ
    char            BAWMAC[16]; // ����MAC
    char            MACJGH[4];  // MAC������, ��ʾ������MAC�ĶԷ�������
    char            PINZHZ[16]; // PIN����, ��ʾ������PIN������(������������)
    unsigned char   YNDIZH[4];  // Ŀ���ַ, �����Ʒ�ʽ�洢
    unsigned char   MBIODZ[4];  // Դ��ַ, �����Ʒ�ʽ�洢
    unsigned char   BOLIUW;     // ϵͳ����λ, �����Ʒ�ʽ�洢
    unsigned char   XXJSBZ;     // ��Ϣ������־, �����Ʒ�ʽ�洢
    unsigned short  SJBSXH;     // 2B �������, ת���������ʽ
    unsigned char   JIOYBZ;     // У���־, �����Ʒ�ʽ�洢
    int             MIYBBH;     // 4B ��Կ�汾��    
} msg_head_in;

// ���׹���ͷ (21bytes)
typedef struct _pub_head_in
{
    char            ZHNGDH[5]; // �ն˺�, �ַ�����ʽ�洢
    char            CHSHDM[4]; // ���д���
    char            YNGYJG[4]; // �������� 
    char            JIO1GY[8]; // ���׹�Ա
} pub_head_in;

// ��������ͷ (64bytes)
typedef struct _data_head_in
{
      char           JIAOYM[4]; // ���״���    
      char           JIOYZM[2]; // ��������    
      char           JIOYMS[1]; // ����ģʽ    
      int            JIOYXH;    // 4B �������    
      unsigned short COMMLN;    // 2B �����װ�����
      unsigned short PNYIL1;    // 2B 0xFFFFΪ��Ч ϵͳƫ��1   
      unsigned short PNYIL2;    // 2B 0xFFFFΪ��Ч ϵͳƫ��2   
      char           QANTLS[12];// ǰ̨��ˮ��  
      char           QANTRQ[8]; // ǰ̨����    
      char           SHOQGY[8]; // ��Ȩ��Ա����
      char           SHOQMM[16];// ��Ȩ���롡��
      char           YWKABZ;    // ��Ȩ��Ա���޿���־  
      char           CZYNXH[2]; // ��Ȩ��Ա�����      
} data_head_in;

