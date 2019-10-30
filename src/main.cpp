#include <stdio.h>
#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

int main()
{
    string token = getenv("TOKEN");

    Bot bot(token);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi! you can say /throw .");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "You can say /throw .");
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        auto photos = bot.getApi().getUserProfilePhotos(message->chat->id);
        auto fileId = bot.getApi().getFile(photos->photos[0][0]->fileId);
        auto fileData = bot.getApi().downloadFile(fileId->filePath);

        auto fileNew = make_shared<InputFile>();
        fileNew->data = fileData;
        fileNew->mimeType = "image/jpeg";
        bot.getApi().sendPhoto(message->chat->id, fileNew);

        bot.getApi().sendMessage(message->chat->id, "ok");
    });

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (
            StringTools::startsWith(message->text, "/start") ||
            StringTools::startsWith(message->text, "/help") ||
            StringTools::startsWith(message->text, "/throw") ||
            false)
        {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Do you need /help ?");
    });

    try
    {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgLongPoll longPoll(bot);
        while (true)
        {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgException &e)
    {
        printf("error: %s\n", e.what());
    }

    return 0;
}