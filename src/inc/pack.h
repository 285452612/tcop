#ifndef PACK_H_
#define PACK_H_

#include "util.h"
#include "element.h"

//��������
#define PACKTYPE_XML                'X'     //XML��������
#define PACKTYPE_STRUCT             'S'     //�ṹ(����)��������
#define PACKTYPE_TCOP               'P'     //ƽ̨��������(����ת��)

#define PACKMAPNODE_SRC             'S'     //��ӳ��ڵ���ΪԴ�����еĽڵ���
#define PACKMAPNODE_DST             'D'     //��ӳ��ڵ���ΪĿ�걨���еĽڵ���

#define PACK_REQ2OP                 'A'     //��OP��������(����������ǰ�õ�����)ת����OP���XML����
#define PACK_BANKREQ                'B'     //���нӿڽ��׵�����XML����
#define PACK_BANKRSP                'C'     //���нӿڽ��׵���ӦXML����
#define PACK_OP2RSP                 'D'     //OPӦ��ʱ����ӦXML����
#define PACK_REQSTRUCTXML           'E'     //����ṹ���Ķ�Ӧ��XML����
#define PACK_RSPSTRUCTXML           'F'     //��Ӧ�ṹ���Ķ�Ӧ��XML����
#define PACK_OPBODY                 'G'     //OP������
#define PACK_TCPACK                 'H'     //ͬ�ǽ��׵�ԭʼ����
#define PACK_RSP2OP                 'I'     //ͬ��Ӧ���Ķ�Ӧ��OP����
#define PACK_BANKXMLRSP             'J'     //���нӿڽ��׵���ӦXML����

#define PACK_MAPOP2RSP              'L'     //OP���ĵ�ͬ�ǵ�Ӧ����ӳ��
#define PACK_MAPOP2BANK             'M'     //OP���ĵ����еĽӿ�������ӳ��
#define PACK_MAPBANK2OP             'N'     //���еĽӿ�Ӧ���ĵ�OP���ĵ�ӳ��
#define PACK_MAPTCXML2OP            'O'     //ͬ��XML���ĵ�OP���ĵ�ӳ��
#define PACK_MAPOP2TCXML            'P'     //OP���ĵ�ͬ��XML������ӳ��

extern char OP_TCPACKTYPE;                  //ͬ�Ǳ�������
extern char OP_PHPACKTYPE;                  //ǰ�ñ�������
extern char OP_BKPACKTYPE;                  //���б�������

#define GetOPVal(name)  XMLGetNodeVal(doc, "//"#name)

xmlDoc *getOPTemplateDoc(int optcode);

#define getOPDoc()  getOPTemplateDoc(OP_OPTCODE)

xmlDoc *getTemplateDocBase(char packattr, int bktcode, int optcode, int tctcode, char *xpath);

#define getTemplateDoc(packattr, bktcode, xpath)  getTemplateDocBase(packattr, bktcode, OP_OPTCODE, OP_TCTCODE, xpath)

#define getTCTemplateDoc(tctcode, xpath) getTemplateDocBase(PACK_TCPACK, 0, OP_OPTCODE, tctcode, xpath)

/*
 * ת��������(ͬ����������뽻��)��OP���屨��
 *
 * ����: �ɹ� OP���� ʧ�� NULL
 */
xmlDoc *ConvertREQ2OP(char *reqbuf, char *reqfile, int *plen);

xmlDoc *ConvertREQ2TCXML(char *reqbuf, char *reqfile, int *plen);

char *ConvertOP2REQ(xmlDoc *opDoc, char *reqbuf, char *reqfile, int *plen);

/*
 * ת��OP���ĵ�����Ӧ����
 *
 * reqbuf: Ϊ�ṹͨѶ����Ԥ��(���������ȴ��������ݿ���)
 *
 * ���� Ӧ���ĳ��� ʧ�� <=0
 */
int ConvertOP2RSP(xmlDoc *opDoc, char *reqbuf, int reqlen, char *rspbuf, char *rspfile);

/*
 * ת��ǰ��Ӧ����(ͬ�������Ӧ)��OP���屨��
 *
 * ����: �ɹ� OP���� ʧ�� NULL
 */
xmlDoc *ConvertPHRsp2OP(xmlDoc *opDoc, char *rspbuf, char *rspfile);

/*
 * ת��OP���ĵ����нӿ�ͨѶ������
 *
 * ����: �ɹ� ͨѶ�����ĳ��� ʧ�� <0
 */
int ConvertOP2BANK(char *buf, int bktcode, xmlDoc *opDoc);

/*
 * ת�����нӿ�ͨѶӦ���ĵ�OP����
 *
 * responseFlag: ��Ӧ��ֶ������ʱ��Ҫ����Ӧ�������־
 *
 * ����: �ɹ� OP���� ʧ�� NULL
 */
xmlDoc *ConvertBANK2OP(xmlDoc *opDoc, int bktcode, char *buf, int responseFlag);

/*
 * ת��ͬ�Ǳ���(�����������,�����Ӧ)��OP���屨��
 *
 * tcbuf: ͬ�������Ӧ����
 * tcfile: ͬ�������Ӧ���ļ���
 * mapNSXPath: ͬ��XML������OP���ĵ�ת��ӳ���ϵ���ϵ�XPATH
 *
 * ����: �ɹ� OP���� ʧ�� NULL
 */
xmlDoc *ConvertTCPack2OP(xmlDoc *opDoc, char *tcbuf, int *plen, char *tcfile, char *mapNSXPath);

/*
 * ת��ͬ��XML����(�������������Ӧ)��OP���屨��
 *
 * mapNSXPath: ͬ�Ǳ�����OP����ת����ӳ���ϵ��XPATH
 *
 * ����: �ɹ� opDoc ʧ�� NULL
 */
xmlDoc *ConvertTCXML2OP(xmlDoc *opDoc, xmlDoc *tcDoc, char *mapNSXPath);

/*
 * ת��XML���ĵ�XML����
 *
 * dstDoc: Ŀ��XML����
 * srcDoc: ԴXML����
 * mapNSPtr: ת������ӳ��ڵ㼯
 * mapWay: PACKMAPNODE_SRC �� PACKMAPNODE_DST
 * ppath: ����ڵ�����Ŀ�걨�Ļ�Դ�����е�λ�õĸ�·��
 * fp: ת��ʱ�Խڵ�ֵ���д���ĺ���
 * 
 * ����: �ɹ� dstDoc ʧ�� NULL
 */
xmlDoc *ConvertXML2XML(xmlDoc *dstDoc, xmlDoc *srcDoc, xmlNodeSetPtr mapNSPtr, 
        char mapWay, char *ppath, FPProcessNodeValue fp);

/*
 * ת��OP���ĵ��ӿ�XML����
 *
 * ����: �ɹ� bkDoc ʧ�� NULL
 */
xmlDoc *ConvertOP2BankXML(int bktcode, xmlDoc *bkDoc, xmlDoc *opDoc);

/*
 * ת���ӿ�XML���ĵ�OP����
 *
 * ����: �ɹ� opDoc ʧ�� NULL
 */
xmlDoc *ConvertBankXML2OP(xmlDoc *opDoc, int bktcode, xmlDoc *bkDoc);

/*
 * ת��OP���ĵ�ͬ��Ӧ��XML����
 *
 * ����: �ɹ� tcDoc ʧ�� NULL
 */
xmlDoc *ConvertOP2TcRspXML(xmlDoc *tcDoc, xmlDoc *opDoc);

/*
 * ת��ͬ��Ӧ��XML���ĵ��ṹ����
 *
 * ����: �ɹ� �ṹ���ĳ��� ʧ�� <0
 */
int ConvertTcRspXML2Struct(char *buf, char *tcDoc);

/*
 * ת���ṹ�嵽XML�ڵ㼯
 *
 * nsPtr Ŀ��XML�ڵ㼯
 * buf �ṹ�����
 *
 * �ɹ�: 0 ʧ��: ������
 */
int ConvertStruct2Nodes(xmlNodeSetPtr nsPtr, const char *buf);

/*
 * ת��XML�ڵ㼯���ṹ��
 *
 * buf Ŀ��ṹ�����
 * nsPtr ԴXML�ڵ㼯
 *
 * �ɹ�:�ṹ�峤�� ʧ��: <0
 */ 
int ConvertNodes2Struct(char *buf, xmlNodeSetPtr nsPtr);

int ConvertOP2TCStruct(char *buf, int *plen, int tctcode, xmlDoc *opDoc);

/*
 * ���汨��
 */
void SavePack(xmlDoc *doc, char savepackType, int bktcode);


#endif
