#include <stdio.h>
#include <string.h>

#define CACHE_SIZE 10240
#define TOTAL_COUNT 1

extern int tcp_client(
        char *servername, int serverport,
        char *sendbuff, int sendlen,
        char *recvbuff, int maxlen, int *recvlen,
        int iTimeOut);

char error_msg[1024];

char *memstr(char *dst, char *src, size_t count)
{
    size_t i, len=strlen(src);
    for(i=0; i<=(count-len); i++)
    {
        if(memcmp(dst+i,src,len) == 0)
            return dst+i;
    }
    return NULL;
}

int load_file(char *filename, char **buffer, int *length)
{
    int l,len;
    FILE *ipt;
    if((ipt=fopen(filename,"rb")) == NULL)
    {
        sprintf(error_msg, "Can't open file[%s]", filename);
        return -1;
    }
    fseek(ipt, 0, SEEK_END);
    *length = ftell(ipt);
    rewind(ipt);
    *buffer = malloc(*length);
    for(len=0; len<*length; len+=l)
    {
        l = fread(*buffer+len, sizeof(char), CACHE_SIZE, ipt);
        if(l <= 0)
            break;
    }
    fclose(ipt);
    if(len < *length)
    {
        sprintf(error_msg, "Can't read file[%s]", filename);
        free(*buffer);
        return -1;
    }
    return 0;
}

int communicate(char *sendbuf, int sendlen, char **recvbuf, int *recvlen)
{
    int rc, port, timeout;
    char ip[16], url[128], *p;

    memset(url, 0, sizeof(url));
    if((p=getenv("TEST_ADDR")) == NULL)
        strcpy(url, "127.0.0.1:5091:30");
    else
        strcpy(url, p);
    sscanf(url, "%[^:]:%d:%d", ip, &port, &timeout);

    *recvbuf = malloc(CACHE_SIZE);
    rc = tcp_client(ip, port, sendbuf, sendlen, *recvbuf, CACHE_SIZE, recvlen, timeout);
    if (rc < 0)
    {
        sprintf(error_msg, "Can't communicate with %s:%d(%d)", ip, port, rc);
        free(*recvbuf);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int i, sendlen, recvlen;
    char tmp[256] = {0};
    char *sendbuf = NULL;
    char *recvbuf = NULL;
    if(argc == 1)
    {
        printf("Usage: %s FILE...\n", argv[0]);
        return 0;
    }
    if(strcmp(argv[1],"bat.req")==0)
    {
        int i_noteno, i_settlamt;
        char *p_noteno, *p_settlamt;

        if(load_file(argv[1],&sendbuf,&sendlen) != 0)
        {
            printf("Error: [%s]\n",error_msg);
            return 0;
        }

        p_noteno = memstr(sendbuf, "00000002", sendlen);
        if(p_noteno == NULL)
        {
            printf("p_noteno is null\n");
            free(sendbuf);
            return 0;
        }
        i_noteno = 2;

        p_settlamt = memstr(sendbuf, "10002.00", sendlen);
        if(p_settlamt == NULL)
        {
            printf("p_noteno is null\n");
            free(sendbuf);
            return 0;
        }
        i_settlamt = 10002;

        for(i=1; i<=TOTAL_COUNT; i++)
        {
            sprintf(tmp, "%08d", i_noteno+i);
            memcpy(p_noteno, tmp, 8);

            sprintf(tmp, "%5d", i_settlamt+i);
            memcpy(p_settlamt, tmp, 5);

            if(communicate(sendbuf,sendlen,&recvbuf,&recvlen) != 0)
            {
                printf("Error: [%s]\n",error_msg);
                continue;
            }
            else
            {
                printf("Success: [%d][%s]\n", recvlen, recvbuf);
                free(recvbuf);
            }
        }
        free(sendbuf);
    }
    else
    {
        for(i=1; i<argc; i++)
        {
            if(load_file(argv[i],&sendbuf,&sendlen) != 0)
            {
                printf("Error: [%s]\n",error_msg);
                continue;
            }
            if(communicate(sendbuf,sendlen,&recvbuf,&recvlen) != 0)
            {
                printf("Error: [%s]\n",error_msg);
                free(sendbuf);
                continue;
            }
            else
            {
                printf("Success: [%d][%s]\n", recvlen, recvbuf);
                free(recvbuf);
                free(sendbuf);
            }
        }
    }
    return 0;
}
