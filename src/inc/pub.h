#ifndef PUB_H_
#define PUB_H_

#include "err.h"
#include "util.h"
#include "sdp.h"

#include <dlfcn.h>

extern char OP_HOME[];

//ͨѶ���Ĵ�С
#define COMMPACK_MAXLEN            4096 
//ͨѶ�ļ������ȴ�С
#define COMMFNAME_MAXLEN           256

#define SAFE_PTR(str)              (((str) != NULL) ? (str) : "")
#define INFO(fmt, arg...)          opLoger(OP_HOME, "info", __FILE__, __LINE__, fmt, ##arg) 
#define INFOLINE()                 opLoger(OP_HOME, "info", NULL, 0, NULL) 
#define DBUG(fmt, arg...)          opLoger(OP_HOME, "debug", __FILE__, __LINE__, fmt, ##arg)

#define RGINFO(fmt, arg...)        opLoger(OP_HOME, "rginfo", __FILE__, __LINE__, fmt, ##arg) 
#define BKINFO(fmt, arg...)        opLoger(OP_HOME, "bkinfo", __FILE__, __LINE__, fmt, ##arg) 
#define DBINFO(fmt, arg...)        opLoger(OP_HOME, "dbinfo", __FILE__, __LINE__, fmt, ##arg) 

#define LDBUG(buf, len)            opFixedLoger(OP_HOME, "debug", __FILE__, __LINE__, buf, len)

#define OSERRMSG                   strerror(errno)

#define returnIfNull(p, ret)       if ((p) == NULL) { INFO("��ֵ�����!"); return (ret); }

#define returnIfNullLoger(p, ret, fmt, arg...)  if ((p) == NULL) { INFO("��ֵ�����!"fmt, ##arg); return (ret); }

#define returnDebug(ret)           do { INFO("���ش���ֵ[%d]", ret); return (ret); } while(1)

int ctorBankDllFunction(const char *funcname);
int ctorRegionDllFunction(const char *funcname);

int ctorDllFunction(const char *funcname, char *);
void dtorDllFunction(int ret, const char *funcname);

/* 
 * ��̬���õĺ���ԭ��
 *
 * ��һ������: void *
 * ����Ĳ���Ϊ��� ...
 * ���ؽ��0�ɹ�,����ʧ��
 */
typedef int (*DllFunctionPrototype)(void *, ...);
extern DllFunctionPrototype dllFuncPtr;

/*
 * ��̬�������ڶ�̬���е�ָ������
 *
 * char *funcname: ������
 * int *pret: ��Ŷ�̬�⺯�����õķ���ֵ(0�ɹ� ����ʧ��)
 * void* voidptr: ��̬�⺯���ĵ�һ��void *����
 * arg...: ��̬�⺯����������̬����
 */
#define callLibraryFunction(pret, funcname, voidptr, arg...) { \
    *pret = ctorBankDllFunction((funcname)); \
    if (*pret == 0) dtorDllFunction(*pret = dllFuncPtr(voidptr, ##arg), (funcname)); }

/* ��̬��������̬���е�ָ������ */
#define callRegionLibraryFunction(pret, funcname, voidptr, arg...) { \
    *pret = ctorRegionDllFunction((funcname)); \
    if (*pret == 0) dtorDllFunction(*pret = dllFuncPtr(voidptr, ##arg), (funcname)); }

/*
 * ��ȡ����ƽ̨�����HOMEĿ¼
 */
char *getHome();

char *logName(char *filename);
void DBUGHexBuf(char *, unsigned char *pbuf, int len);

/*
 * ������Ŀ¼��������ָ���ļ������ظ��ļ��ľ���·��
 *
 * 1> ��Ŀ¼��ָ����Ŀ¼�µ�ָ����ҵ���нڵ��µ�ָ���ļ���
 * 2> ��Ŀ¼��ָ����Ŀ¼�µ�ָ������ڵ��µ�ָ���ļ���
 * 3> ��Ŀ¼��ָ����Ŀ¼�µ�ָ���ļ���
 */
char *getFilePath(const char *homesubdir, const char *fname);

char *getDatFilePath(char *filepath, const char *fname);

long GenSerial(char *serialName, long min, long max, int increment);

#endif
