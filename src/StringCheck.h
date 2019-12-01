#pragma once

inline void fixUsername(std::string &username)
{
    if (username.c_str()[0] == '@') // 首位是@的话去掉
        username = username.c_str() + 1;
}

inline void lowercase(std::string &str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower); // 用户名转小写
}

inline bool lowercaseEq(const std::string &__str1, const std::string &__str2)
{
    string str1 = __str1;
    string str2 = __str2;
    transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
    return str1 == str2;
}
