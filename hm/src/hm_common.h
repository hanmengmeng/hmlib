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

typedef unsigned long long hm_long_64;

#endif
