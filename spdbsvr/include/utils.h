
#ifndef __UTILS_H__
#define __UTILS_H__

typedef int (* ConvertFunc) (void *, ...);

#define PRINT(args...) \
    do \
    { \
        if (_nDebug) {\
            printf(args); \
            printf("\n"); \
            fflush(stdout); \
        } \
    } while (0)

#define ERROR_MSG(msg...)  \
    do \
    { \
        fprintf(stderr, msg);\
        fprintf(stderr, "\n");\
        fflush(stderr);\
        exit(-1); \
    } while(0)

#define WriteLog(prefix, fmt...) WriteLogDebug(prefix, NULL, 0, fmt)
#define err_log(fmt...) WriteLogDebug("err", __FILE__, __LINE__, fmt)

int str2int(char *str, int size);

char *getAppDir();

int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...);

int InitProcManage(const char *pidfile);

int UpdateProcNum( int step);

int tcp_call(char *tcpaddr, short tcpport,char *snd, char *rcv, int *len);

int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...);

void pid_check(const char *pid_file);

int callConvertFunc(const char *filename, const char *funcname, 
        const char *expr, void *src, void *dest);

#endif
