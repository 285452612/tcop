/*********************************************
  PrintText.c
  ������
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

            // ��� delim�ַ�ǰ��б�������
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
    int  iPageLine;     // ����ÿҳ����
    int  iPageBreak;    // �����ҳ��
    int  iPageNum;      // �Ƿ�Ҫҳ��
    char *pPageHeader;
    char *pRptHeader;
    char *pRptBody;
    char *pRptFooter;
    char *pPageFooter;
} RptPara;

typedef struct {
    int iRowCount;      // �����¼��
    char *pRptHeader;
    char *pRptBody;
    char *pRptFooter;
} RptData;

typedef struct {
    int FieldNo;   // ����
    int FieldPos;  // ��ƫ����
    int FieldLen;  // �򳤶�
    int AlignType; // ��Է���ʽ
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
 **      �ѹ̶���ʽ����������       **
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
        Fields[i].AlignType = 0; // Ĭ�Ͼ���
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
                INFO( "��ʽ�ļ� [] ƥ�����!" );
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
        // ����Ų����ֶ���Ϣ
        pField = FindRptField( j+1, Fields );
        if ( pField == NULL )
            continue;

        // ��ʽ�ļ����򳤶�С��ʵ������
        if ( strlen( words[j] ) > pField->FieldLen )
        {
            INFO( "��ʽ�ļ��б��Ϊ %d ���򳤶Ȳ���, ������Ҫ %d.",
                    pField->FieldNo, strlen( words[j] ) );
            *( words[j] + pField->FieldLen )= 0;
        }
        memset( out + pField->FieldPos, ' ', pField->FieldLen );
        if (strlen(words[j]) == 0)
            continue;

        switch ( pField->AlignType )
        {
            case 1:     // �����
                memcpy( out + pField->FieldPos, words[j],  
                        _MIN(pField->FieldLen, strlen(words[j])));
                break;
            case 2:     // �Ҷ���
                memcpy( out + pField->FieldPos+pField->FieldLen
                        -strlen(words[j]), words[j], strlen(words[j]) );
                break;
            case 0:     // ����
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
    Rpt.iRowCount   = 0;      // �����¼��
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
        INFO( "�������������ļ� %s ʧ��!", datafile );
        goto funcHandler;
    }

    //  ȡ�ñ���ͷ����
    Rpt.pRptHeader = XmlGetStringDup( xdata, "/Report/ReportHeader" );
    /*
    if ( Rpt.pRptHeader == NULL )
    {
        INFO( "δ�ҵ� %s ����ͷ����ReportHeader.", datafile );
        goto funcHandler;
    }
    */

    //  ȡ�ñ���������
    Rpt.pRptBody = XmlGetStringDup(xdata, "/Report/ReportBody");
    if ( Rpt.pRptBody == NULL )
    {
        INFO( "δ�ҵ� %s ����������ReportBody.", datafile );
        goto funcHandler;
    }

    //  ȡ�ñ���β����
    Rpt.pRptFooter = XmlGetStringDup( xdata, "/Report/ReportFooter" );
    /*
    if ( Rpt.pRptFooter == NULL )
    {
        INFO( "δ�ҵ� %s ����β����ReportFooter.", datafile );
        goto funcHandler;
    }
    */

    //  ȡ�ñ����ܼ�¼��
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

// �ͷŸ�ʽ�ļ�
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

// ��ø�ʽ�ļ�
RptPara GetRptPara( char *parafile )
{
    xmlDocPtr ParaDoc = NULL;
    RptPara Rpt = {0, 12, 0, NULL, NULL, NULL, NULL, NULL};
    char tmp[100];

    /*
    Rpt.iPageLine   = 0;
    Rpt.iPageBreak  = 12;   // �����ҳ��
    Rpt.pPageHeader = NULL;
    Rpt.pRptHeader  = NULL;
    Rpt.pRptBody    = NULL;
    Rpt.pRptFooter  = NULL;
    Rpt.pPageFooter = NULL;
    */

    ParaDoc = xmlParseFile( parafile );
    if ( ParaDoc == NULL )
    {
        INFO( "���������ʽ�ļ� %s ʧ��!", parafile );
        goto funcHandler;
    }

    // ȡ��ҳ��
    Rpt.iPageBreak = XmlGetInteger(ParaDoc, "/Report/PageBreak");

    //  ȡ��ҳü��ʽ
    Rpt.pPageHeader = XmlGetStringDup(ParaDoc, "/Report/PageHeader");

    //  ȡ�ñ���ͷ��ʽ
    Rpt.pRptHeader = XmlGetStringDup( ParaDoc, "/Report/ReportHeader" );

    //  ȡ�ñ������ʽ
    Rpt.pRptBody = XmlGetStringDup( ParaDoc, "/Report/ReportBody");
    if ( Rpt.pRptBody == NULL)
    {
        INFO( "δ�ҵ� %s �������ʽReportBody.", parafile );
        goto funcHandler;
    }

    //  ȡ�ñ���β��ʽ
    Rpt.pRptFooter = XmlGetStringDup( ParaDoc, "/Report/ReportFooter");

    //  ȡ��ҳ�Ÿ�ʽ
    Rpt.pPageFooter = XmlGetStringDup( ParaDoc, "/Report/PageFooter");

    // ȡÿҳ��¼��
    Rpt.iPageLine = XmlGetInteger(ParaDoc, "/Report/Lines");

    // ȡÿҳ��¼��
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
 **      ��ӡ�ӿں���               **
 *************************************/
int PrintReportList(char *parafile,char *datafile ,char *outfile )
{
    RptPara Para = {0, 12, 0, NULL, NULL, NULL, NULL, NULL};
    RptData Data = {0, NULL, NULL, NULL};
    char *lines[MAX_LINES];
    char Buf[ MAX_DATALEN ];
    int iTotalPage = 1;    // ������ҳ��
    FILE *fp = NULL;
    int i, j, iRet=-1;
    int m;

    Para = GetRptPara( parafile );
    if ( Para.iPageLine <= 0 )
        goto Error;

    Data = GetRptData( datafile );
    if ( Data.iRowCount <= 0 )
        goto Error;

    // ���㱨����ҳ��
    iTotalPage = Data.iRowCount / Para.iPageLine;
    if ( Data.iRowCount % Para.iPageLine != 0 )
        iTotalPage++;

    if((fp = fopen(outfile,"a+")) == NULL)
    {
        INFO("���������ļ�[%s]��!", outfile );
        goto Error;
    }

    m = getcols(Data.pRptBody, lines, MAX_LINES, '\n');
    if (m < Data.iRowCount)
    {
        INFO("�����ļ���¼������!");
        goto Error;
    }

    /*
    // ��ʼ��ҳ���ɱ���
    pCurNode = FirstRecordNode( Data.pRptBody );
    if ( pCurNode == NULL )
    {
        INFO( "%d, %s ��¼����RowCount��ƥ��.",__LINE__,datafile );
        goto Error;
    }
    */

    for ( i = 0; i < iTotalPage; i++ )
    {
        // ����ҳü
        if (Para.pPageHeader != NULL)
            fprintf( fp, "%s", Para.pPageHeader );

        // ���ɱ���ͷ
        if ( WriteData( Para.pRptHeader, Data.pRptHeader, Buf ) != 0 )
        {
            INFO( "%s ���ɱ���ͷ����.", parafile );
            goto Error;
        }
        fprintf( fp, "%s", Buf );

        // ���ɱ�����
        for ( j = 0; j < Para.iPageLine; j++ )
        {
            if (WriteData( Para.pRptBody, lines[i*Para.iPageLine+j], Buf) != 0)
            {
                INFO( "%s ���ɱ��������.", parafile );
                goto Error;
            }
            fprintf( fp, "%s", Buf );

            if ( ( i * Para.iPageLine + j + 1 ) == Data.iRowCount )
                break;
        }

        // ���ɱ���β
        if ( WriteData( Para.pRptFooter, Data.pRptFooter, Buf ) != 0 )
        {
            INFO( "%s ���ɱ���β����.", parafile );
            goto Error;
        }
        fprintf( fp, "%s", Buf );

        // ����ҳ��
        fprintf( fp, "%s", Para.pPageFooter );
        if (Para.iPageNum == 1)
            fprintf( fp, "%30s�� %d ҳ �� %d ҳ\n", " ", iTotalPage, i+1 );

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
    int m; // �ֶ���
    
    ParaDoc = xmlParseFile( parafile );
    if ( ParaDoc == NULL )
    {
        INFO( "���������ʽ�ļ� %s ʧ��!", parafile );
        return -1;
    }

    // ȡ��ҳ��
    PageBreak = XmlGetInteger(ParaDoc, "//PageBreak");
    // ȡ�����ݸ�ʽ
    XmlGetString(ParaDoc, "//ReportBody", para, sizeof(para));

    for ( i = 0; i < MAX_FIELDS; i++ )
    {
        Fields[i].FieldNo = 0;
        Fields[i].FieldPos = 0;
        Fields[i].FieldLen = 0;
        Fields[i].AlignType = 0; // Ĭ�Ͼ���
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
                INFO( "��ʽ�ļ� [] ƥ�����!" );
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
        // ����Ų����ֶ���Ϣ
        pField = FindRptField( j+1, Fields );
        if ( pField == NULL )
            continue;

        // ��ʽ�ļ����򳤶�С��ʵ������
        if ( strlen( words[j] ) > pField->FieldLen )
        {
            INFO( "��ʽ�ļ��б��Ϊ %d ���򳤶Ȳ���, ������Ҫ %d.",
                    pField->FieldNo, strlen( words[j] ) );
            *( words[j] + pField->FieldLen )= 0;
        }
        memset( out + pField->FieldPos, ' ', pField->FieldLen );
        if (strlen(words[j]) == 0)
            continue;

        switch ( pField->AlignType )
        {
            case 1:     // �����
                memcpy( out + pField->FieldPos, words[j],  
                        _MIN(pField->FieldLen, strlen(words[j])));
                break;
            case 2:     // �Ҷ���
                memcpy( out + pField->FieldPos+pField->FieldLen
                        -strlen(words[j]), words[j], strlen(words[j]) );
                break;
            case 0:     // ����
            default:
                memcpy( out+pField->FieldPos+
                        (pField->FieldLen-strlen(words[j]))/2, 
                        words[j], strlen(words[j]) );
                break;
        }
    }

    return 0;
}

