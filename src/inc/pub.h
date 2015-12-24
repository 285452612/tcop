#ifndef PUB_H_
#define PUB_H_

#include "err.h"
#include "util.h"
#include "sdp.h"

#include <dlfcn.h>

extern char OP_HOME[];

//通讯报文大小
#define COMMPACK_MAXLEN            4096 
//通讯文件名长度大小
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

#define returnIfNull(p, ret)       if ((p) == NULL) { INFO("空值被检测!"); return (ret); }

#define returnIfNullLoger(p, ret, fmt, arg...)  if ((p) == NULL) { INFO("空值被检测!"fmt, ##arg); return (ret); }

#define returnDebug(ret)           do { INFO("返回错误值[%d]", ret); return (ret); } while(1)

int ctorBankDllFunction(const char *funcname);
int ctorRegionDllFunction(const char *funcname);

int ctorDllFunction(const char *funcname, char *);
void dtorDllFunction(int ret, const char *funcname);

/* 
 * 动态调用的函数原型
 *
 * 第一个参数: void *
 * 后面的参数为变参 ...
 * 返回结果0成功,否则失败
 */
typedef int (*DllFunctionPrototype)(void *, ...);
extern DllFunctionPrototype dllFuncPtr;

/*
 * 动态调用行内动态库中的指定函数
 *
 * char *funcname: 函数名
 * int *pret: 存放动态库函数调用的返回值(0成功 其它失败)
 * void* voidptr: 动态库函数的第一个void *参数
 * arg...: 动态库函数的其它动态参数
 */
#define callLibraryFunction(pret, funcname, voidptr, arg...) { \
    *pret = ctorBankDllFunction((funcname)); \
    if (*pret == 0) dtorDllFunction(*pret = dllFuncPtr(voidptr, ##arg), (funcname)); }

/* 动态调用区域动态库中的指定函数 */
#define callRegionLibraryFunction(pret, funcname, voidptr, arg...) { \
    *pret = ctorRegionDllFunction((funcname)); \
    if (*pret == 0) dtorDllFunction(*pret = dllFuncPtr(voidptr, ##arg), (funcname)); }

/*
 * 获取联机平台部署的HOME目录
 */
char *getHome();

char *logName(char *filename);
void DBUGHexBuf(char *, unsigned char *pbuf, int len);

/*
 * 按下列目录依次搜索指定文件并返回该文件的绝对路径
 *
 * 1> 主目录下指定子目录下的指定商业银行节点下的指定文件名
 * 2> 主目录下指定子目录下的指定区域节点下的指定文件名
 * 3> 主目录下指定子目录下的指定文件名
 */
char *getFilePath(const char *homesubdir, const char *fname);

char *getDatFilePath(char *filepath, const char *fname);

long GenSerial(char *serialName, long min, long max, int increment);

#endif
