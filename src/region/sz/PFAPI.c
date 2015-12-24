#include "PFAPI.h"
#include "interface.h"

//#define  ORG_BANK   "8973"   
//#define  ORG_CENTER "0133"
char ORG_BANK[5];
char ORG_CENTER[5];
//��������ת����
int ConvertPersonKey( int flag, char *oacctno, char *dacctno, char *opwd, char *orgid, char *dpwd )
{
    int ret =0;

    sprintf( ORG_CENTER, "%s", GetORGCenter() );  
    sprintf( ORG_BANK, "%s", GetORGBank() );  
    BKINFO("***ORG_BANK[%s]ORG_CNETER[%s]***", ORG_BANK, ORG_CENTER);

    if ( flag == 1 ) //���
    {
        BKINFO("���....");
        ret = UnionTransPIN( 1, orgid, oacctno, opwd, 1, ORG_CENTER, dacctno, dpwd );
    }
    else if( flag == 2 ) //����
    {
        BKINFO("����...");
        ret = UnionTransPIN( 1, ORG_CENTER, oacctno, opwd, 1, ORG_BANK, dacctno, dpwd );
    }

    BKINFO("***Ret[%d] BankPwd=[%s]***", ret, dpwd);
    //�ر�����
    UnionReleaseHsmConn();
    return ret;
}
//������Կд��
int WriteKey(  char *key, char *mac )
{
    int ret =0;
    char skey[97], smac[33];
    char rkey[97], rmac[33];

    sprintf( ORG_CENTER, "%s", GetORGCenter() );  
    sprintf( ORG_BANK, "%s", GetORGBank() );  

    BKINFO("***KEY[%s]MAC[%s]***", key, mac);
    BKINFO("***ORG_BANK[%s]ORG_CNETER[%s]***", ORG_BANK, ORG_CENTER);

    memset( smac, 0, sizeof(smac) );
    memset( skey, 0, sizeof(skey) );
    ret = UnionGetWorkKey(1, ORG_CENTER, skey, smac); 
    BKINFO("***Ret[%d]SKEY[%s]***", ret, skey );

    memset( rkey, 0, sizeof(rkey) );
    memset( rmac, 0, sizeof(rmac) );
    sprintf( rmac,"%-32s","");
    //sprintf( rkey,"%-32s%32s", key, skey+32 );
    sprintf( rkey,"%-48s%048d", key, 0 );
    rkey[96]=0x00;

    BKINFO("***RKEY[%s]RMAC[%s]***", rkey, rmac);
    ret = UnionImport3DESWorkKey(1, ORG_CENTER, rkey, rmac); 
    BKINFO("***Ret[%d]***", ret );

    return ret;
}

