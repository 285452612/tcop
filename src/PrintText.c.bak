/*********************************************
  PrintText.c
  报表工具
  Created by Chen Jie
  2006/09/14
*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "SDKpub.h"
#include "SDKxml.h"
#include "interface.h"

#define MAX_FIELDS   100 
#define MAX_FIELDLEN  128
#define MAX_DATALEN   4096
#define MAX_LINES     100000
#define MAX_OPEN_FILES     10

#ifndef _MIN
#define _MIN(a, b) ( (a)<(b) ? (a) : (b) )
#endif

static int getcols( char *line, char *words[], int maxwords, int delim ) 
{
    char *p = line, *p2;
    int nwords = 0;
    int append = 0;

    if (line[strlen(line)-1] == delim)
        append = 1;

    while ( *p != '\0' )
    {
        words[nwords++] = p;
        if ( nwords >= maxwords )
            return nwords; 

        while(1)
        {       
            p2 = strchr( p, delim );
            if ( p2 == NULL )
                break;  

            // 如果 delim字符前有斜杠则忽略
            if (p2-1 == NULL || *(p2-1) != '\\')
                break;  

            memmove(p2-1, p2, strlen(p2)+1);
            p = p2; 
        }       
        if (p2 == NULL)
            break;  

        *p2 = '\0'; 
        p = p2 + 1;
    }

    if (append == 1)
    {
        words[nwords] = strdup(" ");
        *(words[nwords]) = 0;
    }

    return nwords + append;
}

static void ifree(char *p)
{   
    if (p != NULL)
    {
        free(p); p = NULL;
    }
}

typedef struct {
    int  iPageLine;     // 报表每页行数
    int  iPageBreak;    // 报表分页符
    int  iPageNum;      // 是否要页码
    char *pPageHeader;
    char *pRptHeader;
    char *pRptBody;
    char *pRptFooter;
    char *pPageFooter;
} RptPara;

typedef struct {
    int iRowCount;      // 报表记录数
    char *pRptHeader;
    char *pRptBody;
    char *pRptFooter;
} RptData;

typedef struct {
    int FieldNo;   // 域编号
    int FieldPos;  // 域偏移量
    int FieldLen;  // 域长度
    int AlignType; // 域对方方式
} ReportField;

xmlNodePtr FirstRecordNode( xmlNodePtr node )
{
    xmlNodePtr pCurNode;

    pCurNode = node->xmlChildrenNode;
    while ( pCurNode != NULL )
    {
        if ( !strcasecmp( (char *)pCurNode->name, "RECORD" ) )
            break;
        pCurNode = pCurNode->next;
    }

    return pCurNode;
}

xmlNodePtr NextRecordNode( xmlNodePtr node )
{
    node = node->next;
    while ( node != NULL )
    {
        if ( !strcasecmp( (char *)node->name, "RECORD" ) )
            break;
        node = node->next;
    }

    return node;
}


int GetReportFieldNo( char *Buf, int *Len )
{
    char FieldNo[10];
    char tmp[MAX_FIELDLEN];
    char *p;
    int i;

    memset( tmp, 0, sizeof(tmp) );
    strncpy( tmp, Buf, sizeof(tmp)-1 );
    p = tmp;
    for ( i = 0; *p != ' ' && *p != ']'; p++, i++ )
    {
        if ( i == 9 )
            break;
        FieldNo[i] = *p;
    }
    FieldNo[i] = 0;

    *Len = strlen( FieldNo );

    return atoi( FieldNo );
}

ReportField *FindRptField( int iFieldNo, ReportField *pFields )
{
    ReportField *p=NULL;
    int i;

    for ( p = pFields, i=0; i < MAX_FIELDS; p++, i++ )
    {
        if ( p->FieldNo == 0 )
            break;
        if ( p->FieldNo == iFieldNo )
            return p;
    }

    return NULL;
}

/*************************************
 **      把固定格式用数据填入       **
 *************************************/
int WriteData( char *Para, char *Line, char *out )
{
    ReportField Fields[MAX_FIELDS], *pField;
    char para[ MAX_DATALEN ];
    char *line = NULL;
    char *words[MAX_FIELDS];
    int  Len = 0;
    int  FieldNoLen = 0;
    int  i = 0, j = 0, FieldNum;
    char *p=NULL;
    int m;
    
    if (Para == NULL || *Para == 0x00 || strlen(Para) <= 2)
    {
        *out = 0;
        return 0;
    }

    memset( para, 0, sizeof(para) );
    if ( *(Para+strlen(Para)-1) == '\n' )
        strncpy( para, Para, strlen(Para)-1 );
    else
        strncpy( para, Para, strlen(Para) );

    for ( i = 0; i < MAX_FIELDS; i++ )
    {
        Fields[i].FieldNo = 0;
        Fields[i].FieldPos = 0;
        Fields[i].FieldLen = 0;
        Fields[i].AlignType = 0; // 默认居中
    }

    strcpy( out, para );
    p = para;
    for ( i = 0, FieldNum = -1; *p != 0; p++, i++ )
    {
        if ( *p == '[' )
        {
            Fields[++FieldNum].FieldPos = i;
            if ( *(p+1) != ' ' )
                Fields[FieldNum].AlignType = 1;
            Len = 1;
            continue;
        }
        if ( Len >= 1 && *p != ']' )
        {
            if ( *p == '[' )
            {
                INFO( "格式文件 [] 匹配错误!" );
                return -1;
            }
            if ( *p != ' ' )
            {
                Fields[FieldNum].FieldNo = GetReportFieldNo( p, &FieldNoLen );
                Len += FieldNoLen;
                i += FieldNoLen-1;
                p += FieldNoLen-1;
                continue;
            }
            Len++;
            continue;
        }
        if ( Len >= 1 && *p == ']' )
        {
            Fields[FieldNum].FieldLen = Len+1;
            if ( *(p-1) != ' ' )
                Fields[FieldNum].AlignType = 2;
            Len = 0;
            continue;
        }
    }

    if ( Fields[0].FieldNo == 0 )
        return 0;

    line = strdup(Line);
    m = getcols(line, words, MAX_FIELDS, ';');
    for(j = 0; j < m; j++)
    {
        // 按编号查找字段信息
        pField = FindRptField( j+1, Fields );
        if ( pField == NULL )
            continue;

        // 格式文件的域长度小于实际内容
        if ( strlen( words[j] ) > pField->FieldLen )
        {
            INFO( "格式文件中编号为 %d 的域长度不足, 至少需要 %d.",
                    pField->FieldNo, strlen( words[j] ) );
            *( words[j] + pField->FieldLen )= 0;
        }
        memset( out + pField->FieldPos, ' ', pField->FieldLen );
        if (strlen(words[j]) == 0)
            continue;

        switch ( pField->AlignType )
        {
            case 1:     // 左对齐
                memcpy( out + pField->FieldPos, words[j],  
                        _MIN(pField->FieldLen, strlen(words[j])));
                break;
            case 2:     // 右对齐
                memcpy( out + pField->FieldPos+pField->FieldLen
                        -strlen(words[j]), words[j], strlen(words[j]) );
                break;
            case 0:     // 居中
            default:
                memcpy( out+pField->FieldPos+
                        (pField->FieldLen-strlen(words[j]))/2, 
                        words[j], strlen(words[j]) );
                break;
        }
    }
    ifree(line);

    return 0;
}

void FreeRptData( RptData Rpt )
{
    Rpt.iRowCount   = 0;      // 报表记录数
    ifree(Rpt.pRptHeader);
    ifree(Rpt.pRptBody);
    ifree(Rpt.pRptFooter);
}

RptData GetRptData( char *datafile )
{
    xmlDocPtr xdata = NULL;
    RptData Rpt = {0, NULL, NULL, NULL};

    xdata = xmlParseFile( datafile );
    if ( xdata == NULL )
    {
        INFO( "解析报表数据文件 %s 失败!", datafile );
        goto funcHandler;
    }

    //  取得报表头数据
    Rpt.pRptHeader = XmlGetStringDup( xdata, "/Report/ReportHeader" );
    /*
    if ( Rpt.pRptHeader == NULL )
    {
        INFO( "未找到 %s 报表头数据ReportHeader.", datafile );
        goto funcHandler;
    }
    */

    //  取得报表体数据
    Rpt.pRptBody = XmlGetStringDup(xdata, "/Report/ReportBody");
    if ( Rpt.pRptBody == NULL )
    {
        INFO( "未找到 %s 报表体数据ReportBody.", datafile );
        goto funcHandler;
    }

    //  取得报表尾数据
    Rpt.pRptFooter = XmlGetStringDup( xdata, "/Report/ReportFooter" );
    /*
    if ( Rpt.pRptFooter == NULL )
    {
        INFO( "未找到 %s 报表尾数据ReportFooter.", datafile );
        goto funcHandler;
    }
    */

    //  取得报表总记录数
    Rpt.iRowCount = XmlGetInteger(xdata, "/Report/RowCount");

funcHandler:

    xmlFreeDoc(xdata);

    if( Rpt.iRowCount <= 0 )
    {
        FreeRptData( Rpt );
        return Rpt;
    }

    return Rpt;
}

// 释放格式文件
void FreeRptPara( RptPara Rpt )
{
    Rpt.iPageLine = 0;
    Rpt.iPageBreak = 0;
    Rpt.iPageNum = 0;

    ifree( Rpt.pPageHeader );
    ifree( Rpt.pRptHeader );
    ifree( Rpt.pRptBody );
    ifree( Rpt.pRptFooter );
    ifree( Rpt.pPageFooter );
}

// 获得格式文件
RptPara GetRptPara( char *parafile )
{
    xmlDocPtr ParaDoc = NULL;
    RptPara Rpt = {0, 12, 0, NULL, NULL, NULL, NULL, NULL};
    char tmp[100];

    /*
    Rpt.iPageLine   = 0;
    Rpt.iPageBreak  = 12;   // 报表分页符
    Rpt.pPageHeader = NULL;
    Rpt.pRptHeader  = NULL;
    Rpt.pRptBody    = NULL;
    Rpt.pRptFooter  = NULL;
    Rpt.pPageFooter = NULL;
    */

    ParaDoc = xmlParseFile( parafile );
    if ( ParaDoc == NULL )
    {
        INFO( "解析报表格式文件 %s 失败!", parafile );
        goto funcHandler;
    }

    // 取分页符
    Rpt.iPageBreak = XmlGetInteger(ParaDoc, "/Report/PageBreak");

    //  取得页眉格式
    Rpt.pPageHeader = XmlGetStringDup(ParaDoc, "/Report/PageHeader");

    //  取得报表头格式
    Rpt.pRptHeader = XmlGetStringDup( ParaDoc, "/Report/ReportHeader" );

    //  取得报表体格式
    Rpt.pRptBody = XmlGetStringDup( ParaDoc, "/Report/ReportBody");
    if ( Rpt.pRptBody == NULL)
    {
        INFO( "未找到 %s 报表体格式ReportBody.", parafile );
        goto funcHandler;
    }

    //  取得报表尾格式
    Rpt.pRptFooter = XmlGetStringDup( ParaDoc, "/Report/ReportFooter");

    //  取得页脚格式
    Rpt.pPageFooter = XmlGetStringDup( ParaDoc, "/Report/PageFooter");

    // 取每页记录数
    Rpt.iPageLine = XmlGetInteger(ParaDoc, "/Report/Lines");

    // 取每页记录数
    XMLGetVal(ParaDoc, "/Report/Lines", tmp);
    if (strcasecmp(tmp, "yes") == 0)
        Rpt.iPageNum = 1;
    else
        Rpt.iPageNum = 0;

funcHandler:

    if ( Rpt.iPageLine <= 0 )
        FreeRptPara( Rpt );

    xmlFreeDoc( ParaDoc );

    return Rpt;
}


/************************************
 **      打印接口函数               **
 *************************************/
int PrintReportList(char *parafile,char *datafile ,char *outfile )
{
    RptPara Para = {0, 12, 0, NULL, NULL, NULL, NULL, NULL};
    RptData Data = {0, NULL, NULL, NULL};
    char *lines[MAX_LINES];
    char Buf[ MAX_DATALEN ];
    int iTotalPage = 1;    // 报表总页数
    FILE *fp = NULL;
    int i, j, iRet=-1;
    int m;

    Para = GetRptPara( parafile );
    if ( Para.iPageLine <= 0 )
        goto Error;

    Data = GetRptData( datafile );
    if ( Data.iRowCount <= 0 )
        goto Error;

    // 计算报表总页数
    iTotalPage = Data.iRowCount / Para.iPageLine;
    if ( Data.iRowCount % Para.iPageLine != 0 )
        iTotalPage++;

    if((fp = fopen(outfile,"a+")) == NULL)
    {
        INFO("创建报表文件[%s]错!", outfile );
        goto Error;
    }

    m = getcols(Data.pRptBody, lines, MAX_LINES, '\n');
    if (m < Data.iRowCount)
    {
        INFO("数据文件记录数不符!");
        goto Error;
    }

    /*
    // 开始按页生成报表
    pCurNode = FirstRecordNode( Data.pRptBody );
    if ( pCurNode == NULL )
    {
        INFO( "%d, %s 记录数与RowCount不匹配.",__LINE__,datafile );
        goto Error;
    }
    */

    for ( i = 0; i < iTotalPage; i++ )
    {
        // 生成页眉
        if (Para.pPageHeader != NULL)
            fprintf( fp, "%s", Para.pPageHeader );

        // 生成报表头
        if ( WriteData( Para.pRptHeader, Data.pRptHeader, Buf ) != 0 )
        {
            INFO( "%s 生成报表头出错.", parafile );
            goto Error;
        }
        fprintf( fp, "%s", Buf );

        // 生成报表体
        for ( j = 0; j < Para.iPageLine; j++ )
        {
            if (WriteData( Para.pRptBody, lines[i*Para.iPageLine+j], Buf) != 0)
            {
                INFO( "%s 生成报表体出错.", parafile );
                goto Error;
            }
            fprintf( fp, "%s", Buf );

            if ( ( i * Para.iPageLine + j + 1 ) == Data.iRowCount )
                break;
        }

        // 生成报表尾
        if ( WriteData( Para.pRptFooter, Data.pRptFooter, Buf ) != 0 )
        {
            INFO( "%s 生成报表尾出错.", parafile );
            goto Error;
        }
        fprintf( fp, "%s", Buf );

        // 生成页脚
        fprintf( fp, "%s", Para.pPageFooter );
        if (Para.iPageNum == 1)
            fprintf( fp, "%30s总 %d 页 第 %d 页\n", " ", iTotalPage, i+1 );

        if ( Para.iPageBreak != 0 )
            fputc( Para.iPageBreak, fp );
        fprintf( fp, "\n" );
    }
    iRet = 0;

Error:
    if ( fp != NULL )
        fclose( fp );

    FreeRptPara( Para );
    FreeRptData( Data );

    return iRet;
}

xmlDocPtr GetReportDataDoc()
{
    xmlDocPtr doc;
    xmlNodePtr node;

    doc = XMLNewDocEnc("Report", "GB18030");
    if (doc == NULL)
        return NULL;

    node = xmlDocGetRootElement(doc);
    xmlNewChild(node, NULL, (xmlChar *)"RowCount", NULL);
    xmlNewChild(node, NULL, (xmlChar *)"ReportHeader", NULL);
    xmlNewChild(node, NULL, (xmlChar *)"ReportBody", NULL);
    xmlNewChild(node, NULL, (xmlChar *)"ReportFooter", NULL);

    return doc;
}

void WriteRptHeader(FILE *fp, const char *fmt, ...)
{
    va_list args;
    char string[1024];

    va_start( args, fmt );
    string[0] = '\0';
    vsnprintf( string, 1024, fmt, args );
    va_end( args );

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"GB18030\"?>\n"
            "<Report><ReportHeader>%s</ReportHeader><ReportBody>", string);
}
void WriteRptRowCount(FILE *fp, int count)
{
    fprintf(fp, "</ReportBody><RowCount>%d</RowCount>", count);
}
void WriteRptFooter(FILE *fp, const char *fmt, ...)
{
    va_list args;
    char string[1024];

    va_start( args, fmt );
    string[0] = '\0';
    vsnprintf( string, 1024, fmt, args );
    va_end( args );
    fprintf(fp, "<ReportFooter>%s</ReportFooter></Report>\n", string);
}

/*
int main(void)
{
    PrintReportList("InQuery.para","data.tmp","out");

    return 0;
}
*/

int GenFormatData(char *parafile, char *line, char *out)
{
    xmlDocPtr ParaDoc;
    ReportField Fields[MAX_FIELDS], *pField;
    char para[ MAX_DATALEN ];
    char *words[MAX_FIELDS];
    int  Len = 0;
    int  FieldNoLen = 0;
    int  i = 0, j = 0, FieldNum, PageBreak = 12;
    char *p=NULL;
    int m; // 字段数
    
    ParaDoc = xmlParseFile( parafile );
    if ( ParaDoc == NULL )
    {
        INFO( "解析报表格式文件 %s 失败!", parafile );
        return -1;
    }

    // 取分页符
    PageBreak = XmlGetInteger(ParaDoc, "//PageBreak");
    // 取得内容格式
    XmlGetString(ParaDoc, "//ReportBody", para, sizeof(para));

    for ( i = 0; i < MAX_FIELDS; i++ )
    {
        Fields[i].FieldNo = 0;
        Fields[i].FieldPos = 0;
        Fields[i].FieldLen = 0;
        Fields[i].AlignType = 0; // 默认居中
    }

    strcpy( out, para );
    p = para;
    for ( i = 0, FieldNum = -1; *p != 0; p++, i++ )
    {
        if ( *p == '[' )
        {
            Fields[++FieldNum].FieldPos = i;
            if ( *(p+1) != ' ' )
                Fields[FieldNum].AlignType = 1;
            Len = 1;
            continue;
        }
        if ( Len >= 1 && *p != ']' )
        {
            if ( *p == '[' )
            {
                INFO( "格式文件 [] 匹配错误!" );
                return -1;
            }
            if ( *p != ' ' )
            {
                Fields[FieldNum].FieldNo = GetReportFieldNo( p, &FieldNoLen );
                Len += FieldNoLen;
                i += FieldNoLen-1;
                p += FieldNoLen-1;
                continue;
            }
            Len++;
            continue;
        }
        if ( Len >= 1 && *p == ']' )
        {
            Fields[FieldNum].FieldLen = Len+1;
            if ( *(p-1) != ' ' )
                Fields[FieldNum].AlignType = 2;
            Len = 0;
            continue;
        }
    }

    if ( Fields[0].FieldNo == 0 )
        return 0;

    m = getcols(line, words, MAX_FIELDS, ';');
    for(j = 0; j < m; j++)
    {
        // 按编号查找字段信息
        pField = FindRptField( j+1, Fields );
        if ( pField == NULL )
            continue;

        // 格式文件的域长度小于实际内容
        if ( strlen( words[j] ) > pField->FieldLen )
        {
            INFO( "格式文件中编号为 %d 的域长度不足, 至少需要 %d.",
                    pField->FieldNo, strlen( words[j] ) );
            *( words[j] + pField->FieldLen )= 0;
        }
        memset( out + pField->FieldPos, ' ', pField->FieldLen );
        if (strlen(words[j]) == 0)
            continue;

        switch ( pField->AlignType )
        {
            case 1:     // 左对齐
                memcpy( out + pField->FieldPos, words[j],  
                        _MIN(pField->FieldLen, strlen(words[j])));
                break;
            case 2:     // 右对齐
                memcpy( out + pField->FieldPos+pField->FieldLen
                        -strlen(words[j]), words[j], strlen(words[j]) );
                break;
            case 0:     // 居中
            default:
                memcpy( out+pField->FieldPos+
                        (pField->FieldLen-strlen(words[j]))/2, 
                        words[j], strlen(words[j]) );
                break;
        }
    }

    return 0;
}

