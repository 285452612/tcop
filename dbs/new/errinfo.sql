if exists (select 1
            from  sysobjects
            where id = object_id('errinfo')
            and   type = 'U')
   drop table errinfo
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
