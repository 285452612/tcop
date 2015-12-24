/*******************************************
  EncryptData.c
  数据加密.重要应用于口令的加密
  实现算法:Triple DES(ECB mode)
  Created by SUNLAN
  2005/05/22
 *******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/des.h>
#include <openssl/md5.h>
#include "SDKpub.h"
#include "tcop.h"

static const char STR1[]="Sunyard";
static const char STR2[]="Digital";

#define KEY_LEN 8

/******************
  生成密钥
 ******************/
void MakeKey( const char *source, int sourcelen, DES_key_schedule *dk )
{
    char tmp[32], tmpkey[KEY_LEN];
    int i;

    MD5( source, sourcelen, tmp );

    for( i=0; i<KEY_LEN; i++ )
        tmpkey[i] = tmp[i] ^ tmp[8+i];

    DES_set_key( (const_DES_cblock*)tmpkey, dk );

    return;
}

/********************************
  对数据进行压缩
  返回压缩后的数据长度
 ********************************/
int EncryptData( char *STR3, const unsigned char *source, int len, 
        unsigned char *dest, int encflag )
{
    int destlen=0, newlen;
    unsigned char tmp[8];
    unsigned char *p, *q;
    DES_key_schedule dk1, dk2, dk3;

    if( encflag==DES_DECRYPT && (len % sizeof(DES_cblock)) != 0 )
    {
        return( -1 );
    }

    p = (unsigned char *)malloc(len*2);
    q = (unsigned char *)malloc(len*2);
    if (p == NULL || q == NULL)
        return -1;

    memset(p, 0, len*2);
    memset(q, 0, len*2);
    if (encflag==DES_DECRYPT)
    {
        DSP_2_HEX((unsigned char *)source, p, len);
        newlen = len/2;
    }
    else
    {
        memcpy(p, source, len);
        newlen = len;
    }

    MakeKey( STR1, strlen(STR1), &dk1 );
    MakeKey( STR2, strlen(STR2), &dk2 );
    MakeKey( STR3, strlen(STR3), &dk3 );

    while( ( newlen - destlen ) > 0 )
    {
        memset( tmp, 0, sizeof(tmp) );
        memcpy( tmp, p+destlen, (newlen-destlen)<8 ? newlen-destlen : 8 );
#ifdef SCO
        DES_ecb3_encrypt( tmp, q+destlen, &dk1, &dk2, &dk3, encflag );
#else
        DES_ecb3_encrypt( (const_DES_cblock *)tmp, (DES_cblock *)q+destlen, &dk1, &dk2, &dk3, encflag );
#endif
        destlen += 8;
    }

    if (encflag==DES_ENCRYPT)
    {
        HEX_2_DSP(q, dest, destlen);
        destlen *= 2;
        dest[destlen] = 0;
    }
    else
    {
        memcpy(dest, q, destlen);
        dest[destlen] = 0;
    }

    return( destlen );
}
