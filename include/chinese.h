#ifndef __CHINESE_H__
#define __CHINESE_H__

#include "pubdef.h"

typedef struct {
    int  type;
    char *name;
} ST_CHINESE;

typedef struct {
    char *type;
    char *name;
} ST_S_CHINESE;


static char data_empty[] = { 0 };

static ST_CHINESE mailtype_list[] = {
    { MAILTYPE_MAIL,      "邮件" },
    { MAILTYPE_CXS,       "查询书" },
    { MAILTYPE_CFS,       "查复书" },
    { MAILTYPE_CXS_DRAFT, "汇票查询书" },
    { MAILTYPE_CFS_DRAFT, "汇票查复书" },
    { -1, NULL },
};

static ST_CHINESE dcflag_list[] = {
    { PART_DEBIT,      "借记" },
    { PART_CREDIT,     "贷记" },
    { PART_ZS,         "指示" },
    { -1, NULL },
};

static ST_CHINESE classid_list[] = {
    { CLASS_UNIT,      "本币对公业务" },
    { CLASS_PERSON,    "本币人个业务" },
    { CLASS_FOREIGN,   "外币业务" },
    { CLASS_BATCH,     "批量业务" },
    { -1, NULL },
};

static ST_CHINESE clrstat_list[] = {
    { '0',      "未(待)清算" },
    { '1',        "清算成功" },
    //{ CLRSTAT_WAITFUNDHOLD,   "待圈存" },
    //{ CLRSTAT_FUNDHOLD,       "已圈存" },
    //{ CLRSTAT_INQUEUE_NOHOLD, "未圈存待清算" },
    //{ CLRSTAT_INQUEUE_HOLD,   "已圈存待清算" },
    //{ CLRSTAT_END,            "不需清算" },
    //{ CLRSTAT_CANCELFAIL,     "清算失败圈存未解除" },
    { '9',         "清算失败" },
    { 'C',        "已对账" },
    { 'Z',        "已对账" },
    //{ CLRSTAT_UNDOED,         "已冲正" },
    { -1, NULL },
};

static ST_CHINESE curtype_list[] = {
    { 0,      "丙钞" },
    { 2,      "甲汇" },
    { 3,      "乙钞" },
    { 4,      "乙汇" },
    { 6,      "丙钞" },
    { -1, NULL },
};

static ST_S_CHINESE curcode_list[] = {
    { "CNY",        "人民币" },
    { "HKD",        "港币" },
    { "USD",        "美元" },
    { "GBP",        "英镑" },
    { "EUR",        "欧元" },
    { "JPY",        "日元" },
    { NULL, NULL },
};

static ST_CHINESE acc_stat_list[] = {
    { 0,      "未记账" },
    { 1,      "已记账" },
    { 2,      "已冲正" },
    { 3,      "已取消" },
    { 5,      "已挂账" },
    { 9,      "失败" },
    { -1, "未知" },
};

char *GetChineseName(ST_CHINESE *, int type);
char *ChsName(ST_S_CHINESE *, char *type);

#endif
