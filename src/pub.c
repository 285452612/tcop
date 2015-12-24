#include "pub.h"
#include "interface.h"

DllFunctionPrototype dllFuncPtr;
static void *dllHANDLE;

int ctorRegionDllFunction(const char *funcname)
{
    char libname[256] = {0};

    sprintf(libname, "%s/lib/libregion%d.so", OP_HOME, OP_REGIONID);
    return ctorDllFunction(funcname, libname);
}

int ctorBankDllFunction(const char *funcname)
{
    char libname[256] = {0};

    sprintf(libname, "%s/lib/libprocess%d.so", OP_HOME, TCOP_BANKID);

    return ctorDllFunction(funcname, libname);
}

int ctorDllFunction(const char *funcname, char *libname)
{
    int ret = 0;

    dllHANDLE = dlopen(libname, RTLD_LOCAL | RTLD_LAZY);
    if (dllHANDLE != NULL)
    {
        *(void **)(&dllFuncPtr) = dlsym(dllHANDLE, funcname);
        if (dllFuncPtr == NULL) {
            INFO("未找到动态调用函数[%s->%s]!", libname, funcname);
            ret = E_SYS_NODLLFUNC;
        } else {
            INFO("开始调用动态处理程序:%s", funcname);
        }
    } else {
        INFO("加载动态处理库[%s]失败:[%s]", libname, dlerror());
        ret = E_SYS_CALL;
    }
    if (ret != 0)
        dlclose(dllHANDLE);

    return ret;
}

void dtorDllFunction(int ret, const char *funcname)
{
    //if (ret != 0)
        INFO("[%s]动态处理返回[%d]", funcname, ret);
    if (dllHANDLE != NULL) {
        dlclose(dllHANDLE);
        dllHANDLE = NULL;
    }
}

char *getHome()
{
    char *p = NULL;

    if ((p = getenv("TCOP_HOME")) == NULL)
        return getenv("HOME");
    return p;
}

char *logName(char *filename)
{
    static char buff[100] = {0};
    sprintf(buff, "%s_%s.log", filename, getDate(0));
    return buff;
}

char *getFilePath(const char *homesubdir, const char *fname)
{
    static char path[512] = {0};

    //主目录下指定子目录下的指定商业银行节点下的指定文件名
    sprintf(path, "%s/%s/%d%d/%s", OP_HOME, homesubdir, OP_REGIONID, TCOP_BANKID, fname);
    if (access(path, 0) == 0)
        return path;

    //主目录下指定子目录下的指定区域节点下的指定文件名
    sprintf(path, "%s/%s/%d/%s", OP_HOME, homesubdir, OP_REGIONID, fname);
    if (access(path, 0) == 0)
        return path;

    //主目录下指定子目录下的指定文件名
    sprintf(path, "%s/%s/%s", OP_HOME, homesubdir, fname);
    if (access(path, 0) == 0)
        return path;

    INFO("搜索文件[%s/%s//%s]的绝对路径失败", OP_HOME, homesubdir, fname);

    return NULL;
}

char *getDatFilePath(char *filepath, const char *fname)
{
    char *p = NULL;
    char path1[256] = {0};
    char path2[256] = {0};
    char path3[256] = {0};
    char path4[256] = {0};

    //请求平台节点目录下的文件
    sprintf(path1, "%s/dat/%d/%s", OP_HOME, OP_NODEID, fname);
    //银行端节点目录下的文件
    sprintf(path2, "%s/dat/%d/%s", OP_HOME, OP_BANKNODE, fname);
    //银行号下的地区号下的文件
    sprintf(path3, "%s/dat/%d/%d/%s", OP_HOME, TCOP_BANKID, OP_REGIONID, fname);
    //银行号下的文件
    sprintf(path4, "%s/dat/%d/%s", OP_HOME, TCOP_BANKID, fname);

    if (path1[0] != 0 && access(path1, 0) == 0) p = path1;
    else if (path2[0] != 0 && access(path2, 0) == 0) p = path2;
    else if (path3[0] != 0 && access(path3, 0) == 0) p = path3;
    else if (path4[0] != 0 && access(path4, 0) == 0) p = path4;
    else INFO("无法定位dat下配置文件%s:P1[%s]P2[%s]P3[%s]P4[%s]", 
                fname, path1, path2, path3, path4);

    if (p != NULL)
        strcpy(filepath, p);

    return filepath;
}

void DBUGHexBuf(char *desc, unsigned char *pbuf, int len)
{
    char logbuf[2048] = {0};
    int i = 0;
    for (i = 0; i < len; i++)
        sprintf(logbuf + i*2, "%02X", pbuf[i]);
    DBUG("%s:[%s]", desc, logbuf);
}
