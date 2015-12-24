#ifndef NT_CONST_H_
#define NT_CONST_H_
/************************南通票据种类*********************/
#define NOTE_CHECK          2       /* 转账支票 */
#define NOTE_REMIT          4       /* 汇款凭证 */
#define NOTE_PROM           21      /* 银行本票 */
#define NOTE_DRAFT          41      /* 全国银行汇票 */
#define NOTE_ZONE_DRAFT     42      /* 三省一市/区域性银行汇票 */
#define NOTE_CORRXFER       44      /* 来账代转补充凭证
                                                                              Correspondent Transfer */
#define NOTE_CONPAY         46      /* 代理集中支付 Concentration Payment */
#define NOTE_COLLECTION     52      /* 托收凭证 */
#define NOTE_PDC            53      /* 定期借/贷记 Prearranged Debit&Credit */
#define NOTE_FP             56      /* 财政国库支付凭证 Fiscal Payment */
#define NOTE_DRAWBACK       58      /* 税款退还书 */
#define NOTE_TAXPAY         59      /* 税费缴款书 */
#define NOTE_SPCXFER        61      /* 特种转账凭证 Special transfer */
#define NOTE_NETBANK        62      /* 网银 */
#define NOTE_CASH           71      /* 现金存取款 */
#define NOTE_PXFER          72      /* 个人账户间转账 personal transfer */
#define NOTE_POSXFER        75      /* 银行卡POS转账(收款行一借一贷) */
#define NOTE_POSXFER_MULTI  76      /* 银行卡POS转账(收款行多借一贷) */
#define NOTE_FORCHECK       81      /* 外币转账支票 */
#define NOTE_DRAFT_103      82      /* 电汇MT103 */
#define NOTE_DRAFT_202      83      /* 电汇MT202 */
#define NOTE_PFORXFER       84      /* 个人外币账户间转账 */
#define NOTE_FORZS_OUT      88      /* 外币指示付款 */
#define NOTE_FORZS_IN       89      /* 外币指示收账 */
#define NOTE_BDC            91      /* 批量借/贷记 Batch Debit&Credit */


/**********************南通同城借贷标志********************/
#define FL_DEBIT        "01"            //借
#define FL_CREDIT       "02"            //贷

#endif
