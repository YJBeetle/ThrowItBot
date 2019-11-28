#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include <ArtRobot/ArtRobot.h>

#include "Global.h"
#include "Log.h"
#include "UsersData.h"
#include "Throw.h"
#include "InlineQuery.h"
#include "Tg.h"

using namespace std;
using namespace cv;
using namespace TgBot;

std::string botUsername;
UsersData usersData;

int main()
{
    cout << "=================" << endl
         << "|  ThrowItBot!  |" << endl
         << "=================" << endl;

    // init
    usersData.readFromFile();

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) { // 处理收到的直接消息
        LogI("Message: %s: %s", message->chat->username.c_str(), message->text.c_str());

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                if (throwByUserId(bot.getApi(), message->chat->id, message->forwardFrom))
                    sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(bot.getApi(), message->chat->id, "The user's privacy settings do not allow forwarding, can't get the avatar of this user.");
            }
            return;
        }

        if (message->text.c_str()[0] == '@') // 首位是@的话Throw Username
            if (throwByUsername(bot.getApi(), message->chat->id, message->text, message->from->id))
                sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");

        if (
            StringTools::startsWith(message->text, "/start") ||
            StringTools::startsWith(message->text, "/help") ||
            StringTools::startsWith(message->text, "/throw") ||
            false)
        { // 如果是已知的指令则跳过
            return;
        }

        sendMessage(bot.getApi(), message->chat->id, "Do you need /help ?");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        // 您可以说/throw扔自己！
        // 并且您可以转发消息给我，或者@他/她，来让我扔他/她。
        sendMessage(bot.getApi(), message->chat->id, "You can say /throw to throw youself!\nAnd you can forward message to me, or @he/her, to let me throw him/her.");
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        sendMessage(bot.getApi(), message->chat->id, "Do you need to be /throw ?");
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        if (throwByUserId(bot.getApi(), message->chat->id, message->from))
            sendMessage(bot.getApi(), message->chat->id, "( ﹁ ﹁ )");
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        LogI("InlineQuery: %s: %s", inlineQuery->from->username.c_str(), inlineQuery->query.c_str());

        vector<InlineQueryResult::Ptr> results; // 准备results

        string query = inlineQuery->query;
        transform(query.begin(), query.end(), query.begin(), ::tolower); // 转小写

        if (query.c_str()[0] == '@') // 首位是@的话进行精确匹配
            pushStickerToResultByUsername(bot.getApi(), results, query);
        else
            pushStickerToResultByUsernameFuzzy(bot.getApi(), results, query);

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
        LogD("onChosenInlineResult");
    });

    while (true)
    {
        try
        {
            LogI("Starting ...");
            botUsername = bot.getApi().getMe()->username;
            LogI("Bot username: %s", botUsername.c_str());

            TgLongPoll longPoll(bot);
            while (true)
            {
                LogI("Long poll started.");
                longPoll.start();
            }

            // TgWebhookTcpServer webhookServer(8888, bot);
            // string webhookUrl(getenv("WEBHOOK_URL"));
            // bot.getApi().setWebhook(webhookUrl);
            // webhookServer.start();
        }
        catch (TgException &e)
        {
            LogE("Error: %s", e.what());
        }
        catch (...)
        {
            LogE("Unknow error.");
        }
        LogI("Restart.");
    }

    return 0;
}
