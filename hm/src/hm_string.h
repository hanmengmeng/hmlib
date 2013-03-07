#ifndef HM_STRING_CONVERT
#define HM_STRING_CONVERT

#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <functional>

#include "hm_common.h"

namespace hm
{

class StringConvert
{
public:
    StringConvert(const char *str);
    StringConvert(const t_char *str);
    StringConvert(const std::string &str);
    StringConvert(const std::wstring &str);
    ~StringConvert();

    std::string ToUtf8();
    std::wstring ToUtf16();
    std::string ToAnsi();

private:
    bool IsUtf8(const char* str, int length);
    std::wstring AnsiToUnicode(const char* buf);
    std::string UnicodeToAnsi(const t_char* buf);
    std::wstring Utf8ToUnicode(const char* buf);
    std::string UnicodeToUtf8(const t_char* buf);
    std::string MbToUtf8(const char *buf);
    std::string Utf8ToMb(const char *buf);

private:
    std::string mStrA;
    std::wstring mStrW;
    bool mIsUtf8;
};

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

std::string &ToLower(std::string &s);
std::string &ToUpper(std::string &s);

}
#endif
