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

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                throwByUserId(bot.getApi(), message->chat->id, message->forwardFrom, message->from->id) &&
                    sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(bot.getApi(), message->chat->id, "The user's privacy settings do not allow forwarding, can't get the avatar of this user.");
            }
            return;
        }

        if (message->text.c_str()[0] == '@') // 首位是@的话Throw Username
        {
            string username = message->text.c_str() + 1;
            if (lowercaseEq(username, botUsername))
                sendMessage(bot.getApi(), message->chat->id, "(┙>∧<)┙彡 ┻━┻"); // 不允许丢自己
            else
                throwByUsername(bot.getApi(), message->chat->id, username, message->from->id) &&
                    sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");
            return;
        }

        if (message->text.c_str()[0] == '/') // 如果是指令则跳过
            return;

        if (message->chat->type == Chat::Type::Private) // 只有私聊显示help
            sendMessage(bot.getApi(), message->chat->id, "Do you need /help ?");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) { // /help
        // 您可以说/throw扔自己！
        // 并且您可以转发消息给我，或者@他/她，来让我扔他/她。
        sendMessage(bot.getApi(), message->chat->id, "You can say /throw to throw youself!\nAnd you can forward message to me, or @he/her, to let me throw him/her.");
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) { // /start
        sendMessage(bot.getApi(), message->chat->id, "Do you need to be /throw ?");
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) { // /throw
        if (lowercaseEq(message->text, "/throw") ||
            lowercaseEq(message->text, "/throw@" + botUsername))
        { // 正常抛
            if (message->chat->type == Chat::Type::Private)
            { // 私聊
                throwByUserId(bot.getApi(), message->chat->id, message->from, message->from->id) &&
                    sendMessage(bot.getApi(), message->chat->id, "( ﹁ ﹁ )");
            }
            else
            {
                auto stickerFileId = searchFileIdByUsername(bot.getApi(), getUsername(message->from));
                stickerFileId.empty()
                    ? throwByUserId(bot.getApi(), message->chat->id, message->from, message->from->id) &&
                          sendMessage(bot.getApi(), message->chat->id, "( ﹁ ﹁ )")
                    : sendSticker(bot.getApi(), message->chat->id, stickerFileId) &&
                          sendMessage(bot.getApi(), message->chat->id, "( ﹁ ﹁ )");
            }
        }
        else if (StringTools::startsWith(message->text, "/throw ") ||
                 StringTools::startsWith(message->text, "/throw@"))
        { // 抛Username
            string username = message->text.c_str() + 7 /* sizeof("/throw ") - 1 */;
            fixUsername(username);
            if (lowercaseEq(username, botUsername))
                sendMessage(bot.getApi(), message->chat->id, "(┙>∧<)┙彡 ┻━┻"); // 不允许丢自己
            else
            {
                if (message->chat->type == Chat::Type::Private)
                { // 私聊
                    throwByUsername(bot.getApi(), message->chat->id, username, message->from->id) &&
                        sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");
                }
                else
                {
                    auto stickerFileId = searchFileIdByUsername(bot.getApi(), username);
                    stickerFileId.empty()
                        ? throwByUsername(bot.getApi(), message->chat->id, username, message->from->id) &&
                              sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>")
                        : sendSticker(bot.getApi(), message->chat->id, stickerFileId) &&
                              sendMessage(bot.getApi(), message->chat->id, "<(ˉ^ˉ)>");
                }
            }
        }
        else
        { // 语法错误
            sendMessage(bot.getApi(), message->chat->id, "Command error, you should /throw UserName");
        }
    });

    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) { // 未知指令
        sendMessage(bot.getApi(), message->chat->id, "Unknow command.\nDo you need /help ?");
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        LogI("InlineQuery: %s: %s", inlineQuery->from->username.c_str(), inlineQuery->query.c_str());

        vector<InlineQueryResult::Ptr> results; // 准备results

        if (inlineQuery->query.c_str()[0] == '@') // 首位是@的话进行精确匹配
            pushStickerToResultByUsername(bot.getApi(), results, inlineQuery->query);
        else
            pushStickerToResultByUsernameFuzzy(bot.getApi(), results, inlineQuery->query);

        if (results.size() == 0)
        {
            auto result = make_shared<InlineQueryResultArticle>();

            string username = inlineQuery->query;
            fixUsername(username);
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
                text->messageText = "Click to throw @" + inlineQuery->query;
                result->inputMessageContent = text;

                InlineKeyboardButton::Ptr button = make_shared<InlineKeyboardButton>();
                button->text = "Throw @" + inlineQuery->query;
                button->callbackData = inlineQuery->query;
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
        LogI("onCallbackQuery: %s: %s", callbackQuery->from->username.c_str(), callbackQuery->data.c_str());

        string username = callbackQuery->data;
        fixUsername(username);
        if (lowercaseEq(username, botUsername))
            sendMessage(bot.getApi(), callbackQuery->from->id, "(┙>∧<)┙彡 ┻━┻"); // 不允许丢自己
        else
            throwByUsername(bot.getApi(), callbackQuery->from->id, callbackQuery->data, callbackQuery->from->id) &&
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
