drop table bankinfo
go

create table bankinfo (
   bankid               char(12)                       not null,
   bankname             varchar(60)                    not null,
   bankabbr             char(30)                       null,
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
   reserved             varchar(32)                    null,
   reserved1            varchar(16)                    null,
   reserved2            varchar(32)                    null,
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
