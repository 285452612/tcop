/*************************************************
** PROGRAM: DES.H                               **
** AUTHOR:  kenny                               **
** WRITE DATE:       06/24/1998                 **
** LAST MODIFY DATE: 06/24/1998                 **
** COMMENT: DES���ܽ���ͷ�ļ�                   **
**                                              **
*************************************************/
#ifndef   _DES_H
#define   _DES_H

void DES(/* source, dest, key, flag */);
void Do_XOR(/* dest, source, size */);
void MAC(/* packet, packet_size, mac_value, key, mode */);
void HostDes(/* card_no, work_key, pin, encrypt_pin, flag */);
void DSP_2_HEX(/* dsp, hex, count */);
void HEX_2_DSP(/* hex, dsp, count */);


#ifndef   ENCRYPT
#define   ENCRYPT  0         /* DES ���� */
#define   DECRYPT  1
#endif

#ifndef   STAND
#define   STAND    0         /* MAC ��׼ */
#define   BPI      1
#endif

#endif
