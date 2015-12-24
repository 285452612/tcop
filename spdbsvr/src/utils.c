#include <stdio.h>
#include <string.h>
#include  <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include "utils.h"

static char empty_char[] = {0x00};

/*
 * 压缩字符串左端的空格和制表字符
 */
char *ltrim(char *string)
{
    char *start_pos = string;

    while((*start_pos == ' ' || *start_pos == '\t') && ++start_pos);
    return memmove(string, start_pos, strlen(start_pos) + 1);
}

/*
 * 压缩字符串右端的空格和制表字符
 */
char *rtrim(char *string)
{
    register int offset;

    offset = strlen(string) - 1;

    while(offset >= 0 && (string[offset] == ' ' || string[offset] == '\t')) {
        string[offset--] = '\0';
    }

    return string;
}

char *lrtrim(char *string)
{
    return ltrim(rtrim(string));
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

int str2int(char *str, int size)
{
    char buf[20];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, str, (size > 19 ? 19 : size));
    return atoi(buf);
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
        words[nwords] = empty_char;

    return nwords + append;
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

char *CalDate(char* date1, int dd)
{
    time_t time1;
    struct tm tm_time, *t;
    int day, month, year;

    memset(&tm_time, 0, sizeof(tm_time));

    year = atol(date1)/10000;
    month = atol(date1)%10000/100;
    day = atol(date1)%100;

    tm_time.tm_year = year - 1900;
    tm_time.tm_mon = month - 1;
    tm_time.tm_mday = day;
    time1 = mktime((struct tm*)&tm_time) + dd*24*60*60;

    t = localtime(&time1);
    strftime(date1, 9, "%Y%m%d", t);

    return date1;
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

int ExistFile(char *s)
{
	int fd;

	if((fd=open(s, O_RDONLY)) < 0)
		return -1;

	close(fd);

	return 0;
}

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
        fprintf(fp, "%s, %d|" , file, line);
    iRc = vfprintf(fp, pFmt, ap);
    fprintf(fp, "\n");
    va_end(ap);

    if (fp != stderr)
        fclose(fp);

    return 0;
}

void pid_check(const char *pid_file)
{
    struct flock pid_lock;
    char buf[20];
    int pid_fd;
    int pid_tmp;

    pid_lock.l_type = F_WRLCK;
    pid_lock.l_start = 0;
    pid_lock.l_whence = SEEK_SET;
    pid_lock.l_len = 0;

    if((pid_fd = open(pid_file, O_WRONLY|O_CREAT, 0644)) < 0)
    {
        fprintf(stderr, "Failure to open the pid file(%s).", pid_file);
        exit(-255);
    }

    if(fcntl(pid_fd, F_SETLK, &pid_lock) < 0)
    {
        if((errno == EACCES) || (errno == EAGAIN))
        {
            fprintf(stderr, "程序已经运行. 请查看 PID 文件:%s.", pid_file);
            exit(0);
        }
        else
        {
            fprintf(stderr, "Fcntl error in pid_check 1.");
            exit(-253);
        }
    }

    if(ftruncate(pid_fd, 0) < 0)
    {
        fprintf(stderr, "ftruncate error in pid_check 2.");
        exit(-253);
    }

    sprintf(buf, "%ld", (long)getpid());
    if (write(pid_fd, buf, strlen(buf)) != strlen(buf))
    {
        fprintf(stderr,"write error 1.");
        exit(-253);
    }

    if((pid_tmp = fcntl(pid_fd, F_GETFD, 0)) < 0)
    {
        fprintf(stderr,"fcntl error 2.");
        exit(-253);
    }
    pid_tmp |= FD_CLOEXEC;
    if(fcntl(pid_fd, F_SETFD, pid_tmp) < 0)
    {
        fprintf(stderr,"fcntl error 3.");
        exit(-253);
    }
    return;
}

char *getAppDir()
{
    char *p = NULL;
    if ((p = getenv("GMT_APP_DIR")) == NULL)
        if ((p = getenv("APP_DIR")) == NULL)
            return getenv("HOME");
    return p;
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
