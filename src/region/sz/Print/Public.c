#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <libgen.h>
#include "Public.h"

#define IPC_KEY 128
#define RECLEN 200

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

static struct timeval tv_start = {0, 0};
extern int errno;

//取当前日期值字符串 YYYYMMDD
char *GetDate(char *cur_date)
{
    struct tm t;
    time_t now;

    time(&now);
    t = *localtime(&now);
    sprintf(cur_date, "%04d%02d%02d",
            1900 + t.tm_year, t.tm_mon + 1, t.tm_mday);
    return(cur_date);
}

//取当前时间值字符串 HHMMSS
char *GetTime(char *cur_time)
{
    struct tm t;
    time_t now;

    time(&now);
    t = *localtime(&now);
    sprintf(cur_time, "%02d%02d%02d",
            t.tm_hour, t.tm_min, t.tm_sec);
    return(cur_time);
}

void GetDateAndTime(char *cur_date, char *cur_time)
{
    struct tm t;
    time_t now;
 
    time(&now);
    t = *localtime(&now);

    *cur_date = *cur_time = 0x00;
    sprintf(cur_date, "%04d%02d%02d",
            1900 + t.tm_year, t.tm_mon + 1, t.tm_mday);
    sprintf(cur_time, "%02d%02d%02d",
            t.tm_hour, t.tm_min, t.tm_sec);

    return;
}

#if 0
char *gettime(char *s, size_t size, const char *format)
{
    struct tm tm_now;
    time_t now;

    time(&now);
    tm_now = *localtime(&now);

    if (size <= 0)
        return NULL;

    if(s == NULL)
    {
        if ((s = malloc(++size)) == NULL)
            return NULL;
    }
    memset(s, 0, size);

    if (strftime(s, size, format, &tm_now) == 0)
        memset(s, 0, size);

    return s;
}
#endif

/* 写日志 */
int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...)
{
    static  pid_t pid_log = 0;
    FILE    *fp = NULL;
    char    pDate[9], pTime[7];
    char    pFile[256];
    int     iRc = 0;
    va_list ap;

    if (pid_log == 0)
        pid_log = getpid();
    GetDateAndTime(pDate, pTime);
    snprintf(pFile, sizeof(pFile)-1, "%s/log/%s_%s.log", 
            getenv("HOME"), prefix, pDate);
    if ((fp = fopen(pFile, "a+")) == NULL)
    {
        fprintf(stderr, "打开日志文件 %s 错, errno=%d.\n", pFile, errno);
        fp = stderr;
    }

    va_start(ap, pFmt);
    fprintf(fp, "%s %6d|" , pTime, pid_log);
    if (file != NULL)
        fprintf(fp, "%s, %d|" , basename(file), line);
    iRc = vfprintf(fp, pFmt, ap);
    fprintf(fp, "\n");
    va_end(ap);

    if (fp != stderr)
        fclose(fp);

    return 0;
}


//取当前日期时间值字符串 YYYY/MM/DD HH:MM:SS
char *GetDateTime(char *cur_date_time)
{
    struct tm t;
    time_t now;

    time(&now);
    t = *localtime(&now);
    sprintf(cur_date_time, "%04d%s%02d%s%02d %02d%s%02d%s%02d",
            1900 + t.tm_year, "/", t.tm_mon + 1, "/", t.tm_mday, t.tm_hour, ":", t.tm_min, ":", t.tm_sec);
    return(cur_date_time);
}

// 转换字符串成小写
char *strlwr( char *s )
{
    int i;

    for (i=0; s[i] != 0; i++)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] += 'a' -'A';

    return s;
}

// 转换字符串成大写
char *strupr( char *s )
{
    int i;

    for (i=0; s[i] != 0; i++)
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] += 'A' -'a';

    return s;
}

// 将字符串中所有的出现的子串删除
int strrdel(char *s, char *deleted)
{
    int deleted_times;

    deleted_times = 0;
    while(*s) 
    {
        if(strchr(deleted, *s)) 
        {
            memmove(s, s + 1, strlen(s));
            ++deleted_times;
        }
        else s++;
    }

    return deleted_times;
}

// 在字符串中某个位置插入一个字符串
int strsins(char *string, int pos, char *added)
{
    int length;

    if((length = strlen(string)) >= pos && pos >= 0) 
    {
        string += pos;
        length = strlen(added);
        memmove(string + length, string, strlen(string) + 1);
        memcpy(string, added, length);
    }

    return 0;
}

// 在字符串中将所有出现的字符串替换为另一字符串
int strsrpl(char *string, char *old, char *new)
{
    char *ptr = string;
    int length;

    while ((ptr = strstr(ptr, old)) != NULL) 
    {
        length = strlen(old);
        memmove(ptr, ptr + length, strlen(ptr) - length + 1);
        if(new && !strsins(ptr, 0, new) && (ptr += strlen(new)));
    }

    return 0;
}

// XML报文处理
int xmltrim(char *s)
{
    int deleted_times;

    s++;
    deleted_times = 0;
    while(*s)
    {
        if( (*s == ' ' || *s == '\t') && *(s-1) == '>' )
        {
            memmove(s, s + 1, strlen(s));
            ++deleted_times;
        }
        else if ( *s == '\n' || *s == '\r' )
        {
            memmove(s, s + 1, strlen(s));
            ++deleted_times;
        }
        else s++;
    }

    return deleted_times;
}

//返回下一个分隔符后的字段内容. 与系统函数strtok()功能相同,
//但比strtok()函数更健状.
char *StrTok(char *sourcestr,char *splitstr)
{
    static char srcstr[8192];
    static char retstr[8192];
    int i,l;

    if(sourcestr!=NULL)
    {
        memset(srcstr, 0, sizeof(srcstr) );
        strcpy(srcstr,sourcestr);
    }
    l=strlen(srcstr);
    if ( l == 0 )
        return NULL;
    srcstr[l]=splitstr[0];
    for (i=0;i<l;i++)
    {
        if (srcstr[i]==splitstr[0])
        {
            srcstr[i]='\0';
            break;
        }
    }
    srcstr[l]='\0';
    strcpy(retstr,srcstr);
    if (i == l)
        srcstr[i+1]='\0';
    strcpy(srcstr,srcstr+i+1);

    return retstr;
}


int IsExistFile( char *s )
{
    int fd;

    if((fd=open(s, O_RDONLY)) < 0)
        return 0;

    close(fd);

    return 1;
}

int IsNonEmptyStr(char *s)
{
    if (s == NULL)
        return 0;
    if (*s == 0x00)
        return 0;
    return 1;
}

char *GetTmpFileName( char *pFileName )
{
    char buf[256];

    sprintf( buf, "%s/tmpXXXXXX", getenv("FILES_DIR") );
    mkstemp( buf );
    strcpy( pFileName, buf );

    return pFileName;
}

int DiffDate(char* date1, char* date2)
{ 
    time_t time1, time2;  
    struct tm tm_time; 
    int day, month, year;

    memset(&tm_time, 0, sizeof(tm_time));

    //sscanf(date1, "%d/%d/%d", &year, &month, &day);
    year = atol(date1)/10000;
    month = atol(date1)%10000/100;
    day = atol(date1)%100;

    tm_time.tm_year = year - 1900;
    tm_time.tm_mon = month - 1;
    tm_time.tm_mday = day;
    time1 = mktime((struct tm*)&tm_time);

    //sscanf(date2, "%d/%d/%d", &year, &month, &day);
    year = atol(date2)/10000;
    month = atol(date2)%10000/100;
    day = atol(date2)%100;
    tm_time.tm_year = year - 1900;
    tm_time.tm_mon = month - 1;
    tm_time.tm_mday = day;
    time2 = mktime((struct tm*)&tm_time);

    return ((time1-time2)/(24*60*60));
}

char *lastmonth(char* date)
{
    time_t time;
    struct tm tm_time;
    struct tm t;
    int month, year;

    memset(&tm_time, 0, sizeof(tm_time));

    year = atol(date)/10000;
    month = atol(date)%10000/100;

    tm_time.tm_year = year - 1900;
    tm_time.tm_mon = month - 1;
    tm_time.tm_mday = 1;
    tm_time.tm_hour=0;
    tm_time.tm_min=0;
    tm_time.tm_sec=0;
    time = mktime((struct tm*)&tm_time);
    time -= 1;

    t = *localtime(&time);
    strftime(date, 7, "%Y%m", &t);
    date[6] = 0;

    return date;
}

// 金额大小写转换
int  MoneyToChinese ( char *money, char * chinese )
{
    int len, zerotype, i, unit_num , allzero = 1 ;
    char fundstr [ 51 ];
    char *numberchar [ ] =
    {
        "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"
    };
    char *rmbchar [ ] =
    {
        "分", "角", "", "元", "拾", "佰", "仟", "万", "拾", "佰", "仟", 
        "亿", "拾", "佰", "仟"
    };

    sprintf ( fundstr, "%.2lf", atof( money ) );
    len = strlen ( fundstr );
    unit_num = sizeof ( rmbchar ) / sizeof ( rmbchar [ 0 ] );

    for ( i = zerotype = 0, chinese [ 0 ] = '\0' ; i < len ; i++ )
    {
        switch ( fundstr [ i ] )
        {
            case '-':
                {
                    strcat ( chinese, "负" );
                    break;
                }
            case '.':
                {
                    if ( chinese [ 0 ] == '\0' )
                        strcat ( chinese, numberchar [ 0 ] );
                    if ( zerotype == 1 )
                        strcat ( chinese, rmbchar [ 3 ] );
                    zerotype = 0;
                    break;
                }
            case '0':
                {
                    if ( len - i  == 12 && 11 < unit_num )
                        strcat ( chinese, rmbchar [ 11 ] );
                    if ( len - i  == 8 && 7 < unit_num )
                    {
                        if( !allzero )
                            strcat ( chinese, rmbchar [ 7 ] );
                    }
                    zerotype = 1;
                    break;
                }
            default:
                {
                    if ( len - i  < 12 )
                        allzero = 0 ;
                    if ( zerotype == 1 )
                        strcat ( chinese, numberchar [ 0 ] );
                    strcat ( chinese, numberchar [ fundstr [ i ] - '0' ] );
                    if ( len - i - 1 < unit_num )
                        strcat ( chinese, rmbchar [ len - i - 1 ] );
                    zerotype = 0;
                    break;
                }
        }
    }

    if ( memcmp( fundstr + len -2 , "00", 2 ) == 0 ) strcat ( chinese, "整" );
    return 0;
}

int SplitString(char *pBuf, const char *pSplitChar, ...)
{
    va_list pArgs;
    char *p;

    va_start(pArgs, pSplitChar);
    p = va_arg(pArgs, char *);
    if (!p)
    {
        va_end(pArgs);
        return 0;
    }
    for ( ; ; )
    {
        if (*pBuf == 0)
        {
            *(p++) = *pBuf;
            break;
        }
        else if (*pBuf == *pSplitChar)
        {
            pBuf++;
            *p = 0;
            p = va_arg(pArgs, char *);
            if (!p || *pBuf == '\0')
            {
                va_end(pArgs);
                return 0;
            }
        }
        else
        {
            *(p++) = *(pBuf++);
        }
    }
    va_end(pArgs);
    return 0;
}

int BatchUnlinkFile( char *pFileList )
{
    char pFile[256];
    char *p = NULL;

    p = StrTok( pFileList, "+" );
    if ( p == NULL )
        return 0;
    else
    {
        sprintf( pFile, "%s/%s", getenv("FILES_DIR"), p );
        unlink( pFile );
    }

    while ( ( p = StrTok( NULL, "+" ) ) != NULL )
    {
        sprintf( pFile, "%s/%s", getenv("FILES_DIR"), p );
        unlink( pFile );
    }

    return 0;
}

int getcols( char *line, char *words[], int maxwords, int delim )
{
    char *p = line, *p2;
    int nwords = 0;
    int append = 0;

    if (line[strlen(line)-1] == delim)
        append = 1;

    while ( *p != '\0' )
    {
        words[nwords++] = p;
        if ( nwords >= maxwords )
            return nwords;

        while(1)
        {
            p2 = strchr( p, delim );
            if ( p2 == NULL )
                break;

            // 如果 delim字符前有斜杠则忽略
            if (p2-1 == NULL || *(p2-1) != '\\')
                break;

            memmove(p2-1, p2, strlen(p2)+1);
            p = p2;
        }
        if (p2 == NULL)
            break;

        *p2 = '\0';
        p = p2 + 1;
    }

    if (append == 1)
    {
        words[nwords] = strdup(" ");
        *(words[nwords]) = 0;
    }

    return nwords + append;
}

int getcols_s( char *line, char *words[], int maxwords, char *delim )
{
    char *p = line, *p2;
    int nwords = 0;
    int slen;
    slen = strlen(delim);

    while ( *p != '\0' )
    {
        words[nwords++] = p;
        if ( nwords >= maxwords )
            return nwords;

        p2 = strstr( p, delim );
        if ( p2 == NULL )
            break;

        p = p2;
        *p2 = '\0';
        p = p2 + slen;
        if (*p == 0x00)
            words[nwords++] = p;
    }

    return nwords;
}

int linkfile( char *file1, char *file2 )
{
    struct stat statbuf1;
    struct stat statbuf2;
    int ret=0;

    if (stat(file1, &statbuf1) != 0)
    {
        err_log("link file %s: %s", file1, strerror(errno));
        return -1;
    }

    /* 判断目标文件是否存在 */
    if( (ret=stat( file2, &statbuf2))==0 )
    {
        if( S_ISDIR(statbuf2.st_mode) )
        {
            err_log("%s is a a directory!", file2);
            return -1;
        }

        // 如果文件相同返回成功
        if (statbuf1.st_dev == statbuf2.st_dev 
                && statbuf1.st_ino == statbuf2.st_ino)
            return 0;

        if( unlink( file2 )<0 )
        {
            err_log("unlink %s failed! %s", file2, strerror(errno));
            return -1;
        }
    } else if( errno != ENOENT )
    {
        err_log("stat() error, %s.", strerror(errno));
        return -1;
    }

    if( link( file1, file2 )<0 )
    {
        err_log("link %s to %s failed, %s.", file1, file2, strerror(errno));
        return -1;
    }

    return 0;
}

//生成信号量 
int getsem(key_t key)
{
    union semun sem; 
    int semid; 

    sem.val = 0; 

    semid = semget( key, 1, IPC_CREAT|IPC_EXCL|0666 );
    if (semid == -1) 
    {
        if (errno == EEXIST)
        {
            semid = semget( key, 1, IPC_CREAT|0666 );
            if (semid == -1)
            {
                err_log("semget( GET ) 失败.");
                return -1;
            }
            else
                return semid;
        }
        else
        {
            err_log("semget( CREAT ) 失败.");
            return -1;
        }
    }

    //初始化信号量 
    semctl( semid, 0, SETVAL, sem ); 
    return semid;
}

//删除信号量 
void d_sem(int semid)
{ 
    union semun sem; 
    sem.val=0; 
    semctl(semid,0,IPC_RMID,0); 
}

//清除共享内存
void d_shm( key_t key )
{
    struct shmid_ds buf; 
    int shmid, semid;

    shmid=shmget( key, 0, 0 );
    semid=getsem(key);

    shmctl(shmid, IPC_RMID, &buf); 
    d_sem(semid); 

    return; 
}

int set_p(int semid)
{ 
    struct sembuf sem_lock[2]= { {0,0,0}, {0,1,SEM_UNDO}}; 
    int ret;

    if ((ret = semop(semid, &sem_lock[0], 2)) == -1)
        err_log("set_p(), errno=[%d]", errno);
    return ret;
}

int set_v(int semid)
{ 
    struct sembuf sops={0, -1, (IPC_NOWAIT|SEM_UNDO)};
    int ret;

    if ((ret = semop(semid, &sops,1)) == -1)
        err_log("set_v(), errno=[%d]", errno);
    return ret;
}

//等待信号量变成 0   
void waitv(int semid)
{
    struct sembuf sops={0,0,0};   
    semop(semid, &sops,1);
}

int SetSHM( key_t key, char *buf, int len )
{
    int shmid,semid; 
    char *shm; 

    shmid = shmget(key, len+1, IPC_CREAT|0666); 
    if( shmid == -1 )
    { 
        err_log("shmget( CREAT ) 失败.");
        return -1;
    } 

    shm = (char *)shmat( shmid, 0, 0 ); 
    if((int)shm == -1)
    { 
        err_log("shmat() 失败.");
        return -1;
    } 

    semid=getsem(key); 
    if ( semid == -1 )
    {
        err_log("getsem() 失败.");
        shmdt(shm); 
        return -1;
    }

    set_p(semid); 
    memcpy( shm, buf, len );
    shm[len] = 0;
    set_v(semid); 

    shmdt(shm); 

    return 0; 
}

char *GetSHM( key_t key )
{
    int shmid,semid;   
    char* shm;   

    shmid = shmget( key, 0, 0 );   
    if(shmid ==- 1)
        return NULL;

    shm = (char* )shmat( shmid, 0, 0 );
    if((int)shm ==- 1)
    {
        err_log("shmat() 失败.");
        return NULL;
    }   
    semid=getsem(key);   
    if ( semid == -1 )
    {
        shmdt(shm);   
        err_log("getsem() 失败.");
        return NULL;
    }

    waitv(semid);

    return shm;
}

int SetSHMFile( char *path )
{
    struct stat buf;
    caddr_t p;
    key_t key;
    int fd;

    if ( ( key = ftok( path, IPC_KEY ) ) == -1 )
    {
        err_log("ftok() 失败.");
        return -1;
    }

    if ( stat( path, &buf ) == -1 )
    {
        err_log("stat() 失败.");
        return -1;
    }
    if ( buf.st_size <= 0 )
    {
        err_log("文件 %s 长度小于零.", path);
        return -1;
    }

    fd = open( path, O_RDONLY, 0777 );
    if ( fd == -1 )
    {
        err_log("打开文件 %s 失败.", path );
        return -1;
    }

    p = mmap( NULL, (int)buf.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    if ( p == (caddr_t)-1 )
    {
        err_log("mmap %s 失败.", path );
        close( fd );
        return -1;
    }

    if ( SetSHM( key, ( char *)p, buf.st_size ) != 0 )
    {
        err_log("SetSHM %s 失败.");
        close( fd );
        munmap( p, buf.st_size );
        return -1;
    }

    close( fd );
    munmap( p, buf.st_size );

    return 0;
}


char *GetSHMFile( char *path )
{
    key_t key;
    char *p = NULL;

    key = ftok( path, IPC_KEY );
    if ( key == -1 )
    {
        err_log("ftok() 失败.");
        return NULL;
    }

    if ( ( p = GetSHM( key ) ) == NULL )
    {
        if ( SetSHMFile( path ) != 0 )
        {
            err_log("SetSHMFile() 失败.");
            return NULL;
        }
        if ( ( p = GetSHM( key ) ) == NULL )
        {
            err_log("GetSHM() 失败.");
            return NULL;
        }
    }

    return p;
}

void FreeSHM( char *shm )
{
    shmdt(shm);   
}

int DelSHMFile( char *path )
{
    key_t key;

    if ( ( key = ftok( path, IPC_KEY ) ) == -1 )
        return -1;

    d_shm( key );

    return 0;
}

int FileCat( char *file1, char *file2 )
{
    char caBuf[2048];
    FILE *fp1;
    FILE *fp2;

    if ((fp1 = fopen( file1, "a" )) == NULL )
        return -1;
    if ((fp2 = fopen( file2, "r" )) == NULL )
    {
        fclose(fp1);
        return -1;
    }

    while( fgets( caBuf, sizeof(caBuf), fp2) != NULL )
        fprintf( fp1, "%s", caBuf );

    fclose(fp2);
    fclose(fp1);
    unlink( file2 );

    return 0;
}

#if 0
void DSP_2_HEX( unsigned char *dsp,  unsigned char *hex, int count )
{
    int i;
    for(i = 0; i < count; i++)
    {
        hex[i]=((dsp[i*2]<=0x39)?dsp[i*2]-0x30:dsp[i*2]-0x41+10);
        hex[i]=hex[i]<<4;
        hex[i]+=((dsp[i*2+1]<=0x39)?dsp[i*2+1]-0x30:dsp[i*2+1]-0x41+10);
    }
} 

void HEX_2_DSP( unsigned char *hex,  unsigned char *dsp, int count)
{
    int i;
    char ch;
    for(i = 0; i < count; i++)
    {
        ch=(hex[i]&0xf0)>>4;
        dsp[i*2]=(ch>9)?ch+0x41-10:ch+0x30;
        ch=hex[i]&0xf;
        dsp[i*2+1]=(ch>9)?ch+0x41-10:ch+0x30;
    }
}
#endif

//检查指定的年份是否为润年 返回---正确OK:1;不正确:0;
int IsLeapYear(int year)
{
    if ((year%400)==0)
        return 1;
    if ((year%100)==0)
        return 0;
    if ((year%4)==0)
        return 1;
    else
        return 0;
}

//检查指定长度的字符串是否全为数字符 返回---正确OK:1;不正确:0;
int IsNumeric(char *str, int len)
{
    int i;

    for (i=0; i<len; i++)
    {
        if (str[i]<'0' || str[i]>'9')
            return 0;
    }
    return 1;
}

//日期字符串的合法性检查 返回---正确OK:1;不正确:0;
/*
   int IsDate(char *sDate)
   {
   int yyyy, mm, dd;

   if (strlen(sDate) != 8)
   return 0;
   if (IsNumeric(sDate, 8) == 0)
   return 0;

   yyyy = atoi(sDate)/10000;
   mm = atoi(sDate+4)/100;
   dd = atoi(sDate+6);

   switch(mm)
   {
   case 1:
   case 3:
   case 5:
   case 7:
   case 8:
   case 10:
   case 12:
   if (dd>31)
   return 0;
   break;
   case 4:
   case 6:
   case 9:
   case 11:
   if (dd>30)
   return 0;
   break;
   case 2:
   if (!IsLeapYear(yyyy))
   {
   if (dd>28)
   return 0;
   }
   else
   {
   if (dd>29)
   return 0;
   }
   break;
   default:
   return 0;
   }

   if (dd<=0)
   return 0;

   return 1;
   }
   */

//时间字符串的合法性检查 返回---正确OK:1;不正确:0;
int IsTime(char *sTime)
{
    int hh, mm, ss;

    if (strlen(sTime) != 6)
        return 0;
    if (IsNumeric(sTime, 6) == 0)
        return 0;

    hh = atoi(sTime)/10000;
    mm = atoi(sTime+2)/100;
    ss = atoi(sTime+4);
    if (hh>=24 || mm>=60 || ss>=60)
        return 0;
    else
        return 1;
}

//与系统函数sscanf(...)类似的功能,但*pFmt参数只能是"%ns...%ns"形式.
//例如:Sscanf("1234567890", "%1s%2s%3s%4s", s1, s2, s3, s4);
//结果:s1="1", s2="23", s3="456", s4="7890";
//和sscanf(...) 函数不同的是参数s1, s2, ... 所获得的长度完全的由*pFmt决定,
//不受*pBuf中的字符串结束符和空格等影响.
int Sscanf(char *pBuf, const char *pFmt, ...)
{
    va_list pArgs;
    char *p, pFieldLen[6+1];
    int iFieldLen, iNumLen = 0;

    va_start(pArgs, pFmt);
    for ( ; *pFmt; pFmt++ )
    {
        switch (*pFmt)
        {
            case '%':
                iNumLen = 0;
                break;
            case 's':
                pFieldLen[iNumLen++] = 0;
                iFieldLen = atoi(pFieldLen);
                p = va_arg(pArgs, char *);
                if (!p)
                {
                    va_end(pArgs);
                    return 0;
                }
                memcpy(p, pBuf, iFieldLen);
                p[iFieldLen] = 0;
                pBuf += iFieldLen;
                break;
            default:
                pFieldLen[iNumLen++] = *pFmt;
                if (iNumLen > 6)
                {
                    va_end(pArgs);
                    return -1;
                }
                break;
        }
    }
    va_end(pArgs);
    return 0;
}

//日期型数据格式转换 YYYYMMDD ---> YYYY/MM/DD
char* DateS2L(char *sDate)
{
    char tmpbuf[10+1];

    memcpy(tmpbuf, sDate, 4);
    memcpy(tmpbuf + 5, sDate + 4, 2);
    memcpy(tmpbuf + 8, sDate + 6, 2);
    tmpbuf[4] = '/';
    tmpbuf[7] = '/';
    tmpbuf[10] = '\0';

    strcpy(sDate, tmpbuf);
    return sDate;
}

//时间型数据格式转换 HHMMSS ---> HH:MM:SS
char* TimeS2L(char *sTime)
{
    char tmpbuf[8+1];

    memcpy(tmpbuf, sTime, 2);
    memcpy(tmpbuf + 3, sTime + 2, 2);
    memcpy(tmpbuf + 6, sTime + 4, 2);
    tmpbuf[2] = ':';
    tmpbuf[5] = ':';
    tmpbuf[8] = '\0';

    strcpy(sTime, tmpbuf);
    return sTime;
}

//日期型数据格式转换 YYYY/MM/DD ---> YYYYMMDD
char* DateL2S(char *sDate)
{
    char tmpbuf[8+1];

    memcpy(tmpbuf, sDate, 4);
    memcpy(tmpbuf + 4, sDate + 5, 2);
    memcpy(tmpbuf + 6, sDate + 8, 2);
    tmpbuf[8] = '\0';

    strcpy(sDate, tmpbuf);
    return sDate;
}

//时间型数据格式转换 HH:MM:SS ---> HHMMSS
char* TimeL2S(char *sTime)
{
    char tmpbuf[6+1];

    memcpy(tmpbuf, sTime, 2);
    memcpy(tmpbuf + 2, sTime + 3, 2);
    memcpy(tmpbuf + 4, sTime + 6, 2);
    tmpbuf[6] = '\0';

    strcpy(sTime, tmpbuf);
    return sTime;
}

//将数字i转化为len长度的字符串,并填充到str内存中去.
void I2M(char *str, int i, int len)
{
    char strbuf[257];

    memset(strbuf, 0, sizeof(strbuf));
    sprintf(strbuf, "%.*d", len, i);
    memcpy(str, strbuf, len);
}

//将str所指向的内存中len长度的字符串转化为数字返回.
int M2I(char *str, int len)
{
    char strbuf[257];

    memcpy(strbuf, str, len);
    strbuf[len] = 0;

    return atoi(strbuf);
} 

//比较两个字符串,忽略大小写
int StrICmp(char *str1, char *str2)
{
    int i, len;

    len = strlen(str1);
    for(i = 0; i < len; i++)
        if(str1[i] >= 'a' && str1[i] <= 'z')
            str1[i] -= 32;

    len = strlen(str2);
    for(i = 0; i < len; i++)
        if(str2[i] >= 'a' && str2[i] <= 'z')
            str2[i] -= 32;

    return strcmp(str1, str2);
}

//去掉右边的空格.
char *TrimRight(char *str)
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
    return (str);
}

//去掉左边的空格.
char *TrimLeft(char *str)
{
    int i, len;

    len = strlen(str);
    for (i=0; i < len; i++)
    {
        if ((str[i] != ' ') && (str[i] != '\t'))
            break;
    }

    if (i>0)
        strcpy(str, str + i);

    return (str);
}

//去掉左右两边的空格.
char *TrimAll(char *str)
{
    TrimRight(str);
    TrimLeft(str);
    return (str);
}

#if 0
char *FormatMoney(char *str)
{
    char moneystr[100];
    int l,d,r;
    char *p_str;
    char appendix[4];
    char string[100];
    static char rets[20][100];
    static int kd = 0;

    kd++;
    if(kd == 20) kd = 0;

    memset(moneystr, 0, sizeof(moneystr));

    TrimAll(str);
    l = strlen(str);
    if(l == 0 || l > 60) return NULL;

    if(str[0]=='-'||str[0]=='+')
        strcpy(string,&str[1]);
    else
        strcpy(string,str);

    p_str = strchr(string,'.');

    if (p_str == NULL)
        l=strlen(string);
    else
        l=strlen(string)-strlen(p_str);

    r=l/3; d=l%3;

    if(d==0){ r--; d=3; }

    strncpy(moneystr,string,d);

    for(;r>0;r--)
    {
        strcat(moneystr,",");
        strncat(moneystr,string + d,3);
        d+=3;
    }
    if (p_str != NULL)
    {
        memset(appendix, 0, sizeof(appendix));
        if(strlen(p_str) < 3)
        {
            appendix[0] = '.';
            appendix[1] = p_str[1];
            appendix[2] = '0';
        }
        else
            memcpy(appendix, p_str, 3);
        strcat(moneystr,appendix);
    }
    else
        strcat(moneystr,".00");

    memset(rets[kd], 0, 100);
    if(str[0]=='-'||str[0]=='+')
    {
        rets[kd][0] = str[0];
        strcpy(rets[kd] + 1,moneystr);
    }
    else
        strcpy(rets[kd],moneystr);

    return rets[kd];
}
#endif

int FindOneLine(int ColNo,char *SeaStr,char *SeaFile,char *LineStr)
{
    FILE *fp;
    char str[RECLEN+1];
    char idxstr[RECLEN+1];
    char tmpstr[RECLEN+1];
    int ret=0;
    int i;

    if((fp=fopen(SeaFile,"r"))==NULL) return 0;

    TrimAll(SeaStr);
    while (!feof(fp))
    {
        if(fgets(str,RECLEN,fp)==NULL) {fclose(fp);return ret;}
        strcpy(tmpstr,str);
        strcpy(idxstr,StrTok(tmpstr,"|"));
        for(i=2;i<=ColNo;i++)
            strcpy(idxstr,StrTok(NULL,"|"));
        TrimAll(idxstr);
        if (strcmp(SeaStr,idxstr)==0) 
        {
            ret= 1;
            strcpy(LineStr,str);
            break;
        }
    }
    fclose(fp);
    return ret;
}

int UpdateAcctInfo(char *AcctNo,char *AcctName,char *FileName)
{
    FILE *fp;
    char str[RECLEN+1];
    char idxstr[RECLEN+1];
    char tmpstr[RECLEN+1];
    int k;
    int breakflag=0;

    if((fp=fopen(FileName,"r+"))==NULL) 
        if((fp=fopen(FileName,"w"))==NULL) return 0;

    k=strlen(AcctNo);
    while (!feof(fp))
    {
        if(fgets(str,RECLEN,fp)==NULL)
        {
            fseek(fp,0L,SEEK_END);
            break;
        }
        strcpy(tmpstr,str);
        strcpy(idxstr,StrTok(tmpstr,"|"));
        TrimAll(idxstr);
        if (strcmp(AcctNo,idxstr)==0) 
        {
            fseek(fp,-115L,SEEK_CUR);
            breakflag=1;
            break;
        }
        if ( breakflag==1)
            break;
    }
    fprintf(fp,"%32s|%80s|\n",AcctNo,AcctName);
    fclose(fp);
    return 0;
}

//Amount("1234567")="12,345.67"		注意: 生成字符串的最大长度为50
char * Amount(char *sAmountStr, char *sNumericStr)
{
    char tempstr[50+2];
    int i, j, len;

    memset(tempstr, 0, sizeof(tempstr));
    len=strlen(sNumericStr);
    if (len <= 2)
        sprintf(sAmountStr, "0.%2s", sNumericStr);
    else
    {
        len--;
        j=50;
        tempstr[j--]=sNumericStr[len--];
        tempstr[j--]=sNumericStr[len--];
        tempstr[j--]='.';
        i=0;
        do
        {
            if (i > 0 && i%3 == 0)
                tempstr[j--]=',';
            tempstr[j--]=sNumericStr[len--];
            i++;
        } while (len >= 0);
        strcpy(sAmountStr, tempstr + ++j);
    }
    return sAmountStr;
}

// 金额大小写转换
// CapAmount(sCapAmountStr, "1234567")
//sCapAmountStr="壹万贰仟叁佰肆拾伍圆零陆角柒分"
// 注意: 生成字符串的最大长度为128个汉字
char * CapAmount(char *sCapAmountStr, char *sNumericStr)
{
    double money;
    int len, zerotype, i, unit_num, allzero=1;
    char fundstr[51];
    char *numberchar[] =
    {
        "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"
    };
    char *rmbchar[] =
    {
        "分", "角", "", "圆", "拾", "佰", "仟", "万", "拾", "佰", "仟", "亿", "拾", "佰", "仟"
    };

    money=atof(sNumericStr)/100.00;

    sprintf(fundstr, "%.2lf", money);
    len = strlen(fundstr);
    unit_num = sizeof(rmbchar) / sizeof(rmbchar[0]);

    for (i=zerotype=0, sCapAmountStr[0]='\0'; i < len; i++)
    {
        switch (fundstr[i])
        {
            case '-':
                strcat(sCapAmountStr, "负");
                break;
            case '.':
                if (sCapAmountStr[0] == '\0')
                {
                    strcat(sCapAmountStr, numberchar[0]);
                }
                if (zerotype == 1)
                {
                    strcat(sCapAmountStr, rmbchar[3]);
                }
                zerotype = 0;
                break;
            case '0':
                if ((len-i) == 12 && 11 < unit_num)
                {
                    strcat(sCapAmountStr, rmbchar[11]);
                }
                if ((len-i) == 8 && 7 < unit_num)
                {
                    if (!allzero)
                        strcat(sCapAmountStr, rmbchar[7]);
                }
                zerotype = 1;
                break;
            default:
                if ((len - i) < 12)
                    allzero = 0 ;
                if (zerotype == 1)
                {
                    strcat(sCapAmountStr, numberchar[0]);
                }
                strcat(sCapAmountStr, numberchar[fundstr[i] - '0']);
                if ((len-i-1) < unit_num)
                {
                    strcat(sCapAmountStr, rmbchar[len-i-1]);
                }
                zerotype = 0;
                break;
        }
    }

    if (memcmp(fundstr+len-2, "00", 2) == 0)
        strcat(sCapAmountStr, "整");
    return sCapAmountStr;
}

//对配置文件的字符串进行处理, 用空格代替中括号和尖括号外的字符串
//str 是待处理的字符串,
//iInBracket说明是否在括号内iInBracket=0,不是;1,中括号内;2,尖括号内
int ProcPrintLine(char * str)
{
    int i=0, j=0;
    int iInBracket=0;
    char pTmp[1024];

    memset(pTmp, 0, sizeof(pTmp));
    if (str[0] == '@')
        return 0;

    for (i=0; i < strlen(str); i++)
    {
        if ((str[i] == '[') && (iInBracket == 0))
        {
            iInBracket=1;
            pTmp[j++]=str[i];
            continue;
        }
        else if ((str[i] == '<') && (iInBracket == 0))
        {
            iInBracket=2;
            continue;
        }
        else if ((str[i] == '>') && (iInBracket == 2))
        {
            iInBracket=0;
            continue;
        }

        if ((iInBracket == 0) && (str[i] != '<'))
        {
            pTmp[j++]=' ';
            continue;
        }

        if ((str[i] == ']') && (iInBracket == 1))
        {
            iInBracket=0;
        }
        pTmp[j++]=str[i];
    }
    pTmp[strlen(pTmp)]='\n';
    strcpy(str, pTmp);
    return 0;
}

char *IntToStr( int i )
{
    static char buf[20];

    sprintf( buf, "%d", i );

    return buf;
}

char *ChineseDate(long curr_date)
{
    char buf[20];

    sprintf(buf, "%04ld年%02ld月%02ld日", 
            curr_date/10000, curr_date%10000/100, curr_date%100);

    return strdup(buf);
}

ssize_t readln(int fd, void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nread;
    char    *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return(-1);
        } else if (nread == 0)
            break;

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);
}
ssize_t writeln(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

size_t full_write(int fd, const void *buf, size_t count)
{
    size_t total = 0;
    const char *ptr = (const char *) buf;

    while (count > 0)
    {
        size_t n_rw = writeln(fd, ptr, count);
        if (n_rw == (size_t) -1)
            break;
        if (n_rw == 0)
        {
            errno = ENOSPC;
            break;
        }
        total += n_rw;
        ptr += n_rw;
        count -= n_rw;
    }

    return total;
}

size_t full_read(int fd, const void *buf, size_t count)
{
    size_t total = 0;
    const char *ptr = (const char *) buf;

    while (count > 0)
    {
        size_t n_rw = readln(fd, (void *)ptr, count);
        if (n_rw == (size_t) -1)
            break;
        if (n_rw == 0)
        {
            errno = 0;
            break;
        }
        total += n_rw;
        ptr += n_rw;
        count -= n_rw;
    }

    return total;
}

int cat_file(char *file)
{
    char buf[1024];
    size_t n_read;
    int fd, i;

    fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        err_log("Open file %s fail.", file);
        return -1;
    }

    for (;;)
    {
        n_read = readln (fd, buf, sizeof(buf));
        if (n_read < 0)
        {
            close(fd);
            return -1;
        }

        if (n_read == 0)
        {
            close(fd);
            return 0;
        }

        for (i = 0; i < n_read; i++)
            putchar(buf[i]);
        /*
           if (full_write (STDOUT_FILENO, buf, n_read) != n_read)
           {
           close(fd);
           return -1;
           }
           */
    }
}

char *curtail(char *string, char *match)
{
    char *locate;

    if((locate = strstr(string, match)) != NULL) 
        *locate = '\0';

    return string;
}
char *curhead(char *string, char *match)
{
    char *locate;

    if((locate = strstr(string, match)) != NULL) 
        memmove(string, locate+strlen(match), strlen(locate)-strlen(match)+1);
    return string;
}

//按GB18030标准 对字符进行完整性检查
//单字节，其值从0到0x7F 
//双字节，第一个字节的值从0x81到0xFE，第二个字节的值从0x40到0xFE（不包括0x7F） 
//四字节，第一个字节的值从0x81到0xFE，第二个字节的值从0x30到0x39，第三个字节的值从0x81到0xFE，第四个字节的值从0x30到0x39。 
int CheckHZ(unsigned char *data_buf, int len)
{
    int i,j;

    for(i=0,j=0; i<len; j=i)
    {
        if(data_buf[j] <= 0x7F)
        {
            i++;
            continue; //单字节
        }
        else if( ( data_buf[j] >= 0x81 ) && (data_buf[j] <=0xFE) )
        {
            i++;
            if(( data_buf[j+1] >= 0x40)
                    && (data_buf[j+1] <=0xFE)
                    && (data_buf[j+1] !=0x7F))
            {
                i++;
                continue; //双字节汉字
            }

            else if( (data_buf[j+1] >= 0x30) && (data_buf[j+1] <=0x39))
            {
                i++;
                if((data_buf[j+2] >=0x81) && (data_buf[j+2] <=0xFE))
                {
                    i++;
                    if((data_buf[j+3] >=0x30) &&(data_buf[j+3] <=0x39))
                    {
                        i++;
                        continue; //四字节汉字
                    }
                    else
                    {
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }

            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int FileSame(char *path1, char *path2)
{
    struct stat buf1, buf2;

    stat(path1, &buf1);
    stat(path1, &buf2);

    if (buf1.st_dev == buf2.st_dev && buf1.st_ino == buf2.st_ino)
        return 0;

    return -1;
}

// 计数器
int timecost_reset()
{
    gettimeofday(&tv_start, NULL);
    return 0;
}

double timecost_count()
{
    struct timeval tv_now;
    int seconds, milliseconds;

    gettimeofday(&tv_now, NULL);
    seconds = tv_now.tv_sec - tv_start.tv_sec;

    if((milliseconds = (tv_now.tv_usec - tv_start.tv_usec) / 1000) < 0) {
        milliseconds += 1000;
        --seconds;
    }

    return (double)seconds + (double)milliseconds / 1000.0;
}

void timecost_log(char *fmt, ...)
{
    va_list ap;
    char buf[1024];
    char *flag;

    if ((flag = getenv("PERFORMANCE_LOG")) == NULL)
        return;
    if (*flag != '1')
        return;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    WriteLogDebug("timecost", NULL, 0, "%-30s\t%.3f s", buf, timecost_count());
    return;
}

int fill_char_l(char *s, int len, int c)
{
    int count = len - strlen(s);

    if (count <= 0)
        return count;

    memmove(s + count, s, strlen(s));
    memset(s, c, count);

    return count;
}

int fill_char_r(char *s, int len, int c)
{
    int count = len - strlen(s);

    if (count <= 0)
        return count;

    memset(s+strlen(s), c, count);
    return count;
}

int copy_file( char *path, char *newpath )
{
    struct stat buf;
    caddr_t p;
    int fd, newfd;

    if ( stat( path, &buf ) == -1 )
    {
        err_log("stat() 失败.");
        return -1;
    }
    if ( buf.st_size <= 0 )
    {
        err_log("文件 %s 长度小于零.", path);
        return -1;
    }

    fd = open( path, O_RDONLY );
    if ( fd == -1 )
    {
        err_log("打开文件 %s 失败.", path);
        return -1;
    }
    newfd = open( newpath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if ( newfd == -1 )
    {
        err_log("创建文件 %s 失败.", newpath);
        close(fd);
        return -1;
    }

    p = mmap( NULL, (int)buf.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    if ( p == (caddr_t)-1 )
    {
        err_log("mmap %s 失败.", path);
        close(fd);
        close(newfd);
        return -1;
    }

    if (full_write(newfd, p, buf.st_size) != buf.st_size)
    {
        err_log("write %s 失败.", newpath);
        close( fd );
        close( newfd );
        return -1;
    }

    close( newfd );
    close( fd );
    munmap( p, buf.st_size );

    return 0;
}

int CheckGB18030(char *buf, int len)
{
    char *conv = NULL;
    char *p = NULL;

    conv = strndup(buf, len);
    if (conv == NULL)
        return -1;

    if ((p = (char *)encoding_conv(conv, "GB18030", "UTF-8")) == NULL)
    {
        free(conv);
        return -1;
    }

    free(conv);
    free(p);
    return 0;
}

void split_2str(char *buf, int size, char *str1, char *str2)
{
    int len;

    *str1 = 0x00;
    *str2 = 0x00;
    len = strlen(buf);
    if (len <= size)
    {
        strncpy(str1, buf, len);
        *(str1 + len) = 0x00;
    }
    else
    {
        if (CheckGB18030(buf, size) != 0)
        {
            strncpy(str1, buf, size-1);
            *(str1 + size - 1) = 0x00;
            strncpy(str2, buf+size-1, len-size+1);
            *(str2 + len - size + 1) = 0x00;
        }
        else
        {
            strncpy(str1, buf, size);
            *(str1 + size) = 0x00;
            strncpy(str2, buf+size, len-size);
            *(str2 + len - size) = 0x00;
        }
    }
    return;
}

char *get_file_prefix(char *filename)
{
    char *p1 = basename(filename);
    char *p2;

    if ((p2 = strchr(p1, '.')) != NULL)
        *p2 = 0x00;
    return p1;
}

void ifree(char *p)
{
    if (p != NULL)
    {
        free(p); p = NULL;
    }
}
