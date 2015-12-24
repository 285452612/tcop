create table codetype (
	nodeid                          int                              not null  ,
	typeid                          char(4)                          not null  ,
	typename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_codetype'
-----------------------------------------------------------------------------
print 'Creating Index idx_codetype'
go

create unique nonclustered index idx_codetype 
on opdb.tcop.codetype ( nodeid, typeid)
go 

