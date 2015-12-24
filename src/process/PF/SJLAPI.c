#include "interface.h"

static int GetPan(char *trackInfo2, char *pan)
{
    char *p = NULL;
    char tmp[512] = {0};
    int acctlen = 0;

    strcpy(tmp, trackInfo2);
    if ((p = strchr(tmp, '=')) == NULL) {
        BKINFO("取卡号失败:[%s]", tmp);
        return E_PACK_GETVAL;
    }
    *p = 0;
    acctlen = strlen(tmp);

    memset(pan, '0', 16);
    if (acctlen > 12)
        memcpy(pan+4, p-12-1, 12);
    else if (acctlen == 12)
        memcpy(pan+5, tmp, 11);
    else
        memcpy(pan+4, tmp, acctlen);
    pan[16] = 0;

    return 0;
}

int SJL05Encrypt(char *trackInfo2, char *pwd, char *encryptedPin)
{
    char tmp[20] = {0};
    char reqbuf[64] = {0}, rspbuf[64] = {0};
    char mmkindex[6] = {0}, pikinfo[33] = {0};
    char *p = NULL;
    char cmd[64] = {0}, pan[20] = {0}, pin[20] = {0}, pinBlock[20] = {0};
    unsigned char panHex[10] = {0}, pinHex[10] = {0};
    int rsplen = 0, ret = 0;

    DBUG("Ready Encrypt: TrackInfo2=[%s], pwd=[%s]", trackInfo2, pwd);
    if ((ret = GetPan(trackInfo2, pan)) != 0)
        return ret;
    memset(pin, 'F', sizeof pin);
    sprintf(tmp, "%02d%s", strlen(pwd), pwd);
    memcpy(pin, tmp, strlen(tmp));

    DSP_2_HEX(pan, panHex, 8);
    DSP_2_HEX(pin, pinHex, 8);
    Do_XOR(pinHex, panHex, 8);

    if ((p = GetPIKInfo()) == NULL) return E_DB_SELECT;
    strcpy(pikinfo, p);
    if ((p = GetMMKIndex()) == NULL) return E_DB_SELECT;
    strcpy(mmkindex, p);

    sprintf(cmd, "0405%s0110%s", mmkindex, pikinfo);
    DSP_2_HEX(cmd, reqbuf, strlen(cmd)/2);
    memcpy(reqbuf+strlen(cmd)/2, pinHex, 8);
    DBUGHexBuf("加密发送串", reqbuf, 30);

    if (ret = CommSocket("TCOP_UNIONMYJADDR", reqbuf, 30, rspbuf, &rsplen))
        return ret;
    if (rsplen > 1) {
        if (rspbuf[0] == 'A' && rsplen == 9) {
            HEX_2_DSP(rspbuf+1, encryptedPin, 8);
            encryptedPin[16] = 0;
            return 0;
        } else 
            BKINFO("银联加密应答错误:x%02x|x%02x|%d", rspbuf[0], rspbuf[1], rsplen);
    } else
        BKINFO("银联加密应答报文长度错误:%d", rsplen);
    return E_SYS_SJLENCRYPT;
}

int SJL05Decrypt(char *cryptedPinBlock, char *trackInfo2, char *pin)
{
    char tmp[20] = {0};
    char rspbuf[64] = {0};
    char pinBlockHex[10] = {0};
    char pan[20] = {0}, panHex[10] = {0};
    char pinTmp[20] = {0};
    char pik[20] = {0}, mmkindex[10];
    char fullcmd[64] = {0};
    char *p = NULL;
    int rsplen = 0, ret = 0;

    if ((ret = GetPan(trackInfo2, pan)) != 0)
        return ret;
    DSP_2_HEX(pan, panHex, 8);
    DSP_2_HEX(cryptedPinBlock, pinBlockHex, 8);

    if ((p = GetPIKInfo()) == NULL) return E_DB_SELECT;
    DSP_2_HEX(p, pik, 16);
    if ((p = GetMMKIndex()) == NULL) return E_DB_SELECT;
    DSP_2_HEX(p, mmkindex, 2);

    memcpy(fullcmd, "\x04\x22", 2);
    memcpy(fullcmd + 2, mmkindex, 2);
    memcpy(fullcmd + 4, pinBlockHex, 8);
    memcpy(fullcmd + 12, panHex, 8);
    memcpy(fullcmd + 20, "\x10", 1);
    memcpy(fullcmd + 21, pik, 16);
    DBUGHexBuf("解密发送串", fullcmd, 37);

    if (ret = CommSocket("TCOP_UNIONMYJADDR", fullcmd, 37, rspbuf, &rsplen))
        return ret;
    if (rsplen > 1) {
        if (rspbuf[0] == 'A' && rsplen == 9) {
            HEX_2_DSP(rspbuf+1, pinTmp, 8);
            memcpy(tmp, pinTmp, 2);
            sprintf(pin, "%*.*s", atoi(tmp), atoi(tmp), pinTmp+2);
            return 0;
        } else 
            BKINFO("银联加密应答错误:x%02x|x%02x|%d", rspbuf[0], rspbuf[1], rsplen);
    } else
        BKINFO("银联加密应答报文长度错误:%d", rsplen);

    return E_SYS_SJLDECRYPT;
}

//银行密码加密
int BankPwdEncrypt(char *pwd, char *encrypted)
{
    int rsplen = 0;
    int ret = 0;

    ret = CommSocket("TCOP_BANKJMADDR", pwd, strlen(pwd), encrypted, &rsplen);
    if (ret == 0 && rsplen > 0)
        encrypted[rsplen] = 0;
    else
        ret = -1;

    return ret;
}
