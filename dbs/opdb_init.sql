print 'Disk init'
go

use master
go

disk init name='opdbdev1', physname='/home/sybase/data/opdbdev1.dat', size="2000M"
go

disk init name='opdbdev2', physname='/home/sybase/data/opdbdev2.dat', size="2000M"
go

disk init name='opdbdev3', physname='/home/sybase/data/opdbdev3.dat', size="2000M"
go

disk init name='opdblogdev', physname='/home/sybase/data/opdblogdev.dat', size="2000M"
go

sp_addlogin tcop, tcoppw
go

-----------------------------------------------------------------------------
-- DDL for Database 'opdb'
-----------------------------------------------------------------------------
print 'opdb'
go

create database opdb on opdbdev1 = 2000
log on opdblogdev = 2000
go

alter database opdb on opdbdev2 = 2000
go

alter database opdb on opdbdev3 = 2000
go

sp_dboption opdb, 'abort tran on log full', true
go

checkpoint
go

sp_modifylogin tcop, defdb, opdb
go

-----------------------------------------------------------------------------
-- DDL for User 'tcop'
-----------------------------------------------------------------------------
print 'tcop'

use opdb
go 

grant all to tcop
go

exec sp_adduser 'tcop' ,'tcop' ,'public'
go 
