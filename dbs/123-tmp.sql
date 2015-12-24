create table baginfo
(
    regionid    int      not null ,//�ڵ���Ϣ���ɲ�ʹ��
    exchgdate   char(8)  not null ,//��������
    exchground  integer  not null ,//��������
    workdate    char(8)  not null ,//��������
    workround   integer  not null ,//��������
    type        char(1)  not null ,//�����ͣ�1-���ڰ���2-�����
    presbank    char(12) not null ,//���������
    prescbank   char(12) not null ,//���������
    presregion  char(6)  not null ,//�����������
    acptbank    char(12) not null ,//���뽻����
    acptcbank   char(12) not null ,//����������
    acptregion  char(6)  not null ,//���뽻������
    num         integer  not null ,//�ܱ���
    amount      decimal(16,2) not null ,//�ܽ��
    debitnum    integer  not null ,//��Ǳ���
    debitamount decimal(16,2) not null ,//��ǽ��
    creditnum   integer  not null ,//���Ǳ���
    creditamount decimal(16,2) not null ,//���ǽ��
    directnum   integer  not null ,//ֱ��֧������,δ����
    directamount decimal(16,2) not null ,//ֱ��֧�����.δ����
    menu        char(60),//��ע
    excharea     char(6) not null,//��������
    state        char(1) not null//״̬,'C'-������
)
go

create index baginfo_idx1 on baginfo( exchgdate, exchground, type, presbank, acptbank )
go
create unique index baginfo_idx2 on baginfo( workdate , workround , type, presbank, acptbank)
go
