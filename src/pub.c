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
            INFO("δ�ҵ���̬���ú���[%s->%s]!", libname, funcname);
            ret = E_SYS_NODLLFUNC;
        } else {
            INFO("��ʼ���ö�̬�������:%s", funcname);
        }
    } else {
        INFO("���ض�̬�����[%s]ʧ��:[%s]", libname, dlerror());
        ret = E_SYS_CALL;
    }
    if (ret != 0)
        dlclose(dllHANDLE);

    return ret;
}

void dtorDllFunction(int ret, const char *funcname)
{
    //if (ret != 0)
        INFO("[%s]��̬������[%d]", funcname, ret);
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

    //��Ŀ¼��ָ����Ŀ¼�µ�ָ����ҵ���нڵ��µ�ָ���ļ���
    sprintf(path, "%s/%s/%d%d/%s", OP_HOME, homesubdir, OP_REGIONID, TCOP_BANKID, fname);
    if (access(path, 0) == 0)
        return path;

    //��Ŀ¼��ָ����Ŀ¼�µ�ָ������ڵ��µ�ָ���ļ���
    sprintf(path, "%s/%s/%d/%s", OP_HOME, homesubdir, OP_REGIONID, fname);
    if (access(path, 0) == 0)
        return path;

    //��Ŀ¼��ָ����Ŀ¼�µ�ָ���ļ���
    sprintf(path, "%s/%s/%s", OP_HOME, homesubdir, fname);
    if (access(path, 0) == 0)
        return path;

    INFO("�����ļ�[%s/%s//%s]�ľ���·��ʧ��", OP_HOME, homesubdir, fname);

    return NULL;
}

char *getDatFilePath(char *filepath, const char *fname)
{
    char *p = NULL;
    char path1[256] = {0};
    char path2[256] = {0};
    char path3[256] = {0};
    char path4[256] = {0};

    //����ƽ̨�ڵ�Ŀ¼�µ��ļ�
    sprintf(path1, "%s/dat/%d/%s", OP_HOME, OP_NODEID, fname);
    //���ж˽ڵ�Ŀ¼�µ��ļ�
    sprintf(path2, "%s/dat/%d/%s", OP_HOME, OP_BANKNODE, fname);
    //���к��µĵ������µ��ļ�
    sprintf(path3, "%s/dat/%d/%d/%s", OP_HOME, TCOP_BANKID, OP_REGIONID, fname);
    //���к��µ��ļ�
    sprintf(path4, "%s/dat/%d/%s", OP_HOME, TCOP_BANKID, fname);

    if (path1[0] != 0 && access(path1, 0) == 0) p = path1;
    else if (path2[0] != 0 && access(path2, 0) == 0) p = path2;
    else if (path3[0] != 0 && access(path3, 0) == 0) p = path3;
    else if (path4[0] != 0 && access(path4, 0) == 0) p = path4;
    else INFO("�޷���λdat�������ļ�%s:P1[%s]P2[%s]P3[%s]P4[%s]", 
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
