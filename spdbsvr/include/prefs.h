#ifndef __PREFS_H__
#define __PREFS_H__
#include "libxml/xpath.h"
#include "libxml/xmlschemas.h"
#include "libxml/xmlversion.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xmlmemory.h"

#define PREF_INVALID    0x0000
#define PREF_INT    0x0001
#define PREF_UINT   0x0002
#define PREF_REAL   0x0003
#define PREF_STRING 0x0004
#define PREF_BOOLEAN    0x0005
#define PREF_TYPE_MASK  0x00FF

typedef xmlDocPtr ProfilePtr;
void *Profile(ProfilePtr paramDoc, const char *index, const char *key, int type);
ProfilePtr OpenProfile(const char *file);
void CloseProfile(ProfilePtr doc);

#endif
