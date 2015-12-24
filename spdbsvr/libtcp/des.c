/***********************************************
 *  <DES_1.C>  DESº”√‹Ω‚√‹≥Ã–Ú                 *
 ***********************************************/
#include "des.h"
#define DESKEY "FDC32F7756DB1E76"

static unsigned char C[17][28],D[17][28],K[17][48],c,ch1;
static void expand0();
static void setkeystar();
static void encrypt0();
static void discrypt0();
static void compress0();
static void compress016();
static void LS();
static void son();
static void ip();
static void _ip();
static void F();
static void s_box();

void DES(source, dest, key, flag)
unsigned char *source, *dest, *key;
unsigned char flag;
{
	char tmp[64];
	expand0(key, tmp);
	setkeystar(tmp);
	if( flag == ENCRYPT || flag == 'e' || flag == 'E' )
		encrypt0(source, dest);
	else
		discrypt0(source, dest);
}

void Do_XOR(dest, source, size)
unsigned char *dest, *source;
int size;
{
	int i;
	for(i=0; i<size; i++)
		dest[i] ^= source[i];
}

void MAC(packet, packet_size, mac_value, key, mode)
unsigned char *packet, *key, *mac_value;
int  packet_size;
unsigned char mode;
{
	int size=0;

	memset(mac_value, 0, 8);

	while( packet_size > size)
	{
		if( (packet_size - size) <= 8 )
		{
			Do_XOR(mac_value, &packet[size], packet_size - size);
			DES(mac_value, mac_value, key, ENCRYPT);
			return;
		}
		Do_XOR(mac_value, &packet[size], 8);
		if( mode == STAND )
			DES(mac_value, mac_value, key, ENCRYPT);
		size += 8;
	}
}

void DAC(char *packet, int packlen, unsigned char *dac)
{
	unsigned char dac_buf[17];
	memset(dac_buf, 0x00, sizeof(dac_buf));

	MAC(packet, packlen, dac_buf, DESKEY, 0);
	HEX_2_DSP(dac_buf, dac, 8);
}

void  HostDes(card_no, work_key, pin, encrypt_pin, flag)
unsigned char *card_no, *work_key, *pin, *encrypt_pin, flag;
{
	unsigned char  card_buf[16], pin_buf[17], enpin_buf[8];
	unsigned char  key_buf[8];
	int   i, ii;

	DSP_2_HEX(work_key, key_buf, 8);

	memset(card_buf, 'F', sizeof(card_buf));
	memcpy(card_buf, card_no+1, 15);
	DSP_2_HEX(card_buf, card_buf, 8);
	card_buf[0] = 0;

	if(flag == 'e' || flag == 'E' || flag == ENCRYPT)
	{
		enpin_buf[0] = strlen(pin);
		memcpy(pin_buf, pin, strlen(pin));
		ii = strlen(pin);
		for(i = ii; i<17; i++)
		{
			pin_buf[i] = 'F';
		}
		DSP_2_HEX(pin_buf, pin_buf, 8);

		Do_XOR(card_buf, pin_buf, 7);

		memcpy(enpin_buf+1, card_buf, 7);

		DES(enpin_buf, encrypt_pin, key_buf, ENCRYPT);

		return;
	}

	if(flag == 'd' || flag == 'D' || flag == DECRYPT)
	{
		DES(encrypt_pin, pin_buf, key_buf, DECRYPT);

		Do_XOR(pin_buf+1, card_buf, 7);

		HEX_2_DSP(pin_buf+1, pin, 7);

		pin[pin_buf[0]&0x0f] = 0;

		return;
	}
}

void DSP_2_HEX(dsp, hex, count)
char *dsp, *hex;
int  count;
{
	int i;
	for(i = 0; i < count; i++)
	{
	hex[i]=((dsp[i*2]<=0x39)?dsp[i*2]-0x30:dsp[i*2]-0x41+10);
		hex[i]=hex[i]<<4;
	hex[i]+=((dsp[i*2+1]<=0x39)?dsp[i*2+1]-0x30:dsp[i*2+1]-0x41+10);
	}
}     /* Defined already in iso.h */

void HEX_2_DSP(hex, dsp, count)
char *hex, *dsp;
int  count;
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
}      /* Defined already in iso.h */



static void encrypt0(text,mtext)
unsigned char *text,*mtext;
{
	char ll[64],rr[64],LL[64],RR[64];
	char tmp[64];
	int i,j;
	ip(text,ll,rr);

	for (i=1;i<17;i++)
	{
		F(i,ll,rr,LL,RR);
		for (j=0;j<32;j++)
		{
			ll[j]=LL[j];
			rr[j]=RR[j];
		}
	}

	_ip(tmp,rr,ll);

	// modified by baiyso at 2003/03/04
	compress0(tmp,mtext);
}

static void discrypt0(mtext,text)
unsigned char *text,*mtext;
{
	char ll[64],rr[64],LL[64],RR[64];
	char tmp[64];
	int i,j;
	ip(mtext,ll,rr);

	for (i=16;i>0;i--)
	{
		F(i,ll,rr,LL,RR);
		for (j=0;j<32;j++)
		{
			ll[j]=LL[j];
			rr[j]=RR[j];
		}
	}

	_ip(tmp,rr,ll);

	compress0(tmp,text);
}

static void expand0(in,out)
unsigned char *in;
char *out;
{
	int divide;
	int i,j;

	for (i=0;i<8;i++)
	{
		divide=0x80;
		for (j=0;j<8;j++)
		{
			*out++=(in[i]/divide)&1;
			divide/=2;
		}
	}
}


static void compress0(out,in)
unsigned char *in;
char *out;
{
	int times;
	int i,j;

	for (i=0;i<8;i++)
	{
		times=0x80;
		in[i]=0;
		for (j=0;j<8;j++)
		{
			in[i]+=(*out++)*times;
			times/=2;
		}
	}
}

static void compress016(out,in)
unsigned char *in;
char *out;
{
	int times;
	int i,j;

	for (i=0;i<16;i++)
	{
		times=0x8;
		in[i]='0';
		for (j=0;j<4;j++)
		{
			in[i]+=(*out++)*times;
			times/=2;
		}
	}
}


static int pc_1_c[28]={
	57,49,41,33,25,17,9
	,1,58,50,42,34,26,18
	,10,2,59,51,43,35,27
	,19,11,3,60,52,44,36};
static int pc_1_d[28]={
	63,55,47,39,31,23,15
	,7,62,54,46,38,30,22
	,14,6,61,53,45,37,29
	,21,13,5,28,20,12,4};
static int pc_2[48]={
	14,17,11,24,1,5,
	3,28,15,6,21,10,
	23,19,12,4,26,8,
	16,7,27,20,13,2,
	41,52,31,37,47,55,
	30,40,51,45,33,48,
	44,49,39,56,34,53,
	46,42,50,36,29,32};
static int ls_count[16]={
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

static void setkeystar(bits)
unsigned char bits[64];
{
	int i,j;

	for (i=0;i<28;i++)
		C[0][i]=bits[pc_1_c[i]-1];
	for (i=0;i<28;i++)
		D[0][i]=bits[pc_1_d[i]-1];
	for (j=0;j<16;j++)
	{
		LS(C[j],C[j+1],ls_count[j]);
		LS(D[j],D[j+1],ls_count[j]);
		son(C[j+1],D[j+1],K[j+1]);
	}
}


static void LS(bits,buffer,count)
char *bits;
int count;
char *buffer;
{
	int i,j;
	for (i=0;i<28;i++)
	{
		buffer[i]=bits[(i+count)%28];
	}
}

static void son(cc,dd,kk)
char *cc,*dd,*kk;
{
	int i,j;
	char buffer[56];
	for (i=0;i<28;i++)
		buffer[i] = *cc++;

	for (i=28;i<56;i++)
		buffer[i] = *dd++;

	for (i=0;i<48;i++)
		*kk++=buffer[pc_2[i]-1];
}

static int ip_tab[64]={
	58,50,42,34,26,18,10,2,
	60,52,44,36,28,20,12,4,
	62,54,46,38,30,22,14,6,
	64,56,48,40,32,24,16,8,
	57,49,41,33,25,17,9,1,
	59,51,43,35,27,19,11,3,
	61,53,45,37,29,21,13,5,
	63,55,47,39,31,23,15,7};
static int _ip_tab[64]={
	40,8,48,16,56,24,64,32,
	39,7,47,15,55,23,63,31,
	38,6,46,14,54,22,62,30,
	37,5,45,13,53,21,61,29,
	36,4,44,12,52,20,60,28,
	35,3,43,11,51,19,59,27,
	34,2,42,10,50,18,58,26,
	33,1,41,9,49,17,57,25};

static void ip(text,ll,rr)
unsigned char *text;
char *ll,*rr;
{
	int i,j;
	char buffer[64];
	char tmp[64];
	expand0(text,buffer);

	for (i=0;i<32;i++)
		ll[i]=buffer[ip_tab[i]-1];

	for (i=0;i<32;i++)
		rr[i]=buffer[ip_tab[i+32]-1];
}

static void _ip(text,ll,rr)
unsigned char *text;
char *ll,*rr;
{
	int i,j;
	char tmp[64];
	for (i=0;i<32;i++)
		tmp[i]=ll[i];
	for (i=32;i<64;i++)
		tmp[i]=rr[i-32];
	for (i=0;i<64;i++)
		text[i]=tmp[_ip_tab[i]-1];
}


static int e_r[48]={
	32,1,2,3,4,5,4,5,6,7,8,9,
	8,9,10,11,12,13,12,13,14,15,16,17,
	16,17,18,19,20,21,20,21,22,23,24,25,
	24,25,26,27,28,29,28,29,30,31,32,1};

static int P[32]={
	16,7,20,21,29,12,28,17,
	1,15,23,26,5,18,31,10,
	2,8,24,14,32,27,3,9,
	19,13,30,6,22,11,4,25};
static int SSS[16][4][16]={
	14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
	0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,/* err on */
	4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
	15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13,

	15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
	3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
	0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
	13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9,

	10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
	13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
	13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
	1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12,

	7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
	13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
	10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
	3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14, /* err on */

	2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
	14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6, /* err on */
	4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
	11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3,

	12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
	10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
	9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
	4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13,

	4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
	13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
	1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
	6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12,

	13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
	1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
	7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
	2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11};

static void F(n,ll,rr,LL,RR)
char *ll,*rr,*LL,*RR;
int n;
{
	int i,j;
	char buffer[64],tmp[64];
	for (i=0;i<48;i++)
		buffer[i]=rr[e_r[i]-1];
	for (i=0;i<48;i++)
		buffer[i]=(buffer[i]+K[n][i])&1;

	s_box(buffer,tmp);

	for (i=0;i<32;i++)
		buffer[i]=tmp[P[i]-1];

	for (i=0;i<32;i++)
		RR[i]=(buffer[i]+ll[i])&1;

	for (i=0;i<32;i++)
		LL[i]=rr[i];
}

static void s_box(aa,bb)
char *aa,*bb;
{
	int i,j,k,m;
	int y,z;
	char ss[8];
	m=0;
	for (i=0;i<8;i++)
	{
		j=6*i;
		y=aa[j]*2+aa[j+5];
		z=aa[j+1]*8+aa[j+2]*4+aa[j+3]*2+aa[j+4];
		ss[i]=SSS[i][y][z];
		y=0x08;
		for (k=0;k<4;k++)
		{
			bb[m++]=(ss[i]/y)&1;
			y/=2;
		}

	}
}
