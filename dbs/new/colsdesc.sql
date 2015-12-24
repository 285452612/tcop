if exists (select 1
            from  sysobjects
            where id = object_id('colsdesc')
            and   type = 'U')
   drop table colsdesc 
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
