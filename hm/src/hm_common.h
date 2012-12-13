#ifndef HM_COMMON_H
#define HM_COMMON_H

#ifndef hm_string
#ifdef _UNICODE
#define hm_string std::wstring
#else
#define hm_string std::string
#endif
#endif

#endif
