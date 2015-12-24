#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils.h"

static void DSP_2_HEX(char *dsp, char *hex, int count)
{
    int i;
    for(i = 0; i < count; i++)
    {
    hex[i]=((dsp[i*2]<=0x39)?dsp[i*2]-0x30:dsp[i*2]-0x41+10);
        hex[i]=hex[i]<<4;
    hex[i]+=((dsp[i*2+1]<=0x39)?dsp[i*2+1]-0x30:dsp[i*2+1]-0x41+10);
    }
}

static void HEX_2_DSP(char *hex, char *dsp, int count)
{
    int i;
    char ch;
    for(i = 0; i < count; i++)
    {
        ch=(hex[i]&0xf0)>>4;
        dsp[i*2]=(ch>9)?ch+0x41-10:ch+0x30;
        ch=hex[i]&0xf;
        dsp[i*2+1]=(ch>9)?ch+0x41-10:ch+0x30;
    }
}

int enc_passwd(const char *expr, char *src, char *dest)
{
    char seed[33];
    char cardno[33];
    char passwd[10];

    memset(cardno, 0, sizeof(cardno));
    PGetStr(expr, cardno, sizeof(cardno));
    lrtrim(cardno);
    memset(seed, 0, sizeof(seed));
    memcpy(seed, cardno+strlen(cardno)-13, 12);
    PPutStr("SYSPINSEED", seed);

    memset(passwd, 0, sizeof(passwd));
    memcpy(passwd, src, strlen(src));
    //err_log("PUT [SYSPINSEED]=[%s][%s]", seed, passwd);

    cop_encrypt(seed, passwd, dest);

    return 0;
}

int amt_2_cent(const char *expr, char *src, char *dest)
{
    sprintf(dest, expr, atof(src)*100);
    //err_log("dest,src=[%s][%s]", dest, src);
    return 0;
}

int cent_2_amt(const char *expr, char *src, char *dest)
{
    sprintf(dest, expr, atof(src)/100);
    return 0;
}

int cnvt_czlx(const char *expr, char *src, char *dest)
{
    if(*src=='0')
        sprintf(dest, "1");
    else
        sprintf(dest, "8");
    return 0;
}

int set_acctno(const char *expr, void *src, char *dest)
{
    char tmp[256];
    char acctno[33];

    memset(acctno, 0, sizeof(acctno));
    if (*(tmp+39) == '0') // 卡
        PGetStr("KAHAO1", acctno, sizeof(acctno));
    else
        PGetStr("ZHANGH", acctno, sizeof(acctno));

    strcpy(dest, acctno);
    return 0;
}

int get_trndetail(const char *expr, char *src, char *dest)
{
    strcpy(dest, "get_trndetail生成的明细");
    return 0;
}

long GenBankSerial(char *serialName, long min, long max, int increment)
{
    char   fullname[256];
    long   serial = 0L;
    int    iFileId = 0;
    struct flock lck;
    char tmp[16] = {0};

    sprintf(tmp, "%ld", max);
    snprintf(fullname, sizeof(fullname)-1, "%s/bin/.%s_%dserial", 
             getenv("HOME"), serialName, strlen(tmp));
    iFileId = open(fullname, O_RDWR|O_CREAT|O_SYNC, S_IWUSR|S_IRUSR);
    if (iFileId < 0)
    {
        DBUG("open(%s) fail.", fullname);
        return -1;
    }

    lck.l_type   = F_WRLCK;
    lck.l_start  = 0;
    lck.l_len    = 0;
    lck.l_whence = 0;

    while(fcntl(iFileId, F_SETLK , &lck) == -1);
    lseek(iFileId, 0l, SEEK_SET);
    if (read(iFileId, &serial, sizeof(long)) <= 0)
        serial = min;
    else
        serial += increment;

    if (serial > max)
        serial = min;

    lseek(iFileId, 0l, SEEK_SET);
    write(iFileId, &serial, sizeof(long));
    close(iFileId);

    return serial;
}

//贷记交易根据原交易同城提出号取原交易流水
int cvt_getRefidByTCHTCH(const char *expr, char *src, char *dest)
{
    /*李涛修改于2013-5-3，用于浦发sz45交易有问题，数据库查询697错误
    if (db_query_str(dest, 21, "select refid from acctjour "
                "where nodeid=10 and revserial='%s'", src) != 0)
                */
    if (db_query_str(dest, 21, "select refid from acctjour "
                "where revserial='%s'", src) != 0)
        return -1;
    return 0;
}

//贷记交易根据原交易同城提出号取原交易工作日期
int cvt_getWorkdateByTCHTCH(const char *expr, char *src, char *dest)
{
    if (db_query_str(dest, 21, "select workdate from acctjour "
                "where nodeid=10 and revserial='%s'", src) != 0)
        return -1;
    return 0;
}

int cvt_getagreement(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    char tmp1[128]={0};
    char tmp2[128]={0};

    PGetStr("JIOHRQ", tmp1, sizeof(tmp1));//原交易日期
    if(*expr=='1')
    {
        //PGetStr("ZHYODM", tmp2, sizeof(tmp2));//原交易流水
        PGetStr("BAOWLS", tmp2, sizeof(tmp2));//原交易流水
    }
    else//'2'
    {
        cvt_getRefidByTCHTCH("",src,tmp2);
    }
    sprintf(dest,"%s/%s",tmp1,tmp2);
    return 0;
}

int cvt_getnotetypebyrefid(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    char tab[128]={0};
    char tmp[128]={0};
    char date1[128]={0};
    char date2[128]={0};
    char acceptor[128]={0};

    if(expr[0]=='1')//借记交易直接从sop报文中取原交易流水
        //PGetStr("ZHYODM", tmp, sizeof(tmp));//原交易流水
        PGetStr("BAOWLS", tmp, sizeof(tmp));//原交易流水
    else//'2'贷记交易根据同城提出号取原交易流水
        cvt_getRefidByTCHTCH("",src,tmp);

    if(strcmp(tmp,"")==0)
    {
        err_log("原交易流水为空");
        return -1;
    }
    PGetStr("JIOHRQ", date1, sizeof(date1));//原交易日期
    if(db_query_str(date2, 13, "SELECT paraval FROM syspara WHERE nodeid=10 AND paraname='ARCHIVEDATE'")!=0)
        return -1;

    if (DiffDate(date1, date2) <= 0)
        strcpy(tab,"htrnjour");
    else
        strcpy(tab,"trnjour");

    err_log("JIOHRQ[%s]",date1);
    PGetStr("QISHHH", acceptor, sizeof(acceptor));//提入行
    if (db_query_str(dest, 13, "select notetype from %s "
                "where nodeid=10 and workdate='%s' and refid='%s' and acceptor='%s' and inoutflag='2'", tab, date1, tmp, acceptor) != 0)
        return -1;
    return 0;
}

int cvt_getbeneacctbyrefid(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    if(strlen(src))
    {
        strcpy(dest,src);
        return 0;
    }
    char tab[128]={0};
    char tmp[128]={0};
    char date1[128]={0};
    char date2[128]={0};
    char acceptor[128]={0};

    if(expr[0]=='1')//借记交易直接从sop报文中取原交易流水
        //PGetStr("ZHYODM", tmp, sizeof(tmp));//原交易流水
        PGetStr("BAOWLS", tmp, sizeof(tmp));//原交易流水
    else//'2'贷记交易根据同城提出号取原交易流水
        cvt_getRefidByTCHTCH("",src,tmp);

    if(strcmp(tmp,"")==0)
    {
        err_log("原交易流水为空");
        return -1;
    }
    PGetStr("JIOHRQ", date1, sizeof(date1));//原交易日期
    if(db_query_str(date2, 13, "SELECT paraval FROM syspara WHERE nodeid=10 AND paraname='ARCHIVEDATE'")!=0)
        return -1;

    if (DiffDate(date1, date2) <= 0)
        strcpy(tab,"htrnjour");
    else
        strcpy(tab,"trnjour");

    err_log("JIOHRQ[%s]",date1);
    PGetStr("QISHHH", acceptor, sizeof(acceptor));//提入行
    if (db_query_str(dest, 33, "select beneacct from %s "
                "where nodeid=10 and workdate='%s' and refid='%s' and acceptor='%s' and inoutflag='2'", tab, date1, tmp, acceptor) != 0)
        return -1;
    return 0;
}

int cvt_getbenenamebyrefid(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    if(strlen(src))
    {
        strcpy(dest,src);
        return 0;
    }
    char tab[128]={0};
    char tmp[128]={0};
    char date1[128]={0};
    char date2[128]={0};
    char acceptor[128]={0};

    if(expr[0]=='1')//借记交易直接从sop报文中取原交易流水
        //PGetStr("ZHYODM", tmp, sizeof(tmp));//原交易流水
        PGetStr("BAOWLS", tmp, sizeof(tmp));//原交易流水
    else//'2'贷记交易根据同城提出号取原交易流水
        cvt_getRefidByTCHTCH("",src,tmp);

    if(strcmp(tmp,"")==0)
    {
        err_log("原交易流水为空");
        return -1;
    }
    PGetStr("JIOHRQ", date1, sizeof(date1));//原交易日期
    if(db_query_str(date2, 13, "SELECT paraval FROM syspara WHERE nodeid=10 AND paraname='ARCHIVEDATE'")!=0)
        return -1;

    if (DiffDate(date1, date2) <= 0)
        strcpy(tab,"htrnjour");
    else
        strcpy(tab,"trnjour");

    err_log("JIOHRQ[%s]",date1);
    PGetStr("QISHHH", acceptor, sizeof(acceptor));//提入行
    if (db_query_str(dest, 81, "select benename from %s "
                "where nodeid=10 and workdate='%s' and refid='%s' and acceptor='%s' and inoutflag='2'", tab, date1, tmp, acceptor) != 0)
        return -1;
    return 0;
}

int cvt_getpayerbyrefid(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    if(strlen(src))
    {
        strcpy(dest,src);
        return 0;
    }
    char tab[128]={0};
    char tmp[128]={0};
    char date1[128]={0};
    char date2[128]={0};
    char acceptor[128]={0};

    if(expr[0]=='1')//借记交易直接从sop报文中取原交易流水
        //PGetStr("ZHYODM", tmp, sizeof(tmp));//原交易流水
        PGetStr("BAOWLS", tmp, sizeof(tmp));//原交易流水
    else//'2'贷记交易根据同城提出号取原交易流水
        cvt_getRefidByTCHTCH("",src,tmp);

    if(strcmp(tmp,"")==0)
    {
        err_log("原交易流水为空");
        return -1;
    }
    PGetStr("JIOHRQ", date1, sizeof(date1));//原交易日期
    if(db_query_str(date2, 13, "SELECT paraval FROM syspara WHERE nodeid=10 AND paraname='ARCHIVEDATE'")!=0)
        return -1;

    if (DiffDate(date1, date2) <= 0)
        strcpy(tab,"htrnjour");
    else
        strcpy(tab,"trnjour");

    err_log("JIOHRQ[%s]",date1);
    PGetStr("QISHHH", acceptor, sizeof(acceptor));//提入行
    if (db_query_str(dest, 81, "select payer from %s "
                "where nodeid=10 and workdate='%s' and refid='%s' and acceptor='%s' and inoutflag='2'", tab, date1, tmp, acceptor) != 0)
        return -1;
    return 0;
}

int cvt_getnotenobyrefid(const char *expr, char *src, char *dest)//src:TCHTCH expr:dcflag
{
    char tab[128]={0};
    char tmp[128]={0};
    char date1[128]={0};
    char date2[128]={0};
    char acceptor[128]={0};

    if(expr[0]=='1')//借记交易直接从sop报文中取原交易流水
        //PGetStr("ZHYODM", tmp, sizeof(tmp));//原交易流水
        PGetStr("BAOWLS", tmp, sizeof(tmp));//原交易流水
    else//'2'贷记交易根据同城提出号取原交易流水
        cvt_getRefidByTCHTCH("",src,tmp);

    if(strcmp(tmp,"")==0)
    {
        err_log("原交易流水为空");
        return -1;
    }
    PGetStr("JIOHRQ", date1, sizeof(date1));//原交易日期
    if(db_query_str(date2, 13, "SELECT paraval FROM syspara WHERE nodeid=10 AND paraname='ARCHIVEDATE'")!=0)
        return -1;

    if (DiffDate(date1, date2) <= 0)
        strcpy(tab,"htrnjour");
    else
        strcpy(tab,"trnjour");

    err_log("JIOHRQ[%s]",date1);
    PGetStr("QISHHH", acceptor, sizeof(acceptor));//提入行
    if (db_query_str(dest, 13, "select noteno from %s "
                "where nodeid=10 and  workdate='%s' and refid='%s' and acceptor='%s' and inoutflag='2'", tab, date1, tmp, acceptor) != 0)
        return -1;
    return 0;
}

int cvt_checkagreement(const char *expr, char *src, char *dest)
{
    char tmp[128]={0};
    PGetStr("ZHFUMM", tmp, sizeof(tmp));
    if(strcmp(tmp,""))
    {
        strcpy(dest,tmp);
    }
    else
    {
        PGetStr("SZBPMY", tmp, sizeof(tmp));
        strcpy(dest,tmp);
    }
    return 0;
}

int cvt_fix_noteno(const char *expr, char *src, char *dest)
{
    char tmp[9]={0};
    char tmp1[9]={0};

    PGetStr("PNGZHH", tmp, sizeof(tmp)); //前8位
    PGetStr("PNGZXH", tmp1, sizeof(tmp1)); //后8位

    err_log("PNGZHH[%s], PNGZXH[%s]...", tmp, tmp1);
    if(strcmp(tmp,""))
    {
        sprintf(dest,"%s%s", tmp, tmp1);
    }
    else
    {
        strcpy(dest,tmp1);
    }

    return 0;
}
int cvt_trackinfo(const char *expr, char *src, char *dest)
{
    char tmp1[41]={0};
    char tmp2[105]={0};

    PGetStr("CID2XX", tmp1, sizeof(tmp1));
    PGetStr("SNCDAO", tmp2, sizeof(tmp2));
    sprintf(dest, "%79s%37s%104s", "", tmp1, tmp2);

    return 0;
}

int cvt_sz79_serial(const char *expr, char *src, char *dest)
{
    char tmp1[128]={0};
    char tmp2[128]={0};
    char serial[20]={0};
    char QueryType[2]={0};
    PGetStr("CXCFLX", QueryType, sizeof(QueryType));//获取查询类型1查询书2查复书
    if(atoi(QueryType)==1)//查询书
    {
        PGetStr("PDSBNO", tmp1, sizeof(tmp1));//取得机构号
        if(cvt_bankid2exchno("",tmp1,tmp2)!=0)return -1;//取得行号
        strcat(tmp2,"Q");//行号末尾+'Q'作为查询书的流水参考号
        if(gen_serial(expr,tmp2,serial)!=0)return -1;//取得流水
        strcpy(dest, serial);
    }
    else
    {
        PGetStr("SXEDBH", serial, sizeof(serial));//取得原查询书流水
        strcpy(dest, serial);
    }
    return 0;
}

int gen_serial2(const char *expr, char *src, char *dest)
{
    /*
       char tmp[128]={0};
       if(cvt_bankid2exchno("",src,tmp)!=0)
       return -1;//取得行号
       return gen_serial(expr,tmp,dest);
     */
    return gen_serial(expr,src,dest);
}

int gen_serial(const char *expr, char *src, char *dest)
{
    // src为提出行, expr为允许最大流水
    long max;

    if (expr == NULL)
        max = 99999999;
    else
        max = atol(expr);

    //if ((max = GenBankSerial(src, 1L, max, 1)) <= 0)
    if ((max = GenBankSerial("090010", 1L, max, 1)) <= 0)
        return -1;
    sprintf(dest, "%ld", max);
    return 0;
}

int cvt_notetype(const char *expr, char *src, char *dest)
{
    // expr 配置 dcflag, 1-借 2-贷
    if (expr == NULL && *expr != '1' && *expr != '2')
        return -1;

    if (db_query_str(dest, 3, "select tctype from notetypemap "
                "where nodeid=10 and banktype='%s' and dcflag='%s'", src, expr) != 0)
        return -1;
    return 0;
}

int cvt_cut_noteno(const char *expr, char *src, char *dest)
{
    if(strlen(src)<16)
    {
        err_log("错误,支付密码支票号码少于16位!");
        return -1;
    }
    if(*expr=='0')//前8位
        memcpy(dest,src,8);
    else//后8位
        memcpy(dest,src+8,8);
    dest[9]=0x00;
    return 0;
}

int cvt_notetype_c2b(const char *expr, char *src, char *dest)
{
    //err_log("get in func cvt_notetype_c2b");
    if(expr==NULL||src==NULL)
        return -1;
    if(*expr=='0')//不转
    {
        strcpy(dest,src);
        return 0;
    }
    char bkcode[10];
    //err_log( "expr[%s], src[%s]", expr, src);
    return notetype_c2b(src, dest, bkcode);
}

int cvt_bankid2telid(const char *expr, char *src, char *dest)
{
    if (db_query_str(dest, 13, "select autooper from bankinfo "
                "where nodeid=10 and bankid='%s'", src) != 0)
        return -1;
    return 0;
}

int cvt_bankid2payno(const char *expr, char *src, char *dest)
{
    if (db_query_str(dest, 13, "select reserved2 from bankinfo "
                "where nodeid=10 and exchno='%s'", src) != 0)
        return -1;
    return 0;
}

int cvt_bankid2exchno(const char *expr, char *src, char *dest)
{
    if(strcmp(src,"9501")==0)
    {
        strcpy(dest,"090010");
        return 0;
    }
    if (db_query_str(dest, 13, "select exchno from bankinfo "
                "where nodeid=10 and bankid='%s'", src) != 0)
        return -1;
    return 0;
}

int cvt_exchno2bankid(const char *expr, char *src, char *dest)
{
    if (db_query_str(dest, 13, "select bankid from bankinfo "
                "where nodeid=10 and exchno='%s'", src) != 0)
        return -1;
    return 0;
}

int FillSerial(const char *expr, char *src, char *dest)
{
    //err_log("debug line[%3d]get in func FillSerial",__LINE__);
    if(expr==NULL||src==NULL)
        return -1;
    //err_log("debug line[%3d]expr[%s]src[%s]",__LINE__,expr,src);
    int len=atoi(expr);
    int num=atoi(src);
    if(len<=0||num<=0)
        return -1;
    sprintf(dest,"%0*ld",len,num);
    return 0;
}

int cvt_dcflag_b2c(const char *expr, char *src, char *dest)
{
    if(expr==NULL||src==NULL)
        return -1;
    sprintf(dest,"%d",atoi(src)+1);
    return 0;
}

int cvt_dcflag_c2b(const char *expr, char *src, char *dest)
{
    if(expr==NULL||src==NULL)
        return -1;
    sprintf(dest,"%d",atoi(src)-1);
    return 0;
}

int cvt_caozuotype(const char *expr, char *src, char *dest)
{
    if(*src == 0x00)
        return -1;
    switch(*src)
    {
        case '1':
            sprintf(dest,"%s","1001");
            break;
        case '2':
            sprintf(dest,"%s","1002"); 
            break;
        case '3':
            sprintf(dest,"%s","1003");
            break;
        case '4':
            //sprintf(dest,"%s","8104");
            sprintf(dest,"%s","1004");
            break;
        default:
            break;
    }
    return 0;
}
int cvt_print(const char *expr, char *src, char *dest)
{
    if(*src == 0x00)
        return -1;
    switch(*src)
    {
        case '1':
            sprintf(dest,"%s","3111");
            break;
        default:
            break;
    }
    return 0;
}

int cvt_clean_date(const char *expr, char *src, char *dest)
{
    if( (*src == 0x00) || (strcmp(src,"18991231")==0) )
    {
        if (db_query_str(dest, 13, "select paraval from syspara "
                    "where nodeid=10 and  paraname='CURWORKDATE'" ) != 0)
            return -1;
    }
    else
        strcpy(dest,src);

    return 0;
}

int cvt_DueDate(const char *expr, char *src, char *dest)
{
    char duedate[9];
    if(*src == 0x00)
    {
        return -1;
    }
    if(strcmp(src,"18991231"))
    {
        strcpy(dest,src);
    }
    else
    {
        PGetStr(expr, duedate, sizeof(duedate));
        strcpy(dest,duedate);
    }
    return 0;
}

int cvt_result(const char *expr,char *src,char *dest)
{
    sprintf(dest,"%*d",atoi(expr),atoi(src)==0?0:1);
    return 0;
}

//10位转8位日期格式
int cvt_DateL2S(const char *expr,char *src,char *dest)
{
    //err_log("date=[%s]",src);
    memcpy(dest, src, 4);
    memcpy(dest+4, src+5, 2);
    memcpy(dest+6, src+8, 2);
    dest[8] = 0x00;
    //err_log("date=[%s]",dest);
    return 0;
}

