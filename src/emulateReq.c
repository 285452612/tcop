#include "tcpapi.h"
#include <errno.h>

int main(int argc, char *argv[])
{
    char *ip = NULL;
    int port = 0;
    char *trncode = NULL;
    char *pdatafile = NULL;
    char *pfile = NULL;
    TAPIHEAD head = {0};
    FILE *fp = NULL;
    char rspfile[128] = {0};
    char reqbuf[4096] = {0};
    char rspbuf[4096] = {0};

    if (argc != 5 && argc != 6)
    {
        fprintf(stderr, "usage: %s ip port trncode datafile [sendfiles]", argv[0]);
        return 0;
    }

    ip = argc[1];
    port = atoi(argc[2]);
    trncode = argc[3];
    pdatafile = argc[4];
    if (argc == 6)
        pfile = argc[5];

    if ((fp = fopen(pdatafile, "r")) == NULL)
    {
        fprintf(stderr, "读取数据文件失败:%s|%s\n", pdatafile, strerror(errno));
        return 0;
    }
    fread(reqbuf, sizeof(reqbuf), 1, fp);
    fclose(fp);

    head.Sleng = strlen(reqbuf);
    ret = cli_sndrcv(ip, port, &head, reqbuf, pfile, rspbuf, rspfile, 0);
}
