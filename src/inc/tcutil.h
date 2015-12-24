#ifndef TCUTIL_H_
#define TCUTIL_H

#include "crypto.h"

char *GenOperPwd(char *orgid, char *operid, char *name, char *passwd, char *regdate, char *cliper);
int ChkOperPwd(char *orgid, char *operid, char *name, char *passwd, char *regdate, char *real_passwd);

#endif
