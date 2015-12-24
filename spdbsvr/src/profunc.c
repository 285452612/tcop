#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include "utils.h"
#include "udb.h"

//qtjym是前台交易码,例如sz36
int notetype_b2c(char *tctype, const char *qtjym, const char *PNGZZL, const char *PNG1ZL, const char *PNGZZZ, const char *BEIZXX, const char *TJRZFS, const char *BIAOZI)
{
    err_log(" 转化凭证类型 行内->人行:"
            "前台交易码[%s] 凭证类型PNGZZL[%s] 中间变量PNG1ZL[%s] 中间变量PNGZZZ[%s] 中间变量BEIZXX[%s]",qtjym,PNGZZL,PNG1ZL,PNGZZZ,BEIZXX);
    *tctype='\0';
    if(strcmp(qtjym,"sz73")==0)
    {
        if(strcmp(PNGZZL,"0")==0)
            strcpy(tctype,"71");
        else
            strcpy(tctype,"73");
    }
    else if(strcmp(qtjym,"sz74")==0)
    {
        if(strcmp(PNGZZL,"0")==0)
            strcpy(tctype,"72");
        else
            strcpy(tctype,"74");
    }
    else if(strcmp(PNGZZL,"01")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
            strcpy(tctype,"01");
        else if(strcmp(qtjym,"sz36")==0)
        {
            if( !strcmp(TJRZFS,"1") && !strcmp(BIAOZI,"T") )
                strcpy(tctype,"31");
            else
                strcpy(tctype,"02");
        }
        else if(strcmp(qtjym,"sz41")==0)
            strcpy(tctype,"19");
        else
            strcpy(tctype,"01");
    }
    else if(strcmp(PNGZZL,"02")==0)
    {
        strcpy(tctype,"01");
    }
    else if(strcmp(PNGZZL,"59")==0)
    {
        strcpy(tctype,"21");
    }
    else if(strcmp(PNGZZL,"97")==0)
    {
        strcpy(tctype,"17");
    }
    else if(strcmp(PNGZZL,"98")==0)
    {
        strcpy(tctype,"18");
    }
    else if(strcmp(PNGZZL,"yv")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
            strcpy(tctype,"01");
        else if(strcmp(qtjym,"sz36")==0)
            strcpy(tctype,"10");
    }
    else if(strcmp(PNGZZL,"yw")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
            strcpy(tctype,"07");
    }
    else if(strcmp(PNGZZL,"31")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
            strcpy(tctype,"07");
        else
            strcpy(tctype,"09");
    }
    else if(strcmp(PNGZZL,"xh")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
            strcpy(tctype,"18");
    }
    else if(strcmp(PNGZZL,"zv")==0)
    {
        strcpy(tctype,"11");
    }
    else if(strcmp(PNGZZL,"zz")==0)
    {
        if(strcmp(qtjym,"sz37")==0)
        {
            if(strcmp(PNG1ZL,"")!=0)
            {
                if(strcmp(PNG1ZL,"g1")==0)
                    strcpy(tctype,"14");
                else if(strcmp(PNG1ZL,"g2")==0)
                    strcpy(tctype,"12");
                else if(strcmp(PNG1ZL,"g3")==0)
                    strcpy(tctype,"15");
                else if(strcmp(PNG1ZL,"g4")==0)
                    strcpy(tctype,"42");
                else if(strcmp(PNG1ZL,"g5")==0)
                    strcpy(tctype,"15");
                else if(strcmp(PNG1ZL,"g6")==0)
                    strcpy(tctype,"42");
            }
            else
            {
                strcpy(tctype,"23");
            }
            /*
               {
               if(strcmp(PNGZZZ,"yv")==0)
               strcpy(tctype,"01");
               else if(strcmp(PNGZZZ,"yw")==0)
               strcpy(tctype,"07");
               else if(strcmp(PNGZZZ,"31")==0)
               strcpy(tctype,"07");
               else if(strcmp(PNGZZZ,"71")==0)
               strcpy(tctype,"01");
               else if(strcmp(PNGZZZ,"xh")==0)
               strcpy(tctype,"18");
               else if(strcmp(PNGZZZ,"zv")==0)
               strcpy(tctype,"11");
               else if(strcmp(PNGZZZ,"zz")==0)
               strcpy(tctype,"23");
               else if(strcmp(PNGZZZ,"yx")==0)
               strcpy(tctype,"01");
               else if(strcmp(PNGZZZ,"yy")==0)
               strcpy(tctype,"18");
               else if(strcmp(PNGZZZ,"yz")==0)
               strcpy(tctype,"01");
               else if(strcmp(PNGZZZ,"xa")==0)
               strcpy(tctype,"18");
               }
             */
        }
        else if(strcmp(qtjym,"sz36")==0)
        {
            if(strcmp(BEIZXX,"31")==0)
                strcpy(tctype,"09");
            else if(strcmp(BEIZXX,"yv")==0)
                strcpy(tctype,"22");
            else
                strcpy(tctype,"22");
        }
        else
        {
            strcpy(tctype,"01");
        }
    }
    else if(strcmp(PNGZZL,"07")==0)
    {
        strcpy(tctype,"05");
    }
    else if(strcmp(PNGZZL,"08")==0)
    {
        strcpy(tctype,"22");
    }
    else if(strcmp(PNGZZL,"10")==0)
    {
        strcpy(tctype,"09");
    }
    else if(strcmp(PNGZZL,"12")==0)
    {
        strcpy(tctype,"03");
    }
    else if(strcmp(PNGZZL,"20")==0)
    {
        strcpy(tctype,"04");
    }
    else if(strcmp(PNGZZL,"71")==0)
    {
        strcpy(tctype,"31");
    }
    else if(strcmp(PNGZZL,"72")==0)
    {
        strcpy(tctype,"32");
    }
    else if(strcmp(PNGZZL,"74")==0)
    {
        strcpy(tctype,"22");
    }
    else if(strcmp(PNGZZL,"75")==0)
    {
        strcpy(tctype,"23");
    }
    else if(strcmp(PNGZZL,"43")==0)
    {
        strcpy(tctype,"43");
    }
    else if(strcmp(PNGZZL,"zi")==0)
    {
        strcpy(tctype,"21");
    }
    else if(strcmp(PNGZZL,"zr")==0)
    {
        strcpy(tctype,"23");
    }
    if(*tctype=='\0')
    {
        err_log("转换失败!");
        return -1;
    }
    else
    {
        err_log("转换结果[%s]",tctype);
        return 0;
    }
}

int notetype_c2b(char *notetype, char *bktype, char *bkcode)
{
    return notetype_c2b_ex(notetype,bktype,bkcode,1);
}

int notetype_c2b_ex(char *notetype, char *bktype, char *bkcode, int logflag)
{
    xmlDocPtr doc;
    xmlNodePtr node;
    char path[256];
    char *p;

    *bktype = *bkcode = 0x00;
    sprintf(path, "%s/etc/map.xml", getenv("APP_DIR"));
    if ((doc = xmlParseFile(path)) == NULL)
        return -1;

    sprintf(path, "//notetype_c2b/mapping[@name='%s']", notetype);
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    if ((p = XmlNodeGetAttrText(node, "value")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bktype, p);
    if(logflag)
        err_log("notetype[%s] BKType[%s]", notetype, bktype);
    free(p); p = NULL;

    if ((p = XmlNodeGetAttrText(node, "bkcode")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bkcode, p);

    free(p);
    xmlFreeDoc(doc);
    return 0;
}

void save_pack(char *suffix, char *buf, int len)
{
    char file[256];
    FILE *fp;

    sprintf(file, "%s/bankdata/%08ld.%s", getenv("HOME"), getpid(), suffix);
    if ((fp = fopen(file, "w")) == NULL)
        return;
    fwrite(buf, len, 1, fp);
    fclose(fp);
    return;
}

void save_xml(char *trncode, char *buf, int len)
{
    char file[256];
    FILE *fp;
    sprintf(file, "%s/bankdata/%s_%08ld.xml", getenv("HOME"), trncode, getpid());
    if ((fp = fopen(file, "w")) == NULL)
        return;
    fwrite(buf, len, 1, fp);
    fclose(fp);
    return;
}
// 金额大小写转换
int  MoneyToChinese ( char *money, char * chinese )
{
    int len, zerotype, i, unit_num , allzero = 1 ;
    char fundstr [ 51 ];
    char *numberchar [ ] =
    {
        "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"
    };
    char *rmbchar [ ] =
    {
        "分", "角", "", "元", "拾", "佰", "仟", "万", "拾", "佰", "仟",
        "亿", "拾", "佰", "仟"
    };

    sprintf ( fundstr, "%.2lf", atof( money ) );
    len = strlen ( fundstr );
    unit_num = sizeof ( rmbchar ) / sizeof ( rmbchar [ 0 ] );
    for ( i = zerotype = 0, chinese [ 0 ] = '\0' ; i < len ; i++ )
    {
        switch ( fundstr [ i ] )
        {
            case '-':
                {
                    strcat ( chinese, "负" );
                    break;
                }
            case '.':
                {
                    if ( chinese [ 0 ] == '\0' )
                        strcat ( chinese, numberchar [ 0 ] );
                    if ( zerotype == 1 )
                        strcat ( chinese, rmbchar [ 3 ] );
                    zerotype = 0;
                    break;
                }
            case '0':
                {
                    if ( len - i  == 12 && 11 < unit_num )
                        strcat ( chinese, rmbchar [ 11 ] );
                    if ( len - i  == 8 && 7 < unit_num )
                    {
                        if( !allzero )
                            strcat ( chinese, rmbchar [ 7 ] );
                    }
                    zerotype = 1;
                    break;
                }
            default:
                {
                    if ( len - i  < 12 )
                        allzero = 0 ;
                    if ( zerotype == 1 )
                        strcat ( chinese, numberchar [ 0 ] );
                    strcat ( chinese, numberchar [ fundstr [ i ] - '0' ] );
                    if ( len - i - 1 < unit_num )
                        strcat ( chinese, rmbchar [ len - i - 1 ] );
                    zerotype = 0;
                    break;
                }
        }
    }

    if ( memcmp( fundstr + len -2 , "00", 2 ) == 0 ) strcat ( chinese, "整" );
    return 0;
}
