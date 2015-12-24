
create table noteinfo (
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
on noteinfo(notetype, dcflag)
go 


-----------------------------------------------------------------------------
-- DDL for Table 'codetype'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "codetype" >>>>>'
go

create table codetype (
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
on codetype(typeid)
go 


-----------------------------------------------------------------------------
-- DDL for Table 'curcode'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "curcode" >>>>>'
go

create table curcode (
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
on curcode(curcode)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_curcode2'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "idx_curcode2" >>>>>'
go 

create unique nonclustered index idx_curcode2 
on curcode(cursym)
go 

create table exchgarea
(
 areacode    char(4)     not null,
 areaname    char(20)    not null
 )
go

create unique index exchgarea_idx1 on exchgarea(areacode)
go

create table generalcode (
    codetype                        char(4)                          not null  ,
    codeid                          char(60)                         not null  ,
    codename                        char(60)                         not null
)
go

create unique nonclustered index generalcode_idx1
on generalcode ( codetype, codeid)
go

-----------------------------------------------------------------------------
-- DDL for Table 'svcclass'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "svcclass" >>>>>'
go

create table svcclass (
	classid                         int                              not null  ,
	name                            char(32)                             null  ,
	state                           char(1)                              null  ,
	workdate                        char(8)                              null  ,
	cleardate                       char(8)                              null  ,
	curr_round                      int                                  null  ,
	tm_start                        char(6)                              null  ,
	tm_end                          char(6)                              null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'svcclass_idx1'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "svcclass_idx1" >>>>>'
go 

create unique nonclustered index svcclass_idx1 
on svcclass(classid)
go 

-----------------------------------------------------------------------------
-- DDL for Table 'errmsg'
-----------------------------------------------------------------------------
print '<<<<< CREATING Table - "errmsg" >>>>>'
go

create table errmsg (
	errcode                         int                              not null  ,
	errmacro                        char(32)                         not null  ,
	errdesc                         varchar(64)                          null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'errmsg_idx1'
-----------------------------------------------------------------------------

print '<<<<< CREATING Index - "errmsg_idx1" >>>>>'
go 

create unique nonclustered index errmsg_idx1 
on errmsg(errcode)
go 


