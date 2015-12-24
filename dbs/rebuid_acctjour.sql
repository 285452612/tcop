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
