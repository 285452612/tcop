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
    { MAILTYPE_MAIL,      "�ʼ�" },
    { MAILTYPE_CXS,       "��ѯ��" },
    { MAILTYPE_CFS,       "�鸴��" },
    { MAILTYPE_CXS_DRAFT, "��Ʊ��ѯ��" },
    { MAILTYPE_CFS_DRAFT, "��Ʊ�鸴��" },
    { -1, NULL },
};

static ST_CHINESE dcflag_list[] = {
    { PART_DEBIT,      "���" },
    { PART_CREDIT,     "����" },
    { PART_ZS,         "ָʾ" },
    { -1, NULL },
};

static ST_CHINESE classid_list[] = {
    { CLASS_UNIT,      "���ҶԹ�ҵ��" },
    { CLASS_PERSON,    "�����˸�ҵ��" },
    { CLASS_FOREIGN,   "���ҵ��" },
    { CLASS_BATCH,     "����ҵ��" },
    { -1, NULL },
};

static ST_CHINESE clrstat_list[] = {
    { '0',      "δ(��)����" },
    { '1',        "����ɹ�" },
    //{ CLRSTAT_WAITFUNDHOLD,   "��Ȧ��" },
    //{ CLRSTAT_FUNDHOLD,       "��Ȧ��" },
    //{ CLRSTAT_INQUEUE_NOHOLD, "δȦ�������" },
    //{ CLRSTAT_INQUEUE_HOLD,   "��Ȧ�������" },
    //{ CLRSTAT_END,            "��������" },
    //{ CLRSTAT_CANCELFAIL,     "����ʧ��Ȧ��δ���" },
    { '9',         "����ʧ��" },
    { 'C',        "�Ѷ���" },
    { 'Z',        "�Ѷ���" },
    //{ CLRSTAT_UNDOED,         "�ѳ���" },
    { -1, NULL },
};

static ST_CHINESE curtype_list[] = {
    { 0,      "����" },
    { 2,      "�׻�" },
    { 3,      "�ҳ�" },
    { 4,      "�һ�" },
    { 6,      "����" },
    { -1, NULL },
};

static ST_S_CHINESE curcode_list[] = {
    { "CNY",        "�����" },
    { "HKD",        "�۱�" },
    { "USD",        "��Ԫ" },
    { "GBP",        "Ӣ��" },
    { "EUR",        "ŷԪ" },
    { "JPY",        "��Ԫ" },
    { NULL, NULL },
};

static ST_CHINESE acc_stat_list[] = {
    { 0,      "δ����" },
    { 1,      "�Ѽ���" },
    { 2,      "�ѳ���" },
    { 3,      "��ȡ��" },
    { 5,      "�ѹ���" },
    { 9,      "ʧ��" },
    { -1, "δ֪" },
};

char *GetChineseName(ST_CHINESE *, int type);
char *ChsName(ST_S_CHINESE *, char *type);

#endif
