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

            if (i++ >= 19) // 只显示前20个结果
                break;
        }
    }
}

void pushClickToThrow(const Api &api,
                      vector<InlineQueryResult::Ptr> &results,
                      const string &__username)
{
    string username = __username;
    fixUsername(username);

    auto result = make_shared<InlineQueryResultArticle>();

    if (lowercaseEq(username, botUsername))
    {
        result->title = "(┙>∧<)┙彡 ┻━┻"; // 不允许丢自己
        result->id = "nouser";

        auto text = make_shared<InputTextMessageContent>();
        text->messageText = "(┙>∧<)┙彡 ┻━┻";
        result->inputMessageContent = text;
    }
    else
    {
        result->title = "No user found, touch me to throw it.";
        result->id = "nouser";

        auto text = make_shared<InputTextMessageContent>();
        text->messageText = "Click to throw @" + username;
        result->inputMessageContent = text;

        InlineKeyboardButton::Ptr button = make_shared<InlineKeyboardButton>();
        button->text = "Throw it!";
        button->callbackData = username;
        auto replyMarkup = make_shared<InlineKeyboardMarkup>();
        replyMarkup->inlineKeyboard.resize(1);
        replyMarkup->inlineKeyboard[0].resize(1);
        replyMarkup->inlineKeyboard[0][0] = button;
        result->replyMarkup = replyMarkup;
    }

    results.push_back(result);
}

void pushStickerOnInlineQuery(const Api &api,
                              vector<InlineQueryResult::Ptr> &results,
                              const string &query)
{
    if (query.c_str()[0] == '@') // 首位是@的话进行精确匹配
        pushStickerToResultByUsername(api, results, query.c_str() + 1);
    else
        pushStickerToResultByUsernameFuzzy(api, results, query);

    if (results.size() == 0) // 如果列表依然是空的，则显示按钮用于创建
        pushClickToThrow(api, results, query);
}
