disk init name='opdbdev', physname='/home/sybase/data/opdb.dat', size="30M"
go
disk init name='opdblogdev', physname='/home/sybase/data/opdblog.dat', size="30M"
go

sp_addlogin tcop, tcoppw
go

--´´½¨opdb

create database opdb on opdbdev = 30 log on opdblogdev = 30
go

sp_dboption opdb, "abort tran on log full", true
go
checkpoint
go

sp_modifylogin tcop, defdb, opdb
go

use opdb
go

sp_adduser tcop
go

grant all to tcop
go

Grant Create Table to tcop
go
Grant Create View to tcop
go
Grant Create Procedure to tcop
go
Grant Create Default to tcop
go
Grant Create Rule to tcop
go

checkpoint
go
