#include "pub.h"
#include "tcop.h"
char *StrTok( char *, char *);

int main(int argc, char *argv[])
{
	char where[256]={0};
	char bankno[12+1]={0};
	char acct[32+1]={0};
	char userid[30+1]={0};
	char agreement[44+1]={0};
	char sOperType[1+1]={0};
	char sTmp[16]={0};
    char buf[1024]={0};
    char name[80+1]={0};
    FILE *fp;
    char path[64] ={0};
    char buf1[2048] = {0};
    char tmp[512]={0};
    char tmp1[512]={0};
    int ret=-1;

    if( argc != 2)
    {
	    fprintf(stderr, " %s file \n", argv[0] );
	    return -1;
    }

    sprintf( path, "%s/tmp/%s", getenv("HOME"), argv[1] );
    fp = fopen( path, "r" );

    while(1)
    {
        if( fgets(buf, 4096, fp) == NULL )
            break;
        strcpy(userid,  StrTok( buf, "|"));
        strcpy(acct, StrTok( NULL, "|"));
        strcpy(agreement, StrTok( NULL, "|"));
        strcpy(name, StrTok( NULL, "|"));
        strcpy(bankno, StrTok( NULL, "|"));
        all_trim(userid);
        all_trim(acct);
        all_trim(agreement);
        all_trim(name);
        all_trim(bankno);
#if 1
        sprintf(where, "nodeid=%d and agreementid='%s' and payingacct='%s'", 10, agreement, acct);

        ret = db_query_str(sTmp, sizeof(sTmp), "select count(1) from agreement where %s", where);
        if (ret == 0)
        {
            if (atoi(sTmp) == 0)
                sprintf( buf, "insert into agreement values(10, '','%s','%s','','','','%s','%s','','%s','%s','','','','','','%s',0.00,'20120328','','20991231','1','','')", userid, name, acct, name, bankno, bankno, agreement);
            ret = db_exec(buf);
            if( ret )
                return ret;
        }
        else
            return ret;
#endif
    }

    return 0;
}
char *StrTok(char *sourcestr,char *splitstr)
{
    static char srcstr[8192];
    static char retstr[8192];
    int i,l;

    if(sourcestr!=NULL)
        strcpy(srcstr,sourcestr);
    l=strlen(srcstr);
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
