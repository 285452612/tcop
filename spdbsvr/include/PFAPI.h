/*	Author:		Wolfgang Wang
	Copyright:	Union Tech. Guangzhou
	Date:		2002/10/30
*/
#ifndef __PFAPI_H__

#define __PFAPI_H__

/*801*/
int UnionGenMac(int keyVersion,char *agentNo,int lenOfMacData,char *macData,char *mac);
/*802*/
int UnionVerifyMac(int keyVersion,char *agentNo,int lenOfMacData,char *macData,char *mac);
/*803*/
int UnionEncryptPIN(int keyVersion,char *agentNo,char *PIN,char *PAN,char *pinblock);
/*804*/
int UnionTransPIN(int okeyVersion,char *oagentNo,char *opan,char *pinblock,int dkeyVersion,char *dagentNo,char *dpan,char *dpinblock);
/*805*/
int UnionTransSoftPIN(int okeyVersion,char *oagentNo,char *opan,char *pinblock,int dkeyVersion,char *dagentNo,char *dpan,char *dpinblock);
/*806*/
int UnionResetAllKey();
/*807*/
int UnionGen_JK_RTK_Mac(int version,char *serialnum,char *pikormak,int datalen,char *data,char *mac);
/*819*/
int UnionGetZMK (char *serialnum,char *returnzmk,char *returnzmkchkvalue);
/*820 */
int UnionImportZMK (char *serialnum,char *zmk,char *zmkchkvalue);
/*821*/
int UnionGetWorkKey (int version,char *serialnum, char *workkey,char *workkeychk);
/*822 */
int UnionImportWorkKey (int version,char *serialnum, char *workkey,char *workkeychk);

#endif
