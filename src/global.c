#include "app.h"
#include "flow.h"

int  TCOP_BANKID;
int  OP_OPNODE                   = 9999;
int  OP_NODEID;

int  OP_TCTCODE = 0;
int  OP_TRNCODE = 0;
int  OP_OPTCODE = 0;
int  OP_BKTCODE = 0;
int  OP_COMCODE = 0;

char OP_APPTYPE                  = APPTYPE_UNDEF;
char OP_APPPK                    = APPPK_UNCHECK;
int  OP_APPEXTEND                = 0;
int  OP_DOINIT                   = 0;
char OP_HOME[128]                = {0};
char OP_APPNAME[256]             = {0};

char G_REQFILE[COMMFNAME_MAXLEN] = {0};
char G_RSPFILE[COMMFNAME_MAXLEN] = {0};

char *program_name              = "appname";

OPFlows OP_FLOW;
