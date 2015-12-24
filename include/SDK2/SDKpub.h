/**************************************
    SDKpub.h
    SUNLAN
    2005/01/25
**************************************/

#ifndef _SDKPUB_H
#define _SDKPUB_H

#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include "SDKbool.h"

#ifndef PATH_MAX
#ifdef _POSIX_PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#else
#define PATH_MAX 255
#endif
#endif

char *BaseName( const char* );

#ifndef THISFILE
#define THISFILE    BaseName(__FILE__)
#endif

#ifndef LINE_MAX
#define LINE_MAX        2048
#endif

#ifndef _MIN
#define _MIN(a,b)( (a)<(b)? (a):(b) )
#endif

#ifndef _MAX
#define _MAX(a,b)( (a)>(b)? (a):(b) )
#endif

#ifndef THISFILE
#define THISFILE    basename(__FILE__)
#endif

char *ltoa( long l );
/************************************************
 说明: 将整型值以转换为字符串输出
************************************************/

char *ftoa( double f, int p );
/************************************************
 说明: 将浮点型值以转换为字符串输出
 参数:
    f 浮点数
    p 精度
************************************************/

char *r_trim( char *source );

char *l_trim( char *source );

char *all_trim( char *source );

bool isblankstring( const char *s );
/******************************************************************
bool isblankstring( const char *s );
用于判断字符串s是否为空白串.
如一个字符串中仅包含以下字符时被认为是空白串: 空格、制表符、换行
******************************************************************/

int getdata( const char *buff, char sep, int posit, char *dest, int max );

unsigned int count_char( const char *buf, char chr );

#ifdef SCO

int strcasecmp(const char *s1, const char *s2);

int strncasecmp(const char *s1, const char *s2, size_t n); 

#endif  /* SCO */

#ifndef DATELEN_MAX
#define DATELEN_MAX         64
#endif

long current_date();
/*********************************************
long current_date();
用途:取当前日期.
返回日期格式:YYYYMMDD
*********************************************/

long current_time();
/*********************************************
long current_time();
用途:取当前时间.
返回时间格式:hhmmss
*********************************************/

bool IsValidDate( const char *sDate, const char *fmt );
/*********************************************
bool IsValidDate( const char *sDate, const char *fmt );
用途:判断日期字符串sDate是否是符合fmt格式要求的有效日期.
    日期中的年,月,日分别以%Y,%m,%d表示.缺省的日期格式为%Y%m%d
    日期的格式可自定义,例如:"%Y 年 %m 月 %d 日"
返回:
    当sDate为有效日期时返回TRUE,否则返回FALSE
*********************************************/

#define ValidDate( s )    IsValidDate( s, "%Y%m%d" )

bool IsValidTime( const char *sTime );
/*********************************************
bool IsValidTime( const char *sTime );
用途:判断时间是否有效.时间的格式为hhmmss.例如上午8点15分24秒表示为081524
返回:
    当sTime为有效时间时返回TRUE,否则返回FALSE
*********************************************/

int diffday( const char* day1, const char* day2, const char* fmt );
/*****************************************************************************
int diffday( const char* day1, const char* day2, const char* fmt );
用途:判断两个相同格式的日期间相差的天数.
返回:
    在日期格式正确时,如两个日期相同,返回0;否则返回相差的天数,但不区分日期的
    先后.
    在日期无效时,函数返回-1
*****************************************************************************/

int dateconv( const char *s, const char *fmt_s, char *d, const char *fmt_d );
/*****************************************************************************
int dateconv( const char *s, const char *fmt_s, char *d, const char *fmt_d );
用途:将格式为fmt_s的时间s转换为格式为fmt_d的字符串d.
返回:
    转换成功时返回0,否则返回-1
*****************************************************************************/

char *daysafter( const char *date, const char *format, int n );
/*****************************************************************************
char *daysafter( const char *date, const char *format, int n );
用途:计算格式为format的字符串date表示的日期n天后的日期,并以相同的格式返回.
    在计算之前的日期时,n以负数表示.例如-3表示3天前的日期
返回:
    如date为符合format的有效日期,返回格式为format的日期串;否则返回NULL
*****************************************************************************/


int DEC_BCD( int d, unsigned char *b, int w );
/*************************************************
    Convert data format from decimal into BCD
    parameter:
        d: decimal value
        b: BCD
        w: max_size that b can accept
    return:
        0 if success.
*************************************************/

void BCD_DEC( unsigned char *b, int *d );
/*************************************************
    Convert data format from BCD to decimal
    parameters:
*************************************************/

int ChkPath( const char *path, int flag, mode_t mode );
/******************************************
    flag:
      0 return error when directory not exists
      1 create a new directory
    mode:
        path mode. active only flag is 1
    return code:
      0 success
      <0    error
******************************************/

int cpfile( char *src, char *dest );
/****************************************************
将源文件src拷贝为目标文件dest,并复制源文件的属性
****************************************************/

int fileline( char *path );
/**********************************************************
fileline()用于计算一个文本文件中存在的行数.功能类似于wc命令
在成功时返回文件中的记录行数,否则返回-1
***********************************************************/

char *logfname( const char *prefix );
/**********************************************************
 char *logfname( const char *prefix );
 用于生成带有日期的日志文件名称.
 生成的日志文件名称格式为prefix_yyyymmdd.log
 **********************************************************/

void errlog0( const char *errfile, const char *arglist, ... );
#define SDKerrlog   errlog0
#define SDKerrlog0  errlog0

void errlog1( const char *errfile, const char *arglist, ... );
#define SDKerrlog1  errlog1

void errlog2( const char *errfile, const char *arglist, ... );
#define SDKerrlog2  errlog2

void errlog3( const char *errfile, const char *arglist, ... );
#define SDKerrlog3  errlog3

char *SDKgettty();

int readn( int fd, void *buf, int n, int t );

int SDKSetTimer( int n );

/********************************************
    profile operations
********************************************/

int GetProfileString( char *profile, char *AppName, char *KeyName,
    char *KeyVal );

int UpdateProfile( char *profile, char *AppName, char *KeyName,
    char *KeyVal );

int NewSection( char *profile, char *SecName );

int NewKey( char *profile, char *SecName, char *KeyName, char *KeyVal );

int TestSection( char *profile, char *SecName );

int TestKey( char *profile, char *SecName, char *KeyName );

int DeleteKey( char *profile, char *SecName, char *KeyName );

/**************************************
        字符集转换处理
**************************************/

char * EncodingConv(  const char * in, char *encFrom, char *encTo );
/*********************************************************************
说明:字符集转换函数.
参数:
    in: 以encFrom编码的输入数据
    encTo: 输出串的编码格式
返回:
    转换成功时返回转换后的数据;失败时返回NULL
**********************************************************************/


#endif          /* _SDKPUB_H */


