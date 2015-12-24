#ifndef OTHER_H_
#define OTHER_H_

int PrintReportList(char *parafile,char *datafile ,char *outfile );
void WriteRptHeader(FILE *fp, const char *fmt, ...);
void WriteRptRowCount(FILE *fp, int count);
void WriteRptFooter(FILE *fp, const char *fmt, ...);

#endif
