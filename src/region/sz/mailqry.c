#include "comm.h"
#include "interface.h"
#include "chinese.h"
#include "udb.h"
#include "errcode.h"
#include "Public.h"

#define MAIL_MAX_LINE 8
#define MAIL_PAGE_NUM 3

extern char gs_originator[13];
extern char gs_bankname[81];
extern char gs_sysname[61];
extern char gs_oper[];

extern int InitRptVar(xmlDocPtr xmlReq);
extern char *GetTmpFileName(char *);
extern char *FormatMoney(char *str);

#define GetTrnCtl(a) XmlGetStringDup(xmlReq, "/UFTP/TrnCtl/"a)
static int MyCheckHZ(char buf)
{
    unsigned   char   ch;
    ch = buf;
    if (ch > 0xa1)
        if (ch == 0xa3)
            return 1; //全角字符
        else
            return 2; //汉字
    else
        return 0;  //英文，数字，标点
}

static int format_str(char *src, char *dest)
{
    int i,k=1;
    int j=0;
    int t_length = strlen(src);

    for (i = 1; i <= t_length; i++)
    {
        if (MyCheckHZ(src[i-1]) == 0 )
            dest[j++] = src[i-1];
        else
        {
            dest[j++] = src[i-1];
            i++;
            dest[j++] = src[i-1];
        }
        if ( i >= 72*k )
        {
            k++;
            dest[j++] = '\n';
        }
    }
    return k;
}

// 邮件查询
int MailQuery(xmlDocPtr xmlReq, char *filename)
{
    result_set rs;
    char caOutFile[256];
    char condi[1024];
    char content[4096];
    char bankname[81];
    char newdate[20];      //新的日期格式
    char newtime[20];      //新的时间格式
    long startdate;
    long enddate;
    const char *pLINE = "--------------------------------------------------------------";
    FILE *fp=NULL;
    int iRet = 0;
    int i, j, l;

    if (InitRptVar(xmlReq) != 0)
        return E_OTHER;

    /*
       snprintf( caParaFile, sizeof(caParaFile),
       "%s/dat/%d/MailQuery.para", getenv("HOME"), TCOP_BANKID );
     */
    startdate = atol(GetTrnCtl("StartDate"));
    if (startdate == 0L)
        startdate = atol(GetWorkdate());
    enddate = atol(GetTrnCtl("EndDate"));
    if (enddate == 0L)
        enddate = atol(GetWorkdate());

    fp = fopen(GetTmpFileName(caOutFile), "w");

    /*
       WriteRptHeader(fp, "%s;%s;%s;", 
       gs_sysname, ChineseDate(startdate), ChineseDate(enddate));
     */
    sprintf(condi, "acceptor='%s' AND inoutflag='2' AND readflag LIKE '%s%%'"
            " AND workdate BETWEEN '%08ld' AND '%08ld'", 
            gs_originator, GetTrnCtl("Readed"), startdate, enddate);

    iRet = db_query(&rs, "SELECT originator, acceptor,sender, title, content, "
            "workdate, sendtime, refid FROM freemsg WHERE %s", condi);
    if (iRet != 0)
        return iRet;

    for (i = 0; i < db_row_count(&rs); i++)
    {
        fprintf(fp, "%30s%s\n", " ", gs_sysname);
        fprintf(fp, "%20s%s    %s", " ", 
                ChineseDate(startdate), ChineseDate(enddate));

        memset(content, 0, sizeof(content));
        l = format_str(db_cell(&rs, i, 4), content);

        fprintf(fp, "\n发送行:%s  %s\n发件人:%s\n主  题:%s\n内  容:\n",
                db_cell(&rs, i, 0), org_name(db_cell(&rs, i, 0), bankname), 
                db_cell(&rs, i, 2), db_cell(&rs, i, 3));

        fprintf(fp, "%s\n%s\n", pLINE, content);
        for (j = l; j < MAIL_MAX_LINE; j++)
            fprintf(fp, "\n");
        fprintf(fp, "%s\n", pLINE);

        memset(newdate, 0, sizeof(newdate));
        memset(newtime, 0, sizeof(newtime));
        sprintf(newdate, "%04ld-%02ld-%02ld", db_cell_i(&rs, i, 5)/10000, 
                db_cell_i(&rs, i, 5)%10000/100, db_cell_i(&rs, i, 5)%100);
        sprintf(newtime, "%02ld:%02ld:%02ld", db_cell_i(&rs, i, 6)/10000, 
                db_cell_i(&rs, i, 6)%10000/100, db_cell_i(&rs, i, 6)%100);
        fprintf(fp, "接收日期:%s %s   邮件编号: %s\n\n\n",
                newdate, newtime, db_cell(&rs, i, 7));

        if (i % MAIL_PAGE_NUM == 0 && i > 0)
            fprintf(fp, "\f");
    }
    db_free_result(&rs);

    fclose(fp);

    db_exec("UPDATE freemsg SET readflag='1' WHERE %s", condi);

    sprintf(filename, "%s", basename(caOutFile));

    return iRet;
}
