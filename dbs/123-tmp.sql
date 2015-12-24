create table baginfo
(
    regionid    int      not null ,//节点信息，可不使用
    exchgdate   char(8)  not null ,//交换日期
    exchground  integer  not null ,//交换场次
    workdate    char(8)  not null ,//工作日期
    workround   integer  not null ,//工作场次
    type        char(1)  not null ,//包类型，1-场内包，2-场外包
    presbank    char(12) not null ,//提出交换行
    prescbank   char(12) not null ,//提出清算行
    presregion  char(6)  not null ,//提出交换区域
    acptbank    char(12) not null ,//提入交换行
    acptcbank   char(12) not null ,//提入清算行
    acptregion  char(6)  not null ,//提入交换区域
    num         integer  not null ,//总笔数
    amount      decimal(16,2) not null ,//总金额
    debitnum    integer  not null ,//借记笔数
    debitamount decimal(16,2) not null ,//借记金额
    creditnum   integer  not null ,//贷记笔数
    creditamount decimal(16,2) not null ,//贷记金额
    directnum   integer  not null ,//直接支付笔数,未启用
    directamount decimal(16,2) not null ,//直接支付金额.未启用
    menu        char(60),//备注
    excharea     char(6) not null,//交换区域
    state        char(1) not null//状态,'C'-已清算
)
go

create index baginfo_idx1 on baginfo( exchgdate, exchground, type, presbank, acptbank )
go
create unique index baginfo_idx2 on baginfo( workdate , workround , type, presbank, acptbank)
go
