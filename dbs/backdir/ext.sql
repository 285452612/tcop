
drop table organinfo
go
create table organinfo
(
 nodeid int not null,
 orgid char(12) not null ,
 orgname char(80) not null ,
 orgabbr char(20) not null ,
 orgtype char(1) not null ,
 orglevel integer not null ,
 parent char(12),
 attechorg char(12),
 presproxy char(12),
 acptproxy char(12),
 svclist char(32),
 region char(6),
 linkman char(80),
 phone char(20),
 state char(1),
 exchflag char(1) not null,
 exchregion char(50) not null,
 enabledate char(8),
 disabledate char(8),
 exchroute integer
 )
go

create unique index organinfo_idx1 on organinfo (nodeid,orgid,orgtype, orglevel)
go

create index organinfo_idx2 on organinfo (parent)
go

create index organinfo_idx3 on organinfo (attechorg)
go

-- drop table operinfo
-- go
create table operinfo
(
    -- nodeid int not null,
    orgid char(12) not null ,
    operid char(6) not null ,
    name char(80) not null ,
    opertype char(1) not null ,
    deptid char(2),
    pwd char(32),
    state char(1),
    operrole text,
    amtlimit decimal(16,2),
    regdate char(8),
    llogindate char(8),
    llogintime char(6),
    pwdchgdate char(8),
    canceldate char(8)
)
go

create unique index operinfo_idx1 on operinfo (orgid,operid)
go

drop table operjour
go
create table operjour
(
 nodeid int not null,
    seqno integer,
    organid char(12) not null ,
    operid char(6) not null ,
    workdate char(8) not null ,
    time char(6) not null ,
    trncode char(4) not null ,
    memo varchar(128)
)
go

create index operjour_idx1 on operjour (nodeid, workdate, organid, operid)
go

create index operjour_idx2 on operjour (organid, operid, workdate)
go


drop table updates
go
create table updates (
      type          int          not null  ,
      name          char(128)    not null  ,
      version       int          not null
      )
go

create unique nonclustered index updates_idx1 on updates(name)
go

drop table noteinfo
go
create table noteinfo (
 nodeid int not null,
	notetype                        char(2)                          not null  ,
	name                            char(32)                         not null  ,
	svcclass                        int                                  null  ,
	dcflag                          char(1)                          not null  ,
	enableflag                      char(1)                              null  ,
	dupcheck                        char(1)                              null  ,
	validperiod                     int                                  null  ,
	trnexpdays                      int                                  null  ,
	truncflag                       char(1)                              null  ,
	transflag                       char(1)                              null  ,
	authtype                        char(1)                              null  ,
	feepayer                        char(1)                              null  ,
	ruleid                          int                                  null  ,
    cleartype char(1),
    acctchktype char(1),
	notedesc                        varchar(254)                         null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'noteinfo_idx1'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "noteinfo_idx1" >>>>>'
go 

create unique nonclustered index noteinfo_idx1 
on noteinfo(nodeid,notetype, dcflag)
go 


-----------------------------------------------------------------------------
-- DDL for Table 'codetype'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "codetype" >>>>>'
go

drop table codetype
go
create table codetype (
 nodeid int not null,
	typeid                          char(4)                          not null  ,
	typename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 

-----------------------------------------------------------------------------
-- DDL for Index 'idx_codetype'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "idx_codetype" >>>>>'
go 

create unique nonclustered index idx_codetype 
on codetype(nodeid,typeid)
go 


-----------------------------------------------------------------------------
-- DDL for Table 'curcode'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "curcode" >>>>>'
go

drop table curcode
go
create table curcode (
 nodeid int not null,
	curcode                         char(3)                          not null  ,
	cursym                          char(3)                          not null  ,
	name                            char(32)                             null  ,
	units                           int                                  null  ,
	enableflag                      char(1)                              null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_curcode1'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "idx_curcode1" >>>>>'
go 

create unique nonclustered index idx_curcode1 
on curcode(nodeid,curcode)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_curcode2'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "idx_curcode2" >>>>>'
go 

create unique nonclustered index idx_curcode2 
on curcode(cursym)
go 

drop table exchgarea
go
create table exchgarea
(
 nodeid int not null,
 areacode    char(4)     not null,
 areaname    char(20)    not null
 )
go

create unique index exchgarea_idx1 on exchgarea(nodeid,areacode)
go

drop table generalcode
go
create table generalcode (
 nodeid int not null,
    codetype                        char(4)                          not null  ,
    codeid                          char(60)                         not null  ,
    codename                        char(60)                         not null
)
go

create unique nonclustered index generalcode_idx1
on generalcode (nodeid, codetype, codeid)
go

/*==============================================================*/
/* Table: bankinfo                                             */
/*==============================================================*/
create table bankinfo (
   bankid              char(12)                       not null,
   bankname            char(60)                       not null,
   bankabbr            char(30)                       null,
   banklevel           char(1)                        null,
   parent               char(12)                       null,
   regionid             char(6)                        not null,
   autooper             char(12)                       null,
   exchno               char(12)                       not null,
   reserved             char(32)                       null,
   constraint pk_bankinfo primary key (bankid)
)
go

