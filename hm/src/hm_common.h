#ifndef HM_COMMON_H
#define HM_COMMON_H

#include <string>
#include <Windows.h>
#include <tchar.h>

typedef unsigned long long t_long_64;
typedef int t_error;
typedef size_t t_size;

#define t_string std::wstring
#define t_char wchar_t

#define HASH_SHA1_LEN 20
#define HASH_SHA1_STRING_LEN (HASH_SHA1_LEN*2)

typedef struct object_id
{
    unsigned char id[HASH_SHA1_LEN];
}object_id;

#endif
