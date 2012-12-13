#ifndef HM_STRING_CONVERT
#define HM_STRING_CONVERT

#include <string>

namespace hm
{

class StringConvert
{
public:
    StringConvert(const char *str);
    StringConvert(const wchar_t *str);
    StringConvert(const std::string &str);
    StringConvert(const std::wstring &str);
    ~StringConvert();

    std::string ToUtf8();
    std::wstring ToUnicode();
    std::string ToAnsi();

private:
    bool IsUtf8(const char* str, int length);
    std::wstring AnsiToUnicode(const char* buf);
    std::string UnicodeToAnsi(const wchar_t* buf);
    std::wstring Utf8ToUnicode(const char* buf);
    std::string UnicodeToUtf8(const wchar_t* buf);
    std::string MbToUtf8(const char *buf);
    std::string Utf8ToMb(const char *buf);

private:
    std::string mStrA;
    std::wstring mStrW;
    bool mIsUtf8;
};
}
#endif
