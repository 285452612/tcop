if exists (select 1
            from  sysobjects
            where id = object_id('funclist')
            and   type = 'U')
   drop table funclist 
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
