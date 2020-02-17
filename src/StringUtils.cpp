#include "StringUtils.h"
#include <Windows.h>





std::string toUtf8(const std::wstring & aSrcWide)
{
    auto  srcLength = static_cast<int>(aSrcWide.length()) + 1;
    auto len = WideCharToMultiByte(CP_UTF8, 0, aSrcWide.c_str(), srcLength, nullptr, 0, 0, nullptr); 
    std::string res(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, aSrcWide.c_str(), srcLength, &res[0], len, 0, nullptr);
    return res;
}





std::wstring toWide(const std::string & aSrcUtf8)
{
    auto  srcLength = static_cast<int>(aSrcUtf8.length()) + 1;
    auto len = MultiByteToWideChar(CP_UTF8, 0, aSrcUtf8.c_str(), srcLength, nullptr, 0); 
    std::wstring res(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, aSrcUtf8.c_str(), srcLength, &res[0], len);
    return res;
}
