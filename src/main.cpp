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
        LogI("Message: chat->username=%s, chat->id=%d, text=%s", message->chat->username.c_str(), message->chat->id, message->text.c_str());

        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                throwByUserId(api, chatId, message->forwardFrom, message->from->id) &&
                    sendMessage(api, chatId, "<(ˉ^ˉ)>");
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(api, chatId, "The user's privacy settings do not allow forwarding, can't get the avatar of this user.");
            }
            return;
        }

        if (message->text.c_str()[0] == '@') // 首位是@的话Throw Username
        {
            auto &username = message->text;
            throwByUsername(api, chatId, username, message->from->id) &&
                sendMessage(api, chatId, "<(ˉ^ˉ)>");
            return;
        }

        if (message->text.c_str()[0] == '/') // 如果是指令则跳过
            return;

        if (message->chat->type == Chat::Type::Private) // 只有私聊显示help
            sendMessage(api, chatId, "Do you need /help ?");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) { // /help
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        // 您可以说/throw扔自己！
        // 并且您可以转发消息给我，或者@他/她，来让我扔他/她。
        sendMessage(api, chatId, "You can say /throw to throw youself!\nAnd you can forward message to me, or @he/her, to let me throw him/her.");
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) { // /start
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "Do you need to be /throw ?");
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
                    sendMessage(api, chatId, "( ﹁ ﹁ )");
            }
            else
            {
                auto stickerFileId = searchFileIdByUsername(bot.getApi(), getUsername(message->from));
                stickerFileId.empty()
                    ? throwByUserId(api, chatId, message->from, message->from->id) &&
                          sendMessage(api, chatId, "( ﹁ ﹁ )")
                    : sendSticker(api, chatId, stickerFileId) &&
                          sendMessage(api, chatId, "( ﹁ ﹁ )");
            }
        }
        else if (StringTools::startsWith(command, "/throw ") || // "/throw <Username>"
                 StringTools::startsWith(command, "/throw@"))   // "/throw@<Username>", "/throw@ThrowItBot <Username>"
        {                                                       // 抛Username
            string username;
            if (StringTools::startsWith(command, "/throw@" + botUsernameLowercase + " ") || // "/throw@ThrowItBot <Username>"
                StringTools::startsWith(command, "/throw@" + botUsernameLowercase + "@"))   // "/throw@ThrowItBot@<Username>"
                username = message->text.c_str() + 7 /* strlen("/throw@") */ + botUsernameLowercase.length() + 1 /* strlen( ) */;
            else
                username = message->text.c_str() + 7 /* strlen("/throw ") */;

            if (message->chat->type == Chat::Type::Private)
            { // 私聊
                throwByUsername(api, chatId, username, message->from->id) &&
                    sendMessage(api, chatId, "<(ˉ^ˉ)>");
            }
            else
            {
                auto stickerFileId = searchFileIdByUsername(bot.getApi(), username);
                stickerFileId.empty()
                    ? throwByUsername(api, chatId, username, message->from->id) &&
                          sendMessage(api, chatId, "<(ˉ^ˉ)>")
                    : sendSticker(api, chatId, stickerFileId) &&
                          sendMessage(api, chatId, "<(ˉ^ˉ)>");
            }
        }
        else
        { // 语法错误
            sendMessage(api, chatId, "Command error, you should /throw UserName");
        }
    });

    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) { // 未知指令
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "Unknow command.\nDo you need /help ?");
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        auto &query = inlineQuery->query;

        LogI("InlineQuery: %s: %s", inlineQuery->from->username.c_str(), query.c_str());

        vector<InlineQueryResult::Ptr> results; // 准备results

        if (query.c_str()[0] == '@') // 首位是@的话进行精确匹配
            pushStickerToResultByUsername(bot.getApi(), results, query.c_str() + 1);
        else
            pushStickerToResultByUsernameFuzzy(bot.getApi(), results, query);

        if (results.size() == 0)
        {
            string username = query;
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
                button->text = "Throw @" + username;
                button->callbackData = username;
                auto replyMarkup = make_shared<InlineKeyboardMarkup>();
                replyMarkup->inlineKeyboard.resize(1);
                replyMarkup->inlineKeyboard[0].resize(1);
                replyMarkup->inlineKeyboard[0][0] = button;
                result->replyMarkup = replyMarkup;
            }

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
            LogE("answerInlineQuery: %s", e.what());
        }
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr callbackQuery) {
        auto &username = callbackQuery->data;

        LogI("onCallbackQuery: %s: %s", callbackQuery->from->username.c_str(), username.c_str());

        throwByUsername(bot.getApi(), callbackQuery->from->id, username, callbackQuery->from->id) &&
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
