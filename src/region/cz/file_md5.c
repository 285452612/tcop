#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <fcntl.h>

void get_md5( char *data, char *result )
{
    MD5_CTX ctx;
    unsigned char hex[16];

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, strlen(data));
    MD5_Final( hex, &ctx);
    HEX_2_DSP( hex, (unsigned char *)result, 16 );

    return;
}

/*对文件进行MD5数字签名, 将签名写在文件的末尾*/
int SignFileLocal(char *pFileName)
{
    struct stat fs;
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    char md_str[MD5_DIGEST_LENGTH*2+1];
    char *buf;
    int fd;
    int i;

    memset(md_str, 0, sizeof(md_str));
    fd = open(pFileName, O_RDWR);
    if (fd == -1)
        return -1;

    fstat(fd, &fs);
    buf = malloc(fs.st_size + 1);
    if (buf == NULL)
    {
        close(fd);
        return -1;
    }
    memset(buf, 0, fs.st_size + 1);

    MD5_Init(&c);

    i = readln(fd, buf, fs.st_size);

    MD5_Update(&c,buf,(unsigned long)i);
    free(buf);

    MD5_Final(&(md[0]),&c);

    for (i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        if ((md[i]>>4) >9)
            md_str[2*i] = (md[i]>>4)-10+'a';
        else
            md_str[2*i] = (md[i]>>4)+'0';
        if ((md[i] & 0x0f) > 9)
            md_str[2*i+1] = (md[i] & 0x0f)-10+'a';
        else
            md_str[2*i+1] = (md[i] & 0x0f)+'0';
    }

    writeln(fd, "####", 4);
    writeln(fd, md_str, MD5_DIGEST_LENGTH*2);
    writeln(fd, "####", 4);
    close(fd);

    return 0;
}


/*对带数字签名的文件进行MD5验证*/
int VerifyFileLocal(char *pFileName)
{
    int fd, i;
    long lFileLen;
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    char md_str[MD5_DIGEST_LENGTH*2+1];
    char oldmd_str[MD5_DIGEST_LENGTH*2+1];
    char *buf;
    struct stat fs;

    memset(md_str, 0, sizeof(md_str));
    memset(oldmd_str, 0, sizeof(oldmd_str));
    fd = open(pFileName, O_RDONLY);
    if (fd == -1)
        return -1;

    fstat(fd, &fs);

    lFileLen = fs.st_size - 2 * MD5_DIGEST_LENGTH - 8;
    if (lFileLen < 0)
    {
        close(fd);
        return -1;
    }

    MD5_Init(&c);

    buf = malloc(lFileLen+1);
    if (buf == NULL)
    {
        close(fd);
        return -1;
    }
    memset(buf, 0, lFileLen+1);
    i = readln(fd, buf, lFileLen);

    MD5_Update(&c,buf,(unsigned long)i);
    free(buf);

    readln(fd,oldmd_str, 4);
    readln(fd,oldmd_str, MD5_DIGEST_LENGTH*2);

    MD5_Final(&(md[0]),&c);

    for (i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        if ((md[i]>>4) >9)
            md_str[2*i] = (md[i]>>4)-10+'a';
        else
            md_str[2*i] = (md[i]>>4)+'0';
        if ((md[i] & 0x0f) > 9)
            md_str[2*i+1] = (md[i] & 0x0f)-10+'a';
        else
            md_str[2*i+1] = (md[i] & 0x0f)+'0';
    }
    close(fd);

    if (memcmp(oldmd_str, md_str, MD5_DIGEST_LENGTH*2) != 0)
        return -1;

    return 0;
}

/*对带数字签名的文件进行MD5验证, 验证成功后将签名删除掉*/
int VerifyFileAndMove(char *pFileName)
{
    int fd, i;
    long lFileLen;
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    char md_str[MD5_DIGEST_LENGTH*2+1];
    char oldmd_str[MD5_DIGEST_LENGTH*2+1];
    char *buf;
    struct stat fs;

    memset(md_str, 0, sizeof(md_str));
    memset(oldmd_str, 0, sizeof(oldmd_str));
    fd = open(pFileName, O_RDWR);
    if (fd == -1)
        return -1;

    fstat(fd, &fs);
    lFileLen= fs.st_size - 2 * MD5_DIGEST_LENGTH - 8;
    if (lFileLen < 0)
    {
        close(fd);
        return -1;
    }

    MD5_Init(&c);

    buf = malloc(lFileLen+1);
    if (buf == NULL)
    {
        close(fd);
        return -1;
    }
    memset(buf, 0, lFileLen+1);
    i = readln(fd, buf, lFileLen);

    MD5_Update(&c,buf,(unsigned long)i);
    free(buf);

    read(fd,oldmd_str,4);
    read(fd,oldmd_str,MD5_DIGEST_LENGTH*2);

    MD5_Final(&(md[0]),&c);

    for (i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        if ((md[i]>>4) >9)
            md_str[2*i] = (md[i]>>4)-10+'a';
        else
            md_str[2*i] = (md[i]>>4)+'0';
        if ((md[i] & 0x0f) > 9)
            md_str[2*i+1] = (md[i] & 0x0f)-10+'a';
        else
            md_str[2*i+1] = (md[i] & 0x0f)+'0';
    }

    if (memcmp(oldmd_str, md_str, MD5_DIGEST_LENGTH*2) != 0)
    {
        close(fd);
        return -1;
    }
    else
    {
        ftruncate(fd, lFileLen);
        close(fd);
        return 0;
    } 
}

