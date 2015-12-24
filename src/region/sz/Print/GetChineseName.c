#include <stdio.h>
#include "SDKpub.h"
#include "pubdef.h"
#include "chinese.h"

char *ChsName(ST_S_CHINESE *pstChinese, char *type)
{
    int i = 0;

    for ( i = 0; ; i++ )
    {
        if ((pstChinese+i)->type == NULL)
            break;

        if (!strcmp((pstChinese+i)->type, type))
            return (pstChinese+i)->name;
    }

    return data_empty;
}

char *GetChineseName(ST_CHINESE *pstChinese, int type)
{
    int i = 0;

    for ( i = 0; ; i++ )
    {
        if ((pstChinese+i)->type == -1)
            break;

        if( (pstChinese+i)->type == type )
            return (pstChinese+i)->name;
    }

    return data_empty;
}
