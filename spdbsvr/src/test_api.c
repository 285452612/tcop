#include "PFAPI.h"
#include "interface.h"
#include "utils.h"
int main()
{
    int ret;
    char dpwd[17]={0};
    char dpwd1[17]={0};
    char dpwd2[17]={0};
    char dpwd3[17]={0};
    char dpwd4[17]={0};
    char key[100]={0};
    char mac[100]={0};
    char key1[33]={0};
    char mac1[17]={0};

    /*
    ret = UnionImportZMK( "0133", "F6FDDD59D3C5356B6EED23477C631A23", "57AEF96A729A6979");
    sprintf(key1,"%32s", "F6FDDD59D3C5356B6EED23477C631A23");
    key1[32]=0x00;
    sprintf(mac1,"%16s", "57AEF96A729A6979");
    mac1[16]=0x00;
    err_log("***KEY[%s] ", ret );
    ret = UnionImportZMK( "0133", key1, mac1 );
    err_log("***Ret[%d] ", ret );
    ret = UnionGetZMK( "0133", key, mac );
    err_log("***Ret[%d] KEY=[%s] mac[%s]***", ret, key, mac);
    */
    ret = UnionEncryptPIN(1, "9501", "111111", "0185960018597", dpwd);
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd);

    ret = UnionTransPIN( 1, "9501", "0185960018597", dpwd, 1, "0133", "9843010400004892", dpwd1 );
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd1);

    /*
    ret = UnionEncryptPIN(1, "0133", "111111", "9843010400004892", dpwd4);
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd4);
    */

    /*
    ret = UnionTransPIN( 1, "0133", "9843010400004892", dpwd1, 1, "8973", "9843010400004892", dpwd2 );
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd2);

    ret = UnionEncryptPIN(1, "8973", "111111", "9843010400004892", dpwd3);
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd3);
    */

    /*
    ret = UnionTransPIN( 1, "0133", "9843010400004892", dpwd1, 1, "8973", "9843010400004892", dpwd2 );
    err_log("***Ret[%d] BankPwd=[%s]***", ret, dpwd2);
    */

    return 0;
}
