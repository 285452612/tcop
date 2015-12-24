/*==============================================================*/
/* DBMS name:      Sybase AS Enterprise 12.5                    */
/* Created on:     2009-12-10 17:04:55                          */
/*==============================================================*/


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
            where id = object_id('errinfo')
            and   type = 'U')
   drop table errinfo
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
           where  id    = object_id('feetype')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index feetype.index_2
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
            from  sysobjects
            where id = object_id('operinfo')
            and   type = 'U')
   drop table operinfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('organinfo')
            and   name  = 'index_2'
            and   indid > 0
            and   indid < 255)
   drop index organinfo.index_2
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
           where  id    = object_id('recvbox')
            and   name  = 'recvbox_idx3'
            and   indid > 0
            and   indid < 255)
   drop index recvbox.recvbox_idx3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('recvbox')
            and   name  = 'recvbox_idx2'
            and   indid > 0
            and   indid < 255)
   drop index recvbox.recvbox_idx2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('recvbox')
            and   name  = 'recvbox_idx1'
            and   indid > 0
            and   indid < 255)
   drop index recvbox.recvbox_idx1
go

if exists (select 1
            from  sysobjects
            where id = object_id('recvbox')
            and   type = 'U')
   drop table recvbox
go

if exists (select 1
            from  sysobjects
            where id = object_id('regioninfo')
            and   type = 'U')
   drop table regioninfo
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('sendbox')
            and   name  = 'sendbox_idx3'
            and   indid > 0
            and   indid < 255)
   drop index sendbox.sendbox_idx3
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('sendbox')
            and   name  = 'sendbox_idx2'
            and   indid > 0
            and   indid < 255)
   drop index sendbox.sendbox_idx2
go

if exists (select 1
            from  sysindexes
           where  id    = object_id('sendbox')
            and   name  = 'sendbox_idx1'
            and   indid > 0
            and   indid < 255)
   drop index sendbox.sendbox_idx1
go

if exists (select 1
            from  sysobjects
            where id = object_id('sendbox')
            and   type = 'U')
   drop table sendbox
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
   originator           char(12)                        not null,
   refid                char(16)                       not null,
   inoutflag            char(1)                       not null,
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
create unique index index_1 on acctjour (
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
/* Table: errinfo                                               */
/*==============================================================*/
create table errinfo (
   opcode               varchar(8)                     not null,
   nodeid               int                            null,
   tccode               varchar(8)                     null,
   errinfo              varchar(128)                   not null,
   constraint pk_errinfo primary key (opcode, nodeid)
)
go

/*==============================================================*/
/* Table: feelist                                               */
/*==============================================================*/
create table feelist (
   nodeid               int                            not null,
   feedate              char(8)                        not null,
   seqno                varchar(40)                    null,
   acctno               varchar(32)                    not null,
   amount               varchar(20)                    not null,
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
/* Table: feetype                                               */
/*==============================================================*/
create table feetype (
   nodeid               int                            not null,
   typeid               int                            not null,
   name                 varchar(20)                    not null,
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
/* Index: index_2                                               */
/*==============================================================*/
create index index_2 on feetype (
nodeid asc,
name asc
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
   reserved3            varchar(20)                    null
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create unique index index_1 on notetypemap (
nodeid asc,
tctype asc,
dcflag asc
)
go

/*==============================================================*/
/* Index: index_2                                               */
/*==============================================================*/
create index index_2 on notetypemap (
nodeid asc,
tctype asc
)
go

/*==============================================================*/
/* Table: operinfo                                              */
/*==============================================================*/
create table operinfo (
   operid               char(12)                       not null,
   organid              char(12)                       not null,
   opername             char(20)                       null,
   operclass            char(1)                        null,
   passwd               char(32)                       null,
   status               char(1)                        null,
   regdate              char(8)                        null,
   canceldate           char(8)                        null,
   lastlogintime        char(20)                       null,
   lastpwdmodi          char(8)                        null,
   tranquota            decimal(12,2)                  null
)
go

/*==============================================================*/
/* Table: organinfo                                             */
/*==============================================================*/
create table organinfo (
   organid              char(12)                       not null,
   organname            char(60)                       not null,
   organabbr            char(20)                       null,
   organlevel           char(1)                        null,
   parent               char(12)                       null,
   regionid             char(6)                        not null,
   autooper             char(12)                       null,
   exchno               char(12)                       not null,
   reserved             char(32)                       null,
   constraint pk_organinfo primary key (organid)
)
go

/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on organinfo (
regionid asc
)
go

/*==============================================================*/
/* Index: index_2                                               */
/*==============================================================*/
create index index_2 on organinfo (
exchno asc
)
go

/*==============================================================*/
/* Table: recvbox                                               */
/*==============================================================*/
create table recvbox (
   mailid               integer                        not null,
   sender               char(12)                       not null,
   recver               char(12)                       not null,
   writer               char(6)                        null,
   auditor              char(6)                        null,
   mailtype             char(1)                        not null,
   readed               char(1)                        null,
   recvdate             char(8)                        null,
   recvtime             char(6)                        null,
   title                char(80)                       null,
   content              char(2048)                     null,
   replyflag            char(1)                        null,
   replydate            char(8)                        null,
   replytime            char(6)                        null
)
go

/*==============================================================*/
/* Index: recvbox_idx1                                          */
/*==============================================================*/
create unique index recvbox_idx1 on recvbox (
mailid asc
)
go

/*==============================================================*/
/* Index: recvbox_idx2                                          */
/*==============================================================*/
create index recvbox_idx2 on recvbox (
sender asc
)
go

/*==============================================================*/
/* Index: recvbox_idx3                                          */
/*==============================================================*/
create index recvbox_idx3 on recvbox (
recver asc
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
/* Table: sendbox                                               */
/*==============================================================*/
create table sendbox (
   mailid               integer                        not null,
   sender               char(12)                       not null,
   recver               varchar(254)                   not null,
   writer               char(6)                        null,
   auditor              char(6)                        null,
   mailtype             char(1)                        not null,
   sended               char(1)                        null,
   senddate             char(8)                        null,
   sendtime             char(6)                        null,
   title                char(80)                       null,
   content              char(2048)                     null,
   originmailid         integer                        null
)
go

/*==============================================================*/
/* Index: sendbox_idx1                                          */
/*==============================================================*/
create unique index sendbox_idx1 on sendbox (
mailid asc
)
go

/*==============================================================*/
/* Index: sendbox_idx2                                          */
/*==============================================================*/
create index sendbox_idx2 on sendbox (
sender asc
)
go

/*==============================================================*/
/* Index: sendbox_idx3                                          */
/*==============================================================*/
create index sendbox_idx3 on sendbox (
recver asc
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
   agreement            char(18)                       null,
   purpose              char(60)                       null,
   memo                 varchar(254)                   null,
   acctoper             char(16)                       null,
   auditor              char(16)                       null,
   paykey               char(32)                       null,
   workround            char(1)                        null,
   cleardate            char(8)                        null,
   clearround           char(1)                        null,
   cleartime            char(6)                        null,
   clearstate           char(1)                        not null,
   result               int                            default 0 not null,
   truncflag            char(1)                        null,
   printnum             integer                        default 0 null,
   extraxml             varchar(1024)                  null,
   reserved             varchar(254)                   null,
   reserved2            varchar(30)                    null,
   reserved3            varchar(60)                    null
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

