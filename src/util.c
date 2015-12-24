#include "interface.h"
#include "util.h"

#include <unistd.h>
#include <time.h>

char *getCHSDate(char *buf, long curr_date)
{
    sprintf(buf, "%04ld年%02ld月%02ld日", curr_date/10000, curr_date%10000/100, curr_date%100);
    return buf;
}

char *getDate(char sep)
{
    struct tm *pt;
    time_t now;
    static char buf[12] = {0};

    time(&now);
    pt = localtime(&now);
    if (sep != 0)
        sprintf(buf, "%04d%c%02d%c%02d", 1900 + pt->tm_year, sep, pt->tm_mon + 1, sep, pt->tm_mday);
    else
        sprintf(buf, "%04d%02d%02d", 1900 + pt->tm_year, pt->tm_mon + 1, pt->tm_mday);
    return buf;
}

char *getTime(char sep)
{
    struct tm *pt;
    time_t now;
    static char buf[12] = {0};

    time(&now);
    pt = localtime(&now);
    if (sep != 0)
        sprintf(buf, "%02d%c%02d%c%02d", pt->tm_hour, sep, pt->tm_min, sep, pt->tm_sec);
    else
        sprintf(buf, "%02d%02d%02d", pt->tm_hour, pt->tm_min, pt->tm_sec);
    return buf;
}

static void GetDateAndTime(char *cur_date, char *cur_time)
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

//日期十位转八位
char *DateDTO(const char *DateBuff)
{
    static char dst[10] = {0};

    memset(dst, 0, sizeof(dst));
    memcpy(dst, DateBuff, 4);
    memcpy(dst+4, DateBuff+5, 2);
    memcpy(dst+6, DateBuff+8, 2);
    dst[8] = 0;

    return dst;
}

//8位转10位日期格式
char *DateOTD(char *DateBuff)
{
    static char tmp[11] = {0};

    snprintf(tmp, sizeof(tmp), "%4.4s/%2.2s/%2.2s", DateBuff, DateBuff+4, DateBuff+6);

    return tmp;
}

//日期六位转十位
char *DateSTD(char *DateBuff)
{
    static char tmp[11] = {0};

    memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, "20");
    memcpy(tmp+2, DateBuff, 6);
    DateOTD(tmp);

    return tmp;
}

//时间六位转八位
char *TimeSTO(char *time)
{
    static char dst[10] = {0};

    memset(dst, 0, sizeof(dst));
    snprintf(dst, sizeof(dst), "%2.2s:%2.2s:%2.2s", time, time+2, time+4);

    return dst;
}

//时间八位转六位
char *TimeOTS(char *time)
{
    static char dst[10] = {0};

    memset(dst, 0, sizeof(dst));
    memcpy(dst, time, 2);
    memcpy(dst+2, time+3, 2);
    memcpy(dst+4, time+6, 2);
    dst[6] = 0;

    return dst;
}

//分转成元
char *AmtFTY(char *dst, const char *amt)
{
    char buff[32] = {0};

    strcpy(buff, amt);
    sprintf(dst, "%.2lf", atof(buff)/100);
    return dst;
}

//元转成分
char *AmtYTF(char *dst, const char *amt)
{
    char buff[32] = {0};

    strcpy(buff, amt);
    sprintf(dst, "%.0lf", atof(buff)*100);
    return dst;
}

char *convertAmount(char *src)
{
    static char dst[48] = {0};

    memset(dst, 0, sizeof(dst));
    if (strchr(src, '.') != NULL)
        return AmtYTF(dst, src);

    return AmtFTY(dst, src);
}

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
    
    all_trim(str);
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
    } else
        strcpy(rets[kd],moneystr);

    return rets[kd];
}

char *convertDate(char *src)
{
    if (strlen(src) == 8)
        return DateOTD(src);
    else if (strlen(src) == 10)
        return DateDTO(src);
    return src;
}

char *convertTime(char *src)
{
    if (strlen(src) == 6)
        return TimeSTO(src);
    else if (strlen(src) == 8)
        return TimeOTS(src);
    return src;
}

char *vstrcat(const char *fmt, ...)
{
    static char buf[4096] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    return buf;
}

char *getFilesdirFile(char *pFileName)
{
    char buf[256];

    sprintf(buf, "%s/tmpXXXXXX", getenv("FILES_DIR") );
    if (mkstemp(buf) == -1)
        return NULL;
    strcpy(pFileName, buf);

    return pFileName;
}

void opLoger(char *homedir, char *logname, char *file, int line, char *fmt, ...)
{
    char logfile[128] = {0};
    FILE *fp = NULL;
    static char buf[400*1024] = {0};
    char date[9], time[7];
    va_list ap;

    GetDateAndTime(date, time);
    sprintf(logfile, "%s/log/%s_%s.log", homedir, logname, date);
    if (NULL == (fp = fopen(logfile, "a"))) {
        sprintf(logfile, "%s/%s_%s.log", getenv("HOME"), logname, date);
        if ((fp = fopen(logfile, "a")) == NULL)
        {
            fprintf(stderr, "打开日志文件[%s]失败\n", logfile);
            return;
        }
    }

    if (file == NULL)
        fprintf(fp, "%s:%ld>\n", time, getpid());
    else 
    {
        va_start(ap, fmt);
        vsprintf(buf, fmt, ap);
        va_end(ap);
        fprintf(fp, "%s:%ld> %s  <%s|%d\n", time, getpid(), buf, file, line);
    }
    fclose(fp);
}

void opFixedLoger(char *homedir, char *logname, char *file, int line, char *buf, int len)
{
    char logfile[128] = {0};
    FILE *fp = NULL;
    char date[9], time[7];
    static char tmp[10*1024] = {0};

    GetDateAndTime(date, time);
    sprintf(logfile, "%s/log/%s_%s.log", homedir, logname, date);
    if (NULL == (fp = fopen(logfile, "a"))) {
        sprintf(logfile, "%s/%s_%s.log", getenv("HOME"), logname, date);
        if ((fp = fopen(logfile, "a")) == NULL)
        {
            fprintf(stderr, "打开日志文件[%s]失败\n", logfile);
            return;
        }
    }

    if (file == NULL)
        fprintf(fp, "%s:%ld>\n", time, getpid());
    else {
        int i = 0;
        memcpy(tmp, buf, len);
        for (i = 0; i < len; i++)
            if (tmp[i] == 0) tmp[i] = ' ';
        tmp[len] = 0;
        fprintf(fp, "%s:%ld>[%s]  <%s|%d\n", time, getpid(), tmp, file, line);
    }
    fclose(fp);
}

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
