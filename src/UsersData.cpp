#include "UsersData.h"

#include "Log.h"
#include "Global.h"
#include "StringCheck.h"

using namespace std;
using namespace TgBot;

UsersData::UsersData()
{
}

UsersData::~UsersData()
{
}

void UsersData::readFromFile()
{
    ifstream in(USERDATAFILEPATH);
    if (in)
    {
        string username;
        string fileId;
        while (getline(in, username) && getline(in, fileId))
            data[username] = fileId;
    }
}

void UsersData::saveToFile()
{
    ofstream out(USERDATAFILEPATH);
    for (auto &user : data)
    {
        out << user.first << endl;
        out << user.second << endl;
    }
}

string searchFileIdByUsername(const Api &api, const string &__username)
{
    string username = __username;
    fixUsername(username);
    lowercase(username);

    auto s = usersData.data.find(username);
    if (s != usersData.data.end())
    {
        return s->second;
    }
    else
    {
        string stickerName = username + "_by_" + botUsername; // 贴纸名字
        try
        {
            auto stickerSet = api.getStickerSet(stickerName);
            if (stickerSet->stickers.size())
            {
                auto fileId = stickerSet->stickers[0]->fileId;
                usersData.set(username, fileId);
                return fileId;
            }
            else
                return "";
        }
        catch (const std::exception &e)
        {
            return "";
        }
    }
}
