#include "tcop.h"

char *GenOperPwd(char *orgid, char *operid, char *name, char *passwd, char *regdate, char *cliper)
{
    char sBuf[256];
    char sResult[33];

    snprintf(sBuf, sizeof(sBuf), "%-12s%-6s%-80s%s%-8s", orgid, operid, name, passwd, regdate);

    memset(sResult, 0, sizeof(sResult));
    opGetMd5(sBuf, sResult);

    strncpy(cliper, sResult, sizeof(sResult));

    return cliper;
}

int ChkOperPwd(char *orgid, char *operid, char *name, char *passwd, char *regdate, char *real_passwd)
{
    char sBuf[256];
    char sResult[33];

    snprintf(sBuf, sizeof(sBuf), "%-12s%-6s%-80s%s%-8s",
            orgid, operid, name, passwd, regdate);

    memset(sResult, 0, sizeof(sResult));
    opGetMd5(sBuf, sResult);
    if (memcmp(sResult, real_passwd, 32) != 0)
        return -1;

    return 0;
}
