#ifndef __DBSTRU_H__
#define __DBSTRU_H__

#define DATATYPE_CHAR  1
#define DATATYPE_INT   2
#define DATATYPE_DBL   3

typedef struct {
    char *name;
    int  type;
} ST_DATATYPE;

static ST_DATATYPE stTrnjour[] = {
    { "InOutFlag",                     DATATYPE_CHAR },
    { "WorkDate",                      DATATYPE_CHAR },
    { "RefId",                         DATATYPE_CHAR },
    { "SeqNo",                         DATATYPE_INT },
    { "TrnCode",                       DATATYPE_CHAR },
    { "SvcClass",                       DATATYPE_INT },
    { "PresDate",                      DATATYPE_CHAR },
    { "PresTime",                      DATATYPE_CHAR },
    { "Originator",                    DATATYPE_CHAR },
    { "Acceptor",                      DATATYPE_CHAR },
    { "DCFlag",                        DATATYPE_CHAR },
    { "NoteType",                      DATATYPE_CHAR },
    { "NoteNo",                        DATATYPE_CHAR },
    { "CurCode",                       DATATYPE_CHAR },
    { "CurType",                       DATATYPE_CHAR },
    { "IssueDate",                     DATATYPE_CHAR },
    { "SettlAmt",                      DATATYPE_DBL },
    { "IssueAmt",                      DATATYPE_DBL },
    { "RemnAmt",                       DATATYPE_DBL },
    { "PayingAcct",                    DATATYPE_CHAR },
    { "Payer",                         DATATYPE_CHAR },
    { "PayingBank",                    DATATYPE_CHAR },
    { "PCBank",                        DATATYPE_CHAR },
    { "BeneAcct",                      DATATYPE_CHAR },
    { "BeneName",                      DATATYPE_CHAR },
    { "BeneBank",                      DATATYPE_CHAR },
    { "BCBank",                        DATATYPE_CHAR },
    { "Agreement",                     DATATYPE_CHAR },
    { "Purpose",                       DATATYPE_CHAR },
    { "Memo",                          DATATYPE_CHAR },
    { "TermType",                      DATATYPE_CHAR },
    { "TermId",                        DATATYPE_CHAR },
    { "AcctOper",                      DATATYPE_CHAR },
    { "Auditor",                       DATATYPE_CHAR },
    { "AuthDevId",                     DATATYPE_CHAR },
    { "PayKey",                        DATATYPE_CHAR },
    { "TestKey",                       DATATYPE_CHAR },
    { "WorkRound",                     DATATYPE_INT },
    { "ClearDate",                     DATATYPE_CHAR },
    { "ClearRound",                    DATATYPE_INT },
    { "ExchgDate",                     DATATYPE_CHAR },
    { "ExchgRound",                    DATATYPE_INT },
    { "ClearTime",                     DATATYPE_CHAR },
    { "ClearState",                    DATATYPE_CHAR },
    { "Result",                        DATATYPE_CHAR },
    { "FeePayer",                      DATATYPE_CHAR },
    { "FeeCode",                       DATATYPE_INT },
    { "Fee",                           DATATYPE_DBL },
    { "TruncFlag",                     DATATYPE_CHAR },
    { "ClearArea",                     DATATYPE_CHAR },
    { "ExchArea",                      DATATYPE_CHAR },
    { "RouteId",                       DATATYPE_INT },
    { "PresArea",                      DATATYPE_CHAR },
    { "AcptArea",                      DATATYPE_CHAR },
    { "ExtraDataFlag",                 DATATYPE_CHAR },
    { "AttachFile",                    DATATYPE_CHAR },
    { "Reserved",                      DATATYPE_CHAR },
    { "PrintNum",                      DATATYPE_INT },
};

#endif
