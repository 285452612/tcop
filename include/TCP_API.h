/* H_GenerateCode
 * �ϼ��м�Ѻ
 * ����˵��:
 *  �������:
 *      bankno:Ŀ���¼�����Ѻ�����
 *      data:����Ѻ���ݴ�
 *      len:����Ѻ���ݳ���
 *  ���ز���:
 *      mac:���ؼ�Ѻ���
 * ������: 0 �ɹ����
 */
int H_GenerateCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode
 * �ϼ��к�Ѻ (ʹ�õ�ǰ��Կ)
 * ����˵��:
 *  �������:
 *      bankno:Դ�¼�����Ѻ�����
 *      data:����Ѻ���ݴ�
 *      len:����Ѻ���ݳ���
 *      mac:������Ѻ
 * ������: 0 �ɹ����
 */
int H_CheckCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode_c
 * �ϼ��к�Ѻ (ʹ��������Ч��Կ)
 * ����˵��:
 *  �������:
 *      bankno:Դ�¼�����Ѻ�����
 *      data:����Ѻ���ݴ�
 *      len:����Ѻ���ݳ���
 *      mac:������Ѻ
 * ������: 0 �ɹ����
 */
int H_CheckCode_c(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* GenerateCode
 * �¼��м�Ѻ
 * ����˵��:
 *  �������:
 *      data:����Ѻ���ݴ�
 *      len:����Ѻ���ݳ���
 *  ���ز���:
 *      mac:���ؼ�Ѻ���
 * ������: 0 �ɹ����
 */
int GenerateCode(unsigned char *data,int len,unsigned char mac[17]);

/* CheckCode
 * �¼��к�Ѻ
 * ����˵��:
 *  �������:
 *      data:����Ѻ���ݴ�
 *      len:����Ѻ���ݳ���
 *      mac:��������Ѻ
 * ������: 0 �ɹ����
 */
int CheckCode(unsigned char *data,int len,unsigned char mac[17]);

/* Data_Encrypt
 * ���ݼ���
 * ����˵��:
 *  �������:
 *      workdate:��������YYYYMMDD
 *      refid:�����ˮ
 *      data:���������ݴ�
 *      len:���������ݳ���
 *  ���ز���:
 *      enc_data:�������ݴ�
 *      enc_len:�������ݴ�����
 * ������: 0 �ɹ����
 */
int Data_Encrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len);

/* ConvEncData
 * ����ת����
 * ����˵��:
 *  �������:
 *      bankno_1:Դ������Ѻ���豸��
 *      bankno_2:Ŀ���н�����Ѻ���豸��
 *      data:�������ݴ�
 *      len:���ĳ���
 * ������: 0 �ɹ����
 */
int ConvEncData(unsigned char bankno_1[8],unsigned char bankno_2[8],unsigned char *data,int len);

/* Data_Decrypt
 * ���ݽ���
 * ����˵��:
 *  �������:
 *      workdate:��������YYYYMMDD
 *      refid:�����ˮ
 *      data:���������ݴ�
 *      len:���������ݳ���
 *  ���ز���:
 *      dec_data:�������ݴ�
 *      dec_len:�������ݴ�����
 * ������: 0 �ɹ����
 */
int Data_Decrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len);

#define MAX_BUFF 4096

//��Ѻ���ô���
#define ERR_SOCKET			-1 //������Ѻ��ʧ��
#define ERR_RECV			-2 //������Ѻ������ʧ��
#define ERR_SEND			-3 //��������ʧ��
//��Ѻ�����ش�����˵��
#define ERR_CARD_SELFTEST   0x01 //���Լ�Ӳ������
#define ERR_NOLMK 			0x02 //����LMK������Ч
#define ERR_FPGA_SELFTEST   0x03 //FPGA�Լ����
#define ERR_TIMECHIP 		0x04 //ʱ��оƬ�쳣
#define ERR_CARDCMD 		0x10 //����������
#define ERR_CARDDATA_LEN 	0x11 //���ݳ��ȴ���
#define ERR_CARDDATA_CRC 	0x12 //���ݰ�CRCУ�����
#define ERR_OPER_ID 		0x20 //����ԱID�Ŵ���
#define ERR_OPER_PWD 		0x21 //����Ա�������
#define ERR_OPER_CHK0 		0x22 //����Ա�����ݴ����Ϊ0
#define ERR_NOAUTH 			0x23 //�Ƕ��˵�¼������Ȩ״̬��
#define ERR_NOLOGIN 		0x24 //�޹���Ա��¼��δ��¼״̬��
#define ERR_ALREADY_LOGIN   0x25 //����Ա�Ѿ���¼
#define ERR_AIRTHMETIC 		0x30 //ָ�����㷨��Ч
#define ERR_DATALEN_8 		0x31 //���ݳ��ȴ��󣬲��ܱ�8����
#define ERR_DATALEN_16 		0x32 //���ݳ��ȴ��󣬲��ܱ�16����
#define ERR_DATALEN_128 	0x33 //���ݳ��ȴ��󣬲��ܱ�128����
#define ERR_DATALEN_256 	0x34 //���ݳ��ȴ��󣬲��ܱ�256����
#define ERR_AUTH_LMK 		0x35 //ָ��������LMK����ֹ
#define ERR_FLASH_ADD 		0x40 //����FLASH��д��ַԽ��
#define ERR_FLASH_WRITE 	0x41 //д����FLASH�쳣
#define ERR_RANDCHIP 		0x42 //�����оƬ�����쳣
#define ERR_CENCCHIP 		0x43 //�����㷨оƬ�����쳣
#define ERR_ADMINNO 		0x51 //����Ա�Ŵ���	
#define ERR_ADMINAGIN 	    0x52 //����Ա�ѵ�¼	
#define ERR_ADMINPWD 		0x53 //����Ա�������	
#define ERR_CHECKTAG 		0x54 //TagУ�����	
#define ERR_CHECKDAC 		0x55 //DACУ�����	
#define ERR_KEYFLAG 		0x56 //��Կ��־����	
#define ERR_BANKNO 			0x57 //�����кŴ���	
#define ERR_DATALEN 		0x58 //�������ݳ��ȴ���	
#define ERR_LOGINTWO 		0x59 //��¼�Ĺ���Ա������������	
#define ERR_SELFTEST   	    0x5A //�Լ�ʧ��
#define ERR_ENCODE 			0x5B //��Ѻ����	
#define ERR_LOGINTHREE 	    0x5C //��¼�Ĺ���Ա������������	
#define ERR_CHECKLMK 		0x5D //ϵͳ����Կ�������	
#define ERR_COMMAND 		0x5E //δ֪����
#define ERR_KEYAGIN 		0x5F //��Կ������
#define ERR_COM10           0x60 //����ҵ����Ѻ��ͨѶ���ݰ����ȴ���
#define ERR_COM11           0x61 //����ҵ����Ѻ��ͨѶ��ʱ
#define ERR_COM12           0x62 //����ҵ����Ѻ��ͨѶ����У�����
#define ERR_COM13           0x63 //����ҵ����Ѻ��ͨѶ���ִ���
#define ERR_COM14           0x64 //����ҵ����Ѻ��ͨѶδ֪����
#define ERR_FLASH           0x65 //��ҵ����Ѻ��дFlash ����
#define ERR_LENGTH          0x66 //���ݰ����ȴ���
#define ERR_DATAFORMAT      0x67 //���ݰ���ʽ����
#define ERR_DATACRC     	0x68 //���ݰ�CRC����
#define ERR_BANKNO_ISEMPTY 	0x69 //�к�Ϊ��
#define ERR_CARDTIMEOUT		0x6A //��Ѻ������ʱ	
#define ERR_CARDWRITE		0x6B //д��Ѻ������	
#define ERR_CARDREAD    	0x6C //����Ѻ������
#define ERR_CARDBANKNO		0x6D //�к�����
#define ERR_PINFORMAT 		0x80 //PIN��ʽ����
#define ERR_CHKVAL          0x81 //CHKVAL����
#define ERR_BANK_NO_INDEX   0x82 //��ȡBANKNOʱ��Ŵ������
