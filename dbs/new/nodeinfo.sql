
if exists (select 1
            from  sysobjects
            where id = object_id('nodeinfo')
            and   type = 'U')
   drop table nodeinfo
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
   reserved2            varchar(32)                    null
)
go
/*==============================================================*/
/* Index: index_1                                               */
/*==============================================================*/
create index index_1 on nodeinfo (
nodeid asc
)
go
