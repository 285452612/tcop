#ifndef _REMOTE_ST_H
#define _REMOTE_ST_H

// 交易数据
typedef struct _data_remote_in_0002
{
    char           JIAOYM[4];     // 交易代码    
    char           BWAIBBZ[1];    // 本外币标志
    char           BIZHO[3];      // 币种   
    char           TCHULS[10];    // 提出流水号
    char           TCHUHH[6];     // 提出行号
    char           TRUHHH[6];     // 提入行号   
    char           PIOJUZL[2];    // 票据种类
    char           PZHHAO[20];    // 凭证号
    char           QIANFRQ[10];   // 签发日期
    char           SHKRZHH[32];   // 收款人账号
    char           SHKRHUM[80];   // 收款人户名
    char           SHKRKHHH[6];   // 收款人开户行号
    char           FUKRZHH[32];   // 付款人账号
    char           FUKRHUM[80];   // 付款人户名
    char           FUKRKHHH[6];    // 付款人开户行号
    char           DUIFHHH[6];     // 对方行行号
    char           DUIFHHM[60];    // 对方行行名
    char           JINE[15];   // 金额
    char           XIANE[15];   // 限额
    char           ZHFMM[20];   // 支付密码
    char           TCHENGMM[16];   // 同城密押
    char           SHIYYT[60];   // 事由用途
    char           BEIYONG[200];   // 备用
    char           WLIP[15];   // 网络ip地址
    char           ZHJIP[15];   // 主机ip地址
    char           ZHONDHAO[10];   // 终端号
    char           TCHHCZYH[4];   // 提出行操作员号
    char           CHULJG[2];   // 处理结果
    char           TOUZHJGBZ[1];   // 透支警告标志
    char           TITBZ[20];   // 其他标志
} data_remote_in_0002;

// 交易数据
typedef struct _data_remote_in_0043
{
    char           JIAOYM[4];     // 交易代码    
    char           OPENBANK[6];   // 开户行号
    char           EXCHANGENO[6]; // 交换号
    char           ACCTNO[32];    // 账号
    char           ACCTNAME[80];  // 户名
    char           FILENAME[64];  // 文件名
    char           STARTDATE[10]; // 开始日期
    char           FINALDATE[10]; // 结束日期
    char           FLAG[1];       // 标志
    char           NETIP[15];     // 网络IP
    char           HOSTIP[15];    // 主机IP
    char           TERMID[10];    // 终端号
    char           OPERNO[4];     // 操作员号
    char           RESULT[2];     // 处理结果
} data_remote_in_0043;
#endif
