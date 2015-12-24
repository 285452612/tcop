#ifndef _DB_WRAPPER_H_
#define _DB_WRAPPER_H_

#include "udm_common.h"
#include "udm_sqldbms.h"
#include "udm_db.h"
#include "udm_utils.h"
#include "udm_vars.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/globals.h>
#include <libxml/xmlschemas.h>

typedef UDM_SQLRES result_set;

#ifdef DB2
    #define IS_NULL                 "coalesce"
#elif SYBASE
    #define IS_NULL                 "isnull"
#elif ORACLE
    #define IS_NULL                 "nvl"
#endif

#define ROOTNODE xmlDocGetRootElement
#define ConvIn(in) (xmlChar *)encoding_conv((char *)in, "GB18030", "UTF-8")
#define ConvOut(in) (char *)encoding_conv((char *)in, "UTF-8", "GB18030")
#define UTF8_TO_GB18030(src, src_size, dest, dest_size) Iconv(src, src_size, "UTF-8", dest, dest_size, "GB18030")
#define GB18030_TO_UTF8(src, src_size, dest, dest_size) Iconv(src, src_size, "GB18030", dest, dest_size, "UTF-8")

xmlDocPtr XmlNewDocEnc(char *root, const char *encoding);
xmlNodePtr XmlNewTextChild(xmlNodePtr node, char *name, char *val);
char *XmlGetString(xmlDocPtr doc, const char *xpath, char *val, size_t size);
int XmlSetString(xmlDocPtr doc, const char *xpath, char *val);
int XmlSetVal(xmlDocPtr doc, const char *xpath, char *val);
char *XmlGetStringDup( xmlDocPtr doc, const char *xpath);
xmlXPathObjectPtr getnodeset(xmlDocPtr doc, const char *xpath);
xmlNodePtr XmlLocateNode(xmlDocPtr doc, const char *xpath);
xmlNodePtr XmlLocateNodeNew(xmlDocPtr doc, const char *xpath);
char *XmlNodeGetAttrText(xmlNodePtr node, char *name);
xmlNodePtr XmlFetchNode(xmlNodePtr pnNode, const char *xKey);

#define DBLOG "db"

#define db_log(fmt...) WriteLogDebug(DBLOG, __FILE__, __LINE__, fmt)

#define SQLBUFF_MAX 8192

#define db_debug(fmt...) do \
{ \
    char *flag; \
    if ((flag = getenv("TCOP_DBDEBUG")) == NULL) \
    break; \
    if (*flag != '1') \
    break; \
    WriteLogDebug(DBLOG, NULL, 0, fmt); \
} while (0)

int db_connect(const char *connection_string);
int db_close();

int db_exec_debug(char *file, int line, const char *fmt, ...);
int db_query_debug(char *file, int line, result_set *result, const char *fmt, ... );

#ifdef DEBUG
#define db_exec(fmt...) db_exec_debug(__FILE__, __LINE__, fmt)
#define db_query(result, fmt...) db_query_debug(__FILE__, __LINE__, result, fmt)
#else
int db_exec(const char *fmt, ...);
int db_query(result_set *result, const char *fmt, ...);
#endif

int db_query_str(char *buf, size_t len, const char *fmt, ...);
int db_query_constr( char *str, char *pSplit, const char *fmt, ... );
int db_query_file(FILE *fp, char *pSplit, const char *fmt, ...);
int db_query_xml(xmlDocPtr *pxmlDoc, const char *fmt, ... );
int db_export_file(FILE *fp, char *chardel,char *coldel, const char *fmt, ...);
int db_sqlcode();

//Free result
void db_free_result(result_set *result);

//Get cell data:
char   db_is_null(result_set *result, int row, int col);
char*  db_cell(result_set *result, int row, int col);
char*  db_cell_by_name(result_set *result, int row, const char *name);
int    db_cell_i(result_set *result, int row, int col);
int    db_cell_l(result_set *result, int row, int col);
double db_cell_d(result_set *result, int row, int col);

//Table dimenstiona:
int   db_col_count(result_set *);
int   db_row_count(result_set *);
char* db_esc_str(char *dst, const char *src);
int   db_load_data(char *tbname, char *filename, int c);
int   db_hasrecord(const char *pcTblName, const char *pcSelCond);
int   db_begin();
int   db_commit();
int   db_rollback();
#endif
