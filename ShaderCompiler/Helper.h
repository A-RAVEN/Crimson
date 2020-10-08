#pragma once
#include <string>
class Helper
{
public:
    static std::wstring StringToWideString(std::string str);
    static std::string WidestringToString(std::wstring wstr);
};

