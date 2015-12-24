use cddb
go

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
