#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "utils.h"

int callConvertFunc(const char *sofile, const char *funcname, 
        const char *expr, void *src, void *dest)
{
    ConvertFunc cvt_func;
    char filename[256];
    void *module;
    int rc;

    snprintf(filename, sizeof(filename), "%s/%s", getAppDir(), sofile);
    module = dlopen (filename, RTLD_LOCAL | RTLD_LAZY);
    if (!module)
    {
        err_log("%s: %d, %s.", filename, errno, strerror(errno));
        return -1;
    }

    cvt_func = dlsym (module, funcname);
    if (cvt_func == NULL)
    {
        err_log("%s: symbol %s is NULL.", filename, funcname);
        if (dlclose (module))
            err_log("%s: %d, %s.", filename, errno, strerror(errno));
        return -2;
    }

    if ((rc = cvt_func((void *)expr, src, dest)) != 0)
    {
        err_log("%s: symbol %s ret=%d.", filename, funcname, rc);
        if (dlclose (module))
            err_log("%s: %d, %s.", filename, errno, strerror(errno));
        return -3;
    }

    if (dlclose(module) != 0)
        err_log("%s: %d, %s.", filename, errno, strerror(errno));

    return 0;
}
