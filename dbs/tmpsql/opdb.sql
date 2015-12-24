-- Sybase DDLGen Utility/12.5.4/GA/1/S/1.4/ase1254/Mon May 22 10:14:43 PDT 2006


-- Sybase, Inc. 机密财产
-- Copyright 2001, 2002
-- Sybase, Inc. 保留所有权利。
-- 依据美国版权法保留所有未公开的权利。
-- 本软件包含 Sybase, Inc. 的机密和商业秘密信息。
-- 美国政府使用、复制或公开本软件及文档受政府部门与
-- Sybase, Inc. 之间的许可协议的约束，
-- 或者受其它授予政府部门使用本软件权利的书面协议和所有适用的
-- FAR 条款（例如，FAR 52.227-19）的约束。
-- Sybase, Inc. One Sybase Drive, Dublin, CA 94568, USA


-- 使用下列参数启动 DDLGen
-- -Usa -P*** -SSYBASE -Dopdb -Oopdb.sql 
-- 由陈杰修改
-- 时间 03/23/10 10:16:23 GMT+08:00

-----------------------------------------------------------------------------
-- DDL for Disk init.
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.bankinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.bankinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table bankinfo (
	bankid                          char(12)                         not null  ,
	bankname                        char(60)                         not null  ,
	bankabbr                        char(30)                             null  ,
	banklevel                       char(1)                              null  ,
	parent                          char(12)                             null  ,
	regionid                        char(6)                          not null  ,
	autooper                        char(12)                             null  ,
	exchno                          char(12)                         not null  ,
	reserved                        char(32)                             null  ,
		CONSTRAINT pk_bankinfo PRIMARY KEY CLUSTERED ( bankid )  on 'default' 
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.svcclass'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.svcclass'
go 

use opdb
go 

setuser 'tcop'
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


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'svcclass_idx1'
-----------------------------------------------------------------------------
print 'Creating Index svcclass_idx1'
go

create unique nonclustered index svcclass_idx1 
on opdb.tcop.svcclass ( classid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.errmsg'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.errmsg'
go 

use opdb
go 

setuser 'tcop'
go 

create table errmsg (
	errcode                         int                              not null  ,
	errmacro                        char(32)                         not null  ,
	errdesc                         varchar(64)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'errmsg_idx1'
-----------------------------------------------------------------------------
print 'Creating Index errmsg_idx1'
go

create unique nonclustered index errmsg_idx1 
on opdb.tcop.errmsg ( errcode)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.feetype'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.feetype'
go 

use opdb
go 

setuser 'tcop'
go 

create table feetype (
	nodeid                          int                              not null  ,
	typeid                          int                              not null  ,
	name                            varchar(20)                      not null  ,
	flag                            char(1)                          not null  ,
	batchdate                       char(8)                              null  ,
	value                           varchar(20)                      not null  ,
	typedesc                        varchar(64)                      not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create nonclustered index index_1 
on opdb.tcop.feetype ( nodeid, typeid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.organinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.organinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table organinfo (
	nodeid                          int                              not null  ,
	orgid                           char(12)                         not null  ,
	orgname                         char(80)                         not null  ,
	orgabbr                         char(20)                         not null  ,
	orgtype                         char(1)                          not null  ,
	orglevel                        int                              not null  ,
	parent                          char(12)                         not null  ,
	attechorg                       char(12)                         not null  ,
	presproxy                       char(12)                         not null  ,
	acptproxy                       char(12)                         not null  ,
	svclist                         char(32)                         not null  ,
	region                          char(6)                          not null  ,
	linkman                         char(80)                         not null  ,
	phone                           char(20)                         not null  ,
	state                           char(1)                          not null  ,
	exchflag                        char(1)                          not null  ,
	exchregion                      char(50)                         not null  ,
	enabledate                      char(8)                          not null  ,
	disabledate                     char(8)                          not null  ,
	exchroute                       int                              not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'organinfo_idx1'
-----------------------------------------------------------------------------
print 'Creating Index organinfo_idx1'
go

create unique nonclustered index organinfo_idx1 
on opdb.tcop.organinfo ( nodeid, orgid, orgtype, orglevel)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'organinfo_idx2'
-----------------------------------------------------------------------------
print 'Creating Index organinfo_idx2'
go

create nonclustered index organinfo_idx2 
on opdb.tcop.organinfo ( parent)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'organinfo_idx3'
-----------------------------------------------------------------------------
print 'Creating Index organinfo_idx3'
go

create nonclustered index organinfo_idx3 
on opdb.tcop.organinfo ( attechorg)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.updates'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.updates'
go 

use opdb
go 

setuser 'tcop'
go 

create table updates (
	type                            int                              not null  ,
	name                            char(128)                        not null  ,
	version                         int                              not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'updates_idx1'
-----------------------------------------------------------------------------
print 'Creating Index updates_idx1'
go

create unique nonclustered index updates_idx1 
on opdb.tcop.updates ( name)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.noteinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.noteinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table noteinfo (
	nodeid                          int                              not null  ,
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
	cleartype                       char(1)                          not null  ,
	acctchktype                     char(1)                          not null  ,
	notedesc                        varchar(254)                         null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'noteinfo_idx1'
-----------------------------------------------------------------------------
print 'Creating Index noteinfo_idx1'
go

create unique nonclustered index noteinfo_idx1 
on opdb.tcop.noteinfo ( nodeid, notetype, dcflag)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.codetype'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.codetype'
go 

use opdb
go 

setuser 'tcop'
go 

create table codetype (
	nodeid                          int                              not null  ,
	typeid                          char(4)                          not null  ,
	typename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'idx_codetype'
-----------------------------------------------------------------------------
print 'Creating Index idx_codetype'
go

create unique nonclustered index idx_codetype 
on opdb.tcop.codetype ( nodeid, typeid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.curcode'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.curcode'
go 

use opdb
go 

setuser 'tcop'
go 

create table curcode (
	nodeid                          int                              not null  ,
	curcode                         char(3)                          not null  ,
	cursym                          char(3)                          not null  ,
	name                            char(32)                             null  ,
	units                           int                                  null  ,
	enableflag                      char(1)                              null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'idx_curcode1'
-----------------------------------------------------------------------------
print 'Creating Index idx_curcode1'
go

create unique nonclustered index idx_curcode1 
on opdb.tcop.curcode ( nodeid, curcode)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'idx_curcode2'
-----------------------------------------------------------------------------
print 'Creating Index idx_curcode2'
go

create unique nonclustered index idx_curcode2 
on opdb.tcop.curcode ( cursym)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.exchgarea'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.exchgarea'
go 

use opdb
go 

setuser 'tcop'
go 

create table exchgarea (
	nodeid                          int                              not null  ,
	areacode                        char(4)                          not null  ,
	areaname                        char(20)                         not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'exchgarea_idx1'
-----------------------------------------------------------------------------
print 'Creating Index exchgarea_idx1'
go

create unique nonclustered index exchgarea_idx1 
on opdb.tcop.exchgarea ( nodeid, areacode)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.generalcode'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.generalcode'
go 

use opdb
go 

setuser 'tcop'
go 

create table generalcode (
	nodeid                          int                              not null  ,
	codetype                        char(4)                          not null  ,
	codeid                          char(60)                         not null  ,
	codename                        char(60)                         not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'generalcode_idx1'
-----------------------------------------------------------------------------
print 'Creating Index generalcode_idx1'
go

create unique nonclustered index generalcode_idx1 
on opdb.tcop.generalcode ( nodeid, codetype, codeid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.operjour'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.operjour'
go 

use opdb
go 

setuser 'tcop'
go 

create table operjour (
	nodeid                          int                              not null  ,
	seqno                           int                              not null  ,
	bankno                          char(16)                         not null  ,
	operid                          char(12)                         not null  ,
	date                            char(8)                          not null  ,
	time                            char(6)                          not null  ,
	trcode                          char(6)                          not null  ,
	memo                            varchar(128)                         null  ,
	reserved1                       char(4)                              null  ,
	reserved2                       varchar(32)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create nonclustered index index_1 
on opdb.tcop.operjour ( nodeid, bankno, operid, date)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.operinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.operinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table operinfo (
	nodeid                          int                              not null  ,
	operid                          char(12)                         not null  ,
	bankno                          char(12)                             null  ,
	organid                         char(16)                         not null  ,
	name                            char(20)                         not null  ,
	bklevel                         char(1)                              null  ,
	class                           char(1)                          not null  ,
	passwd                          char(32)                         not null  ,
	status                          char(1)                          not null  ,
	regdate                         char(8)                              null  ,
	canceldate                      char(8)                              null  ,
	lastlogintime                   char(20)                             null  ,
	pwdchgdate                      char(8)                              null  ,
	amtlimit                        varchar(20)                          null  ,
	rights                          text                                 null  ,
	reserved1                       char(4)                              null  ,
	reserved2                       varchar(16)                          null  ,
	reserved3                       varchar(32)                          null   
)
lock allpages
 on 'default'
go 

sp_placeobject 'default', 'tcop.operinfo.toperinfo'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create unique nonclustered index index_1 
on opdb.tcop.operinfo ( nodeid, operid, bankno)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_2'
-----------------------------------------------------------------------------
print 'Creating Index index_2'
go

create unique nonclustered index index_2 
on opdb.tcop.operinfo ( nodeid, operid, organid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.acctjour'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.acctjour'
go 

use opdb
go 

setuser 'tcop'
go 

create table acctjour (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	originator                      varchar(12)                      not null  ,
	refid                           char(16)                         not null  ,
	inoutflag                       char(1)                          not null  ,
	trncode                         char(6)                          not null  ,
	acctserial                      char(32)                         not null  ,
	revserial                       char(32)                             null  ,
	acctorg                         varchar(12)                      not null  ,
	acctoper                        varchar(12)                      not null  ,
	revoper                         varchar(12)                          null  ,
	result                          char(1)                         DEFAULT  '0' 
  not null  ,
	chkstate                        char(1)                         DEFAULT  '0' 
  not null  ,
	reserved1                       varchar(16)                          null  ,
	reserved2                       varchar(16)                          null  ,
	reserved3                       varchar(32)                          null  ,
	memo                            varchar(64)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create nonclustered index index_1 
on opdb.tcop.acctjour ( workdate, acctserial, acctorg)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_2'
-----------------------------------------------------------------------------
print 'Creating Index index_2'
go

create nonclustered index index_2 
on opdb.tcop.acctjour ( workdate, acctserial)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_3'
-----------------------------------------------------------------------------
print 'Creating Index index_3'
go

create nonclustered index index_3 
on opdb.tcop.acctjour ( nodeid, workdate, originator, refid, inoutflag)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_4'
-----------------------------------------------------------------------------
print 'Creating Index index_4'
go

create unique nonclustered index index_4 
on opdb.tcop.acctjour ( nodeid, workdate, originator, refid, inoutflag, trncode)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.agreement'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.agreement'
go 

use opdb
go 

setuser 'tcop'
go 

create table agreement (
	nodeid                          int                                  null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.freemsg'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.freemsg'
go 

use opdb
go 

setuser 'tcop'
go 

create table freemsg (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	refid                           varchar(20)                      not null  ,
	originator                      char(12)                         not null  ,
	acceptor                        char(12)                         not null  ,
	rebankno                        char(12)                             null  ,
	inoutflag                       char(1)                          not null  ,
	classid                         int                                  null  ,
	title                           varchar(80)                      not null  ,
	content                         varchar(1024)                    not null  ,
	replycontent                    varchar(1024)                        null  ,
	sender                          varchar(12)                      not null  ,
	replyer                         varchar(12)                          null  ,
	replyflag                       char(1)                         DEFAULT  '0' 
      null  ,
	readflag                        char(1)                         DEFAULT  '0' 
      null  ,
	sendtime                        char(6)                              null  ,
	replytime                       char(6)                              null  ,
	replydate                       char(8)                              null  ,
	reserved1                       varchar(16)                          null  ,
	reserved2                       varchar(32)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create nonclustered index index_1 
on opdb.tcop.freemsg ( nodeid, workdate, refid, originator)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.bankjour'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.bankjour'
go 

use opdb
go 

setuser 'tcop'
go 

create table bankjour (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	workround                       char(1)                          not null  ,
	inoutflag                       char(1)                          not null  ,
	refid                           char(16)                         not null  ,
	originator                      char(12)                         not null  ,
	acceptor                        char(12)                         not null  ,
	classid                         int                                  null  ,
	dcflag                          char(1)                              null  ,
	notetype                        char(2)                              null  ,
	noteno                          char(12)                             null  ,
	curcode                         char(3)                              null  ,
	curtype                         char(1)                              null  ,
	settlamt                        decimal(16,2)                        null  ,
	payingacct                      char(32)                             null  ,
	beneacct                        char(32)                             null  ,
	payer                           char(60)                             null  ,
	benename                        char(60)                             null  ,
	flag                            char(1)                              null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create unique nonclustered index index_1 
on opdb.tcop.bankjour ( nodeid, workdate, inoutflag, refid, originator)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.ebanksumm'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.ebanksumm'
go 

use opdb
go 

setuser 'tcop'
go 

create table ebanksumm (
	nodeid                          int                              not null  ,
	branchid                        char(12)                         not null  ,
	bankid                          char(12)                         not null  ,
	workdate                        char(8)                          not null  ,
	workround                       int                              not null  ,
	cleardate                       char(8)                          not null  ,
	clearround                      int                              not null  ,
	svcclass                        int                              not null  ,
	curcode                         char(3)                          not null  ,
	curtype                         char(1)                          not null  ,
	pres_debit_num                  int                                  null  ,
	pres_debit_total                decimal(16,2)                        null  ,
	pres_credit_num                 int                                  null  ,
	pres_credit_total               decimal(16,2)                        null  ,
	acpt_debit_num                  int                                  null  ,
	acpt_debit_total                decimal(16,2)                        null  ,
	acpt_credit_num                 int                                  null  ,
	acpt_credit_total               decimal(16,2)                        null  ,
	balance                         decimal(16,2)                        null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'ebanksumm_idx1'
-----------------------------------------------------------------------------
print 'Creating Index ebanksumm_idx1'
go

create unique nonclustered index ebanksumm_idx1 
on opdb.tcop.ebanksumm ( nodeid, branchid, workdate, clearround, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'ebanksumm_idx2'
-----------------------------------------------------------------------------
print 'Creating Index ebanksumm_idx2'
go

create nonclustered index ebanksumm_idx2 
on opdb.tcop.ebanksumm ( nodeid, bankid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.errinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.errinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table errinfo (
	nodeid                          int                              not null  ,
	errcode                         varchar(12)                      not null  ,
	errinfo                         varchar(128)                         null  ,
		CONSTRAINT pk_errinfo PRIMARY KEY CLUSTERED ( nodeid, errcode )  on 'default' 
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.errmap'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.errmap'
go 

use opdb
go 

setuser 'tcop'
go 

create table errmap (
	nodeid                          int                              not null  ,
	errcode                         varchar(12)                      not null  ,
	mapnode                         int                              not null  ,
	mapcode                         varchar(8)                       not null  ,
		CONSTRAINT pk_errmap PRIMARY KEY CLUSTERED ( nodeid, errcode, mapnode )  on 'default' 
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.feelist'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.feelist'
go 

use opdb
go 

setuser 'tcop'
go 

create table feelist (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	inoutflag                       char(1)                          not null  ,
	originator                      char(12)                         not null  ,
	refid                           char(16)                         not null  ,
	feedate                         char(8)                              null  ,
	acctno                          varchar(32)                          null  ,
	feepayer                        char(1)                              null  ,
	amount                          decimal(16,2)                    not null  ,
	result                          char(1)                         DEFAULT  '0' 
  not null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.feeset'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.feeset'
go 

use opdb
go 

setuser 'tcop'
go 

create table feeset (
	nodeid                          int                              not null  ,
	acctno                          varchar(32)                      not null  ,
	typeid                          int                              not null  ,
	acctname                        varchar(80)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.feesum'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.feesum'
go 

use opdb
go 

setuser 'tcop'
go 

create table feesum (
	nodeid                          int                              not null  ,
	branchid                        char(12)                         not null  ,
	bankid                          char(12)                         not null  ,
	classid                         int                                  null  ,
	notetype                        char(2)                          not null  ,
	curcode                         char(3)                              null  ,
	curtype                         char(1)                              null  ,
	workdate                        char(8)                              null  ,
	workround                       int                                  null  ,
	cleardate                       char(8)                              null  ,
	clearround                      int                                  null  ,
	num                             int                                  null  ,
	fee                             decimal(16,2)                        null  ,
	dueflag                         char(1)                              null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create nonclustered index index_1 
on opdb.tcop.feesum ( nodeid, branchid, notetype, curcode, curtype, workdate, workround)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.htrnjour'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.htrnjour'
go 

use opdb
go 

setuser 'tcop'
go 

create table htrnjour (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	refid                           char(16)                         not null  ,
	inoutflag                       char(1)                          not null  ,
	regioncode                      char(6)                              null  ,
	trncode                         char(6)                              null  ,
	classid                         int                                  null  ,
	presdate                        char(8)                          not null  ,
	prestime                        char(6)                          not null  ,
	originator                      char(12)                         not null  ,
	acceptor                        char(12)                         not null  ,
	innerorganid                    char(12)                             null  ,
	dcflag                          char(1)                          not null  ,
	notetype                        char(2)                          not null  ,
	bknotetype                      varchar(8)                           null  ,
	noteno                          char(32)                         not null  ,
	curcode                         char(3)                              null  ,
	curtype                         char(1)                              null  ,
	issuedate                       char(8)                          not null  ,
	settlamt                        decimal(16,2)                    not null  ,
	issueamt                        decimal(16,2)                        null  ,
	remnamt                         decimal(16,2)                        null  ,
	payingacct                      char(32)                             null  ,
	payer                           char(80)                             null  ,
	payingbank                      char(12)                             null  ,
	pcbank                          char(12)                             null  ,
	beneacct                        char(32)                             null  ,
	benename                        char(80)                             null  ,
	benebank                        char(12)                             null  ,
	bcbank                          char(12)                             null  ,
	agreement                       char(18)                             null  ,
	purpose                         char(60)                             null  ,
	memo                            varchar(254)                         null  ,
	acctoper                        char(16)                             null  ,
	auditor                         char(16)                             null  ,
	paykey                          char(32)                             null  ,
	workround                       char(1)                              null  ,
	cleardate                       char(8)                              null  ,
	clearround                      char(1)                              null  ,
	cleartime                       char(6)                              null  ,
	clearstate                      char(1)                          not null  ,
	result                          int                             DEFAULT  0 
  not null  ,
	truncflag                       char(1)                              null  ,
	printnum                        int                             DEFAULT  0 
      null  ,
	fee                             decimal(16,2)                        null  ,
	feepayer                        char(1)                              null  ,
	extraxml                        varchar(1024)                        null  ,
	reserved                        varchar(254)                         null  ,
	reserved2                       varchar(30)                          null  ,
	reserved3                       varchar(60)                          null   
)
lock datarows
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx2'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx2'
go

create nonclustered index trnjour_idx2 
on opdb.tcop.htrnjour ( workdate)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx3'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx3'
go

create nonclustered index trnjour_idx3 
on opdb.tcop.htrnjour ( workdate, originator, notetype, noteno)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx4'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx4'
go

create nonclustered index trnjour_idx4 
on opdb.tcop.htrnjour ( originator)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx5'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx5'
go

create nonclustered index trnjour_idx5 
on opdb.tcop.htrnjour ( acceptor)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx6'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx6'
go

create nonclustered index trnjour_idx6 
on opdb.tcop.htrnjour ( notetype, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx7'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx7'
go

create nonclustered index trnjour_idx7 
on opdb.tcop.htrnjour ( classid, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_8'
-----------------------------------------------------------------------------
print 'Creating Index index_8'
go

create unique nonclustered index index_8 
on opdb.tcop.htrnjour ( nodeid, workdate, refid, inoutflag, originator)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.notetypemap'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.notetypemap'
go 

use opdb
go 

setuser 'tcop'
go 

create table notetypemap (
	nodeid                          int                              not null  ,
	tctype                          char(4)                          not null  ,
	dcflag                          char(1)                              null  ,
	truncflag                       char(1)                              null  ,
	banktype                        varchar(8)                       not null  ,
	bknotename                      varchar(60)                          null  ,
	reserved1                       varchar(20)                          null  ,
	reserved2                       varchar(6)                           null  ,
	reserved3                       varchar(20)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create unique nonclustered index index_1 
on opdb.tcop.notetypemap ( nodeid, tctype, dcflag)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_2'
-----------------------------------------------------------------------------
print 'Creating Index index_2'
go

create nonclustered index index_2 
on opdb.tcop.notetypemap ( nodeid, tctype)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.reconinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.reconinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table reconinfo (
	nodeid                          int                              not null  ,
	bankid                          char(12)                         not null  ,
	workdate                        char(8)                          not null  ,
	workround                       int                              not null  ,
	cleardate                       char(8)                          not null  ,
	clearround                      int                              not null  ,
	svcclass                        int                              not null  ,
	curcode                         char(3)                          not null  ,
	curtype                         char(1)                          not null  ,
	pres_debit_num                  int                                  null  ,
	pres_debit_total                decimal(16,2)                        null  ,
	pres_credit_num                 int                                  null  ,
	pres_credit_total               decimal(16,2)                        null  ,
	acpt_debit_num                  int                                  null  ,
	acpt_debit_total                decimal(16,2)                        null  ,
	acpt_credit_num                 int                                  null  ,
	acpt_credit_total               decimal(16,2)                        null  ,
	balance                         decimal(16,2)                        null  ,
	flag                            char(1)                              null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'reconinfo_idx1'
-----------------------------------------------------------------------------
print 'Creating Index reconinfo_idx1'
go

create unique nonclustered index reconinfo_idx1 
on opdb.tcop.reconinfo ( nodeid, bankid, workdate, workround, svcclass, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'reconinfo_idx2'
-----------------------------------------------------------------------------
print 'Creating Index reconinfo_idx2'
go

create nonclustered index reconinfo_idx2 
on opdb.tcop.reconinfo ( nodeid, workdate, workround)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'reconinfo_idx3'
-----------------------------------------------------------------------------
print 'Creating Index reconinfo_idx3'
go

create nonclustered index reconinfo_idx3 
on opdb.tcop.reconinfo ( nodeid, bankid)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.recvbox'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.recvbox'
go 

use opdb
go 

setuser 'tcop'
go 

create table recvbox (
	nodeid                          int                              not null  ,
	mailid                          int                              not null  ,
	sender                          char(12)                         not null  ,
	recver                          char(12)                         not null  ,
	writer                          varchar(12)                          null  ,
	auditor                         varchar(12)                          null  ,
	mailtype                        char(1)                          not null  ,
	readed                          char(1)                         DEFAULT  '0' 
  not null  ,
	recvdate                        char(8)                          not null  ,
	recvtime                        char(6)                              null  ,
	title                           char(80)                             null  ,
	content                         char(2048)                           null  ,
	replyflag                       char(1)                         DEFAULT  '0' 
  not null  ,
	replydate                       char(8)                              null  ,
	replytime                       char(6)                              null  ,
	svcclass                        char(1)                              null  ,
	reserved                        varchar(16)                          null  ,
	reserved2                       varchar(32)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'recvbox_idx1'
-----------------------------------------------------------------------------
print 'Creating Index recvbox_idx1'
go

create unique nonclustered index recvbox_idx1 
on opdb.tcop.recvbox ( mailid, nodeid, sender, recvdate)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'recvbox_idx2'
-----------------------------------------------------------------------------
print 'Creating Index recvbox_idx2'
go

create nonclustered index recvbox_idx2 
on opdb.tcop.recvbox ( sender)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'recvbox_idx3'
-----------------------------------------------------------------------------
print 'Creating Index recvbox_idx3'
go

create nonclustered index recvbox_idx3 
on opdb.tcop.recvbox ( recver)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.regioninfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.regioninfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table regioninfo (
	regionid                        char(6)                          not null  ,
	regionname                      char(60)                         not null  ,
	regionlevel                     char(1)                              null  ,
	superior                        char(6)                              null  ,
	commnode                        char(8)                          not null  ,
	enableflag                      char(1)                         DEFAULT  '0' 
      null  ,
		CONSTRAINT pk_regioninfo PRIMARY KEY CLUSTERED ( commnode )  on 'default' 
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.sendbox'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.sendbox'
go 

use opdb
go 

setuser 'tcop'
go 

create table sendbox (
	nodeid                          int                              not null  ,
	mailid                          int                              not null  ,
	sender                          char(12)                         not null  ,
	recver                          varchar(254)                     not null  ,
	writer                          varchar(12)                          null  ,
	auditor                         varchar(12)                          null  ,
	mailtype                        char(1)                          not null  ,
	sended                          char(1)                         DEFAULT  '0' 
  not null  ,
	senddate                        char(8)                          not null  ,
	sendtime                        char(6)                              null  ,
	title                           char(80)                             null  ,
	content                         char(2048)                           null  ,
	originmailid                    int                                  null  ,
	svcclass                        char(1)                              null  ,
	reserved                        varchar(16)                          null  ,
	reserved2                       varchar(32)                          null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'sendbox_idx1'
-----------------------------------------------------------------------------
print 'Creating Index sendbox_idx1'
go

create unique nonclustered index sendbox_idx1 
on opdb.tcop.sendbox ( nodeid, senddate, mailid, sender)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'sendbox_idx2'
-----------------------------------------------------------------------------
print 'Creating Index sendbox_idx2'
go

create nonclustered index sendbox_idx2 
on opdb.tcop.sendbox ( sender)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'sendbox_idx3'
-----------------------------------------------------------------------------
print 'Creating Index sendbox_idx3'
go

create nonclustered index sendbox_idx3 
on opdb.tcop.sendbox ( recver)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.syspara'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.syspara'
go 

use opdb
go 

setuser 'tcop'
go 

create table syspara (
	nodeid                          int                              not null  ,
	paraname                        char(32)                         not null  ,
	paraval                         char(64)                         not null  ,
	paradesc                        char(64)                             null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'index_1'
-----------------------------------------------------------------------------
print 'Creating Index index_1'
go

create unique nonclustered index index_1 
on opdb.tcop.syspara ( nodeid, paraname)
go 


-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.thirdinfo'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.thirdinfo'
go 

use opdb
go 

setuser 'tcop'
go 

create table thirdinfo (
	nodeid                          int                                  null  ,
	thirdcode                       char(2)                              null   
)
lock allpages
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Table'opdb.tcop.trnjour'
-----------------------------------------------------------------------------
print 'Creating Table opdb.tcop.trnjour'
go 

use opdb
go 

setuser 'tcop'
go 

create table trnjour (
	nodeid                          int                              not null  ,
	workdate                        char(8)                          not null  ,
	refid                           char(16)                         not null  ,
	inoutflag                       char(1)                          not null  ,
	regioncode                      char(6)                              null  ,
	trncode                         char(6)                              null  ,
	classid                         int                                  null  ,
	presdate                        char(8)                          not null  ,
	prestime                        char(6)                          not null  ,
	originator                      char(12)                         not null  ,
	acceptor                        char(12)                         not null  ,
	innerorganid                    char(12)                             null  ,
	dcflag                          char(1)                          not null  ,
	notetype                        char(2)                          not null  ,
	bknotetype                      varchar(8)                           null  ,
	noteno                          char(32)                         not null  ,
	curcode                         char(3)                              null  ,
	curtype                         char(1)                              null  ,
	issuedate                       char(8)                          not null  ,
	settlamt                        decimal(16,2)                    not null  ,
	issueamt                        decimal(16,2)                        null  ,
	remnamt                         decimal(16,2)                        null  ,
	payingacct                      char(32)                             null  ,
	payer                           char(80)                             null  ,
	payingbank                      char(12)                             null  ,
	pcbank                          char(12)                             null  ,
	beneacct                        char(32)                             null  ,
	benename                        char(80)                             null  ,
	benebank                        char(12)                             null  ,
	bcbank                          char(12)                             null  ,
	agreement                       char(18)                             null  ,
	purpose                         char(60)                             null  ,
	memo                            varchar(254)                         null  ,
	acctoper                        char(16)                             null  ,
	auditor                         char(16)                             null  ,
	paykey                          char(32)                             null  ,
	workround                       char(1)                              null  ,
	cleardate                       char(8)                              null  ,
	clearround                      char(1)                              null  ,
	cleartime                       char(6)                              null  ,
	clearstate                      char(1)                          not null  ,
	result                          int                             DEFAULT  0 
  not null  ,
	truncflag                       char(1)                              null  ,
	printnum                        int                             DEFAULT  0 
      null  ,
	fee                             decimal(16,2)                        null  ,
	feepayer                        char(1)                              null  ,
	extraxml                        varchar(1024)                        null  ,
	reserved                        varchar(254)                         null  ,
	reserved2                       varchar(30)                          null  ,
	reserved3                       varchar(60)                          null   
)
lock datarows
 on 'default'
go 


setuser
go 

-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx2'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx2'
go

create nonclustered index trnjour_idx2 
on opdb.tcop.trnjour ( workdate)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx3'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx3'
go

create nonclustered index trnjour_idx3 
on opdb.tcop.trnjour ( workdate, originator, notetype, noteno)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx4'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx4'
go

create nonclustered index trnjour_idx4 
on opdb.tcop.trnjour ( originator)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx5'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx5'
go

create nonclustered index trnjour_idx5 
on opdb.tcop.trnjour ( acceptor)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx6'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx6'
go

create nonclustered index trnjour_idx6 
on opdb.tcop.trnjour ( notetype, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'trnjour_idx7'
-----------------------------------------------------------------------------
print 'Creating Index trnjour_idx7'
go

create nonclustered index trnjour_idx7 
on opdb.tcop.trnjour ( classid, curcode, curtype)
go 


-----------------------------------------------------------------------------
-- DDL for Index 'index_8'
-----------------------------------------------------------------------------
print 'Creating Index index_8'
go

create unique nonclustered index index_8 
on opdb.tcop.trnjour ( nodeid, workdate, refid, inoutflag, originator)
go 


-----------------------------------------------------------------------------
-- Dependent DDL for Object(s)
-----------------------------------------------------------------------------
use opdb
go

exec sp_changedbowner 'sa'
go

exec master.dbo.sp_dboption opdb, 'abort tran on log full', true
go

checkpoint
go

use opdb
go 

sp_addthreshold opdb, 'logsegment', 104, sp_thresholdaction
go 

Grant Select on dbo.sysalternates to public
go

Grant Select on dbo.sysattributes to public
go

Grant Select on dbo.syscolumns to public
go

Grant Select on dbo.syscolumns(accessrule) to public
go

Grant Select on dbo.syscolumns(cdefault) to public
go

Grant Select on dbo.syscolumns(colid) to public
go

Grant Select on dbo.syscolumns(domain) to public
go

Grant Select on dbo.syscolumns(encrdate) to public
go

Grant Select on dbo.syscolumns(encrlen) to public
go

Grant Select on dbo.syscolumns(encrtype) to public
go

Grant Select on dbo.syscolumns(id) to public
go

Grant Select on dbo.syscolumns(length) to public
go

Grant Select on dbo.syscolumns(name) to public
go

Grant Select on dbo.syscolumns(number) to public
go

Grant Select on dbo.syscolumns(offset) to public
go

Grant Select on dbo.syscolumns(prec) to public
go

Grant Select on dbo.syscolumns(printfmt) to public
go

Grant Select on dbo.syscolumns(remote_name) to public
go

Grant Select on dbo.syscolumns(remote_type) to public
go

Grant Select on dbo.syscolumns(scale) to public
go

Grant Select on dbo.syscolumns(status) to public
go

Grant Select on dbo.syscolumns(status2) to public
go

Grant Select on dbo.syscolumns(type) to public
go

Grant Select on dbo.syscolumns(usertype) to public
go

Grant Select on dbo.syscolumns(xdbid) to public
go

Grant Select on dbo.syscolumns(xstatus) to public
go

Grant Select on dbo.syscolumns(xtype) to public
go

Grant Select on dbo.syscomments to public
go

Grant Select on dbo.sysconstraints to public
go

Grant Select on dbo.sysdepends to public
go

Grant Select on dbo.sysindexes to public
go

Grant Select on dbo.sysjars to public
go

Grant Select on dbo.syskeys to public
go

Grant Select on dbo.syslogs to public
go

Grant Select on dbo.sysobjects(cache) to public
go

Grant Select on dbo.sysobjects(ckfirst) to public
go

Grant Select on dbo.sysobjects(crdate) to public
go

Grant Select on dbo.sysobjects(deltrig) to public
go

Grant Select on dbo.sysobjects(expdate) to public
go

Grant Select on dbo.sysobjects(id) to public
go

Grant Select on dbo.sysobjects(indexdel) to public
go

Grant Select on dbo.sysobjects(instrig) to public
go

Grant Select on dbo.sysobjects(loginame) to public
go

Grant Select on dbo.sysobjects(name) to public
go

Grant Select on dbo.sysobjects(objspare) to public
go

Grant Select on dbo.sysobjects(schemacnt) to public
go

Grant Select on dbo.sysobjects(seltrig) to public
go

Grant Select on dbo.sysobjects(sysstat) to public
go

Grant Select on dbo.sysobjects(sysstat2) to public
go

Grant Select on dbo.sysobjects(type) to public
go

Grant Select on dbo.sysobjects(uid) to public
go

Grant Select on dbo.sysobjects(updtrig) to public
go

Grant Select on dbo.sysobjects(userstat) to public
go

Grant Select on dbo.sysobjects(versionts) to public
go

Grant Select on dbo.syspartitions to public
go

Grant Select on dbo.sysprocedures to public
go

Grant Select on dbo.sysprotects to public
go

Grant Select on dbo.sysqueryplans to public
go

Grant Select on dbo.sysreferences to public
go

Grant Select on dbo.sysroles to public
go

Grant Select on dbo.syssegments to public
go

Grant Select on dbo.sysstatistics to public
go

Grant Select on dbo.systabstats to public
go

Grant Select on dbo.systhresholds to public
go

Grant Select on dbo.systypes to public
go

Grant Select on dbo.sysusermessages to public
go

Grant Select on dbo.sysusers to public
go

Grant Select on dbo.sysxtypes to public
go

Grant Create Table to tcop
go
Grant Create View to tcop
go
Grant Create Procedure to tcop
go
Grant null to tcop
go
Grant Create Default to tcop
go
Grant null to tcop
go
Grant Create Rule to tcop
go


-- DDLGen 已完成
-- 时间 03/23/10 10:16:38 GMT+08:00
