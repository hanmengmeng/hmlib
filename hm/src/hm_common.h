#ifndef HM_COMMON_H
#define HM_COMMON_H

#include <string>
#include <Windows.h>
#include <tchar.h>

#ifndef hm_string
#ifdef _UNICODE
#define hm_string std::wstring
#else
#define hm_string std::string
#endif
#endif

#define hm_wstring std::wstring
#define hm_astring std::string

typedef unsigned long long hm_long_64;

#define HASH_SHA1_LEN 20

typedef struct object_id
{
    unsigned char id[HASH_SHA1_LEN];
}object_id;

#endif
