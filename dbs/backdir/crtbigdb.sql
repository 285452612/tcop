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

--´´½¨opdb

create database opdb on opdbdev = 500 log on opdblogdev = 1000
go

create database hopdb on hopdbdev1 = 2000 log on hopdblogdev = 2000
go

alter database hopdb on hopdbdev2 = 2000
go

alter database hopdb on hopdbdev3 = 2000
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
