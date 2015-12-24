#ifndef CRYPTO_H_
#define CRYPTO_H_

#ifndef DES_ENCRYPT
#define DES_ENCRYPT     1
#endif

#ifndef DES_DECRYPT
#define DES_DECRYPT     0
#endif

void DSP_2_HEX(unsigned char *dsp, unsigned char *hex, int count);
void HEX_2_DSP(unsigned char *hex, unsigned char *dsp, int count);
void Do_XOR(unsigned char *dest, unsigned char *source, int size);
void Do_OR(unsigned char *dest, unsigned char *source, int size);
int OFP_Decrypt(char *in_code, char *out_code);
int OFP_Encrypt(char *in_code, char *out_code);

void opGetMd5(char *data, char *result);
void opGenMd5(unsigned char *data, int len, unsigned char *result);

int opBase64Encode(char *out, char *in, int size);
int opBase64Decode(char *out, char *in, int size);

int Data_Encrypt_Soft10(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len);
int Data_Decrypt_Soft10(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len);

int GetPanByAcctno(char *acctno, char *pan);
int GetPanByTrack2(char *track2, char *pan);

#endif
