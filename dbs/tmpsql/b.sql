
drop table trnjour
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
   agreement            char(30)                       null,
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
   extraxml             text,
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

