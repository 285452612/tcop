#ifndef OPTCODE_H_
#define OPTCODE_H_

/** 平台交易码定义 */

#define OPT_TRAN_BASE       100      //交易类

#define OPT_TRAN_OUTINPUT   OPT_TRAN_BASE+1      //提出录入
#define OPT_TRAN_OUTCHECK   OPT_TRAN_BASE+2      //提出复核
#define OPT_TRAN_OUTMODIFY  OPT_TRAN_BASE+3      //提出修改
#define OPT_TRAN_IN         OPT_TRAN_BASE+4      //提入交易
#define OPT_TRAN_QRYSIGLE   OPT_TRAN_BASE+5      //单笔记录查询
#define OPT_TRAN_ACCOUNT    OPT_TRAN_BASE+6      //手工单笔入账
#define OPT_TRAN_CZ         OPT_TRAN_BASE+7      //手工单笔冲正或取消
#define OPT_TRAN_OUT        OPT_TRAN_BASE+8      //直接提出交易
#define OPT_TRAN_QUERY      OPT_TRAN_BASE+20     //多笔记录查询

#define OPT_INFO_BASE       200      //信息类

#define OPT_INFO_REQSEND    OPT_INFO_BASE+1      //查询书录入                        
#define OPT_INFO_RSPSEND    OPT_INFO_BASE+2      //查复书录入                        
#define OPT_INFO_MSGSEND    OPT_INFO_BASE+3      //自由格式发送                     
#define OPT_INFO_QRYSINGLE  OPT_INFO_BASE+4      //查询查复书单笔查询                        
#define OPT_INFO_INREQBOOK  OPT_INFO_BASE+11     //提入查询书
#define OPT_INFO_INRSPBOOK  OPT_INFO_BASE+12     //提入查复书
#define OPT_INFO_INMSG      OPT_INFO_BASE+13     //提入自由格式

#define OPT_ADM_COMMON      300     //通用管理类

#define OPT_ADM_DELTRAN     OPT_ADM_COMMON+1     //提出交易删除

#define OPT_ADM_ACCT        400     //账户管理类

#define OPT_ACCT_BANKQRY    OPT_ADM_ACCT+1       //行内账户查询
#define OPT_ACCT_REGISTER   OPT_ADM_ACCT+2       //开户
#define OPT_ACCT_CANCEL     OPT_ADM_ACCT+3       //销户
#define OPT_ACCT_MODIFY     OPT_ADM_ACCT+4       //账户修改
#define OPT_ACCT_PBCQRY     OPT_ADM_ACCT+5       //中心账户信息查询

#define OPT_ADM_OPER        500     //柜员管理类

#define OPT_OPER_LOGIN      OPT_ADM_OPER+1       //柜员登录
#define OPT_OPER_LOGOUT     OPT_ADM_OPER+2       //柜员退出
#define OPT_OPER_MODPWD     OPT_ADM_OPER+3       //修改密码
#define OPT_OPER_CANCEL     OPT_ADM_OPER+4       //柜员注销
#define OPT_OPER_CLRPWD     OPT_ADM_OPER+5       //清空密码
#define OPT_OPER_RESET      OPT_ADM_OPER+6       //强制签退
#define OPT_OPER_AUTHORIZE  OPT_ADM_OPER+7       //柜员授权
#define OPT_OPER_GETRIGHTS  OPT_ADM_OPER+8       //获取柜员权限

#define OPT_ADM_SYS         600     //系统管理类

#define OPT_SYS_DOWNFILE    OPT_ADM_SYS+1        //参数更新
#define OPT_SYS_QRYARG      OPT_ADM_SYS+2        //平台参数查询
#define OPT_SYS_XYHREG      OPT_ADM_SYS+3        //协议号注册
#define OPT_SYS_XYHCANCEL   OPT_ADM_SYS+4        //协议号注销
#define OPT_SYS_XYHQRY      OPT_ADM_SYS+5        //协议号查询

#define OPT_QRYPRT_BASE     700     //查询打印类

#define OPT_QRYPRT_NOTES    OPT_QRYPRT_BASE+1    //提出提入凭证查询
#define OPT_QRYPRT_OPER     OPT_QRYPRT_BASE+2    //柜员信息查询

#define OPT_FUND_BASE       900     //资金类

#define OPT_FUND_DOWNDZ     OPT_FUND_BASE+1      //下载对账数据

#endif
