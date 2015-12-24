#include "interface.h"

static char *GetAcctNo(char *trackInfo2)
{
    char *p = NULL;

    BKINFO("acctno=[%s]", trackInfo2);
    if (strlen(trackInfo2) < 8)
        return NULL;
    if ((p = strchr(trackInfo2, '=')) == NULL) {
        return trackInfo2;
    }
    *p = 0;
    return trackInfo2;
}

int SYDEncrypt(char *trackInfo2, char *pwd, char *encryptedPin)
{
    char *acctno = NULL;
    char bankno[13];
    int rc;

    if ((acctno = GetAcctNo(trackInfo2)) == NULL)
        return E_PACK_GETVAL;

    sprintf(bankno, "20%s", GetCBankno());
    if ((rc = PINBlock_Encrypt(bankno, acctno, pwd, encryptedPin)) != 0)
    {
        BKINFO("PINBlock_Encrypt() fail, ret = [%d].", rc);
        return E_SYS_SJLENCRYPT;
    }

    return 0;
}

int SYDDecrypt(char *cryptedPinBlock, char *trackInfo2, char *pin)
{
    char *acctno = NULL;
    char bankno[13];
    int rc;

    if ((acctno = GetAcctNo(trackInfo2)) == NULL)
        return E_PACK_GETVAL;

    sprintf(bankno, "20%s", GetCBankno());
    if ((rc = PINBlock_Decrypt(bankno, acctno, cryptedPinBlock, pin)) != 0)
    {
        BKINFO("PINBlock_Decrypt() fail, ret = [%d].", rc);
        return E_SYS_SJLDECRYPT;
    }

    return 0;
}
