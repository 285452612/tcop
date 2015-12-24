#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sdp.h"

//初始化字符数组内容全为0
#define izero(s)  (memset((s), 0, sizeof((s))))

char *getCHSDate(char *, long);
char *getDate(char sep);
char *getTime(char sep);

char *DateDTO(const char *);
char *DateOTD(char *);
char *DateSTD(char *);
char *TimeSTO(char *);
char *TimeOTS(char *);

char *AmtFTY(char *dst, const char *amt);
char *AmtYTF(char *dst, const char *amt);

char *convertAmount(char *src);
char *convertDate(char *src);
char *convertTime(char *src);

char *FormatMoney(char *str);

char *vstrcat(const char *fmt, ...);
char *getFilesdirFile(char *pFileName);

void opLoger(char *homedir, char *logname, char *file, int line, char *fmt, ...);
void opFixedLoger(char *homedir, char *logname, char *file, int line, char *buf, int len);

#endif
