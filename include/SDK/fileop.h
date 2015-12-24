/*******************************
	FILE OPERATION
*******************************/
#include <limits.h>
#include "SDKpub.h"

#ifndef LINE_MAX
#ifdef _POSIX2_LINE_MAX
#define LINE_MAX _POSIX2_LINE_MAX
#else
#define LINE_MAX 512
#endif
#endif

struct lineinfo{
	int lineno;			/*�к�*/
	char linestr[LINE_MAX];		/*����*/
	int len;			/*�д�����*/
	struct lineinfo *line_prev;	/*ǰһ�нڵ�*/
	struct lineinfo *line_next;	/*��һ�нڵ�*/
	int used;	/* used ? 0 NO 1 YES */
};

/* DEFINE ERRORS */
#define	EF_SUCC	0		/* SUCCESS */
#define EF_NOMORELINE	-1	/* NO MORE LINE */
#define	EF_NOSUCHLINE	-2
#define EF_NOTFOUND	-3	/* string not found */
#define EF_OPEN		-4	/* open file error */
#define EF_SYS		-999	/* system errors */

/* interfaces */

struct lineinfo *InitBuf( char *path );

struct lineinfo *LoadFile( char *path );

bool IsFileNull();

void ReleaseBuf();

int GetCurrLineNo();

struct lineinfo *CurrentLine();

int GotoLine( int n );

#define GotoFileHead()	GotoLine(1)

void GotoFileEnd();

int PreLine();

int NextLine();

int SaveFile( char *path );

void ChangeLine( char *str );

int AppendLine( char *str );

int InsertLine( char *str );

int SearchString( char *str );

int DeleteLine();

