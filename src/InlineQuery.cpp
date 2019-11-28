#include "InlineQuery.h"

#include "Log.h"
#include "Global.h"

using namespace std;
using namespace TgBot;

string searchFileIdByUsername(const Api &api, const string &username)
{
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

void pushStickerToResultByUsername(const Api &api,
                                   vector<InlineQueryResult::Ptr> &results,
                                   const string &__username)
{
    string username;
    if (__username.c_str()[0] == '@') // 首位是@的话去掉
        username = __username.c_str() + 1;
    else
        username = __username;
    transform(username.begin(), username.end(), username.begin(), ::tolower); // 用户名转小写

    auto fileId = searchFileIdByUsername(api, username);
    if (!fileId.empty())
    {
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = username;
        result->stickerFileId = fileId;
        results.push_back(result);
    }
}

void pushStickerToResultByUsernameFuzzy(const Api &api,
                                        vector<InlineQueryResult::Ptr> &results,
                                        const string &__keywords)
{ // 快速搜索 不查询服务器
    string keywords = __keywords;
    transform(keywords.begin(), keywords.end(), keywords.begin(), ::tolower); // 转小写

    int i = 0;
    for (auto user : usersData.data)
    {
        if (user.first.find(keywords) != string::npos)
        {
            auto result = make_shared<InlineQueryResultCachedSticker>();
            result->id = user.first;
            result->stickerFileId = user.second;
            results.push_back(result);
        }
        if (i++ >= 19) // 只显示前20个结果
            break;
    }
}
