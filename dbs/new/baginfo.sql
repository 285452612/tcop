drop table baginfo 
go
create table baginfo
(
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

