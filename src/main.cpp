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
#include "StringCheck.h"

using namespace std;
using namespace cv;
using namespace TgBot;

std::string botUsername;
std::string botUsernameLowercase;
int32_t botId = 0;
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
        LogI("Message: \n\tchat: <%s>(%lld)\n\tfrom: <%s>(%d)\n%s", message->chat->username.c_str(), message->chat->id, message->from->username.c_str(), message->from->id, message->text.c_str());

        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->forwardDate && message->chat->type == Chat::Type::Private) // 是转发的消息 并且是私聊
        {
            if (message->forwardFrom)
            {
                throwByUserId(api, chatId, message->forwardFrom, message->from->id) &&
                    sendMessage(api, chatId, "<(ˉ^ˉ)>");
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(api, chatId, "该用户的隐私设置不允许转发，所以无法获取他ID。\n请尝试直接在这里@他");
            }
            return;
        }

        if (message->text.c_str()[0] == '@' && message->chat->type == Chat::Type::Private) // 首位是@ 并且是私聊 的话Throw Username
        {
            auto &username = message->text;
            throwByUsername(api, chatId, username, message->from->id) &&
                sendMessage(api, chatId, "<(ˉ^ˉ)>");
            return;
        }

        if (message->text.c_str()[0] == '/') // 如果是指令则跳过
            return;

        if (message->chat->type == Chat::Type::Private) // 只有私聊显示help
            sendMessage(api, chatId, "你在说什么？\n如果想要什么帮助的话请给我发 /help");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) { // /help
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            sendMessage(api, chatId, "你可以说 /throw 来扔你自己并更新你的头像表情包！\n如果想扔别人并更新其头像表情包，你可以转发他的消息给我，或者在这里@他。");
        }
        else
        {
            sendMessage(api, chatId, "你可以说 /throw 来扔你自己！\n如果想扔别人，你可以转发他的消息私聊给我，或者在这里发送 /throw 他的用户名。\n如果你希望更新某个用户的头像表情包，请私聊我并在私聊中@他。");
        }
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) { // /start
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "快给我发 /throw 试试 ლ(╹◡╹ლ)");
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) { // /throw
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        string command = message->text;
        lowercase(command);

        if (command == "/throw" ||                       // "/throw"
            command == "/throw@" + botUsernameLowercase) // "/throw@ThrowItBot"
        {                                                // 正常抛
            if (message->chat->type == Chat::Type::Private)
            { // 私聊
                throwByUserId(api, chatId, message->from, message->from->id) &&
                    sendMessage(api, chatId, "你的头像表情包更新成功( ﹁ ﹁ )");
            }
            else
            {                                                                                          // 如果是群聊
                auto stickerFileId = searchFileIdByUsername(bot.getApi(), getUsername(message->from)); // 先搜索是否存在
                stickerFileId.empty()
                    ? throwByUserId(api, chatId, message->from, message->from->id) // 不存在则创建
                    : sendSticker(api, chatId, stickerFileId);                     // 存在则直接丢表情
            }
        }
        else if (StringTools::startsWith(command, "/throw ") || // "/throw <Username>"
                 StringTools::startsWith(command, "/throw@"))   // "/throw@<Username>", "/throw@ThrowItBot <Username>"
        {                                                       // 抛Username
            string username;
            if (StringTools::startsWith(command, "/throw@" + botUsernameLowercase + " ") || // "/throw@ThrowItBot <Username>"
                StringTools::startsWith(command, "/throw@" + botUsernameLowercase + "@"))   // "/throw@ThrowItBot@<Username>"
                username = message->text.c_str() + 7 /* strlen("/throw@") */ + botUsernameLowercase.length() + 1 /* strlen( ) || strlen(@) */;
            else if (StringTools::startsWith(command, "/throw @" + botUsernameLowercase + " ") || // "/throw @ThrowItBot <Username>"
                     StringTools::startsWith(command, "/throw @" + botUsernameLowercase + "@"))   // "/throw @ThrowItBot@<Username>"
                username = message->text.c_str() + 8 /* strlen("/throw @") */ + botUsernameLowercase.length() + 1 /* strlen( ) || strlen(@) */;
            else
                username = message->text.c_str() + 7 /* strlen("/throw ") || strlen("/throw@") */;

            if (message->chat->type == Chat::Type::Private)
            { // 私聊
                throwByUsername(api, chatId, username, message->from->id) &&
                    sendMessage(api, chatId, username + "的头像表情包更新成功<(ˉ^ˉ)>");
            }
            else
            {                                                                        // 如果是群聊
                auto stickerFileId = searchFileIdByUsername(bot.getApi(), username); // 先搜索是否存在
                stickerFileId.empty()
                    ? throwByUsername(api, chatId, username, message->from->id) // 不存在则创建
                    : sendSticker(api, chatId, stickerFileId);                  // 存在则直接丢表情
            }
        }
        else
        { // 语法错误
            sendMessage(api, chatId, "命令格式错误，正确的格式为：/throw UserName");
        }
    });

    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) { // 未知指令
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            sendMessage(api, chatId, "你在说什么？\n如果想要什么帮助的话请给我发 /help");
        }
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        LogI("InlineQuery: \n\tfrom: <%s>(%d)\n%s", inlineQuery->from->username.c_str(), inlineQuery->from->id, inlineQuery->query.c_str());

        auto &query = inlineQuery->query;

        vector<InlineQueryResult::Ptr> results; // 准备results

        pushStickerOnInlineQuery(bot.getApi(), results, query);

        // debug json
        // TgTypeParser tgTypeParser;
        // cout << tgTypeParser.parseArray<InlineQueryResult>(&TgTypeParser::parseInlineQueryResult, results) << endl;

        try
        {
            bot.getApi().answerInlineQuery(inlineQuery->id, results);
        }
        catch (TgException &e)
        {
            LogE("answerInlineQuery: %s", e.what());
        }
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr callbackQuery) {
        LogI("onCallbackQuery: \n\tfrom: <%s>(%d)\n%s", callbackQuery->from->username.c_str(), callbackQuery->from->id, callbackQuery->data.c_str());

        auto &username = callbackQuery->data;

        if (throwByUsername(bot.getApi(), callbackQuery->from->id, username, callbackQuery->from->id))
        {
            sendMessage(bot.getApi(), callbackQuery->from->id, "<(ˉ^ˉ)>");
            try
            {
                bot.getApi().answerCallbackQuery(callbackQuery->id, "Success, now you can find it in inline query.");
                // bot.getApi().answerCallbackQuery(callbackQuery->id, "喵喵喵", false, "t.me/addstickers/wmliyin_by_ThrowYouBot");
                // bot.getApi().answerCallbackQuery(callbackQuery->id, "喵喵喵", false, "t.me/ThrowYouBot?start=XXXX");
            }
            catch (TgException &e)
            {
                LogE("TgBot::Api::answerCallbackQuery: %s", e.what());
            }
        }
    });

    while (true)
    {
        try
        {
            LogI("Starting ...");
            botUsernameLowercase = botUsername = bot.getApi().getMe()->username;
            lowercase(botUsernameLowercase);
            botId = bot.getApi().getMe()->id;
            LogI("Bot username: %s %d", botUsername.c_str(), botId);

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
