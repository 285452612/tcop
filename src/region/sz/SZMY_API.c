#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/md5.h>
#include <openssl/rc4.h>
#include "md5.h"

#include "TCP_API.h"

#define SVR_MAX 100
#define MYKLOG  "myj" 

#ifndef _MIN
#define _MIN(a,b) ((a)<(b)? (a):(b))
#endif

char TimeFlag;
int TCMYTimeOut=-1;
unsigned char pTempBuff[2*MAX_BUFF];
static struct sigaction act, oldact;
static void AlarmTimeout( int signo );

struct key_svr
{ 
    int  id;                //密押机编号
    char ip[15+1];          //密押机IP地址
    long  port;             //通讯端口
    int  timeout;           //超时时间
    char state[1+1];        //密押机状态
    time_t testtime;        //测试时间
}; 

struct cur_svr
{
    int myjtype;   //密押机类型
    int sumcnt;    //密押机总台数
    int curid;     //当前访问密押机编号
    int testflag;   //测试状态
    time_t testtime;        //测试时间
};

static int Connect_Server(int *sockfd, struct sockaddr_in *serv_addr);

//取当前日期值字符串 YYYYMMDD
static long GetDate()
{
    struct tm t;
    time_t now;

    time(&now);
    t = *localtime(&now);
    return ((1900 + t.tm_year) *10000 + (t.tm_mon + 1) *100 + t.tm_mday);
}

//取当前时间值字符串 HHMMSS
static long GetTime()
{
    struct tm t;
    time_t now;

    time(&now);
    t = *localtime(&now);
    return ( t.tm_hour *10000 + t.tm_min *100 + t.tm_sec);
}

/* 写日志 */
static int WriteLog(char *pFileName, char *pFmt, ...)
{
    va_list ap;
    FILE    *fp=NULL;
    char    pFile[ 512 ];
    int iRc = 0;

    sprintf(pFile, "%s/log/%s_%08ld.log",
            getenv("HOME"),pFileName,GetDate());
    fp=fopen(pFile, "a+");
    if ( fp == NULL)
    {
        fprintf(stderr, "打开日志文件 %s 错, errno=%d.\n", pFileName, errno);
        return -1;
    }

    va_start(ap, pFmt);
    fprintf(fp, "<%06ld> " ,GetTime() );
    iRc = vfprintf(fp, pFmt, ap);
    fprintf(fp, "\n");
    va_end(ap);

    fflush(fp);
    fclose(fp);

    return 0;
}


static void hextoasc(unsigned char *asc,unsigned char *hex,int len)
{
    int i;
    int ch;
    for(i=0;i<len;i++)
    {
        ch=hex[i]>>4;
        if(ch>=0&&ch<=9)
        {
            asc[i*2]=ch+'0';
        }
        if(ch>=0xA&&ch<=0xF)
        {
            asc[i*2]=ch-0xa+'A';
        }
        ch=hex[i]&0x0F;
        if(ch>=0&&ch<=9)
        {
            asc[i*2+1]=ch+'0';
        }
        if(ch>=0xA&&ch<=0xF)
        {
            asc[i*2+1]=ch-0xa+'A';
        }
    }
}

static void asctohex(unsigned char *hex,unsigned char *asc,int len)
{
    int i;
    unsigned char ch;

    for(i=0;i<len;i++)
    {
        ch=asc[i];
        if(ch>='0'&&ch<='9')
        {
            ch-='0';
        }
        if(ch>='A'&&ch<='F')
        {
            ch=ch-'A';
            ch=ch+10;
        }

        if(i%2==0)
        {
            hex[i/2]=ch<<4;
        }
        else
        { 
            hex[i/2]=hex[i/2]|ch; 
        } 
    }
}

static void get_md5(unsigned char *data, int len, unsigned char *result )
{
    char unsigned tmp[17];
    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    memset(tmp, 0 ,sizeof(tmp));
    MD5_Final( tmp, &ctx);
    memcpy(result, tmp, 8);

    return;
}

static int base64_encode(unsigned char *out, unsigned char *in, int size)
{
    int loop, total;
    unsigned char *translate;

    total = size / 3 * 4 + ((size % 3)? 4: 0);
    if (out == NULL || in == NULL)
        return total;

    translate = out;
    for (loop = 0; loop + 3 <= size; loop += 3, in += 3, out += 4) {
        out[0] = (in[0] & 0xFC) >> 2;
        out[1] = ((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4);
        out[2] = ((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6);
        out[3] = in[2] & 0x3F;
    }

    switch (size %= 3)
    {
        case 0:
            break;
        case 1:
            out[0] = (in[0] & 0xFC) >> 2;
            out[1] = (in[0] & 0x03) << 4;
            out[2] = 65;
            out[3] = 65;
            break;
        case 2:
            out[0] = (in[0] & 0xFC) >> 2;
            out[1] = ((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4);
            out[2] = (in[1] & 0x0F) << 2;
            out[3] = 65;
            break;
        default:
            break;
    }

    for (loop = 0, out = translate; loop < total; loop++, ++out) {
        if (*out < 26)
            *out += 'A';
        else if (*out < 52)
            *out += 'a' - 26;
        else if (*out < 62)
            *out += '0' - 52;
        else if (*out == 62)
            *out = '+';
        else if (*out == 63)
            *out = '/';
        else
            *out = '=';
    }

    return total;
}

static int base64_decode(unsigned char *out, unsigned char *in, int size)
{
    char code, code1, code2, code3;
    int i, count, loop, bytes;

    for (loop = 0, count = 0, bytes = 0; loop < size; loop++) {
        code = (unsigned char)*in++;
        if ('A' <= code && code <= 'Z')
            code -= 'A';
        else if ('a' <= code && code <= 'z') {
            code -= 'a';
            code += 26;
        }
        else if ('0' <= code && code <= '9') {
            code -= '0';
            code += 52;
        }
        else if (code == '+')
            code = 62;
        else if (code == '/')
            code = 63;
        else if (code == '=' && size - loop <= 2)
            break;
        else
            continue;

        switch (i = count % 4)
        {
            case 0:
                if (count > 0) {
                    *out++ = code1;
                    *out++ = code2;
                    *out++ = code3;
                    bytes += 3;
                }
                code1 = code << 2;
                break;
            case 1:
                code1 |= (code & 0x30) >> 4;
                code2 = (code & 0x0F) << 4;
                break;
            case 2:
                code2 |= (code & 0x3C) >> 2;
                code3 = (code & 0x03) << 6;
                break;
            case 3:
            default:
                code3 |= code;
                break;
        }

        ++count;
    }

    if (count != 0) {
        *out++ = code1;
        ++bytes;
        size -= loop;
        if (size <= 1) {
            *out++ = code2;
            ++bytes;
        }
        if (size == 0) {
            *out = code3;
            ++bytes;
        }
    }

    return bytes;
}

static void trim_right(char *str)
{
	int i, len;

	len = strlen(str);
	for (i = len - 1; i >= 0; i--)
	{
		if ((str[i] == ' ') || (str[i] == '\n') || (str[i] == '\t'))
			str[i] = '\0';
                else
                        break; 
	}
}

static void trim_left(char *str)
{
	int i, len;

	len = strlen(str);
	while (strlen(str) > 0)
	{
		if ((str[0] == ' ') || (str[0] == '\t'))
			strcpy(str, str + 1);
                else 
                        break; 
	}
}

static void trim_all(char *str)
{
	trim_right(str);
	trim_left(str);
}

/*  比较两个字符串,忽略大小写 */
int stricmp(char *sp1, char *sp2)
{
    int i;
    char str1[4096], str2[4096];
    	
    memset(str1, 0, sizeof(str1));
    strcpy(str1, sp1);
    
    memset(str2, 0, sizeof(str2));
    strcpy(str2, sp2);
    
    for(i = 0; i < strlen(str1); i++)
         if(str1[i] >= 'a' && str1[i] <= 'z')
              str1[i] -= 32;

    for(i = 0; i < strlen(str2); i++)
         if(str2[i] >= 'a' && str2[i] <= 'z')
              str2[i] -= 32;

    return strcmp(str1, str2);
}

/****************************************************************/
/* 读取初始化文件 .ini 中信息函数 : int GetProfileString()      */
/* 参数 : ininame - .ini 文件路迳名                             */ 
/*        segname - 字段名( 如 [segname] )                      */   
/*        keyname - 关键字名 ( keyname = keystr )               */
/*        keystr  - 关键字值 ( 返回 )                           */ 
/****************************************************************/      
static int GetProfileString(char *ininame, 
                            char *segname, 
                            char *keyname, 
                            char *keystr)
{
    FILE *fp; 
    char str[1024];
    char key_name[1024];
    char * flag;
    size_t  i;
    int  match_flag; 

    match_flag = 0; 

    if(!(fp = fopen(ininame, "r")))
        return -1;

    do{
        memset(str, 0, sizeof(str));

        if(!(flag = fgets(str, 1024, fp)))
        {
            fclose(fp);
            return -1;
        }  

        trim_all(str);

        if(strlen(str) >= 2 && str[0] == '[' && str[strlen(str) - 1] == ']')
        {
            str[0] = ' ';
            str[strlen(str) - 1] = ' ';
            trim_all(str);

            if(!stricmp(str, segname))
            {
                match_flag = 1;
                break;
            }
        }
    }while(flag != NULL);   

    if(!match_flag) 
        return -1;

    do{
        memset(str, 0, sizeof(str));
        memset(key_name, 0, sizeof(key_name));

        if(!(flag = fgets(str, 1024, fp)))
            break;

        trim_all(str);

        if(strlen(str) >= 2 && str[0] == '[' && str[strlen(str) - 1] == ']')
            break;   

        for(i = 0; i < strlen(str); i++)
        {
            if(str[i] == '=')
            {
                strncpy(key_name, str, i);
                trim_all(key_name);

                if(!stricmp(key_name, keyname))
                {
                    memcpy(keystr, str + i + 1, strlen(str) - i);
                    trim_all(keystr); 

                    fclose(fp);       

                    return 0;
                }
            }
        } 
    }while(flag != NULL);

    fclose(fp);
    return -1; 
}

static void gen_md5(unsigned char *data, int len, unsigned char *result )
{
    char tmp[17];
    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    memset(tmp, 0 ,sizeof(tmp));
    MD5_Final( tmp, &ctx);
    memcpy(result, tmp, 8);

    return;
}

static void catch(int signo)
{
    TimeFlag = 1;
    WriteLog(MYKLOG,"%s|%d timeout",__FILE__,__LINE__);
    signo=signo;
}

static int Init_Shm(int shmid)
{
    int shm_size ;
    int i,j;
    char profile[65];
    char buff[256];
    char MYJname[60];
    char *ps;
    struct cur_svr *cur_ps;
    struct key_svr *svr_ps;

    shm_size = sizeof(struct cur_svr) + SVR_MAX * sizeof(struct key_svr);

    memset(buff, 0, sizeof(buff));
    memset(profile, 0, sizeof(profile));
    sprintf(profile, "%s/etc/server.ini", getenv("HOME"));

    ps = (char *)shmat(shmid, 0 , 0);
    cur_ps = (struct cur_svr *)(ps);
    svr_ps = (struct key_svr *)(ps +sizeof(struct cur_svr));

    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJTYPE", "MYJTYPE", buff ))
    {
        WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                "Get MYJ info [MYJTYPE], MYJTYPE from server.ini err");
        return -1;
    }
    else
        cur_ps->myjtype = atoi(buff);

    memset( svr_ps, 0, shm_size - sizeof(struct cur_svr));

    j = 0;
    for(i = 0; i <SVR_MAX; i++)
    {
        sprintf(MYJname ,"MYJ%d",i);

        svr_ps[j].id = i;
        memset(buff, 0, sizeof(buff));
        if( GetProfileString( profile, MYJname, "IP", buff ))
            continue;
        else
            memcpy(svr_ps[j].ip, buff, strlen(buff));

        memset(buff, 0, sizeof(buff));
        if( GetProfileString( profile, MYJname, "Port", buff ))
            continue;
        else
            svr_ps[j].port = atoi(buff);

        memset(buff, 0, sizeof(buff));
        if( GetProfileString( profile, MYJname, "TimeOut", buff ))
            continue;
        else
            svr_ps[j].timeout = atoi(buff);

        svr_ps[j].state[0]='0';
        svr_ps[j].testtime = 0;
        j++;

    }
    if( j == 0)
    {
        WriteLog( MYKLOG   , "%s|%d| %s", __FILE__, __LINE__, 
                "Get MYJ info from server.ini err");
        return -1;
    }
    else
    {
        cur_ps->sumcnt = j;
        cur_ps->testflag = 0;
        cur_ps->curid = svr_ps[0].id;
    }
    shmdt((const void *)ps);
    return 0 ;
}

void TestState(int shmid )
{
    int i;
    int result;
    char *ps;
    struct cur_svr *cur_ps;
    struct key_svr *svr_ps;
    int sockfd;
    struct sockaddr_in serv_addr;

    ps = (char *)shmat(shmid, 0 , 0);
    cur_ps = (struct cur_svr *)(ps);
    svr_ps = (struct key_svr *)(ps +sizeof(struct cur_svr));

    cur_ps->testflag = 1;
    time(&(cur_ps->testtime)); 
    for(i = 0; i<SVR_MAX; i++)
    {
        if(svr_ps[i].state[0] == 0)
            break;

        memset(&serv_addr, 0, sizeof(struct sockaddr_in));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(svr_ps[i].ip);
        serv_addr.sin_port = htons(svr_ps[i].port);
        TCMYTimeOut = svr_ps[i].timeout;
        result = Connect_Server(&sockfd, &serv_addr);
        if (result)
            svr_ps[i].state[0] = '9';
        else
        {
            svr_ps[i].state[0] = '0';
            shutdown(sockfd, 2);
            close(sockfd);
        }
        time(&svr_ps[i].testtime); 
    }
    cur_ps->testflag = 0;
    shmdt((const void *)ps); 
}

static int Get_MYJ_Info(int *curid, int *myjid, struct sockaddr_in *serv_addr, 
        int *myjtype)
{
    char profile[65];
    char buff[200];
    int  shm_key;
    key_t shmid = 0;
    int  shm_size;
    int  i, j = 0;
    int  sumcnt, err_k = 0;
    int  ret;
    char *ps;
    struct key_svr *svr_ps;
    struct cur_svr *cur_ps;
    time_t nowtime;


    shm_size = sizeof(struct cur_svr) + SVR_MAX * sizeof(struct key_svr);

    memset(profile, 0, sizeof(profile));
    sprintf(profile, "%s/etc/server.ini", getenv("HOME"));
    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJSHMKEY", "ShmKey", buff ))
    {
        WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                "Get MYJ info [MYJSHMKEY], ShmKey from server.ini err");
        return  -1; 
    }
    else
        shm_key = atoi(buff);

    shmid = shmget(shm_key, shm_size, 0666);
    if(shmid <0)  //未获取到共享内存
    {
        //初始化共享内存
        shmid = shmget(shm_key, shm_size, 0666 | IPC_CREAT);
        if( shmid < 0) 
        {
            WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                    "初始化共享内存失败!\n");
            return  -1;
        }

        if ( Init_Shm( shmid ) < 0)
        {
            WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                    "初始化共享内存失败!\n");
            return  -1;
        }
    }

    ps = (char *)shmat(shmid, 0 , 0);
    cur_ps = (struct cur_svr *)(ps);
    *myjtype = cur_ps->myjtype;
    sumcnt = cur_ps->sumcnt;
    svr_ps = (struct key_svr *)(ps +sizeof(struct cur_svr));

    ret = -1;
    j = 0;
    for(i = cur_ps->curid +1; i <SVR_MAX; )
    {
        j++;
        if(j >SVR_MAX)
            break;

        if((i == SVR_MAX -1) || ((svr_ps[i].state[0] == 0)&&(i != 0)))
        {
            i = 0;
            continue;
        }

        if((svr_ps[i].state[0] == 0)&&(i == 0))
        {
            ret = -1; 
            break;
        }

        if(svr_ps[i].state[0] == '0')//正常状态
        {
            memset(serv_addr, 0, sizeof(struct sockaddr_in));
            serv_addr->sin_family=AF_INET;
            serv_addr->sin_addr.s_addr = inet_addr(svr_ps[i].ip);
            serv_addr->sin_port = htons(svr_ps[i].port);
            ret = 0;
            cur_ps->curid = i;
            *curid = i;
            *myjid = svr_ps[i].id;
            TCMYTimeOut = svr_ps[i].timeout;
            break;
        }
        else
            err_k ++; //处于异常状态的密押机数量
        i++;
    } 

    if(ret == -1)
    {
        WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                "配置的各密押机状态均异常!\n");
    }

    if(err_k > 0)
    {
        for( i = 0, err_k = 0 ; i < sumcnt; i++)
        {
            if(svr_ps[i].state[0] != '0')//正常状态
                err_k++;
        }

        time(&nowtime); 
        if(( err_k >= (int)(sumcnt/2) )
                &&((long)nowtime != (long)cur_ps->testtime)
                && (fork() == 0)) //子进程
        {
            WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                    "测试密押机状态!\n");
            cur_ps->testflag = 1;
            time(&(cur_ps->testtime));
            shmdt((const void *)ps); 
            TestState( shmid );
            WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                    "测试密押机状态完毕!\n");
            exit(0);
        }
    }

    shmdt((const void *)ps); 
    return ret;
}

static int Set_MYJ_Info( int i, char *state)
{
    char profile[65];
    char buff[200];
    int  shm_key;
    key_t shmid = 0;
    int  shm_size;
    char *ps;
    struct key_svr *svr_ps;
    struct cur_svr *cur_ps;

    shm_size = sizeof(struct cur_svr) + SVR_MAX * sizeof(struct key_svr);

    memset(profile, 0, sizeof(profile));
    sprintf(profile, "%s/etc/server.ini", getenv("HOME"));
    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJSHMKEY", "ShmKey", buff ))
    {
        WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                "Get MYJ info [MYJSHMKEY], ShmKey from server.ini err");
        return  -1; 
    }
    else
        shm_key = atoi(buff);

    shmid = shmget(shm_key, shm_size, 0666);
    if(shmid <0)
    {
        WriteLog( MYKLOG, "%s|%d| %s", __FILE__, __LINE__, 
                "shmget err!\n");
        return  -1;
    }

    ps = (char *)shmat(shmid, 0 , 0);
    cur_ps = (struct cur_svr *)(ps);
    svr_ps = (struct key_svr *)(ps +sizeof(struct cur_svr));

    svr_ps[i].state[0] = state[0];
    time(&svr_ps[i].testtime);

    shmdt((const void *)ps); 
    return 0;
}

/*连接密押机*/
static int Connect_Server(int *sockfd, struct sockaddr_in *serv_addr)
{
    int    ret = 0;
    void (*pFunc)(int);
    struct   linger   lig;
    int   iLen;

    if(( *sockfd = socket(AF_INET,SOCK_STREAM,0) ) <  0  ) 
    {
        ret = -1;
        WriteLog(MYKLOG,"%s|%d SOCKET初始化出错,%s!",__FILE__,__LINE__,strerror(errno));
        goto ENDCONNECT;
    }

    TimeFlag = 0;
    alarm(TCMYTimeOut);
    pFunc=signal(SIGALRM,catch);
    if(pFunc==SIG_ERR)
    {
        ret = -4;
        WriteLog(MYKLOG,"%s|%d 设置超时信号出错,%s!",__FILE__,__LINE__,strerror(errno));
        goto ENDCONNECT;
    }
    if( (ret = connect( *sockfd,(struct sockaddr *)serv_addr,sizeof(struct sockaddr) )) < 0 )
    {
        WriteLog(MYKLOG,"%s|%d 连接加密机失败,ret = %d, %d, %s!",__FILE__,__LINE__,ret, errno, strerror(errno));
        ret = - 2 ;
        goto ENDCONNECT;
    }
    signal(SIGALRM,SIG_IGN);
    alarm(0);
    lig.l_onoff=1;
    lig.l_linger=0;
    iLen=sizeof(struct   linger);
    setsockopt(*sockfd,SOL_SOCKET,SO_LINGER,(char   *)&lig,iLen);

    return 0;

ENDCONNECT:
    signal(SIGALRM,SIG_IGN);
    alarm(0);
    shutdown(*sockfd,2);
    close(*sockfd);
    return  -1;
}


static int Connect_MYJ( int *sockfd, int *myjtype)
{
    int ret, curid, myjid, testtimes=0;
    struct sockaddr_in serv_addr;

GETMYJINFO:
    testtimes++;
    ret = Get_MYJ_Info( &curid, &myjid, &serv_addr, myjtype);
    if( ret != 0)
    {
        WriteLog(MYKLOG,"%s|%d获取密押机信息失败，重置密押机信息!",__FILE__,__LINE__);
        return -1; //获取密押机配置信息失败
    }

    ret = Connect_Server(sockfd, &serv_addr);
    if(ret != 0)
    {
        WriteLog(MYKLOG,"%s|%d连接加密机%d失败!",__FILE__,__LINE__,myjid);
        Set_MYJ_Info(curid, "9");
        if(testtimes <3)
            goto GETMYJINFO;
        else
        {
            return -2; //连接密押机失败
        }
    }
    return 0;
}

// 设置超时
static void SetAlarm( int p_iSecond )
{
    act.sa_handler = AlarmTimeout;
    //act.sa_flags = SA_ONESHOT | SA_NOMASK;
    sigaction( SIGALRM, &act, &oldact );
    alarm( p_iSecond );
}

// 清除超时
static void ClearAlarm( void )
{
    sigaction( SIGALRM, &oldact, NULL );
    alarm( 0 );
}

// 超时处理
static void AlarmTimeout( int signo )
{
    ClearAlarm();
}

static int SendData(int id, unsigned char *buf,int BufLen)
{
    int n_len;
    int n_result;
    if (BufLen==0)
        n_len = strlen((char *)buf);
    else
        n_len=BufLen;

    n_result=ERR_SEND;
    SetAlarm(TCMYTimeOut);
    //n_result=send(id,buf,n_len,0);
    n_result=ERR_SEND;
    n_result=write(id,buf, n_len);
    ClearAlarm();
    return n_result;
}


static int RecvData(int id, unsigned char *buf,int BufLen)
{
    int n_result;

    SetAlarm(TCMYTimeOut);
    //n_result=recv(id,buf,BufLen,0);
    n_result=ERR_RECV;
    n_result=read(id,buf,BufLen);
    ClearAlarm();
    return n_result;
}


static int DoJY(int sock,int cmd,unsigned char *data,int *len)
{
    int ret;
    unsigned char send_data[MAX_BUFF];
    unsigned char recv_data[MAX_BUFF];

    memcpy(send_data,"SYD",3);
    send_data[6]=(*len+2)/256;
    send_data[7]=(*len+2)%256;
    send_data[8]=cmd;
    send_data[9]=0xff;//rult

    if(*len>MAX_BUFF-3)
        return ERR_DATALEN;


    memcpy(send_data+10,data,*len);

    //my_trace(0,"send_data",send_data,*len+10);

    ret=SendData(sock,send_data,*len+10);
    if(ret<0)
        return -2;

    ret=RecvData(sock,recv_data,MAX_BUFF);
    if(ret<=0)
    {
        *len=0;
        return -3;
    }

    if(recv_data[6]!=0)
    {
        *len=0;
        return recv_data[6];
    }

    *len=recv_data[3]*256+recv_data[4]-2;
    memcpy(data,recv_data+7,*len);

    return 0;
}

int GenerateCode(unsigned char *data,int len,unsigned char mac[17])
{
    int ret;
    int all_len;
    int myjtype;
    int sock;

    if(len>MAX_BUFF-8)
        return ERR_DATALEN;

#ifndef BY_MD5
    memcpy(pTempBuff, data, len);
    all_len = len;
#else
    SoftMD5(pTempBuff, data, len);
    all_len = 16;
#endif	

    ret = Connect_MYJ(&sock, &myjtype);
    if ( ret )
        return ret;

    ret=DoJY(sock,0xB0,pTempBuff,&all_len);
    if(ret!=0)
    {
        WriteLog(MYKLOG,"%s|%d DoJY return %d",__FILE__,__LINE__,ret);
        shutdown(sock,2);
        close(sock);
        return ret;
    }

    hextoasc(mac,pTempBuff,8);
    mac[16]=0;
    shutdown(sock,2);
    close(sock);
    return 0;
}

int CheckCode(unsigned char *data,int len,unsigned char mac[17])
{
    int ret;
    int all_len;
    int myjtype;
    int sock;

    if(len>MAX_BUFF-8)
        return ERR_DATALEN;

    asctohex(pTempBuff,mac,16);
#ifndef BY_MD5	
    memcpy(pTempBuff+8,data,len);
    all_len=len+8;
#else
    SoftMD5(pTempBuff+8,data,len);
    all_len=16+8;
#endif

    ret = Connect_MYJ(&sock, &myjtype);
    if ( ret )
        return ret;

    ret=DoJY(sock,0xB1,pTempBuff,&all_len);
    if(ret!=0)
    {
        WriteLog(MYKLOG,"%s|%d DoJY return %d",__FILE__,__LINE__,ret);
        shutdown(sock,2);
        close(sock);
        return ret;
    }
    shutdown(sock,2);
    close(sock);
    return 0;

}

static unsigned char *str_xor(unsigned char *data, unsigned char *xor, int len)
{
    int i;

    for (i = 0; i < len; i++)
        data[i] ^= xor[i];

    return data;
}

int Data_Encrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len)
{
    unsigned char pBuff[100];
    unsigned char MD5_data[8];
    unsigned char MD5_result[8];
    const int MOD = 8;
    int count;
    int ret;
    int i;
    int myjtype;
    int sock;

    *enc_len = 0;
    get_md5(data, len, MD5_data);
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(16, strlen(refid)), refid, _MIN(16, strlen(refid)));
    get_md5(pBuff, 24, MD5_result);

    sprintf((char *)pTempBuff, "%04d", len);
    memcpy(pTempBuff + 4, MD5_data, 8);
    memcpy(pTempBuff + 4 + 8, data, len);
    len += 12;
    count = ((len % MOD == 0) ? (len / MOD) : (len / MOD + 1));
    if ((len = count * 8) >= MAX_BUFF)
        return ERR_DATALEN;

    for (i = 0; i < count; i++)
        str_xor(pTempBuff + 8 * i, MD5_result, 8);

    ret = Connect_MYJ(&sock, &myjtype);
    if ( ret )
        return ret;

    ret = DoJY(sock, 0xB2, pTempBuff, &len);
    if(ret!=0)
    {
        WriteLog(MYKLOG,"%s|%d DoJY return %d",__FILE__,__LINE__,ret);
        shutdown(sock,2);
        close(sock);
        return ret;
    }

    *enc_len = base64_encode(enc_data, pTempBuff, len);

    shutdown(sock,2);
    close(sock);
    return 0;
}

int Data_Encrypt_Soft(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len)
{
    char pBuff[100];
    char MD5_data[8];
    char MD5_result[8];
    const int MOD = 8;
    int count;
    int ret;
    int i;
    int iKeySize = 41;    
    unsigned char *ucKey = "iGDkRTLl5QYkJ99adoD2cA=V;1260502979323129";
    RC4_KEY k;    
    char *buff=NULL;

    *enc_len = 0;
    gen_md5(data, len, MD5_data);
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(16, strlen(refid)), refid, _MIN(16, strlen(refid)));
    gen_md5(pBuff, 24, MD5_result);

    sprintf(pTempBuff, "%04d", len);
    memcpy(pTempBuff + 4, MD5_data, 8);
    memcpy(pTempBuff + 4 + 8, data, len);
    len += 12;
    count = ((len % MOD == 0) ? (len / MOD) : (len / MOD + 1));
    if ((len = count * 8) >= MAX_BUFF)
        return ERR_DATALEN;

    for (i = 0; i < count; i++)
        str_xor(pTempBuff + 8 * i, MD5_result, 8);

    memset( &k, 0, sizeof(k) );    
    RC4_set_key( &k, iKeySize, ucKey );    

    buff=(unsigned char*)malloc( len );    
    if(buff == NULL)
    {
        WriteLog(MYKLOG,"%s|%d,malloc error",__FILE__, __LINE__);
        return -1;
    }
    RC4( &k, len, pTempBuff, buff );    
    memcpy(pTempBuff, buff, len);
    free(buff);    

    // ret = DoJY(sock, 0xB2, pTempBuff, &len);

    *enc_len = base64_encode(enc_data, pTempBuff, len);
    return 0;
}

int Data_Decrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len)
{
    unsigned char pBuff[100];
    unsigned char MD5_data[8];
    unsigned char MD5_result[8];
    char c_len[5];
    const int MOD = 8;
    int nLen;
    int ret;
    int i;
    int myjtype;
    int sock;

    *dec_len = 0;
    nLen = base64_decode(pTempBuff, data, len);
    if (nLen % MOD != 0)
    {
        WriteLog(MYKLOG,"%s|%d nLen= %d",__FILE__,__LINE__,nLen);
        return ERR_DATALEN;
    }
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(strlen(refid), 16), refid, _MIN(strlen(refid), 16));
    get_md5(pBuff, 24, MD5_result);

    ret = Connect_MYJ(&sock, &myjtype);
    if ( ret )
        return ret;

    ret = DoJY(sock, 0xB3, pTempBuff, &nLen);
    if (ret != 0)
    {
        WriteLog(MYKLOG,"%s|%d DoJY return %d",__FILE__,__LINE__,ret);
        shutdown(sock,2);
        close(sock);
        return ret;
    }

    for (i = 0; i < (nLen / MOD); i++)
        str_xor(pTempBuff + 8 * i, MD5_result, MOD);

    memset(c_len, 0, sizeof(c_len));
    memcpy(c_len, pTempBuff, 4);
    *dec_len = atoi(c_len);
    get_md5(pTempBuff + 4 + 8, *dec_len, MD5_data);
    if (memcmp(pTempBuff + 4, MD5_data, 8) != 0)
    {
        *dec_len = 0;
        shutdown(sock,2);
        close(sock);
        return ERR_CARDDATA_CRC;
    }

    memcpy(dec_data, pTempBuff + 4 + 8, *dec_len);

    shutdown(sock,2);
    close(sock);
    return 0;
}

int Data_Decrypt_Soft(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len)
{
    char pBuff[100];
    char MD5_data[8];
    char MD5_result[8];
    char c_len[5];
    const int MOD = 8;
    int nLen;
    int ret;
    int i;
    int iKeySize = 41;    
    unsigned char *ucKey = "iGDkRTLl5QYkJ99adoD2cA=V;1260502979323129";
    RC4_KEY k;    
    char *buff=NULL;

    *dec_len = 0;
    nLen = base64_decode(pTempBuff, data, len);
    if (nLen % MOD != 0)
    {
        WriteLog(MYKLOG, "%s|%d, nLen= %d", __FILE__, __LINE__, nLen);
        return ERR_DATALEN;
    }
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(strlen(refid), 16), refid, _MIN(strlen(refid), 16));
    gen_md5(pBuff, 24, MD5_result);

    memset( &k, 0, sizeof(k) );    
    RC4_set_key( &k, iKeySize, ucKey );    

    buff=(unsigned char*)malloc( nLen );    
    if(buff == NULL)
    {
        WriteLog(MYKLOG, "%s|%d,malloc error", __FILE__, __LINE__);
        return -1;
    }
    RC4( &k, nLen, pTempBuff, buff );    
    memcpy(pTempBuff, buff, nLen);
    free(buff);    

    for (i = 0; i < (nLen / MOD); i++)
        str_xor(pTempBuff + 8 * i, MD5_result, MOD);

    memset(c_len, 0, sizeof(c_len));
    memcpy(c_len, pTempBuff, 4);
    *dec_len = atoi(c_len);
    gen_md5(pTempBuff + 4 + 8, *dec_len, MD5_data);
    if (memcmp(pTempBuff + 4, MD5_data, 8) != 0)
    {
        *dec_len = 0;
        return ERR_CARDDATA_CRC;
    }

    memcpy(dec_data, pTempBuff + 4 + 8, *dec_len);
    return 0;
}

//异或
void xor(unsigned char* source, unsigned char* dest, int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        dest[i] ^= source[i];
    }
}

//获取主密押机信息
int Connect_Primary_MYJ( int *sockfd)
{
    int ret;
    struct sockaddr_in serv_addr;
    char profile[60], buff[20];

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;

    memset(profile, 0, sizeof(profile));
    sprintf(profile, "%s/etc/server.ini", getenv("HOME"));
    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJ1", "IP", buff ))
    {
        WriteLog( MYKLOG, "%s|%d|从配置文件[%s]获取主密押机(MYJ1)信息失败",
                __FILE__, __LINE__, profile);
        return -1;
    }
    serv_addr.sin_addr.s_addr = inet_addr(buff);
    WriteLog( MYKLOG, "%s|%d|从配置文件[%s]获取主密押机(MYJ1)信息[%s]",
                __FILE__, __LINE__,profile, buff);

    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJ1", "Port", buff ))
    {
        WriteLog( MYKLOG, "%s|%d|从配置文件[%s]获取主密押机(MYJ1)信息失败",
                __FILE__, __LINE__, profile);
        return -1;
    }
    serv_addr.sin_port = htons(atoi(buff));

    memset(buff, 0, sizeof(buff));
    if( GetProfileString( profile, "MYJ1", "PIKTimeOut", buff ))
    {
        WriteLog( MYKLOG, "%s|%d|从配置文件[%s]获取主密押机(MYJ1)信息PIKTimeOut失败",
                __FILE__, __LINE__, profile);
        return -1;
    }
    TCMYTimeOut = atoi(buff);

    ret = Connect_Server(sockfd, &serv_addr);
    if(ret != 0)
    {
        WriteLog(MYKLOG,"%s|%d连接主密押机失败!",__FILE__,__LINE__);
        return -1;
    }
    return 0;
}

//写入PIK
int WritePIK(unsigned char bankno[8],
        unsigned char *PIK,
        unsigned char *MAC)
{
    int ret;
    int all_len;
    int sock;

    memset(pTempBuff,0,MAX_BUFF);
    memcpy(pTempBuff,bankno,8);
    asctohex( pTempBuff + 8, PIK, 32 );
    asctohex( pTempBuff + 24, MAC, 8 );

    all_len = 28;

    ret = Connect_Primary_MYJ(&sock);
    if ( ret )
        return ret;

    ret = DoJY(sock, 0xC7, pTempBuff, &all_len);
    if(ret!=0)
    {
        WriteLog(MYKLOG, "%s|%d myjid=[1] DoJY return %d", 
                __FILE__,__LINE__, ret);
        return ret;
    }

    shutdown(sock,2);
    close(sock);
    return 0;
}

/*个人密码加密,输出的pinblock均为16字节的0-F的*/
//bankno 密钥编号 可从配置文件读入
//acctno 帐号
//pin    密码明文
//enc_pinblock  加密后的密码密文
int PINBlock_Encrypt(unsigned char *bankno,
        unsigned char *acctno,
        unsigned char *pin, 
        unsigned char *enc_pinblock)
{
    int ret;
    int all_len;
    int myjtype;
    int myjid;
    int sock;
    unsigned char pan[17],tmppin[17], hex_pan[9], hex_pin[9]; 
    unsigned char hex_pinblock[9], pinblock[17], tmpbuf[100];
    int acctlen;

    memset(pTempBuff,0,MAX_BUFF);
    memcpy(pTempBuff,bankno,8);

    memset(pan, 0, sizeof(pan));
    memset(pan, '0', 16);

    acctlen = strlen(acctno);
    if(acctlen >12)
        memcpy(pan+4, acctno +acctlen -13, 12);
    else if( acctlen == 12)
        memcpy(pan+5, acctno, 11);
    else
        memcpy(pan+4, acctno, acctlen);

    asctohex( hex_pan, pan, 16 );

    memset(tmppin, 0, sizeof(tmppin));
    memset(tmppin, 'F', 16);

    sprintf(tmpbuf,"%02d",strlen(pin));
    memcpy(tmppin, tmpbuf, 2);
    memcpy(tmppin+2, pin, strlen(pin));

    asctohex( hex_pin, tmppin, 16 );

    xor(hex_pan, hex_pin, 8);

    memcpy( pTempBuff + 8, hex_pin, 8 );
    memset(pinblock, 0, sizeof(pinblock));
    hextoasc(pinblock, hex_pin, 8);

    all_len = 8 +8 ;

    ret = Connect_MYJ(&sock, &myjtype);
    if ( ret )
        return ret;

    ret = DoJY(sock, 0xC8, pTempBuff, &all_len);
    if(ret!=0)
    {
        WriteLog(MYKLOG,"%s|%d myjid=[%d] DoJY return %d",
                __FILE__, __LINE__, myjid, ret);
        return ret;
    }

    hextoasc( enc_pinblock, pTempBuff, 8 );

    shutdown(sock,2);
    close(sock);
    return 0;
}

/*个人密码解密,输入的pinblock均为16字节的0-F的*/
//bankno 密钥编号 可从配置文件读入
//acctno 帐号
//pinblock    密码密文
//dec_pin  解密后的密码明文
int PINBlock_Decrypt(unsigned char *bankno,
        unsigned char *acctno, 
        unsigned char *pinblock, 
        unsigned char *dec_pin)
{
    int ret;
    int all_len;
    int myjtype;
    int myjid;
    int sock;
    int acctlen;
    unsigned char pan[17],tmppin[17], hex_pan[9], hex_pin[9]; 
    unsigned char dec_pinblock[17], hex_pinblock[9], tmpbuf[100];

    memset(pTempBuff,0,MAX_BUFF);
    memcpy(pTempBuff,bankno,8);

    if(strlen(pinblock) != 16)
    {
        WriteLog(MYKLOG,"%s|%dPINBlock[%s]长度不为16!",__FILE__,__LINE__, pinblock);
        return ERR_DATALEN;
    }

    asctohex( pTempBuff + 8, pinblock, 16 );

    all_len = 8 +8 ;

    ret = Connect_MYJ(&sock, &myjtype );
    if ( ret )
        return ret;

    ret = DoJY(sock, 0xC9, pTempBuff, &all_len);
    if(ret!=0)
    {
        WriteLog(MYKLOG,"%s|%d myjid=[%d] DoJY return %d", 
                __FILE__,__LINE__, myjid, ret);
        return ret;
    }
    shutdown(sock,2);
    close(sock);

    hextoasc( dec_pinblock, pTempBuff, 8 );
   
    memcpy(hex_pinblock, pTempBuff, 8);

    memset(pan, 0, sizeof(pan));
    memset(pan, '0', 16);
    acctlen = strlen(acctno);
    if(acctlen >12)
        memcpy(pan+4, acctno +acctlen -13, 12);
    else if( acctlen == 12)
        memcpy(pan+5, acctno, 11);
    else
        memcpy(pan+4, acctno, acctlen);

    asctohex( hex_pan, pan, 16 );
    xor(hex_pan, hex_pinblock, 8);

    hextoasc( dec_pinblock, hex_pinblock, 8 );
    memset(tmpbuf, 0, sizeof(tmpbuf));
    memcpy(tmpbuf, dec_pinblock, 2);

    memset(tmppin, 0, sizeof(tmppin));
    memcpy(tmppin, dec_pinblock+2, atoi(tmpbuf));
    strcpy(dec_pin, tmppin);

    return 0;
}

