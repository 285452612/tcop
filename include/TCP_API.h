/* H_GenerateCode
 * 上级行加押
 * 参数说明:
 *  输入参数:
 *      bankno:目的下级行密押机编号
 *      data:待加押数据串
 *      len:待加押数据长度
 *  返回参数:
 *      mac:返回加押结果
 * 返回码: 0 成功；�
 */
int H_GenerateCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode
 * 上级行核押 (使用当前密钥)
 * 参数说明:
 *  输入参数:
 *      bankno:源下级行密押机编号
 *      data:待核押数据串
 *      len:待核押数据长度
 *      mac:待核密押
 * 返回码: 0 成功；�
 */
int H_CheckCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode_c
 * 上级行核押 (使用所有有效密钥)
 * 参数说明:
 *  输入参数:
 *      bankno:源下级行密押机编号
 *      data:待核押数据串
 *      len:待核押数据长度
 *      mac:待核密押
 * 返回码: 0 成功；�
 */
int H_CheckCode_c(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* GenerateCode
 * 下级行加押
 * 参数说明:
 *  输入参数:
 *      data:待加押数据串
 *      len:待加押数据长度
 *  返回参数:
 *      mac:返回加押结果
 * 返回码: 0 成功；�
 */
int GenerateCode(unsigned char *data,int len,unsigned char mac[17]);

/* CheckCode
 * 下级行核押
 * 参数说明:
 *  输入参数:
 *      data:待核押数据串
 *      len:待核押数据长度
 *      mac:待核验密押
 * 返回码: 0 成功；�
 */
int CheckCode(unsigned char *data,int len,unsigned char mac[17]);

/* Data_Encrypt
 * 数据加密
 * 参数说明:
 *  输入参数:
 *      workdate:工作日期YYYYMMDD
 *      refid:提出流水
 *      data:待加密数据串
 *      len:待加密数据长度
 *  返回参数:
 *      enc_data:密文数据串
 *      enc_len:密文数据串长度
 * 返回码: 0 成功；�
 */
int Data_Encrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len);

/* ConvEncData
 * 数据转加密
 * 参数说明:
 *  输入参数:
 *      bankno_1:源加密密押机设备号
 *      bankno_2:目的行解密密押机设备号
 *      data:密文数据串
 *      len:密文长度
 * 返回码: 0 成功；�
 */
int ConvEncData(unsigned char bankno_1[8],unsigned char bankno_2[8],unsigned char *data,int len);

/* Data_Decrypt
 * 数据解密
 * 参数说明:
 *  输入参数:
 *      workdate:工作日期YYYYMMDD
 *      refid:提出流水
 *      data:待解密数据串
 *      len:待解密数据长度
 *  返回参数:
 *      dec_data:明文数据串
 *      dec_len:明文数据串长度
 * 返回码: 0 成功；�
 */
int Data_Decrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len);

#define MAX_BUFF 4096

//密押调用错误
#define ERR_SOCKET			-1 //连接密押机失败
#define ERR_RECV			-2 //接收密押机数据失败
#define ERR_SEND			-3 //发送数据失败
//密押机返回错误码说明
#define ERR_CARD_SELFTEST   0x01 //卡自检硬件错误
#define ERR_NOLMK 			0x02 //卡中LMK数据无效
#define ERR_FPGA_SELFTEST   0x03 //FPGA自检错误
#define ERR_TIMECHIP 		0x04 //时钟芯片异常
#define ERR_CARDCMD 		0x10 //命令代码错误
#define ERR_CARDDATA_LEN 	0x11 //数据长度错误
#define ERR_CARDDATA_CRC 	0x12 //数据包CRC校验错误
#define ERR_OPER_ID 		0x20 //管理员ID号错误
#define ERR_OPER_PWD 		0x21 //管理员口令错误
#define ERR_OPER_CHK0 		0x22 //管理员口令容错次数为0
#define ERR_NOAUTH 			0x23 //非二人登录（非授权状态）
#define ERR_NOLOGIN 		0x24 //无管理员登录（未登录状态）
#define ERR_ALREADY_LOGIN   0x25 //管理员已经登录
#define ERR_AIRTHMETIC 		0x30 //指定的算法无效
#define ERR_DATALEN_8 		0x31 //数据长度错误，不能被8整除
#define ERR_DATALEN_16 		0x32 //数据长度错误，不能被16整除
#define ERR_DATALEN_128 	0x33 //数据长度错误，不能被128整除
#define ERR_DATALEN_256 	0x34 //数据长度错误，不能被256整除
#define ERR_AUTH_LMK 		0x35 //指定索引的LMK被禁止
#define ERR_FLASH_ADD 		0x40 //数据FLASH读写地址越界
#define ERR_FLASH_WRITE 	0x41 //写数据FLASH异常
#define ERR_RANDCHIP 		0x42 //随机数芯片操作异常
#define ERR_CENCCHIP 		0x43 //国密算法芯片操作异常
#define ERR_ADMINNO 		0x51 //管理员号错误	
#define ERR_ADMINAGIN 	    0x52 //管理员已登录	
#define ERR_ADMINPWD 		0x53 //管理员口令错误	
#define ERR_CHECKTAG 		0x54 //Tag校验错误	
#define ERR_CHECKDAC 		0x55 //DAC校验错误	
#define ERR_KEYFLAG 		0x56 //密钥标志错误	
#define ERR_BANKNO 			0x57 //银行行号错误	
#define ERR_DATALEN 		0x58 //加密数据长度错误	
#define ERR_LOGINTWO 		0x59 //登录的管理员人数少于两人	
#define ERR_SELFTEST   	    0x5A //自检失败
#define ERR_ENCODE 			0x5B //密押错误	
#define ERR_LOGINTHREE 	    0x5C //登录的管理员人数少于三人	
#define ERR_CHECKLMK 		0x5D //系统主密钥较验错误	
#define ERR_COMMAND 		0x5E //未知命令
#define ERR_KEYAGIN 		0x5F //密钥已设置
#define ERR_COM10           0x60 //与商业行密押卡通讯数据包长度错误
#define ERR_COM11           0x61 //与商业行密押卡通讯超时
#define ERR_COM12           0x62 //与商业行密押卡通讯数据校验错误
#define ERR_COM13           0x63 //与商业行密押卡通讯握手错误
#define ERR_COM14           0x64 //与商业行密押卡通讯未知错误
#define ERR_FLASH           0x65 //商业行密押卡写Flash 错误
#define ERR_LENGTH          0x66 //数据包长度错误
#define ERR_DATAFORMAT      0x67 //数据包格式错误
#define ERR_DATACRC     	0x68 //数据包CRC错误
#define ERR_BANKNO_ISEMPTY 	0x69 //行号为空
#define ERR_CARDTIMEOUT		0x6A //密押卡处理超时	
#define ERR_CARDWRITE		0x6B //写密押卡错误	
#define ERR_CARDREAD    	0x6C //读密押卡错误
#define ERR_CARDBANKNO		0x6D //行号已满
#define ERR_PINFORMAT 		0x80 //PIN格式错误
#define ERR_CHKVAL          0x81 //CHKVAL错误
#define ERR_BANK_NO_INDEX   0x82 //获取BANKNO时序号错误错误
