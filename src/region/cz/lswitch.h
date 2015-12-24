/*--------------------------------------*/
/*      常州人民银行交换接口    */
/*--------------------------------------*/

#ifndef LSWITCH_H_
#define LSWITCH_H_

/*提入提出数据接口*/
typedef struct 
{
    char    bzlx[2];        /*币种类型*/
    char    yw[2];          /*业务类型*/
    char    pzlx[3];        /*业务种类*/
    char    tczh[37];       /*提出帐号*/
    char    tchh[5];        /*提出行号*/
    char    tkmc[37];       /*提出客户名*/
    char    trzh[37];       /*对方帐号*/
    char    trhh[5];        /*对方行号*/
    char    rkmc[37];       /*对方客户名*/
    char    hrhm[25];       /*汇入行号*/
    char    jdbz[2];        /*借贷标志*/
    char    je[13];         /*凭证金额*/
    char    pzbh[11];       /*凭证编号*/
    char    pzsy[29];       /*凭证事由*/
    char    my[13];         /*地方密押*/
    char    mm[17];         /*支付密码*/
    char    qfrq[9];        /*签发日期*/
    char    zje[12];        /*汇票签发金额*/
    char    ye[12];         /*汇票多余金额*/
    char    trplsh[6];      /*凭证序号*/
    char    mmxy[6];        /*支付密码校验值*/
    char    tcplsh[6];      /*网上传输提出序号*/
    char    hchm[25];       /*汇出行号*/
    char    fbhh[13];       /*发报行号*/
    char    sbhh[13];       /*收报行号*/
    char    pztjh[11];      /*凭证提交号*/
}TRTC;

/*对帐数据接口*/
typedef struct
{
    char    bzlx[2];        /*币种类型*/
    char    jhhh[5];        /*交换行号*/
    char    sj[7];          /*对帐产生时间*/
    char    bz[2];          /*分场标志*/
    char    pzbz[2];        /*对帐凭证标志*/
    char    dzpc[3];        /*对帐批次*/
    char    rq[7];          /*日期*/
    char    blsh[6];        /*邮包流水号*/

    char    tjfbs[6];       /*本次清算提出借方笔数*/
    char    tdfbs[6];       /*本次清算提出贷方笔数*/
    char    tjfje[15];      /*本次清算提出借方金额*/
    char    tdfje[15];      /*本次清算提出贷方金额*/

    char    rjfbs[6];       /*本次清算提入借方笔数*/
    char    rdfbs[6];       /*本次清算提入贷方笔数*/
    char    rjfje[15];      /*本次清算提入借方金额*/
    char    rdfje[15];      /*本次清算提入贷方金额*/

    char    ztjfbs[6];      /*本次总提出借方笔数*/
    char    ztdfbs[6];      /*本次总提出贷方笔数*/
    char    ztjfje[15];     /*本次总提出借方金额*/
    char    ztdfje[15];     /*本次总提出贷方金额*/

    char    zrjfbs[6];      /*本次总提入借方笔数*/
    char    zrdfbs[6];      /*本次总提入贷方笔数*/
    char    zrjfje[15];     /*本次总提入借方金额*/
    char    zrdfje[15];     /*本次总提入贷方金额*/
}DZSJ;

/*清算明细接口*/
typedef struct
{
    char    bzlx[2];        /*币种类型*/
    char    sj[7];          /*清算时间*/
    char    qshh[5];        /*清算行号*/
    char    ejqshh[5];      /*二级清算单位行号*/
    char    jhhh[5];        /*清算单位*/
    char    rq[7];          /*清算日期*/
    char    ysje[17];       /*应收金额*/
    char    yfje[17];       /*应付金额*/
}QSMX;

/*提入借方凭证回执*/
typedef struct
{
    char    trhh[5];        /*提入行号*/
    char    trplsh[6];      /*提入凭证流水号*/
    char    jdbz[2];        /*借贷标志*/
    char    hzdm[3];        /*回执代码*/
    char    mmxy[6];        /*密码校验值*/
}TRHZ;

/*提出凭证回执*/
typedef struct
{
    char    tchh[5];        /*提出行号*/
    char    fslsh[6];       /*原提出序号*/
    char    jdbz[2];        /*借贷标志*/
    char    hzdm[3];        /*回执代码*/
    char    mmxy[6];        /*密码校验值*/
    char    tcplsh[6];      /*网上提出序号*/
}TCHZ;

/*提出提入帐号接口*/
typedef struct
{
    char    zhh[37];        /*提出帐号*/
    char    khmc[37];       /*提出帐号名称*/
    char    zhtjdm[7];      /*帐号统计码*/
    char    czbz[2];        /*操作码*/
    char    zhzt[2];        /*状态码*/
    char    jhhh[5];        /*交换行号*/
    char    sfbz[2];        /*收费标志*/
}TRTCZH;

/*提出无效凭证输出*/
typedef struct
{
    char    tchh[5];        /*提出行号*/
    char    fslsh[6];       /*原提出序号*/
    char    hzdm[3];        /*无效原因代码*/
}TCWX;

/*定期借记合同接口 add by zxg 2006-05-04*/
typedef struct
{
    char    dwdm[6];        /*单位代码*/
    char    bz[2];          /*维护类型：0－新增或更新；1－注销*/
    char    hth[45];        /*合同号*/
    char    tchh[5];        /*收款行行号*/
    char    tczh[37];       /*收款帐号*/
    char    tckh[61];       /*提出客户*/
    char    trhh[5];        /*付款行行号*/
    char    trzh[37];       /*付款人帐号*/
    char    trkh[61];       /*付款人名称*/
    char    lsh[6];         /*流水号*/
}TCHT;

#endif
