#include "InlineQuery.h"

#include "Log.h"
#include "Global.h"
#include "UsersData.h"
#include "StringCheck.h"

using namespace std;
using namespace TgBot;

void pushStickerToResultByUsername(const Api &api,
                                   vector<InlineQueryResult::Ptr> &results,
                                   const string &__username)
{
    string username = __username;
    fixUsername(username);
    lowercase(username);

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
    lowercase(keywords);

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
