
#pragma pack(1)

// 系统信息头的格式如下：（55bytes）  报文头总长140B 
typedef struct _msg_head_in
{
    unsigned short  SHJBCD;     // 2B 数据包长度, 转换成网络格式
    char            BAWMAC[16]; // 报文MAC
    char            MACJGH[4];  // MAC机构号, 表示报文中MAC的对方机构号
    char            PINZHZ[16]; // PIN种子, 表示报文中PIN的种子(随机数参与加密)
    unsigned char   YNDIZH[4];  // 目标地址, 二进制方式存储
    unsigned char   MBIODZ[4];  // 源地址, 二进制方式存储
    unsigned char   BOLIUW;     // 系统保留位, 二进制方式存储
    unsigned char   XXJSBZ;     // 信息结束标志, 二进制方式存储
    unsigned short  SJBSXH;     // 2B 报文序号, 转换成网络格式
    unsigned char   JIOYBZ;     // 校验标志, 二进制方式存储
    int             MIYBBH;     // 4B 密钥版本号    
} msg_head_in;

// 交易公共头 (21bytes)
typedef struct _pub_head_in
{
    char            ZHNGDH[5]; // 终端号, 字符串方式存储
    char            CHSHDM[4]; // 城市代码
    char            YNGYJG[4]; // 机构代码 
    char            JIO1GY[8]; // 交易柜员
} pub_head_in;

// 交易数据头 (64bytes)
typedef struct _data_head_in
{
      char           JIAOYM[4]; // 交易代码    
      char           JIOYZM[2]; // 交易子码    
      char           JIOYMS[1]; // 交易模式    
      int            JIOYXH;    // 4B 交易序号    
      unsigned short COMMLN;    // 2B 本交易包长度
      unsigned short PNYIL1;    // 2B 0xFFFF为无效 系统偏移1   
      unsigned short PNYIL2;    // 2B 0xFFFF为无效 系统偏移2   
      char           QANTLS[12];// 前台流水号  
      char           QANTRQ[8]; // 前台日期    
      char           SHOQGY[8]; // 授权柜员　　
      char           SHOQMM[16];// 授权密码　　
      char           YWKABZ;    // 授权柜员有无卡标志  
      char           CZYNXH[2]; // 授权柜员卡序号      
} data_head_in;

