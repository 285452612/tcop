drop index ebanksumm.ebanksumm_idx2
go
drop index ebanksumm.ebanksumm_idx1
go
create unique index ebanksumm_idx1 on ebanksumm (
    nodeid asc,
    branchid asc,
    workdate asc,
    clearround asc,
    curcode asc,
    curtype asc,
    svcclass asc
)
go
create index ebanksumm_idx2 on ebanksumm (
        nodeid asc,
        bankid asc,
        svcclass asc
        )
go
