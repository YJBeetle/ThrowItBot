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
