#ifndef COMM_H_
#define COMM_H_

#include "tcpapi.h"

#include "pub.h"
#include "interface.h"

//与某节点通讯
int CommToNode(int nodeid, int trncode, char *reqbuf, char *reqfile, 
        char *rspbuf, char *rspfile, int *plen);

//与对应前置通讯
#define CommToPH(trncode, reqbuf, reqfile, rspbuf, rspfile, plen) \
    CommToNode(OP_PHNODEID, trncode, reqbuf, reqfile, rspbuf, rspfile, plen)

#define CommToPHNoFile(trncode, reqbuf, rspbuf, plen) \
    CommToNode(OP_PHNODEID, trncode, reqbuf, NULL, rspbuf, NULL, plen)

//转发至前置
#define TransferToPH(reqbuf, reqfile, rspbuf, rspfile, plen) \
    CommToNode(OP_PHNODEID, (OP_COMCODE ? OP_COMCODE : OP_TCTCODE), reqbuf, reqfile, rspbuf, rspfile, plen)

xmlDoc *CommDocToPH(int *ret, int trncode, xmlDoc *reqdoc, char *pfile);

#define CommDocToPHNoFile(pret, trncode, reqdoc) CommDocToPH(pret, trncode, reqdoc, NULL)

#define BLKSIZE  4096 

/**
 * 与银行通讯原始函数
 *
 * preq: 通讯请求数据结构(如果请求头为0则只发送请求体)
 * prsp: 通讯应答数据结构(如果请求头长度大于0则先接收请求头数据,
 *       若func为NULL则自动接收应答体长度的应答数据,否则执行func并将返回值作为接收应答体的长度)
 * func: 返回接收应答体数据的长度(若prsp中应答体长度>0可以为NULL,不为NULL则以func的返回结果作为接收的长度)
 *
 * 注: 若prsp中未指定应答体长度并且func为NULL则表示尽可能接收数据,最大值为BLKSIZE
 */
int CommToBankNative(char *addr, int port, BankCommData *preq, 
        BankCommData *prsp, FPGetRecvBodyLength func, int timeout);

#endif
