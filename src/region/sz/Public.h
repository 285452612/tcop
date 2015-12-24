#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#define err_log(fmt...) WriteLogDebug("info", __FILE__, __LINE__, fmt)
#define WriteLog(prefix, fmt...) WriteLogDebug(prefix, NULL, 0, fmt)

char *ChineseDate(long curr_date);
int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...);

#endif
