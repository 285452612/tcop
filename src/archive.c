#include "tcpapi.h"
#include "region_common.h"

#define ERRLOG logName("archive")

int main(int argc, char *argv[])
{
    TCPHEAD head;
    char req[512] = "<?xml version='1.0' encoding='GB18030'?><UFTP/>";
    char rsp[2048] = {0};
    char port[12] = {0};
    char tmp[128] = {0};
    int ret = 0;

    sprintf(tmp, "%s/etc/tcp.ini", getenv("HOME"));

    if (ret = GetProfileString(tmp, "LOCALE", "port", port))
    {
        SDKerrlog(ERRLOG, "%s|%d|ȡƽ̨����˿�ʧ��![%s]", __FILE__, __LINE__, tmp);
        return 0;
    }

    memset(&head, 0, sizeof(head));
    strcpy(head.TrType, "9999");
    head.Sleng = strlen(req);
    head.PackInfo |= htonl(PI_DCOMPRESS);

    ret = cli_sndrcv("localhost", atoi(port), &head, req, NULL, rsp, NULL, 60);
    if (ret)
    {
        SDKerrlog(ERRLOG, "%s|%d|�鵵��������ƽ̨ͨѶʧ��,������=[%s]", __FILE__, __LINE__, head.RetCode);
        return ret;
    }

    return 0;
}
