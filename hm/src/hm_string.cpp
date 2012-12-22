#include "hm_string.h"
#include <vector>
#include <Windows.h>

namespace hm
{
StringConvert::StringConvert(const char *str)
{
    if (NULL == str)
    {
        str = "";
    }
    if (IsUtf8(str, strlen(str)))
    {
        mIsUtf8 = true;
    }
    else
    {
        mIsUtf8 = false;
    }
    mStrA = str;
    mStrW = L"";
}

StringConvert::StringConvert( const t_char *str )
{
    if (NULL == str)
    {
        str = L"";
    }
    mIsUtf8 = false;
    mStrA = "";
    mStrW = str;
}

StringConvert::StringConvert( const std::string &str )
{
    if (IsUtf8(str.c_str(), str.length()))
    {
        mIsUtf8 = true;
    }
    else
    {
        mIsUtf8 = false;
    }
    mStrA = str;
    mStrW = L"";
}

StringConvert::StringConvert( const std::wstring &str )
{
    mIsUtf8 = false;
    mStrA = "";
    mStrW = str;
}

StringConvert::~StringConvert()
{
}

std::string StringConvert::ToUtf8()
{
    if (!mStrA.empty())
    {
        if (mIsUtf8)
        {
            return mStrA;
        }
        else
        {
            return MbToUtf8(mStrA.c_str());
        }
    }
    else if (!mStrW.empty())
    {
        return UnicodeToUtf8(mStrW.c_str());
    }
    else
    {
        return "";
    }
}

std::wstring StringConvert::ToUnicode()
{
    if (!mStrA.empty())
    {
        return AnsiToUnicode(mStrA.c_str());
    }
    else if (!mStrW.empty())
    {
        return mStrW.c_str();
    }
    else
    {
        return L"";
    }
}

std::string StringConvert::ToAnsi()
{
    if (!mStrA.empty())
    {
        if (mIsUtf8)
        {
            return Utf8ToMb(mStrA.c_str());
        }
        else
        {
            return mStrA;
        }
    }
    else if (!mStrW.empty())
    {
        return UnicodeToAnsi(mStrW.c_str());
    }
    else
    {
        return "";
    }
}

bool StringConvert::IsUtf8( const char* str, int length )
{
    int i = 0;
    int nBytes = 0;
    unsigned char chr = 0;
    bool bAllAscii = true;

    while (i < length)
    {
        chr = *(str + i);
        if ((chr & 0x80) != 0)
            bAllAscii = false;
        if (nBytes == 0)
        {
            if ((chr & 0x80) != 0)
            {
                while ((chr & 0x80) != 0)
                {
                    chr <<= 1;
                    nBytes++;
                }
                if (nBytes < 2 || nBytes > 6)
                    return false;
                nBytes--;
            }
        }
        else
        {
            if ((chr & 0xc0) != 0x80)
                return false;
            nBytes--;
        }
        ++i;
    }
    if (bAllAscii)
    {
        return false;
    }

    return nBytes == 0;
}

std::wstring StringConvert::AnsiToUnicode(const char* buf)
{
    int len = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
    if (len == 0) return L"";

    std::vector<t_char> unicode(len);
    ::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], len);

    return &unicode[0];
}

std::string StringConvert::UnicodeToAnsi(const t_char* buf)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

    std::vector<char> utf8(len);
    ::WideCharToMultiByte(CP_ACP, 0, buf, -1, &utf8[0], len, NULL, NULL);

    return &utf8[0];
}

std::wstring StringConvert::Utf8ToUnicode(const char* buf)
{
    int len = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
    if (len == 0) return L"";

    std::vector<t_char> unicode(len);
    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &unicode[0], len);

    return &unicode[0];
}

std::string StringConvert::UnicodeToUtf8(const t_char* buf)
{
    int len = ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

    std::vector<char> utf8(len);
    ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], len, NULL, NULL);

    return &utf8[0];
}

std::string StringConvert::MbToUtf8( const char *buf )
{
    std::wstring wstr = AnsiToUnicode(buf);
    return UnicodeToUtf8(wstr.c_str());
}

std::string StringConvert::Utf8ToMb( const char *buf )
{
    std::wstring wstr = Utf8ToUnicode(buf);
    return UnicodeToAnsi(wstr.c_str());
}

}
