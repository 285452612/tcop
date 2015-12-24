#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "prefs.h"

/*
 * Returns a type given its string representation.
 */
static int type_from_str(char *name)
{
    int type;

    if (strstr(name, "int") == name) type = PREF_INT;
    else if (strstr(name, "uint") == name) type = PREF_UINT;
    else if (strstr(name, "real") == name) type = PREF_REAL;
    else if (strstr(name, "string") == name) type = PREF_STRING;
    else if (strstr(name, "boolean") == name) type = PREF_BOOLEAN;
    else return PREF_INVALID;

    return type;
}

/*
 * Returns TRUE if the given type is valid.
 */
static unsigned char type_is_valid(int type)
{
    switch (PREF_TYPE_MASK & type) {
        case PREF_INT:
        case PREF_UINT:
        case PREF_REAL:
        case PREF_STRING:
        case PREF_BOOLEAN:
            break;
        default:
            return 0;
    }
    return 1;
}


ProfilePtr OpenProfile(const char *file)
{
    static ProfilePtr paramDoc;
    if ((paramDoc = xmlParseFile(file)) == NULL)
    {
        err_log("FAIL: Initialization profile failed, %s.", file);
        return NULL;
    }
    return paramDoc;
}

void *Profile(ProfilePtr paramDoc, const char *index, const char *key, int type)
{
    char xpath[256];
    char str[512];
    char *ret;

    if (paramDoc == NULL)
    {
        err_log("FAIL: Uninitialized parameter files.");
        return;
    }
    snprintf(xpath, sizeof(xpath), "/profile/%s/attr[@name='%s']", index, key);
    XmlGetString(paramDoc, xpath, str, sizeof(str));
    if (str[0] == 0x00)
    {
        err_log("未取到配置文件参数%s!", xpath);
        return NULL;
    }

    switch (PREF_TYPE_MASK & type)
    {
        case PREF_INT:
            ret = malloc(sizeof(long));
            *(long *)ret = strtol(str, NULL, 10);
            break;
        case PREF_UINT:
            ret = malloc(sizeof(long));
            *(unsigned long *)ret = strtoul(str, NULL, 10);
            break;
        case PREF_REAL:
            ret = malloc(sizeof(double));
            *(double *)ret = strtod(str, NULL);
            break;
        case PREF_STRING:
            ret = strdup(str);
            break;
        case PREF_BOOLEAN:
            ret = malloc(sizeof(int));
            if (strcasecmp(str,"false")==0)
                *(int *)ret = 0;
            else if (strcasecmp(str,"true")==0)
                *(int *)ret = 1;
            else
                *(int *)ret = (atoi(str) ? 1 : 0);
            break;
        default:
            ret = NULL;
            break;
    }

    return ret;
}

void CloseProfile(ProfilePtr doc)
{
    xmlFreeDoc(doc);
    return;
}
