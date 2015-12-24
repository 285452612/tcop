#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "err.h"
#include "udb_wrapper.h"

static UDM_DB internal_db_handle;
static char data_empty[1] = { 0 };
static int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...);
static int getcols( char *line, char *words[], int maxwords, int delim );

static char *table_name(char *s_sql)
{
    char *sql = strdup(s_sql);
    char *pTableName;
    char *p;

    if (sql == NULL)
        return NULL;

    pTableName = (char *)malloc( 129 );
    if ( pTableName == NULL )
    {
        free(sql);
        return NULL;
    }
    memset( pTableName, 0, 129 );

    p = strtok( sql, "\t " );
    if ( p == NULL)
    {
        free( pTableName );
        free(sql);
        return NULL;
    }

    while( ( p = strtok( NULL, "\t " ) ) != NULL )
    {
        if ( strcasecmp( p, "from" ) != 0 )
            continue;
        if ( ( p = strtok( NULL, "\t " ) ) != NULL )
        {
#ifndef _MIN
#define _MIN(a, b) ( (a)<(b) ? (a) : (b) )
#endif
            strncpy( pTableName, p, _MIN(strlen(p), 128) );
            free(sql);
            return pTableName;
        }
    }

    free( pTableName );
    free(sql);
    return NULL;
}

int db_connect(const char *connection_string)
{
    char *string;
    int rc;

    if (connection_string == NULL)
    {
        if ((string = getenv("TCOP_DBURL")) == NULL)
        {
            db_log("env TCOP_DBURL not found.");
            return E_DB_URLCFG;
        }
    } else
        string = (char *)connection_string;

    UdmDBInit(&internal_db_handle);
    if ((rc = UdmDBSetAddr(&internal_db_handle, string, UDM_OPEN_MODE_WRITE)) != UDM_OK)
    {
        db_log("UdmDBSetAddr [%s] fail.", string);
        return E_DB_URLCFG;
    }

    if (internal_db_handle.sql->SQLConnect(&internal_db_handle) != UDM_OK)
    {
        db_log("Connect [%s] fail, (%d)[%s].", string, 
                internal_db_handle.errcode, internal_db_handle.errstr);
        return E_DB_OPEN;
    }
    return 0;
}

int db_close()
{
    if (internal_db_handle.connected != 0)
        internal_db_handle.sql->SQLClose(&internal_db_handle);
    return 0;
}

int db_free()
{
    if (internal_db_handle.connected != 0)
        internal_db_handle.sql->SQLClose(&internal_db_handle);
    UdmDBFree(&internal_db_handle);
    return 0;
}

#ifdef DEBUG
int db_exec_debug(char *file, int line, const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);

    if (UdmSQLExecDirect(&internal_db_handle, NULL, sqlstr) != UDM_OK)
    {
        WriteLogDebug(DBLOG, file, line, "%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        return E_DB;
    } 
    
    db_debug("%s", sqlstr);

    return 0;
}

int db_query_debug(char *file, int line, result_set *presult, const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);

    if (UdmSQLQuery(&internal_db_handle, presult, sqlstr) != UDM_OK)
    {
        WriteLogDebug(DBLOG, file, line, "%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        return E_DB_SELECT;
    }

    db_debug("%s", sqlstr);

    if (db_row_count(presult) == 0)
    {
        WriteLogDebug(DBLOG, file, line, "record not found.\nSQL: %s.", sqlstr);
        return E_DB_NORECORD;
    }

    return 0;
}
#else
int db_exec(const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);

    if (UdmSQLExecDirect(&internal_db_handle, NULL, sqlstr) != UDM_OK)
    {
        db_log("SQL error, errcode=%d, errstr=%s", internal_db_handle.errcode, internal_db_handle.errstr);
        db_log("SQLSTR: %s.", sqlstr);
        return E_DB;
    }
    return 0;
}

int db_query(result_set *presult, const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);

    if (UdmSQLQuery(&internal_db_handle, presult, sqlstr) != UDM_OK)
    {
        db_log("Query SQL fail:%s.\n%s", sqlstr, internal_db_handle.errstr);
        return E_DB_SELECT;
    }
    if (db_row_count(presult) == 0)
    {
        db_log("SQL not found, SQLSTR=%s.", sqlstr);
        UdmSQLFreeResultSimple(&internal_db_handle, presult);
        return E_DB_NORECORD;
    }

    return 0;
}
#endif

int db_query_nolog( result_set *presult, const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, presult, sqlstr) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        return E_DB_SELECT;
    }

    return 0;
}

int db_begin()
{
    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    return UdmSQLBegin(&internal_db_handle);
}

int db_commit()
{
    return UdmSQLCommit(&internal_db_handle);
}

int db_rollback()
{
    return db_exec("rollback work");
}

void db_free_result(result_set *result)
{
    UdmSQLFreeResultSimple(&internal_db_handle, result);
}

static char *rtrim(char *string)
{
    register int offset;

    offset = strlen(string) - 1;

    while(offset >= 0 && (string[offset] == ' ' || string[offset] == '\t')) {
        string[offset--] = '\0';
    }

    return string;
}

char *db_cell(result_set *result, int row, int col)
{
    char *p = (char *)UdmSQLValue(result, row, col);
    return (p ? rtrim(p) : data_empty);
}

char *db_cell_by_name(result_set *result, int row, const char *name)
{
    int col;

    if (!result) return data_empty;

    for ( col=0; col < result->nCols; col++ )
    {
        if (!strcasecmp(result->Fields[col].sqlname, name))
            break;
    }
    if ( col == result->nCols )
        return data_empty;

    return db_cell(result, row, col);
}

int db_cell_i(result_set *result, int row, int col)
{
    int i;
    i = atoi((const char *)db_cell(result, row, col));
    return i;
}

int db_cell_l(result_set *result, int row, int col)
{
    int i;
    i = atol((const char *)db_cell(result, row, col));
    return i;
}

double db_cell_d(result_set *result, int row, int col)
{
    double d;
    d = atof((const char *)db_cell(result, row, col));
    return d;
}

int db_col_count(result_set *result)
{
    return (result ? result->nCols : 0);
}

int db_row_count(result_set *result)
{
    return (result ? result->nRows : 0);
}

int db_query_str(char *buf, size_t len, const char *fmt, ...)
{
    char sqlstr[SQLBUFF_MAX];
    va_list args;
    UDM_SQLRES res;

    memset(buf, 0, len);
    sqlstr[0] = '\0';
    va_start(args, fmt);
    vsnprintf(sqlstr, sizeof(sqlstr)-1, fmt, args);
    va_end(args);

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        UdmSQLFreeResultSimple(&internal_db_handle, &res);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    if (UdmSQLNumRows(&res) != 1)
    {
        db_log("db_query_str: NOT EXPECTED ANSWER, SQL=%s.", sqlstr);
        UdmSQLFreeResultSimple(&internal_db_handle, &res);
        return E_DB_NORECORD;
    }

    strncpy(buf, db_cell(&res, 0, 0), len-1);
    UdmSQLFreeResultSimple(&internal_db_handle, &res);
    return 0;
}

int db_query_file( FILE *fp, char *pSplit, const char *fmt, ... )
{
    result_set res;
    char sqlstr[SQLBUFF_MAX];
    va_list args;
    int i, j;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("SQL [%s] error.\nerrcode=%d, errstr=%s", sqlstr, internal_db_handle.errcode, internal_db_handle.errstr);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    for (i = 0; i < res.nRows; i++)
    {
        for (j = 0; j < res.nCols-1; j++)
            fprintf(fp, "%s%s", db_cell(&res, i, j), pSplit);
        fprintf(fp, "%s\n", db_cell(&res, i, j));
    }

    UdmSQLFreeResultSimple(&internal_db_handle, &res);

    if (res.nRows == 0)
        return E_DB_NORECORD;

    return 0;
}

int db_export_file( FILE *fp, char *chardel, char *coldel, const char *fmt, ... )
{
    result_set res;
    char sqlstr[SQLBUFF_MAX];
    va_list args;
    int i, j;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("SQL [%s] error.\nerrcode=%d, errstr=%s", 
                sqlstr, internal_db_handle.errcode, internal_db_handle.errstr);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    for (i = 0; i < res.nRows; i++)
    {
        for (j = 0; j < res.nCols-1; j++)
            fprintf(fp, "%s%s%s%s",chardel, db_cell(&res,i,j), chardel, coldel);
        fprintf(fp, "%s%s%s\n", chardel, db_cell(&res, i, j), chardel);
    }

    UdmSQLFreeResultSimple(&internal_db_handle, &res);
    return 0;
}

int db_query_strs(char *sql, char*pdst, ...)
{
    result_set res;
    va_list ap;
    void *dstTmp = pdst;
    int i = 0;

    if (internal_db_handle.connected == 0)
        db_connect(NULL);

    if (UdmSQLQuery(&internal_db_handle, &res, sql) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sql);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sql);

    if (res.nRows == 0)
        return E_DB_NORECORD;
    if (res.nRows != 1)
        return E_DB_SELECT;

    va_start(ap, pdst);
    for (i = 0; i < res.nCols; i++)
    {
        strcpy(dstTmp, rtrim(db_cell(&res, 0, i)));
        dstTmp = va_arg(ap, char*);
    }
    va_end(ap);
    UdmSQLFreeResultSimple(&internal_db_handle, &res);

    return 0;
}

int db_query_constr( char *str, char *pSplit, const char *fmt, ... )
{
    result_set res;
    char sqlstr[SQLBUFF_MAX];
    va_list args;
    int i, j;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );
    *str = 0x00;

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    for (i = 0; i < res.nRows; i++)
    {
        for (j = 0; j < res.nCols; j++)
        {
            strcat(str, db_cell(&res, i, j));
            strcat(str, pSplit);
        }
        strcat(str, "\n");
    }

    UdmSQLFreeResultSimple(&internal_db_handle, &res);
    return 0;
}

int db_hasrecord(const char *pcTblName, const char *pcSelCond)
{
    result_set res;
    char sqlstr[2048];
    int  count=0;

    if ( pcSelCond == NULL || strlen( pcSelCond ) == 0 )
        snprintf(sqlstr, sizeof(sqlstr), "SELECT COUNT(1) FROM %s", pcTblName);
    else
        snprintf(sqlstr, sizeof(sqlstr), "SELECT COUNT(1) FROM %s WHERE %s",
                pcTblName, pcSelCond );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        UdmSQLFreeResultSimple(&internal_db_handle, &res);
        return 0;
    }
    else
        db_debug("%s", sqlstr);
    count = db_cell_i(&res, 0, 0);
    UdmSQLFreeResultSimple(&internal_db_handle, &res);
    if (count <= 0)
        return 0;
    return 1;
}

char *db_esc_str(char *dst, const char *src)
{
    size_t len = strlen(src);
    return UdmSQLEscStrGeneric(&internal_db_handle, dst, src, len);
}

int insert_tbl(char *dbtable, char *fields[], int fieldnum)
{
    result_set rs;
    char sqlstr[2048];
    char tmp[2048];
    int i;

    snprintf(sqlstr, sizeof(sqlstr), "select * from %s where 1=2", dbtable);
    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &rs, sqlstr) != UDM_OK)
    {
        db_log("%sSQL: %s.", internal_db_handle.errstr, sqlstr);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    if (fieldnum != rs.nCols)
    {
        db_log("insert_tbl fail, fieldnum[%d] != cols[%d]", fieldnum, rs.nCols);
        UdmSQLFreeResultSimple(&internal_db_handle, &rs);
        return E_DB_INSERT;
    }
    //(rs.Fields+i)->sqlname;
    memset(sqlstr, 0, sizeof(sqlstr));
    for (i = 0; i < fieldnum; i++)
    {
#ifdef SYBASE
        if ((rs.Fields+i)->sqltype == 0 || (rs.Fields+i)->sqltype == 4)
#else
        if ((rs.Fields+i)->sqltype == 1 || (rs.Fields+i)->sqltype == 12 
                || (rs.Fields+i)->sqltype == -1) //DB2
#endif
        {
            strcat(sqlstr, "'");
            if (strchr(fields[i], '\'') != NULL)
                strcat(sqlstr, db_esc_str(tmp, fields[i]));
            else
                strcat(sqlstr, fields[i]);
            strcat(sqlstr, "'");
        }
        else
        {
            if (!strcasecmp(fields[i], "null"))
                strcat(sqlstr, "0");
            else
                strcat(sqlstr, fields[i]);
        }
        strcat(sqlstr, ",");
    }
    sqlstr[strlen(sqlstr)-1] = 0;
    UdmSQLFreeResultSimple(&internal_db_handle, &rs);

    if (db_exec("INSERT INTO %s VALUES(%s)", dbtable, sqlstr) != 0)
        return E_DB_SELECT;
    return 0;
}

int db_sqlcode()
{
    return internal_db_handle.errcode;
}

int db_load_data(char *tbname, char *filename, int c)
{
    FILE *fp = NULL;
    char line[4096];
    char *fields[256];
    int  fieldnum;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        db_log("打开文件%s失败", filename);
        return -1;
    }

    while(fgets(line, sizeof(line), fp) != NULL)
    {
        if (strlen(line) <= 1)
            break;
        line[(int)strlen(line)-1] = 0;

        fieldnum = getcols(line, fields, 256, c);

        //保存到数据库表中
        if (insert_tbl(tbname, fields, fieldnum) != 0)
        {
            db_log("insert_tbl %s fail.", tbname);
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);

    return 0;
}

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

static void GetDateAndTime(char *cur_date, char *cur_time)
{
    struct tm t;
    time_t now;
 
    time(&now);
    t = *localtime(&now);

    *cur_date = *cur_time = 0x00;
    sprintf(cur_date, "%04d%02d%02d",
            1900 + t.tm_year, t.tm_mon + 1, t.tm_mday);
    sprintf(cur_time, "%02d%02d%02d",
            t.tm_hour, t.tm_min, t.tm_sec);

    return;
}

/* 写日志 */
static int WriteLogDebug(char *prefix, char *file, int line, char *pFmt, ...)
{
    static  pid_t pid_log = 0;
    FILE    *fp = NULL;
    char    pDate[9], pTime[7];
    char    pFile[256];
    int     iRc = 0;
    va_list ap;

    if (pid_log == 0)
        pid_log = getpid();
    GetDateAndTime(pDate, pTime);
    snprintf(pFile, sizeof(pFile)-1, "%s/log/%s_%s.log", 
            getenv("HOME"), prefix, pDate);
    if ((fp = fopen(pFile, "a+")) == NULL)
    {
        fprintf(stderr, "打开日志文件 %s 错, errno=%d.\n", pFile, errno);
        fp = stderr;
    }

    va_start(ap, pFmt);
    fprintf(fp, "%s %6d|" , pTime, pid_log);
    if (file != NULL)
        fprintf(fp, "%s, %d|" , basename(file), line);
    iRc = vfprintf(fp, pFmt, ap);
    fprintf(fp, "\n");
    va_end(ap);

    if (fp != stderr)
        fclose(fp);

    return 0;
}

int db_query_xml( xmlDocPtr *pxmlDoc, const char *fmt, ... )
{
    UDM_SQLRES res;
    char sqlstr[SQLBUFF_MAX];
    xmlNodePtr pNode = NULL;
    xmlNodePtr pFieldNode = NULL;
    va_list args;
    char *pTableName;
    char tmp[128];
    int i, j;
    int rc = E_DB;

    va_start( args, fmt );
    sqlstr[0] = '\0';
    vsnprintf( sqlstr, sizeof(sqlstr)-1, fmt, args );
    va_end( args );

    if (internal_db_handle.connected == 0)
        db_connect(NULL);
    if (UdmSQLQuery(&internal_db_handle, &res, sqlstr) != UDM_OK)
    {
        db_log("SQL [%s] error.\nerrcode=%d, errstr=%s", 
                sqlstr, internal_db_handle.errcode, internal_db_handle.errstr);
        return E_DB_SELECT;
    }
    else
        db_debug("%s", sqlstr);

    if ( res.nRows == 0 )
    {
        db_log("Record Not Found.\nSQL String: %s.", sqlstr);
        rc = E_DB_NORECORD;
        goto err_handler;
    }

    pTableName = table_name(sqlstr);
    if (pTableName == NULL)
    {
        db_log("Get TableName fail.\nSQL String: %s.", sqlstr);
        goto err_handler;
    }
    *pxmlDoc = XmlNewDocEnc(pTableName, "GB18030");
    free( pTableName );
    if ( *pxmlDoc == NULL )
    {
        db_log("XML func error.\nSQL String: %s.", sqlstr);
        goto err_handler;
    }

    pNode = xmlDocGetRootElement( *pxmlDoc );

    sprintf(tmp, "%d", res.nRows);
    if ( XmlNewTextChild(pNode, "ROWCOUNT", tmp) == NULL )
    {
        db_log("XML func error.\nSQL String: %s.", sqlstr);
        goto err_handler;
    }

    for (i = 0; i < res.nRows; i++)
    {
        if ((pFieldNode = XmlNewTextChild(pNode, "RECORD", NULL)) == NULL)
        {
            db_log("XML New RECORD fail.\nSQL String: %s.", sqlstr );
            goto err_handler;
        }

        sprintf( tmp, "%d", i+1);
        if (xmlNewProp(pFieldNode, "rowid", tmp) == NULL)
        {
            db_log("XML New rowid fail.\nSQL String: %s.", sqlstr);
            goto err_handler;
        }

        for (j = 0; j < res.nCols; j++)
            XmlNewTextChild(pFieldNode, res.Fields[j].sqlname, 
                    db_cell(&res, i, j));
    }
    rc = 0;

err_handler:
    UdmSQLFreeResultSimple(&internal_db_handle, &res);
    if (rc != 0 && *pxmlDoc != NULL)
        xmlFreeDoc( *pxmlDoc );
    return rc;
}

char *org_name(char *orgid, char *orgname)
{
    result_set rs;

    *orgname = 0x00;
    if (db_query(&rs, "select distinct orgname from organinfo "
                "where orgid='%s' and orgtype='1'", orgid) != 0)
        return orgname;
    strncpy(orgname, db_cell(&rs, 0, 0), 80);
    db_free_result(&rs);

    return orgname;
}
