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

bool pushStickerToResultByUsername(const Api &api, vector<InlineQueryResult::Ptr> &results, const string &username)
{
    auto fileId = searchFileIdByUsername(api, username);
    if (!fileId.empty())
    {
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = username;
        result->stickerFileId = fileId;
        results.push_back(result);
        return true;
    }
    return false;
}
