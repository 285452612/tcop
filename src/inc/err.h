#ifndef ERR_H_
#define ERR_H_

#define ERR_BASE                0
#define ERR_SUCC                0
#define ERR_SUCCBASE            90

#define isSuccess(ret)          ((ret) == ERR_SUCC || ((ret) % 100 / ERR_SUCCBASE == 1))

#define E_OTHER                 999                         /* 其它错误 */

#define ERR_APP                 ERR_BASE+100    //应用错误

#define E_APP_ACCOUNTSUCC       ERR_APP+ERR_SUCCBASE+1      /* 记账成功 */
#define E_APP_ACCOUNTFAIL       ERR_APP+2                   /* 记账失败 */
#define E_APP_CZSUCC            ERR_APP+3                   /* 冲正成功 */
#define E_APP_CZFAIL            ERR_APP+4                   /* 冲正失败 */
#define E_APP_CANCELSUCC        ERR_APP+5                   /* 取消成功 */
#define E_APP_CANCELFAIL        ERR_APP+6                   /* 取消失败 */
#define E_APP_ACCOUNTNOCZ       ERR_APP+7                   /* 记账成功冲正失败 */
#define E_APP_ACCOUNTANDCZ      ERR_APP+8                   /* 记账成功冲正成功 */
#define E_APP_ACCOUNTNOCANCEL   ERR_APP+9                   /* 记账成功取消失败 */
#define E_APP_ACCOUNTANDCANCEL  ERR_APP+10                  /* 记账成功取消成功 */
#define E_APP_NEEDGRANT         ERR_APP+11                  /* 需要授权 */
#define E_APP_GRANTSUCC         ERR_APP+ERR_SUCCBASE+2      /* 授权成功 */
#define E_APP_GRANTFAIL         ERR_APP+13                  /* 授权失败 */
#define E_APP_NONEEDACCOUNT     ERR_APP+14                  /* 交易失败 */

#define ERR_DB                  ERR_BASE+200    //数据库错误

#define E_DB                    ERR_DB+1                    /* 数据库操作错 */
#define E_DB_URLCFG             ERR_DB+2                    /* 数据库URL配置错 */
#define E_DB_OPEN               ERR_DB+3                    /* 数据库打开错 */
#define E_DB_SELECT             ERR_DB+4                    /* 数据库select错 */
#define E_DB_INSERT             ERR_DB+5                    /* 数据库Insert错 */
#define E_DB_UPDATE             ERR_DB+6                    /* 数据库update错 */
#define E_DB_DELETE             ERR_DB+7                    /* 数据库delete错 */
#define E_DB_NORECORD           ERR_DB+8                    /* 没有记录 */
#define E_DB_NOTSUPPORT         ERR_DB+9                    /* 数据库操作不支持 */

#define ERR_SYS                 ERR_BASE+300    //系统错误

#define E_SYS_COMM              ERR_SYS+1                   /* 通讯故障 */
#define E_SYS_COMM_CFG          ERR_SYS+2                   /* 通讯配置错 */
#define E_SYS_COMM_PH           ERR_SYS+3                   /* 与前置通信失败 */
#define E_SYS_COMM_BANK         ERR_SYS+4                   /* 与银行通信失败 */
#define E_SYS_CALL              ERR_SYS+5                   /* 系统调用错 */
#define E_SYS_NODLLFUNC         ERR_SYS+6                   /* 动态库中未找到相应的函数 */
#define E_SYS_ADDDIGEST         ERR_SYS+7                   /* 加押错 */
#define E_SYS_CHKDIGEST         ERR_SYS+8                   /* 核押错 */
#define E_SYS_SJLENCRYPT        ERR_SYS+9                   /* 银联加密错 */
#define E_SYS_SJLDECRYPT        ERR_SYS+10                  /* 银联验密错 */
#define E_SYS_SYDENCRYPT        ERR_SYS+11                  /* 密押机加密错 */
#define E_SYS_SYDDECRYPT        ERR_SYS+12                  /* 密押机验密错 */
#define E_SYS_BANKENCRYPT       ERR_SYS+13                  /* 银行加密错 */

#define ERR_PACK                ERR_BASE+400    //报文错误

#define E_PACK_INIT             ERR_PACK+1                  /* 报文初始化错 */
#define E_PACK_GETVAL           ERR_PACK+2                  /* 报文取值错 */
#define E_PACK_CONVERT          ERR_PACK+3                  /* 报文转换错 */
#define E_PACK_TYPE             ERR_PACK+4                  /* 报文类型错 */
#define E_PACK_CFG              ERR_PACK+5                  /* 报文配置错 */

#define ERR_TRAN                ERR_BASE+500    //交易错误

#define E_TRAN_EXISTSUCC        ERR_TRAN+ERR_SUCCBASE+1     /* 交易已成功存在 */
#define E_TRAN_EXISTFAIL        ERR_TRAN+1                  /* 交易已失败存在 */
#define E_TRAN_EXISTUNKNOW      ERR_TRAN+2                  /* 交易状态未知存在 */

#define E_TRAN_QUERYREPLIED     ERR_TRAN+20                 /* 查询书已回复*/

#define ERR_MNG                 ERR_BASE+600    //管理类错误

#define E_MNG_OPER_NOTEXIST     ERR_MNG+1                   /* 柜员不存在 */ 
#define E_MNG_OPER_PASSWD       ERR_MNG+2                   /* 柜员密码错 */
#define E_MNG_OPER_ONLINE       ERR_MNG+3                   /* 柜员已登录 */
#define E_MNG_OPER_CANCEL       ERR_MNG+4                   /* 柜员已注销 */
#define E_MNG_OPER_RIGHTS       ERR_MNG+5                   /* 柜员权限错 */
#define E_MNG_XY_NOTEXIST       ERR_MNG+6                   /* 协议不存在 */
#define E_MNG_XY_CANCEL         ERR_MNG+7                   /* 协议已注销 */
#define E_MNG_PAYPWD            ERR_MNG+8                   /* 支付密码错误 */


#endif
