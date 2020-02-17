#pragma once

#include <string>




/** Converts wide string to utf8 string. */
std::string toUtf8(const std::wstring & aSrcWide);

/** Converts utf8 string to wide string. */
std::wstring toWide(const std::string & aSrcUtf8);
