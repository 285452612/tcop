/* H_GenerateCode
 * ÉÏ¼¶ĞĞ¼ÓÑº
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      bankno:Ä¿µÄÏÂ¼¶ĞĞÃÜÑº»ú±àºÅ
 *      data:´ı¼ÓÑºÊı¾İ´®
 *      len:´ı¼ÓÑºÊı¾İ³¤¶È
 *  ·µ»Ø²ÎÊı:
 *      mac:·µ»Ø¼ÓÑº½á¹û
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int H_GenerateCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode
 * ÉÏ¼¶ĞĞºËÑº (Ê¹ÓÃµ±Ç°ÃÜÔ¿)
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      bankno:Ô´ÏÂ¼¶ĞĞÃÜÑº»ú±àºÅ
 *      data:´ıºËÑºÊı¾İ´®
 *      len:´ıºËÑºÊı¾İ³¤¶È
 *      mac:´ıºËÃÜÑº
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int H_CheckCode(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* H_CheckCode_c
 * ÉÏ¼¶ĞĞºËÑº (Ê¹ÓÃËùÓĞÓĞĞ§ÃÜÔ¿)
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      bankno:Ô´ÏÂ¼¶ĞĞÃÜÑº»ú±àºÅ
 *      data:´ıºËÑºÊı¾İ´®
 *      len:´ıºËÑºÊı¾İ³¤¶È
 *      mac:´ıºËÃÜÑº
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int H_CheckCode_c(unsigned char bankno[8],unsigned char *data,int len,unsigned char mac[17]);

/* GenerateCode
 * ÏÂ¼¶ĞĞ¼ÓÑº
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      data:´ı¼ÓÑºÊı¾İ´®
 *      len:´ı¼ÓÑºÊı¾İ³¤¶È
 *  ·µ»Ø²ÎÊı:
 *      mac:·µ»Ø¼ÓÑº½á¹û
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int GenerateCode(unsigned char *data,int len,unsigned char mac[17]);

/* CheckCode
 * ÏÂ¼¶ĞĞºËÑº
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      data:´ıºËÑºÊı¾İ´®
 *      len:´ıºËÑºÊı¾İ³¤¶È
 *      mac:´ıºËÑéÃÜÑº
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int CheckCode(unsigned char *data,int len,unsigned char mac[17]);

/* Data_Encrypt
 * Êı¾İ¼ÓÃÜ
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      workdate:¹¤×÷ÈÕÆÚYYYYMMDD
 *      refid:Ìá³öÁ÷Ë®
 *      data:´ı¼ÓÃÜÊı¾İ´®
 *      len:´ı¼ÓÃÜÊı¾İ³¤¶È
 *  ·µ»Ø²ÎÊı:
 *      enc_data:ÃÜÎÄÊı¾İ´®
 *      enc_len:ÃÜÎÄÊı¾İ´®³¤¶È
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int Data_Encrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len);

/* ConvEncData
 * Êı¾İ×ª¼ÓÃÜ
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      bankno_1:Ô´¼ÓÃÜÃÜÑº»úÉè±¸ºÅ
 *      bankno_2:Ä¿µÄĞĞ½âÃÜÃÜÑº»úÉè±¸ºÅ
 *      data:ÃÜÎÄÊı¾İ´®
 *      len:ÃÜÎÄ³¤¶È
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int ConvEncData(unsigned char bankno_1[8],unsigned char bankno_2[8],unsigned char *data,int len);

/* Data_Decrypt
 * Êı¾İ½âÃÜ
 * ²ÎÊıËµÃ÷:
 *  ÊäÈë²ÎÊı:
 *      workdate:¹¤×÷ÈÕÆÚYYYYMMDD
 *      refid:Ìá³öÁ÷Ë®
 *      data:´ı½âÃÜÊı¾İ´®
 *      len:´ı½âÃÜÊı¾İ³¤¶È
 *  ·µ»Ø²ÎÊı:
 *      dec_data:Ã÷ÎÄÊı¾İ´®
 *      dec_len:Ã÷ÎÄÊı¾İ´®³¤¶È
 * ·µ»ØÂë: 0 ³É¹¦£»Æ
 */
int Data_Decrypt(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len);

#define MAX_BUFF 4096

//ÃÜÑºµ÷ÓÃ´íÎó
#define ERR_SOCKET			-1 //Á¬½ÓÃÜÑº»úÊ§°Ü
#define ERR_RECV			-2 //½ÓÊÕÃÜÑº»úÊı¾İÊ§°Ü
#define ERR_SEND			-3 //·¢ËÍÊı¾İÊ§°Ü
//ÃÜÑº»ú·µ»Ø´íÎóÂëËµÃ÷
#define ERR_CARD_SELFTEST   0x01 //¿¨×Ô¼ìÓ²¼ş´íÎó
#define ERR_NOLMK 			0x02 //¿¨ÖĞLMKÊı¾İÎŞĞ§
#define ERR_FPGA_SELFTEST   0x03 //FPGA×Ô¼ì´íÎó
#define ERR_TIMECHIP 		0x04 //Ê±ÖÓĞ¾Æ¬Òì³£
#define ERR_CARDCMD 		0x10 //ÃüÁî´úÂë´íÎó
#define ERR_CARDDATA_LEN 	0x11 //Êı¾İ³¤¶È´íÎó
#define ERR_CARDDATA_CRC 	0x12 //Êı¾İ°üCRCĞ£Ñé´íÎó
#define ERR_OPER_ID 		0x20 //¹ÜÀíÔ±IDºÅ´íÎó
#define ERR_OPER_PWD 		0x21 //¹ÜÀíÔ±¿ÚÁî´íÎó
#define ERR_OPER_CHK0 		0x22 //¹ÜÀíÔ±¿ÚÁîÈİ´í´ÎÊıÎª0
#define ERR_NOAUTH 			0x23 //·Ç¶şÈËµÇÂ¼£¨·ÇÊÚÈ¨×´Ì¬£©
#define ERR_NOLOGIN 		0x24 //ÎŞ¹ÜÀíÔ±µÇÂ¼£¨Î´µÇÂ¼×´Ì¬£©
#define ERR_ALREADY_LOGIN   0x25 //¹ÜÀíÔ±ÒÑ¾­µÇÂ¼
#define ERR_AIRTHMETIC 		0x30 //Ö¸¶¨µÄËã·¨ÎŞĞ§
#define ERR_DATALEN_8 		0x31 //Êı¾İ³¤¶È´íÎó£¬²»ÄÜ±»8Õû³ı
#define ERR_DATALEN_16 		0x32 //Êı¾İ³¤¶È´íÎó£¬²»ÄÜ±»16Õû³ı
#define ERR_DATALEN_128 	0x33 //Êı¾İ³¤¶È´íÎó£¬²»ÄÜ±»128Õû³ı
#define ERR_DATALEN_256 	0x34 //Êı¾İ³¤¶È´íÎó£¬²»ÄÜ±»256Õû³ı
#define ERR_AUTH_LMK 		0x35 //Ö¸¶¨Ë÷ÒıµÄLMK±»½ûÖ¹
#define ERR_FLASH_ADD 		0x40 //Êı¾İFLASH¶ÁĞ´µØÖ·Ô½½ç
#define ERR_FLASH_WRITE 	0x41 //Ğ´Êı¾İFLASHÒì³£
#define ERR_RANDCHIP 		0x42 //Ëæ»úÊıĞ¾Æ¬²Ù×÷Òì³£
#define ERR_CENCCHIP 		0x43 //¹úÃÜËã·¨Ğ¾Æ¬²Ù×÷Òì³£
#define ERR_ADMINNO 		0x51 //¹ÜÀíÔ±ºÅ´íÎó	
#define ERR_ADMINAGIN 	    0x52 //¹ÜÀíÔ±ÒÑµÇÂ¼	
#define ERR_ADMINPWD 		0x53 //¹ÜÀíÔ±¿ÚÁî´íÎó	
#define ERR_CHECKTAG 		0x54 //TagĞ£Ñé´íÎó	
#define ERR_CHECKDAC 		0x55 //DACĞ£Ñé´íÎó	
#define ERR_KEYFLAG 		0x56 //ÃÜÔ¿±êÖ¾´íÎó	
#define ERR_BANKNO 			0x57 //ÒøĞĞĞĞºÅ´íÎó	
#define ERR_DATALEN 		0x58 //¼ÓÃÜÊı¾İ³¤¶È´íÎó	
#define ERR_LOGINTWO 		0x59 //µÇÂ¼µÄ¹ÜÀíÔ±ÈËÊıÉÙÓÚÁ½ÈË	
#define ERR_SELFTEST   	    0x5A //×Ô¼ìÊ§°Ü
#define ERR_ENCODE 			0x5B //ÃÜÑº´íÎó	
#define ERR_LOGINTHREE 	    0x5C //µÇÂ¼µÄ¹ÜÀíÔ±ÈËÊıÉÙÓÚÈıÈË	
#define ERR_CHECKLMK 		0x5D //ÏµÍ³Ö÷ÃÜÔ¿½ÏÑé´íÎó	
#define ERR_COMMAND 		0x5E //Î´ÖªÃüÁî
#define ERR_KEYAGIN 		0x5F //ÃÜÔ¿ÒÑÉèÖÃ
#define ERR_COM10           0x60 //ÓëÉÌÒµĞĞÃÜÑº¿¨Í¨Ñ¶Êı¾İ°ü³¤¶È´íÎó
#define ERR_COM11           0x61 //ÓëÉÌÒµĞĞÃÜÑº¿¨Í¨Ñ¶³¬Ê±
#define ERR_COM12           0x62 //ÓëÉÌÒµĞĞÃÜÑº¿¨Í¨Ñ¶Êı¾İĞ£Ñé´íÎó
#define ERR_COM13           0x63 //ÓëÉÌÒµĞĞÃÜÑº¿¨Í¨Ñ¶ÎÕÊÖ´íÎó
#define ERR_COM14           0x64 //ÓëÉÌÒµĞĞÃÜÑº¿¨Í¨Ñ¶Î´Öª´íÎó
#define ERR_FLASH           0x65 //ÉÌÒµĞĞÃÜÑº¿¨Ğ´Flash ´íÎó
#define ERR_LENGTH          0x66 //Êı¾İ°ü³¤¶È´íÎó
#define ERR_DATAFORMAT      0x67 //Êı¾İ°ü¸ñÊ½´íÎó
#define ERR_DATACRC     	0x68 //Êı¾İ°üCRC´íÎó
#define ERR_BANKNO_ISEMPTY 	0x69 //ĞĞºÅÎª¿Õ
#define ERR_CARDTIMEOUT		0x6A //ÃÜÑº¿¨´¦Àí³¬Ê±	
#define ERR_CARDWRITE		0x6B //Ğ´ÃÜÑº¿¨´íÎó	
#define ERR_CARDREAD    	0x6C //¶ÁÃÜÑº¿¨´íÎó
#define ERR_CARDBANKNO		0x6D //ĞĞºÅÒÑÂú
#define ERR_PINFORMAT 		0x80 //PIN¸ñÊ½´íÎó
#define ERR_CHKVAL          0x81 //CHKVAL´íÎó
#define ERR_BANK_NO_INDEX   0x82 //»ñÈ¡BANKNOÊ±ĞòºÅ´íÎó´íÎó
