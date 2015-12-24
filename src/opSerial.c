#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pub.h"

#ifdef S_IRUSR
#define MKDIR_UMASK (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#else
#define MKDIR_UMASK 0755
#endif

static int itsdir(const char *name)
{
    char       myname[256];
    int        accres;

    snprintf(myname, sizeof(myname), "%s/.", name);
    accres = access(myname, F_OK);
    return accres == 0;
}

static int mkdirs(char *argname)
{
    char       *name;
    char       *cp;

    if (argname == NULL || *argname == '\0')
        return 0;

    cp = name = strdup(argname);
    if (cp == NULL)
        return -1;
    while ((cp = strchr(cp + 1, '/')) != 0)
    {
        *cp = '\0';
        if (!itsdir(name))
        {
            if (mkdir(name, MKDIR_UMASK) != 0)
            {
                if (errno != EEXIST || !itsdir(name))
                {
                    free(name);
                    return -1;
                }
            }
        }
        *cp = '/';
    }
    free(name);
    return 0;
}

//最大支持2147483647
long GenSerial(char *serialName, long min, long max, int increment)
{
    char   fullname[256];
    long   serial = 0L;
    int    iFileId = 0;
    struct flock lck;
    char tmp[16] = {0};

    sprintf(tmp, "%ld", max);
    snprintf(fullname, sizeof(fullname)-1, "%s/bin/.%s_%dserial", OP_HOME, serialName, strlen(tmp));
    iFileId = open(fullname, O_RDWR|O_CREAT|O_SYNC, S_IWUSR|S_IRUSR);
    if (iFileId < 0)
    {
        if (mkdirs(fullname) != 0)
        {
            DBUG("mkdirs(%s) != 0", fullname);
            return -1;
        }
        iFileId = open( fullname , O_RDWR|O_CREAT|O_SYNC , S_IWUSR|S_IRUSR );
        if (iFileId < 0)
        {
            DBUG("mkdirs(%s) != 0", fullname);
            return -1;
        }
    }

    lck.l_type   = F_WRLCK;
    lck.l_start  = 0;
    lck.l_len    = 0;
    lck.l_whence = 0;

    while(fcntl(iFileId, F_SETLK , &lck) == -1);
    lseek(iFileId, 0l, SEEK_SET);
    if (read(iFileId, &serial, sizeof(long)) <= 0)
        serial = min;
    else
        serial += increment;

    if (serial > max)
        serial = min;

    lseek(iFileId, 0l, SEEK_SET);
    write(iFileId, &serial, sizeof(long));
    close(iFileId);

    DBUG("max=[%ld] min=[%ld] cur=[%ld]", max, min, serial);
    return serial;
}
