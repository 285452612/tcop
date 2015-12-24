/*  dll test
 *
 *  gcc dltest.c -o dltest -ldl
 */
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>

typedef void *(* SayHelloFunc) (const char *message, ...);

int just_say_hello (const char *filename)
{
    SayHelloFunc  say_hello;
    void      *module;
    char *p;

    module = dlopen (filename, RTLD_LAZY);
    if (!module)
    {
        fprintf(stderr, "%s\n", dlerror ());
        return -1;
    }

    say_hello = dlsym (module, "say_hello");
    if (say_hello == NULL)
    {
        fprintf(stderr, "symbol say_hello is NULL\n");
        if (!dlclose (module))
            printf ("%s: %s\n", filename, dlerror ());
        return -1;
    }

    p = (char *)say_hello ("Hello world!", "OK");
    printf("END PRINT=(%s).\n", p);
    free(p);

    if (dlclose(module) != 0)
        printf ("%s: %s\n", filename, dlerror ());

    return 0;
}

int just_say_int (const char *filename, char *funcname)
{
    SayHelloFunc  say_hello;
    void      *module;
    int *p;

    module = dlopen (filename, RTLD_LAZY);
    if (!module)
    {
        fprintf(stderr, "%s\n", dlerror ());
        return -1;
    }

    say_hello = dlsym (module, funcname);
    if (say_hello == NULL)
    {
        fprintf(stderr, "symbol %s is NULL\n", funcname);
        if (!dlclose (module))
            printf ("%s: %s\n", filename, dlerror ());
        return -1;
    }

    p = (int *)say_hello ("Hello world2!");
    printf("END PRINT=(%d).\n", *p);
    free(p);

    if (dlclose(module) != 0)
        printf ("%s: %s\n", filename, dlerror ());

    return 0;
}

int main(int argc, char *argv[])
{
    just_say_hello (argv[1]);

    just_say_int (argv[1], "ret_int");

    return 0;
}
