#ifndef COMM_H_
#define COMM_H_

#include "tcpapi.h"

#include "pub.h"
#include "interface.h"

//��ĳ�ڵ�ͨѶ
int CommToNode(int nodeid, int trncode, char *reqbuf, char *reqfile, 
        char *rspbuf, char *rspfile, int *plen);

//���Ӧǰ��ͨѶ
#define CommToPH(trncode, reqbuf, reqfile, rspbuf, rspfile, plen) \
    CommToNode(OP_PHNODEID, trncode, reqbuf, reqfile, rspbuf, rspfile, plen)

#define CommToPHNoFile(trncode, reqbuf, rspbuf, plen) \
    CommToNode(OP_PHNODEID, trncode, reqbuf, NULL, rspbuf, NULL, plen)

//ת����ǰ��
#define TransferToPH(reqbuf, reqfile, rspbuf, rspfile, plen) \
    CommToNode(OP_PHNODEID, (OP_COMCODE ? OP_COMCODE : OP_TCTCODE), reqbuf, reqfile, rspbuf, rspfile, plen)

xmlDoc *CommDocToPH(int *ret, int trncode, xmlDoc *reqdoc, char *pfile);

#define CommDocToPHNoFile(pret, trncode, reqdoc) CommDocToPH(pret, trncode, reqdoc, NULL)

#define BLKSIZE  4096 

/**
 * ������ͨѶԭʼ����
 *
 * preq: ͨѶ�������ݽṹ(�������ͷΪ0��ֻ����������)
 * prsp: ͨѶӦ�����ݽṹ(�������ͷ���ȴ���0���Ƚ�������ͷ����,
 *       ��funcΪNULL���Զ�����Ӧ���峤�ȵ�Ӧ������,����ִ��func��������ֵ��Ϊ����Ӧ����ĳ���)
 * func: ���ؽ���Ӧ�������ݵĳ���(��prsp��Ӧ���峤��>0����ΪNULL,��ΪNULL����func�ķ��ؽ����Ϊ���յĳ���)
 *
 * ע: ��prsp��δָ��Ӧ���峤�Ȳ���funcΪNULL���ʾ�����ܽ�������,���ֵΪBLKSIZE
 */
int CommToBankNative(char *addr, int port, BankCommData *preq, 
        BankCommData *prsp, FPGetRecvBodyLength func, int timeout);

#endif
