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
 ˵��: ������ֵ��ת��Ϊ�ַ������
************************************************/

char *ftoa( double f, int p );
/************************************************
 ˵��: ��������ֵ��ת��Ϊ�ַ������
 ����:
    f ������
    p ����
************************************************/

char *r_trim( char *source );

char *l_trim( char *source );

char *all_trim( char *source );

bool isblankstring( const char *s );
/******************************************************************
bool isblankstring( const char *s );
�����ж��ַ���s�Ƿ�Ϊ�հ״�.
��һ���ַ����н����������ַ�ʱ����Ϊ�ǿհ״�: �ո��Ʊ��������
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
��;:ȡ��ǰ����.
�������ڸ�ʽ:YYYYMMDD
*********************************************/

long current_time();
/*********************************************
long current_time();
��;:ȡ��ǰʱ��.
����ʱ���ʽ:hhmmss
*********************************************/

bool IsValidDate( const char *sDate, const char *fmt );
/*********************************************
bool IsValidDate( const char *sDate, const char *fmt );
��;:�ж������ַ���sDate�Ƿ��Ƿ���fmt��ʽҪ�����Ч����.
    �����е���,��,�շֱ���%Y,%m,%d��ʾ.ȱʡ�����ڸ�ʽΪ%Y%m%d
    ���ڵĸ�ʽ���Զ���,����:"%Y �� %m �� %d ��"
����:
    ��sDateΪ��Ч����ʱ����TRUE,���򷵻�FALSE
*********************************************/

#define ValidDate( s )    IsValidDate( s, "%Y%m%d" )

bool IsValidTime( const char *sTime );
/*********************************************
bool IsValidTime( const char *sTime );
��;:�ж�ʱ���Ƿ���Ч.ʱ��ĸ�ʽΪhhmmss.��������8��15��24���ʾΪ081524
����:
    ��sTimeΪ��Чʱ��ʱ����TRUE,���򷵻�FALSE
*********************************************/

int diffday( const char* day1, const char* day2, const char* fmt );
/*****************************************************************************
int diffday( const char* day1, const char* day2, const char* fmt );
��;:�ж�������ͬ��ʽ�����ڼ���������.
����:
    �����ڸ�ʽ��ȷʱ,������������ͬ,����0;���򷵻���������,�����������ڵ�
    �Ⱥ�.
    ��������Чʱ,��������-1
*****************************************************************************/

int dateconv( const char *s, const char *fmt_s, char *d, const char *fmt_d );
/*****************************************************************************
int dateconv( const char *s, const char *fmt_s, char *d, const char *fmt_d );
��;:����ʽΪfmt_s��ʱ��sת��Ϊ��ʽΪfmt_d���ַ���d.
����:
    ת���ɹ�ʱ����0,���򷵻�-1
*****************************************************************************/

char *daysafter( const char *date, const char *format, int n );
/*****************************************************************************
char *daysafter( const char *date, const char *format, int n );
��;:�����ʽΪformat���ַ���date��ʾ������n��������,������ͬ�ĸ�ʽ����.
    �ڼ���֮ǰ������ʱ,n�Ը�����ʾ.����-3��ʾ3��ǰ������
����:
    ��dateΪ����format����Ч����,���ظ�ʽΪformat�����ڴ�;���򷵻�NULL
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
��Դ�ļ�src����ΪĿ���ļ�dest,������Դ�ļ�������
****************************************************/

int fileline( char *path );
/**********************************************************
fileline()���ڼ���һ���ı��ļ��д��ڵ�����.����������wc����
�ڳɹ�ʱ�����ļ��еļ�¼����,���򷵻�-1
***********************************************************/

char *logfname( const char *prefix );
/**********************************************************
 char *logfname( const char *prefix );
 �������ɴ������ڵ���־�ļ�����.
 ���ɵ���־�ļ����Ƹ�ʽΪprefix_yyyymmdd.log
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
        �ַ���ת������
**************************************/

char * EncodingConv(  const char * in, char *encFrom, char *encTo );
/*********************************************************************
˵��:�ַ���ת������.
����:
    in: ��encFrom�������������
    encTo: ������ı����ʽ
����:
    ת���ɹ�ʱ����ת���������;ʧ��ʱ����NULL
**********************************************************************/


#endif          /* _SDKPUB_H */


