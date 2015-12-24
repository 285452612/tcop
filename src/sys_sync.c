#include "tcpapi.h"
#include "region_common.h"

#define ERRLOG logName("sync")

int main(int argc, char *argv[])
{
    TCPHEAD head;
    char req[512];
    char rsp[2048] = {0};
    char port[12] = {0};
    char tmp[128] = {0};
    int ret = 0;

    sprintf(tmp, "%s/etc/tcp.ini", getenv("HOME"));

    if (ret = GetProfileString(tmp, "LOCALE", "port", port))
    {
        SDKerrlog(ERRLOG, "%s|%d|取平台服务端口失败![%s]", __FILE__, __LINE__, tmp);
        return 0;
    }

    memset(&head, 0, sizeof(head));
    if (argc >= 2)
        strcpy(head.TrType, argv[1]);
    else
        strcpy(head.TrType, "1601");
    sprintf(req, "<?xml version='1.0' encoding='GB18030'?><UFTP><MsgHdrRq>"
            "<TrnCode>%s</TrnCode>"
            "<Originator>090010</Originator><Acceptor></Acceptor>"
            "<Sender>109</Sender><Recver>000</Recver>"
            "</MsgHdrRq></UFTP>",
            head.TrType);
    head.Sleng = strlen(req);
    head.PackInfo |= htonl(PI_DCOMPRESS);
    ret = cli_sndrcv("localhost", atoi(port), &head, req, NULL, rsp, NULL, 60);
    if (ret)
    {
        SDKerrlog(ERRLOG, "%s|%d|状态同步请求交易与平台通讯失败,错误码=[%s]", 
                __FILE__, __LINE__, head.RetCode);
        return ret;
    }

    return 0;
}
