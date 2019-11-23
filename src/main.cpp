#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include <ArtRobot/ArtRobot.h>

#include "UsersData.h"
#include "ThrowIt.h"
#include "Global.h"

using namespace std;
using namespace cv;
using namespace TgBot;

const char UserImgSearchStr[] = "<img class=\"tgme_page_photo_image\" src=\"";
const int UserImgSearchStrLen = sizeof(UserImgSearchStr) - 1;

std::string botUsername;
UsersData usersData;

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

int main()
{
    cout << "ThrowItBot start!" << endl;

    // init
    usersData.readFromFile();

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        cout << "Message: " << message->chat->username << ": " << message->text << endl;

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                try
                {
                    throwUser(bot.getApi(), message->chat->id, message->forwardFrom);
                    bot.getApi().sendMessage(message->chat->id, "<(ˉ^ˉ)>", false, 0, std::make_shared<GenericReply>(), "", true);
                }
                catch (TgException &e)
                {
                    cerr << "Throw error: " << e.what() << endl;
                }
            }
            else
            {
                try
                {
                    bot.getApi().sendMessage(message->chat->id, "The user's privacy settings do not allow forwarding, can't get the avatar of this user.", false, 0, std::make_shared<GenericReply>(), "", true);
                }
                catch (TgException &e)
                {
                    cerr << "sendMessage error: " << e.what() << endl;
                }
            }
            return;
        }

        if (message->text.c_str()[0] == '@') // 首位是@的话去网页拉取头像
        {
            cout << "ThrowAt: " << message->text << endl;

            string username = message->text.c_str() + 1;

            CurlHttpClient curl;
            string url = "https://t.me/" + username;
            vector<HttpReqArg> args;
            string html = curl.makeRequest(url, args);

            auto startpos = html.find(UserImgSearchStr);
            if (startpos != string::npos)
            {
                startpos += UserImgSearchStrLen;
                auto endpos = html.find_first_of("\"", startpos);
                string imgurl = html.substr(startpos, endpos - startpos);

                bot.getApi().sendChatAction(message->chat->id, "upload_photo"); // 设置正在发送

                string img = curl.makeRequest(imgurl, args);

                try
                {
                    throwImage(bot.getApi(), message->chat->id, username, "Throw @" + username, img);
                    bot.getApi().sendMessage(message->chat->id, "<(ˉ^ˉ)>", false, 0, std::make_shared<GenericReply>(), "", true);
                }
                catch (TgException &e)
                {
                    cerr << "throwItImage error: " << e.what() << endl;
                }
            }
            else
            {
                cerr << "No User." << endl;
                try
                {
                    bot.getApi().sendMessage(message->chat->id, "No User.", false, 0, std::make_shared<GenericReply>(), "", true);
                }
                catch (TgException &e)
                {
                    cerr << "sendMessage error: " << e.what() << endl;
                }
            }
            return;
        }

        if (
            StringTools::startsWith(message->text, "/start") ||
            StringTools::startsWith(message->text, "/help") ||
            StringTools::startsWith(message->text, "/throw") ||
            false)
        { // 如果是已知的指令则跳过
            return;
        }

        try
        {
            bot.getApi().sendMessage(message->chat->id, "Do you need /help ?", false, 0, std::make_shared<GenericReply>(), "", true);
        }
        catch (TgException &e)
        {
            cerr << "sendMessage error: " << e.what() << endl;
        }
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        try
        {
            // 您可以说/throw扔自己！
            // 并且您可以转发消息给我，或者@他/她，来让我扔他/她。
            bot.getApi().sendMessage(message->chat->id, "You can say /throw to throw youself!\nAnd you can forward message to me, or @he/her, to let me throw him/her.", false, 0, std::make_shared<GenericReply>(), "", true);
        }
        catch (TgException &e)
        {
            cerr << "sendMessage error: " << e.what() << endl;
        }
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        try
        {
            bot.getApi().sendMessage(message->chat->id, "Do you need to be /throw ?", false, 0, std::make_shared<GenericReply>(), "", true);
        }
        catch (TgException &e)
        {
            cerr << "sendMessage error: " << e.what() << endl;
        }
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        try
        {
            throwUser(bot.getApi(), message->chat->id, message->from);
            bot.getApi().sendMessage(message->chat->id, "( ﹁ ﹁ ) ", false, 0, std::make_shared<GenericReply>(), "", true);
        }
        catch (TgException &e)
        {
            cerr << "Throw error: " << e.what() << endl;
        }
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        cout << "InlineQuery: " << inlineQuery->from->username << ": " << inlineQuery->query << endl;

        vector<InlineQueryResult::Ptr> results; // 准备results

        string query = inlineQuery->query;
        transform(query.begin(), query.end(), query.begin(), ::tolower); // 转小写

        if (query.c_str()[0] == '@') // 首位是@的话进行精确匹配
        {
            pushStickerToResultByUsername(bot.getApi(), results, query.c_str() + 1);
        }
        else
        {
            // 快速搜索
            int i = 0;
            for (auto user : usersData.data)
            {
                if (user.first.find(query) != string::npos)
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

        if (results.size() == 0)
        {
            auto text = make_shared<InputTextMessageContent>();
            text->messageText = "@" + botUsername;
            auto result = make_shared<InlineQueryResultArticle>();
            result->title = "No user found";
            result->id = "nouser";
            result->inputMessageContent = text;
            results.push_back(result);
        }

        // debug json
        // TgTypeParser tgTypeParser;
        // cout << tgTypeParser.parseArray<InlineQueryResult>(&TgTypeParser::parseInlineQueryResult, results) << endl;

        try
        {
            bot.getApi().answerInlineQuery(inlineQuery->id, results);
        }
        catch (TgException &e)
        {
            cerr << "InlineQuery error: " << e.what() << endl;
        }
    });

    bot.getEvents().onChosenInlineResult([&bot](ChosenInlineResult::Ptr chosenInlineResult) {
        cout << "onChosenInlineResult" << endl;
    });

    while (true)
    {
        try
        {
            cout << "Starting ..." << endl;
            botUsername = bot.getApi().getMe()->username;
            cout << "Bot username: " << botUsername << endl;

            TgLongPoll longPoll(bot);
            while (true)
            {
                cout << "Long poll started." << endl;
                longPoll.start();
            }

            // TgWebhookTcpServer webhookServer(8888, bot);
            // string webhookUrl(getenv("WEBHOOK_URL"));
            // bot.getApi().setWebhook(webhookUrl);
            // webhookServer.start();
        }
        catch (TgException &e)
        {
            cerr << "error: " << e.what() << endl;
        }
        catch (...)
        {
            cerr << "error" << endl;
        }
    }

    return 0;
}
