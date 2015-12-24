/* 平台联调测试客户端 */

#include "tcpapi.h"
#include "pub.h"

int main(int argc, char *argv[])
{
    TCPHEAD head = {0};
    xmlDoc *req = NULL; 
    xmlDoc *rsp = NULL;
    unsigned char *reqbuf = NULL;
    char rspbuf[8092] = {0};
    int trncode = 0;
    char *p = NULL;
    char tmp[128] = {0};
    char reqfile[512] = {0};
    char rspfile[512] = {0};
    int len = 0;

    fprintf(stderr, "Usage: opClient 行内交易码 [请求传输文件名]\n\n");

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "命令行参数不正确!\n");
        return 0;
    }
    if ((p = getenv("TCOP_TESTNODE")) == NULL)
    {
        fprintf(stderr, "环境变量TCOP_TESTNODE未设置!\n");
        return 0;
    }
    sprintf(tmp, "%s/%s/%s.xml", getenv("HOME"), p, argv[1]);

    if (argc == 3)
        strcpy(reqfile, argv[3]);

    if ((req = xmlRecoverFile(tmp)) == NULL)
    {
        fprintf(stderr, "加载请求报文文件[%s]失败:%s\n", tmp, OSERRMSG);
        return 0;
    }
    trncode = atoi(XMLGetNodeVal(req, "//MsgHdrRq/TrnCode"));

    xmlDocDumpMemory(req, &reqbuf, &len);

    memset(&head, 0, sizeof(head));
    sprintf(head.TrType, "%04d", trncode);
    head.Sleng = len;
    head.PackInfo |= htonl(PI_DCOMPRESS);

    //需要传输文件
    if (strlen(reqfile))
    {
        head.PackInfo |= htonl(PI_FCOMPRESS);
        p = reqfile;
    } else 
        p = NULL;

    fprintf(stderr, "发送报文:[\n%s\n]文件:[%s]\n\n\n", reqbuf, reqfile);

    memset(rspbuf, 0, sizeof(rspbuf));
    if (cli_sndrcv("localhost", 5090, &head, reqbuf, p, rspbuf, rspfile, 30) < 0)
    {
        fprintf(stderr, "与平台通讯失败:%s\n\n", head.RetCode);
        return 0;
    }

    fprintf(stderr, "接收报文:[\n%s\n]文件:[%s]\n\n", rspbuf, rspfile);

    return 0;
}
