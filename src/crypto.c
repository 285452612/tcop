#include <openssl/md5.h>
#include <openssl/rc4.h>

#include "tcop.h"
#include "crypto.h"

#define MAX_BUFF 4096

/* Ω‚√‹ */
int OFP_Decrypt(char *in_code, char *out_code)
{
    char tmp_code[7];
    char str[2];
    char miw[10][3];
    int i=0,j=0;

    for(i=0;i<=9;i++)
        memset(&miw[i], 0, 3);
    memcpy(miw[1], "05", 2);
    memcpy(miw[2], "72", 2);
    memcpy(miw[3], "94", 2);
    memcpy(miw[4], "38", 2);
    memcpy(miw[5], "16", 2);
    memcpy(miw[6], "49", 2);
    memcpy(miw[7], "61", 2);
    memcpy(miw[8], "50", 2);
    memcpy(miw[9], "83", 2);
    memcpy(miw[0], "27", 2);

    memset(tmp_code, 0, sizeof tmp_code);
    tmp_code[4] = in_code[0];
    tmp_code[0] = in_code[1];
    tmp_code[3] = in_code[2];
    tmp_code[5] = in_code[3];
    tmp_code[1] = in_code[4];
    tmp_code[2] = in_code[5];

    for(i=0;i<=5;i++)
    {
        memset(str, 0, sizeof str);
        str[0] = tmp_code[i];
        j = atoi(str);
        if(i%2 == 0)
            out_code[i] = miw[j][0];
        else
            out_code[i] = miw[j][1];
    }
    out_code[6] = 0;
    return 0;
}

/* º”√‹ */
int OFP_Encrypt(char *in_code, char *out_code)
{
    char tmp_code[7];
    char str[2];
    char miw[10][3];
    int i=0,j=0;

    for(i=0;i<=9;i++)
        memset(&miw[i], 0, 3);
    memcpy(miw[1], "57", 2);
    memcpy(miw[2], "02", 2);
    memcpy(miw[3], "49", 2);
    memcpy(miw[4], "63", 2);
    memcpy(miw[5], "81", 2);
    memcpy(miw[6], "75", 2);
    memcpy(miw[7], "20", 2);
    memcpy(miw[8], "94", 2);
    memcpy(miw[9], "36", 2);
    memcpy(miw[0], "18", 2);

    memset(tmp_code, 0, sizeof tmp_code);
    for(i=0;i<=5;i++)
    {
        memset(str, 0, sizeof str);
        str[0] = in_code[i];
        j = atoi(str);
        if(i%2 == 0)
            tmp_code[i] = miw[j][0];
        else
            tmp_code[i] = miw[j][1];
    }
    tmp_code[6] = 0;

    out_code[1] = tmp_code[0];
    out_code[4] = tmp_code[1];
    out_code[5] = tmp_code[2];
    out_code[2] = tmp_code[3];
    out_code[0] = tmp_code[4];
    out_code[3] = tmp_code[5];
    out_code[6] = 0;
    return 0;
}

void Do_XOR(unsigned char *dest, unsigned char *source, int size)
{
    int i;  
    for (i = 0; i < size; i++)
        dest[i] ^= source[i];
}

void Do_OR(unsigned char *dest, unsigned char *source, int size)
{
    int i;  
    for (i = 0; i < size; i++)
        dest[i] ^= source[i];
}

void DSP_2_HEX( unsigned char *dsp,  unsigned char *hex, int count )
{
    int i;
    for(i = 0; i < count; i++)
    {
        hex[i]=((dsp[i*2]<=0x39)?dsp[i*2]-0x30:dsp[i*2]-0x41+10);
        hex[i]=hex[i]<<4;
        hex[i]+=((dsp[i*2+1]<=0x39)?dsp[i*2+1]-0x30:dsp[i*2+1]-0x41+10);
    }
} 

void HEX_2_DSP( unsigned char *hex,  unsigned char *dsp, int count)
{
    int i;
    char ch;
    for(i = 0; i < count; i++)
    {
        ch=(hex[i]&0xf0)>>4;
        dsp[i*2]=(ch>9)?ch+0x41-10:ch+0x30;
        ch=hex[i]&0xf;
        dsp[i*2+1]=(ch>9)?ch+0x41-10:ch+0x30;
    }
}

void opGetMd5(char *data, char *result)
{
    MD5_CTX ctx;
    unsigned char hex[16];

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, strlen(data));
    MD5_Final(hex, &ctx);
    HEX_2_DSP(hex, (unsigned char *)result, 16);
}

void opGenMd5(unsigned char *data, int len, unsigned char *result)
{
    char tmp[17];
    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    memset(tmp, 0 ,sizeof(tmp));
    MD5_Final( tmp, &ctx);
    memcpy(result, tmp, 8);
}

int opBase64Encode(char *out, char *in, int size)
{
    int loop, total;
    char *translate;

    total = size / 3 * 4 + ((size % 3)? 4: 0);
    if (out == NULL || in == NULL)
        return total;

    translate = out;
    for (loop = 0; loop + 3 <= size; loop += 3, in += 3, out += 4) {
        out[0] = (in[0] & 0xFC) >> 2;
        out[1] = ((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4);
        out[2] = ((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6);
        out[3] = in[2] & 0x3F;
    }

    switch (size %= 3)
    {
        case 0:
            break;
        case 1:
            out[0] = (in[0] & 0xFC) >> 2;
            out[1] = (in[0] & 0x03) << 4;
            out[2] = 65;
            out[3] = 65;
            break;
        case 2:
            out[0] = (in[0] & 0xFC) >> 2;
            out[1] = ((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4);
            out[2] = (in[1] & 0x0F) << 2;
            out[3] = 65;
            break;
        default:
            break;
    }

    for (loop = 0, out = translate; loop < total; loop++, ++out) {
        if (*out < 26)
            *out += 'A';
        else if (*out < 52)
            *out += 'a' - 26;
        else if (*out < 62)
            *out += '0' - 52;
        else if (*out == 62)
            *out = '+';
        else if (*out == 63)
            *out = '/';
        else
            *out = '=';
    }

    return total;
}

int opBase64Decode(char *out, char *in, int size)
{
    char code, code1, code2, code3;
    int i, count, loop, bytes;

    for (loop = 0, count = 0, bytes = 0; loop < size; loop++) {
        code = (unsigned char)*in++;
        if ('A' <= code && code <= 'Z')
            code -= 'A';
        else if ('a' <= code && code <= 'z') {
            code -= 'a';
            code += 26;
        }
        else if ('0' <= code && code <= '9') {
            code -= '0';
            code += 52;
        }
        else if (code == '+')
            code = 62;
        else if (code == '/')
            code = 63;
        else if (code == '=' && size - loop <= 2)
            break;
        else
            continue;

        switch (i = count % 4)
        {
            case 0:
                if (count > 0) {
                    *out++ = code1;
                    *out++ = code2;
                    *out++ = code3;
                    bytes += 3;
                }
                code1 = code << 2;
                break;
            case 1:
                code1 |= (code & 0x30) >> 4;
                code2 = (code & 0x0F) << 4;
                break;
            case 2:
                code2 |= (code & 0x3C) >> 2;
                code3 = (code & 0x03) << 6;
                break;
            case 3:
            default:
                code3 |= code;
                break;
        }

        ++count;
    }

    if (count != 0) {
        *out++ = code1;
        ++bytes;
        size -= loop;
        if (size <= 1) {
            *out++ = code2;
            ++bytes;
        }
        if (size == 0) {
            *out = code3;
            ++bytes;
        }
    }

    return bytes;
}

int Data_Encrypt_Soft10(char *workdate, char *refid, unsigned char *data,int len, unsigned char *enc_data, int *enc_len)
{
    unsigned char pTempBuff[2*MAX_BUFF];
    char pBuff[100];
    char MD5_data[8];
    char MD5_result[8];
    const int MOD = 8;
    int count;
    int ret;
    int i;
    int iKeySize = 41;    
    unsigned char *ucKey = "iGDkRTLl5QYkJ99adoD2cA=V;1260502979323129";
    RC4_KEY k;    
    char *buff=NULL;
    *enc_len = 0;
    opGenMd5(data, len, MD5_data);
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(16, strlen(refid)), refid, _MIN(16, strlen(refid)));
    opGenMd5(pBuff, 24, MD5_result);
    sprintf(pTempBuff, "%04d", len);
    memcpy(pTempBuff + 4, MD5_data, 8);
    memcpy(pTempBuff + 4 + 8, data, len);
    len += 12;
    count = ((len % MOD == 0) ? (len / MOD) : (len / MOD + 1));
    if ((len = count * 8) >= MAX_BUFF)
        return -2; //ERR_DATALEN;
    for (i = 0; i < count; i++)
        Do_XOR(pTempBuff + 8 * i, MD5_result, 8);
    memset( &k, 0, sizeof(k) );    
    RC4_set_key( &k, iKeySize, ucKey );    
    buff=(unsigned char*)malloc( len );    
    if(buff == NULL)
        return -1;
    RC4( &k, len, pTempBuff, buff );    
    memcpy(pTempBuff, buff, len);
    free(buff);    
    // ret = DoJY(sock, 0xB2, pTempBuff, &len);
    *enc_len = opBase64Encode(enc_data, pTempBuff, len);
    return 0;
}

int Data_Decrypt_Soft10(char *workdate, char *refid, unsigned char *data,int len, unsigned char *dec_data, int *dec_len)
{
    unsigned char pTempBuff[2*MAX_BUFF];
    char pBuff[100];
    char MD5_data[8];
    char MD5_result[8];
    char c_len[5];
    const int MOD = 8;
    int nLen;
    int ret;
    int i;
    int iKeySize = 41;    
    unsigned char *ucKey = "iGDkRTLl5QYkJ99adoD2cA=V;1260502979323129";
    RC4_KEY k;    
    char *buff=NULL;
    *dec_len = 0;
    nLen = opBase64Decode(pTempBuff, data, len);
    if (nLen % MOD != 0)
        return -2; //ERR_DATALEN;
    memcpy(pBuff, workdate, 8);
    memset(pBuff + 8, '0', 16);
    memcpy(pBuff + 8 + 16 - _MIN(strlen(refid), 16), refid, _MIN(strlen(refid), 16));
    opGenMd5(pBuff, 24, MD5_result);
    memset( &k, 0, sizeof(k) );    
    RC4_set_key( &k, iKeySize, ucKey );    
    buff=(unsigned char*)malloc( nLen );    
    if(buff == NULL)
        return -1;
    RC4( &k, nLen, pTempBuff, buff );    
    memcpy(pTempBuff, buff, nLen);
    free(buff);    
    for (i = 0; i < (nLen / MOD); i++)
        Do_XOR(pTempBuff + 8 * i, MD5_result, MOD);
    memset(c_len, 0, sizeof(c_len));
    memcpy(c_len, pTempBuff, 4);
    *dec_len = atoi(c_len);
    opGenMd5(pTempBuff + 4 + 8, *dec_len, MD5_data);
    if (memcmp(pTempBuff + 4, MD5_data, 8) != 0)
    {
        *dec_len = 0;
        return -3; //ERR_CARDDATA_CRC;
    }
    memcpy(dec_data, pTempBuff + 4 + 8, *dec_len);
    return 0;
}

int GetPanByAcctno(char *acctno, char *pan) 
{
    char *p = NULL;
    int acctlen = 0;
    char tmp[64] = {0};

    acctlen = strlen(acctno);
    p = acctno + acctlen + 1;

    memset(pan, '0', 16);
    if (acctlen > 12) 
        memcpy(pan+4, p-12-1, 12);
    else if (acctlen == 12) 
        memcpy(pan+5, tmp, 11);
    else
        memcpy(pan+4, tmp, acctlen);
    pan[16] = 0;

    BKINFO("GetPanByAcctno:[%s]", pan);

    return 0;
}

int GetPanByTrack2(char *track2, char *pan)
{
    char *p = NULL;
    char tmp[512] = {0};

    strcpy(tmp, track2);
    if ((p = strchr(tmp, '=')) == NULL) {
        BKINFO("»°ø®∫≈ ß∞‹,Track2=[%s]", tmp);
        return -1; 
    }   
    *p = 0;

    return GetPanByAcctno(tmp, pan);
}
