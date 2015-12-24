/*==============================================================*/
/* DBMS name:      Sybase AS Enterprise 12.5                    */
/* Created on:     2010-4-7 11:34:47                            */
/*==============================================================*/


if exists (select 1
            from  sysobjects
            where id = object_id('colsdesc')
            and   type = 'U')
   drop table colsdesc 
go

if exists (select 1
            from  sysobjects
            where id = object_id('codetype')
            and   type = 'U')
   drop table codetype 
go

if exists (select 1
            from  sysobjects
            where id = object_id('generalcode')
            and   type = 'U')
   drop table generalcode  
go

if exists (select 1
            from  sysobjects
            where id = object_id('funclist')
            and   type = 'U')
   drop table funclist 
go

if exists (select 1
            from  sysobjects
            where id = object_id('nodeinfo')
            and   type = 'U')
   drop table nodeinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('acctjour')
            and   name  = 'index_4'
            and   indid > 0
            and   indid < 255)
   drop index acctjour.index_4
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('acctjour')
            and   name  = 'index_3'
            and   indid > 0
            and   indid < 255)
   drop index acctjour.index_3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('acctjour')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index acctjour.index_2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('acctjour')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index acctjour.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('acctjour')
            and   type = 'U')
   drop table acctjour
go

if exists (select 1
            from  sysobjects
            where id = object_id('agreement')
            and   type = 'U')
   drop table agreement
go

if exists (select 1
            from  sysobjects
            where id = object_id('baginfo')
            and   type = 'U')
   drop table baginfo 
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('bankinfo')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index bankinfo.index_2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('bankinfo')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index bankinfo.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('bankinfo')
            and   type = 'U')
   drop table bankinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('bankjour')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index bankjour.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('bankjour')
            and   type = 'U')
   drop table bankjour
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('ebanksumm')
            and   name  = 'ebanksumm_idx2'
            and   indid > 0
            and   indid < 255)
   drop index ebanksumm.ebanksumm_idx2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('ebanksumm')
            and   name  = 'ebanksumm_idx1'
            and   indid > 0
            and   indid < 255)
   drop index ebanksumm.ebanksumm_idx1
go

if exists (select 1
            from  sysobjects
            where id = object_id('ebanksumm')
            and   type = 'U')
   drop table ebanksumm
go

if exists (select 1
            from  sysobjects
            where id = object_id('errinfo')
            and   type = 'U')
   drop table errinfo
go

if exists (select 1
            from  sysobjects
            where id = object_id('errmap')
            and   type = 'U')
   drop table errmap
go

if exists (select 1
            from  sysobjects
            where id = object_id('feelist')
            and   type = 'U')
   drop table feelist
go

if exists (select 1
            from  sysobjects
            where id = object_id('feeset')
            and   type = 'U')
   drop table feeset
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('feesum')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index feesum.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('feesum')
            and   type = 'U')
   drop table feesum
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('feetype')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index feetype.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('feetype')
            and   type = 'U')
   drop table feetype
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('freemsg')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index freemsg.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('freemsg')
            and   type = 'U')
   drop table freemsg
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'index_8'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.index_8
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx7'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx7
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx6'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx6
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx5'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx5
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx4'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx4
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx3'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('htrnjour')
            and   name  = 'trnjour_idx2'
            and   indid > 0
            and   indid < 255)
   drop index htrnjour.trnjour_idx2
go

if exists (select 1
            from  sysobjects
            where id = object_id('htrnjour')
            and   type = 'U')
   drop table htrnjour
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('noteinfo')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index noteinfo.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('noteinfo')
            and   type = 'U')
   drop table noteinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('notetypemap')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index notetypemap.index_2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('notetypemap')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index notetypemap.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('notetypemap')
            and   type = 'U')
   drop table notetypemap
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('operinfo')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index operinfo.index_2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('operinfo')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index operinfo.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('operinfo')
            and   type = 'U')
   drop table operinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('operjour')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index operjour.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('operjour')
            and   type = 'U')
   drop table operjour
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('organinfo')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index organinfo.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('organinfo')
            and   type = 'U')
   drop table organinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('queryinfo')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index queryinfo.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('queryinfo')
            and   type = 'U')
   drop table queryinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('reconinfo')
            and   name  = 'reconinfo_idx3'
            and   indid > 0
            and   indid < 255)
   drop index reconinfo.reconinfo_idx3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('reconinfo')
            and   name  = 'reconinfo_idx2'
            and   indid > 0
            and   indid < 255)
   drop index reconinfo.reconinfo_idx2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('reconinfo')
            and   name  = 'reconinfo_idx1'
            and   indid > 0
            and   indid < 255)
   drop index reconinfo.reconinfo_idx1
go

if exists (select 1
            from  sysobjects
            where id = object_id('reconinfo')
            and   type = 'U')
   drop table reconinfo
go

if exists (select 1
            from  sysobjects
            where id = object_id('regioninfo')
            and   type = 'U')
   drop table regioninfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('syspara')
            and   name  = 'index_1'
            and   indid > 0
            and   indid < 255)
   drop index syspara.index_1
go

if exists (select 1
            from  sysobjects
            where id = object_id('syspara')
            and   type = 'U')
   drop table syspara
go

if exists (select 1
            from  sysobjects
            where id = object_id('thirdinfo')
            and   type = 'U')
   drop table thirdinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'index_8'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.index_8
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx7'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx7
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx6'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx6
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx5'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx5
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx4'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx4
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx3'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('trnjour')
            and   name  = 'trnjour_idx2'
            and   indid > 0
            and   indid < 255)
   drop index trnjour.trnjour_idx2
go

if exists (select 1
            from  sysobjects
            where id = object_id('trnjour')
            and   type = 'U')
   drop table trnjour
go

/*==============================================================*/
/* Table: acctjour                                              */
/*==============================================================*/
create table acctjour (
   nodeid               int                            not null,
   workdate             char(8)                        not null,
   originator           varchar(12)                    not null,
   refid                char(16)                       not null,
   inoutflag            char(1)                        not null,
   trncode              char(6)                        not null,
   acctserial           char(32)                       not null,
   revserial            char(32)                       null,
   acctorg              varchar(12)                    not null,
   acctoper             varchar(12)                    not null,
   revoper              varchar(12)                    null,
   result               char(1)                        default '0' not null,
   chkstate             char(1)                        default '0' not null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(16)                    null,
   reserved3            varchar(32)                    null,
   memo                 varchar(64)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on acctjour (
workdate asc,
acctserial asc,
acctorg asc
)
go

/*==============================================================*/
/* Index: index_2                                               */
/*==============================================================*/
create index index_2 on acctjour (
workdate asc,
acctserial asc
)
go

/*==============================================================*/
/* Index: index_3                                               */
/*==============================================================*/
create index index_3 on acctjour (
nodeid asc,
workdate asc,
originator asc,
refid asc,
inoutflag asc
)
go

/*==============================================================*/
/* Index: index_4                                               */
/*==============================================================*/
create unique index index_4 on acctjour (
nodeid asc,
workdate asc,
originator asc,
refid asc,
inoutflag asc,
trncode asc
)
go

/****************************************************************/
/* Table: baginfo                                                      */
/****************************************************************/
create table baginfo
(
    regionid    int      not null ,
    exchgdate   char(8)  not null ,
    exchground  integer  not null ,
    workdate    char(8)  not null ,
    workround   integer  not null ,
    type        char(1)  not null ,
    presbank    char(12) not null ,
    prescbank   char(12) not null ,
    presregion  char(6)  not null ,
    acptbank    char(12) not null ,
    acptcbank   char(12) not null ,
    acptregion  char(6)  not null ,
    num         integer  not null ,
    amount      decimal(16,2) not null ,
    debitnum    integer  not null ,
    debitamount decimal(16,2) not null ,
    creditnum   integer  not null ,
    creditamount decimal(16,2) not null ,
    directnum   integer  not null ,
    directamount decimal(16,2) not null ,
    menu        char(60),
    excharea     char(6) not null,
    state        char(1) not null
)
go

create index baginfo_idx1 on baginfo( exchgdate, exchground, type, presbank, acptbank )
go
create unique index baginfo_idx2 on baginfo( workdate , workround , type, presbank, acptbank)
go
/*==============================================================*/
/* Table: agreement                                             */
/*==============================================================*/
create table agreement
(
 nodeid      int         not null ,
 beneid      char(5)     not null ,
 userid      char(30)    not null ,
 username    char(80)    not null ,
 svcid       char(5)     not null ,
 cityid      char(4)     ,
 inputtime   char(14)    not null ,
 payingacct  char(32)        not null ,
 payer       char(80)    not null ,
 payertype   char(1)     not null ,
 bankid      char(12)        not null ,
 psbankid    char(12)    not null ,
 addr        char(80)    ,
 postalcode  char(6),
 linkman     char(20),
 phone1      char(20),
 phone2      char(20),
 agreementid   char(44)        not null ,
 amtlimit    decimal(16,2)   ,
 operdate    char(8)         not null ,
 opertime    char(6)         not null ,
 enddate     char(8) ,
 state       char(1) ,
 innerbankno char(12) ,
 inneragreement char(44)
)
go
print "create unique index agreement_idx1 on agreement (agreementid)..."
go
create unique index agreement_idx1 on agreement (agreementid)
go
create unique index agreement_idx2 on agreement(agreementid, payingacct )
go
/*==============================================================*/
/* Table: bankinfo                                              */
/*==============================================================*/
create table bankinfo (
   nodeid               int                            not null,
   bankid               char(12)                       not null,
   bankname             varchar(60)                    not null,
   bankabbr             char(30)                       not null,
   banklevel            char(1)                        null,
   parent               char(12)                       null,
   regionid             char(6)                        not null,
   autooper             varchar(12)                    null,
   autoorg              varchar(12)                    null,
   exchno               char(12)                       not null,
   clearacct            varchar(32)                    null,
   debitacct            varchar(32)                    null,
   creditacct           varchar(32)                    null,
   returnacct           varchar(32)                    null,
   reserved             varchar(64)                    null,
   reserved1            varchar(64)                    null,
   reserved2            varchar(64)                    null,
   constraint pk_bankinfo primary key (bankid)
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on bankinfo (
regionid asc
)
go

/*==============================================================*/
/* Index: index_2                                               */
/*==============================================================*/
create index index_2 on bankinfo (
exchno asc
)
go

/*==============================================================*/
/* Table: bankjour                                              */
/*==============================================================*/
create table bankjour (
   nodeid               int                            not null,
   workdate             char(8)                        not null,
   workround            char(1)                        not null,
   inoutflag            char(1)                        not null,
   refid                char(16)                       not null,
   originator           char(12)                       not null,
   acceptor             char(12)                       not null,
   classid              integer                        null,
   dcflag               char(1)                        null,
   notetype             char(2)                        null,
   noteno               char(12)                       null,
   curcode              char(3)                        null,
   curtype              char(1)                        null,
   settlamt             decimal(16,2)                  null,
   payingacct           char(32)                       null,
   beneacct             char(32)                       null,
   payer                char(60)                       null,
   benename             char(60)                       null,
   flag                 char(1)                        null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on bankjour (
nodeid asc,
workdate asc,
inoutflag asc,
refid asc,
originator asc
)
go

/*==============================================================*/
/* Table: ebanksumm                                             */
/*==============================================================*/
create table ebanksumm (
   nodeid               int                            not null,
   branchid             char(12)                       not null,
   bankid               char(12)                       not null,
   workdate             char(8)                        not null,
   workround            int                            not null,
   cleardate            char(8)                        not null,
   clearround           int                            not null,
   svcclass             int                            not null,
   curcode              char(3)                        not null,
   curtype              char(1)                        not null,
   pres_debit_num       int                            null,
   pres_debit_total     decimal(16,2)                  null,
   pres_credit_num      int                            null,
   pres_credit_total    decimal(16,2)                  null,
   acpt_debit_num       int                            null,
   acpt_debit_total     decimal(16,2)                  null,
   acpt_credit_num      int                            null,
   acpt_credit_total    decimal(16,2)                  null,
   balance              decimal(16,2)                  null
)
go

/*==============================================================*/
/* Index: ebanksumm_idx1                                        */
/*==============================================================*/
create unique index ebanksumm_idx1 on ebanksumm (
nodeid asc,
branchid asc,
workdate asc,
clearround asc,
curcode asc,
curtype asc,
workround asc,
svcclass asc
)
go

/*==============================================================*/
/* Index: ebanksumm_idx2                                        */
/*==============================================================*/
create index ebanksumm_idx2 on ebanksumm (
nodeid asc,
bankid asc,
workdate asc,
workround asc
)
go

/*==============================================================*/
/* Table: errinfo                                               */
/*==============================================================*/
create table errinfo (
   nodeid               int                            not null,
   errcode              varchar(12)                    not null,
   errinfo              varchar(128)                   null,
   clearstate           char(1)                        null,
   constraint pk_errinfo primary key (nodeid, errcode)
)
go

/*==============================================================*/
/* Table: errmap                                                */
/*==============================================================*/
create table errmap (
   nodeid               int                            not null,
   errcode              varchar(12)                    not null,
   mapnode              int                            not null,
   mapcode              varchar(8)                     not null,
   constraint pk_errmap primary key (nodeid, errcode, mapnode)
)
go

/*==============================================================*/
/* Table: feelist                                               */
/*==============================================================*/
create table feelist (
   nodeid               int                            not null,
   workdate             char(8)                        not null,
   inoutflag            char(1)                        not null,
   originator           char(12)                       not null,
   refid                char(16)                       not null,
   feedate              char(8)                        null,
   acctno               varchar(32)                    null,
   feepayer             char(1)                        null,
   amount               decimal(16,2)                  not null,
   result               char(1)                        default '0' not null
)
go

/*==============================================================*/
/* Table: feeset                                                */
/*==============================================================*/
create table feeset (
   nodeid               int                            not null,
   acctno               varchar(32)                    not null,
   typeid               int                            not null,
   acctname             varchar(80)                    null
)
go

/*==============================================================*/
/* Table: feesum                                                */
/*==============================================================*/
create table feesum (
   nodeid               int                            not null,
   branchid             char(12)                       not null,
   bankid               char(12)                       not null,
   classid              int                            null,
   notetype             char(2)                        not null,
   curcode              char(3)                        null,
   curtype              char(1)                        null,
   workdate             char(8)                        null,
   workround            int                            null,
   cleardate            char(8)                        null,
   clearround           int                            null,
   num                  int                            null,
   fee                  decimal(16,2)                  null,
   dueflag              char(1)                        null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on feesum (
nodeid asc,
branchid asc,
notetype asc,
curcode asc,
curtype asc,
workdate asc,
workround asc
)
go

/*==============================================================*/
/* Table: feetype                                               */
/*==============================================================*/
create table feetype (
   nodeid               int                            not null,
   typeid               int                            not null,
   name                 varchar(20)                    not null,
   flag                 char(1)                        not null,
   batchdate            char(8)                        null,
   value                varchar(20)                    not null,
   typedesc             varchar(64)                    not null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on feetype (
nodeid asc,
typeid asc
)
go

/*==============================================================*/
/* Table: freemsg                                               */
/*==============================================================*/
create table freemsg (
   nodeid               int                            not null,
   workdate             char(8)                        not null,
   refid                varchar(20)                    not null,
   originator           char(12)                       not null,
   acceptor             char(12)                       not null,
   rebankno             char(12)                       null,
   inoutflag            char(1)                        not null,
   classid              int                            null,
   title                varchar(80)                    not null,
   content              varchar(1024)                  not null,
   replycontent         varchar(1024)                  null,
   sender               varchar(12)                    not null,
   replyer              varchar(12)                    null,
   replyflag            char(1)                        default '0' null,
   readflag             char(1)                        default '0' null,
   sendtime             char(6)                        null,
   replytime            char(6)                        null,
   replydate            char(8)                        null,
   result               int                            null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on freemsg (
nodeid asc,
workdate asc,
refid asc,
originator asc
)
go

/*==============================================================*/
/* Table: htrnjour                                              */
/*==============================================================*/
create table htrnjour (
        nodeid               int                            not null,
        workdate             char(8)                        not null,
        refid                char(16)                       not null,
        inoutflag            char(1)                        not null,
        regioncode           char(6)                        null,
        trncode              char(6)                        null,
        classid              integer                        null,
        presdate             char(8)                        not null,
        prestime             char(6)                        not null,
        originator           char(12)                       not null,
        acceptor             char(12)                       not null,
        innerorganid         char(12)                       null,
        dcflag               char(1)                        not null,
        notetype             char(2)                        not null,
        bknotetype           varchar(8)                     null,
        noteno               char(32)                       not null,
        curcode              char(3)                        null,
        curtype              char(1)                        null,
        issuedate            char(8)                        not null,
        settlamt             decimal(16,2)                  not null,
        issueamt             decimal(16,2)                  null,
        remnamt              decimal(16,2)                  null,
        payingacct           char(32)                       null,
        payer                char(80)                       null,
        payingbank           char(12)                       null,
        pcbank               char(12)                       null,
        beneacct             char(32)                       null,
        benename             char(80)                       null,
        benebank             char(12)                       null,
        bcbank               char(12)                       null,
        agreement            char(60)                       null,
        purpose              char(60)                       null,
        memo                 varchar(254)                   null,
        acctoper             char(16)                       null,
        auditor              char(16)                       null,
        paykey               char(32)                       null,
        exchgdate            char(8)                        null,
        exchground           char(1)                        null,
        workround            char(1)                        null,
        cleardate            char(8)                        null,
        clearround           char(1)                        null,
        cleartime            char(6)                        null,
        clearstate           char(1)                        not null,
        result               int                            default 0 not null,
        truncflag            char(1)                        null,
        printnum             integer                        default 0 null,
        fee                  decimal(16,2)                  null,
        feepayer             char(1)                        null,
        feeflag              char(1)                        null,
        bankfee              decimal(16,2)                  null,
        stflag               char(1)                        null,
        chkflag              char(1)                        null,
        tpflag               char(1)                        null,
        resflag1             char(1)                        null,
        resflag2             char(1)                        null,
        extraxml             text                           null,
        reserved             varchar(254)                   null,
        reserved2            varchar(30)                    null,
        reserved3            varchar(60)                    null,
        constraint pk_htrnjour primary key (nodeid, workdate, refid, inoutflag, originator)
)
lock datarows
go

/*==============================================================*/
/* Index: trnjour_idx2                                          */
/*==============================================================*/
create index trnjour_idx2 on htrnjour (
workdate asc
)
go

/*==============================================================*/
/* Index: trnjour_idx3                                          */
/*==============================================================*/
create index trnjour_idx3 on htrnjour (
workdate asc,
originator asc,
notetype asc,
noteno asc
)
go

/*==============================================================*/
/* Index: trnjour_idx4                                          */
/*==============================================================*/
create index trnjour_idx4 on htrnjour (
originator asc
)
go

/*==============================================================*/
/* Index: trnjour_idx5                                          */
/*==============================================================*/
create index trnjour_idx5 on htrnjour (
acceptor asc
)
go

/*==============================================================*/
/* Index: trnjour_idx6                                          */
/*==============================================================*/
create index trnjour_idx6 on htrnjour (
notetype asc,
curcode asc,
curtype asc
)
go

/*==============================================================*/
/* Index: trnjour_idx7                                          */
/*==============================================================*/
create index trnjour_idx7 on htrnjour (
classid asc,
curcode asc,
curtype asc
)
go

/*==============================================================*/
/* Index: index_8                                               */
/*==============================================================*/
create unique index index_8 on htrnjour (
nodeid asc,
workdate asc,
refid asc,
inoutflag asc,
originator asc
)
go

/*==============================================================*/
/* Table: noteinfo                                              */
/*==============================================================*/
create table noteinfo (
   nodeid               int                            not null,
   notetype             char(2)                        null,
   name                 varchar(60)                    null,
   classid              int                            null,
   dcflag               char(1)                        null,
   enableflag           char(1)                        null,
   dupcheck             char(1)                        null,
   validperiod          int                            null,
   trnexpdays           int                            null,
   truncflag            char(1)                        null,
   transflag            char(1)                        null,
   authtype             char(1)                        null,
   feepayer             char(1)                        null,
   cleartype            char(1)                        null,
   acctchktype          char(1)                        null,
   notedesc             varchar(256)                   null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(32)                    null,
   reserved3            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on noteinfo (
nodeid asc,
notetype asc,
dcflag asc
)
go

/*==============================================================*/
/* Table: notetypemap                                           */
/*==============================================================*/
create table notetypemap (
   nodeid               int                            not null,
   tctype               char(4)                        not null,
   dcflag               char(1)                        null,
   truncflag            char(1)                        null,
   banktype             varchar(8)                     not null,
   bknotename           varchar(60)                    null,
   reserved1            varchar(20)                    null,
   reserved2            varchar(6)                     null,
   reserved3            varchar(20)                    null,
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on notetypemap (
nodeid asc,
tctype asc,
banktype asc,
dcflag asc
)
go


/*==============================================================*/
/* Table: operinfo                                              */
/*==============================================================*/
create table operinfo (
   nodeid               int                            not null,
   operid               char(12)                       not null,
   bankno               char(12)                       null,
   organid              char(16)                       not null,
   name                 char(20)                       not null,
   bklevel              char(1)                        null,
   class                char(1)                        not null,
   passwd               char(32)                       not null,
   status               char(1)                        not null,
   regdate              char(8)                        null,
   canceldate           char(8)                        null,
   lastlogintime        char(20)                       null,
   pwdchgdate           char(8)                        null,
   amtlimit             varchar(20)                    null,
   rights               text                           null,
   reserved1            char(4)                        null,
   reserved2            varchar(16)                    null,
   reserved3            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on operinfo (
nodeid asc,
operid asc,
bankno asc
)
go

/*==============================================================*/
/* Index: index_2                                               */
/*==============================================================*/
create unique index index_2 on operinfo (
nodeid asc,
operid asc,
organid asc
)
go

/*==============================================================*/
/* Table: operjour                                              */
/*==============================================================*/
create table operjour (
   nodeid               int                            not null,
   seqno                int                            not null,
   bankno               char(16)                       not null,
   operid               char(12)                       not null,
   date                 char(8)                        not null,
   time                 char(6)                        not null,
   trcode               char(6)                        not null,
   memo                 varchar(128)                   null,
   reserved1            char(4)                        null,
   reserved2            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on operjour (
nodeid asc,
bankno asc,
operid asc,
date asc
)
go

/*==============================================================*/
/* Table: organinfo                                             */
/*==============================================================*/
create table organinfo (
   nodeid               int                            not null,
   orgid                char(12)                       not null,
   orgname              varchar(80)                    not null,
   orgabbr              varchar(30)                    not null,
   orgtype              char(1)                        not null,
   orglevel             char(1)                        not null,
   parent               char(12)                       null,
   attechorg            char(12)                       null,
   presproxy            char(12)                       null,
   acptproxy            char(12)                       null,
   svclist              char(32)                       null,
   region               char(6)                        null,
   linkman              varchar(60)                    null,
   phone                varchar(20)                    null,
   state                char(1)                        null,
   exchflag             char(1)                        null,
   exchregion           char(50)                       null,
   enabledate           char(8)                        null,
   disabledate          char(8)                        null,
   exchroute            int                            null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on organinfo (
nodeid asc,
orgid asc
)
go

/*==============================================================*/
/* Table: queryinfo                                             */
/*==============================================================*/
create table queryinfo (
   nodeid               int                            not null,
   workdate             char(8)                        not null,
   refid                varchar(20)                    not null,
   originator           char(12)                       not null,
   outbank              char(12)                       null,
   acceptor             char(12)                       not null,
   inbank               char(12)                       null,
   rebankno             char(12)                       null,
   inoutflag            char(1)                        not null,
   classid              int                            null,
   otrandate            char(8)                        null,
   orefid               varchar(20)                    null,
   oinoutflag           char(1)                        null,
   ooriginator          char(12)                       null,
   dcflag               char(1)                        null,
   notetype             char(2)                        null,
   noteno               varchar(20)                    null,
   settlamt             decimal(16,2)                  null,
   issuedate            char(8)                        null,
   secret               varchar(20)                    null,
   paykey               varchar(32)                    null,
   testkey              varchar(32)                    null,
   payingacct           varchar(32)                    null,
   payer                varchar(80)                    null,
   beneacct             varchar(32)                    null,
   benename             varchar(80)                    null,
   content              varchar(512)                   not null,
   replycontent         varchar(512)                   null,
   replydate            char(8)                        null,
   state                char(1)                        default '0' null,
   transflag            char(1)                        null,
   readflag             char(1)                        default '0' null,
   prtnum               int                            default 0 null,
   inputoper            varchar(12)                    not null,
   checkoper            varchar(12)                    null,
   regressdate          char(8)                        null,
   regressrefid         varchar(20)                    null,
   result               int                            null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(32)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on queryinfo (
nodeid asc,
workdate asc,
originator asc,
refid asc,
inoutflag asc
)
go

/*==============================================================*/
/* Table: reconinfo                                             */
/*==============================================================*/
create table reconinfo (
   nodeid               int                            not null,
   bankid               char(12)                       not null,
   workdate             char(8)                        not null,
   workround            int                            not null,
   cleardate            char(8)                        not null,
   clearround           int                            not null,
   svcclass             int                            not null,
   curcode              char(3)                        not null,
   curtype              char(1)                        not null,
   pres_debit_num       int                            null,
   pres_debit_total     decimal(16,2)                  null,
   pres_credit_num      int                            null,
   pres_credit_total    decimal(16,2)                  null,
   acpt_debit_num       int                            null,
   acpt_debit_total     decimal(16,2)                  null,
   acpt_credit_num      int                            null,
   acpt_credit_total    decimal(16,2)                  null,
   balance              decimal(16,2)                  null,
   flag                 char(1)                        null
)
go

/*==============================================================*/
/* Index: reconinfo_idx1                                        */
/*==============================================================*/
create unique index reconinfo_idx1 on reconinfo (
nodeid asc,
bankid asc,
workdate asc,
workround asc,
svcclass asc,
curcode asc,
curtype asc
)
go

/*==============================================================*/
/* Index: reconinfo_idx2                                        */
/*==============================================================*/
create index reconinfo_idx2 on reconinfo (
nodeid asc,
workdate asc,
workround asc
)
go

/*==============================================================*/
/* Index: reconinfo_idx3                                        */
/*==============================================================*/
create index reconinfo_idx3 on reconinfo (
nodeid asc,
bankid asc
)
go

/*==============================================================*/
/* Table: regioninfo                                            */
/*==============================================================*/
create table regioninfo (
   regionid             char(6)                        not null,
   regionname           char(60)                       not null,
   regionlevel          char(1)                        null,
   superior             char(6)                        null,
   commnode             char(8)                        not null,
   enableflag           char(1)                        default '0' null,
   constraint pk_regioninfo primary key (commnode)
)
go

/*==============================================================*/
/* Table: syspara                                               */
/*==============================================================*/
create table syspara (
   nodeid               int                            not null,
   paraname             char(32)                       not null,
   paraval              char(64)                       not null,
   paradesc             char(64)                       null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on syspara (
nodeid asc,
paraname asc
)
go

/*==============================================================*/
/* Table: thirdinfo                                             */
/*==============================================================*/
create table thirdinfo (
   nodeid               int                            null,
   thirdcode            char(2)                        null
)
go

/*==============================================================*/
/* Table: trnjour                                               */
/*==============================================================*/
create table trnjour (
            nodeid               int                            not null,
            workdate             char(8)                        not null,
            refid                char(16)                       not null,
            inoutflag            char(1)                        not null,
            regioncode           char(6)                        null,
            trncode              char(6)                        null,
            classid              integer                        null,
            presdate             char(8)                        not null,
            prestime             char(6)                        not null,
            originator           char(12)                       not null,
            acceptor             char(12)                       not null,
            innerorganid         char(12)                       null,
            dcflag               char(1)                        not null,
            notetype             char(2)                        not null,
            bknotetype           varchar(8)                     null,
            noteno               char(32)                       not null,
            curcode              char(3)                        null,
            curtype              char(1)                        null,
            issuedate            char(8)                        not null,
            settlamt             decimal(16,2)                  not null,
            issueamt             decimal(16,2)                  null,
            remnamt              decimal(16,2)                  null,
            payingacct           char(32)                       null,
            payer                char(80)                       null,
            payingbank           char(12)                       null,
            pcbank               char(12)                       null,
            beneacct             char(32)                       null,
            benename             char(80)                       null,
            benebank             char(12)                       null,
            bcbank               char(12)                       null,
            agreement            char(60)                       null,
            purpose              char(60)                       null,
            memo                 varchar(254)                   null,
            acctoper             char(16)                       null,
            auditor              char(16)                       null,
            paykey               char(32)                       null,
            exchgdate            char(8)                        null,
            exchground           char(1)                        null,
            workround            char(1)                        null,
            cleardate            char(8)                        null,
            clearround           char(1)                        null,
            cleartime            char(6)                        null,
            clearstate           char(1)                        not null,
            result               int                            default 0 not null,
            truncflag            char(1)                        null,
            printnum             integer                        default 0 null,
            fee                  decimal(16,2)                  null,
            feepayer             char(1)                        null,
            feeflag              char(1)                        default '0' null,
            bankfee              decimal(16,2)                  null,
            stflag               char(1)                        default '0' null,
            chkflag              char(1)                        default '0' null,
            tpflag               char(1)                        default '0' null,
            resflag1             char(1)                        default '0' null,
            resflag2             char(1)                        default '0' null,
            extraxml             text                           null,
            reserved             varchar(254)                   null,
            reserved2            varchar(30)                    null,
            reserved3            varchar(60)                    null,
            constraint pk_trnjour primary key (nodeid, workdate, refid, inoutflag, originator)
)
lock datarows
go

/*==============================================================*/
/* Index: trnjour_idx2                                          */
/*==============================================================*/
create index trnjour_idx2 on trnjour (
workdate asc
)
go

/*==============================================================*/
/* Index: trnjour_idx3                                          */
/*==============================================================*/
create index trnjour_idx3 on trnjour (
workdate asc,
originator asc,
notetype asc,
noteno asc
)
go

/*==============================================================*/
/* Index: trnjour_idx4                                          */
/*==============================================================*/
create index trnjour_idx4 on trnjour (
originator asc
)
go

/*==============================================================*/
/* Index: trnjour_idx5                                          */
/*==============================================================*/
create index trnjour_idx5 on trnjour (
acceptor asc
)
go

/*==============================================================*/
/* Index: trnjour_idx6                                          */
/*==============================================================*/
create index trnjour_idx6 on trnjour (
notetype asc,
curcode asc,
curtype asc
)
go

/*==============================================================*/
/* Index: trnjour_idx7                                          */
/*==============================================================*/
create index trnjour_idx7 on trnjour (
classid asc,
curcode asc,
curtype asc
)
go

/*==============================================================*/
/* Index: index_8                                               */
/*==============================================================*/
create unique index index_8 on trnjour (
nodeid asc,
workdate asc,
refid asc,
inoutflag asc,
originator asc
)
go

/*==============================================================*/
/* Table: colsdesc*/
/*==============================================================*/
create table colsdesc(
   nodeid               int                            not null,
   tablename            varchar(20)                    not null,
   colname              varchar(20)                    not null,
   coldesc              varchar(64)                     not null
)
go
/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on colsdesc(
nodeid asc,
tablename asc,
colname asc
)
go

/*==============================================================*/
/* Table: funclist*/
/*==============================================================*/
create table funclist(
   nodeid               int                            not null,
   funclevel            char(1)                        not null,
   funcno               varchar(6)                     not null,
   funcname             varchar(40)                    null,
   funcdesc             varchar(20)                    null,
   reserved1            varchar(2)                    null,
   reserved2            varchar(32)                    null,
   constraint pk_funclist primary key (nodeid,funcno)
)
go
/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on funclist(
nodeid asc,
funcno asc
)
go
/*==============================================================*/
/* Table: nodeinfo*/
/*==============================================================*/
create table nodeinfo (
   nodeid               int                            not null,
   nodename             varchar(40)                    null,
   nodedesc             varchar(20)                    not null,
   nodetype             varchar(1)                     not null,
   enableflag           varchar(1)                     not null,
   reserved1            varchar(2)                     null,
   reserved2            varchar(32)                    null,
   constraint pk_nodeinfo primary key (nodeid)
)
go
/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on nodeinfo (
nodeid asc
)
go

/*==============================================================*/
/* Table: codetype*/
/*==============================================================*/
create table codetype (
	nodeid                          int                              not null  ,
	typeid                          char(4)                          not null  ,
	typename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_codetype'
-----------------------------------------------------------------------------
print 'Creating Index idx_codetype'
go

create unique nonclustered index idx_codetype 
on codetype ( nodeid, typeid)
go 

/*==============================================================*/
/* Table: generalcode*/
/*==============================================================*/
create table generalcode (
	nodeid                          int                              not null  ,
	codetype                        char(4)                          not null  ,
	codeid                          char(60)                         not null  ,
	codename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 

-----------------------------------------------------------------------------
-- DDL for Index 'generalcode_idx1'
-----------------------------------------------------------------------------
print 'Creating Index generalcode_idx1'
go

create unique nonclustered index generalcode_idx1 
on generalcode ( nodeid, codetype, codeid)
go 
