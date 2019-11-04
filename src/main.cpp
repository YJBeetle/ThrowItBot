#include <vector>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include "ArtRobot/ArtRobot.h"
#include "ArtRobot/ArtRobot.h"

using namespace std;
using namespace cv;
using namespace TgBot;
using namespace ArtRobot;

void throwIt(const Api &api, int64_t chatId, User::Ptr user)
{
    api.sendChatAction(chatId, "upload_photo"); // 设置正在发送

    auto userPhotosInfo = api.getUserProfilePhotos(user->id);

    if (userPhotosInfo->totalCount) // 照片数不为0
    {
        auto username = user->username;

        auto &userPhotosInfoFirst = userPhotosInfo->photos[0];
        auto userImgPath = api.getFile(userPhotosInfoFirst[userPhotosInfoFirst.size() - 1]->fileId); // 取用最大的图片
        auto userImgData = api.downloadFile(userImgPath->filePath);                                  // 图像数据（maybe jpg）

        auto body = Component::Group();                                      // body
        auto bg = make_shared<Component::Image>(0, 0, 512, 512, 0, "p.png"); // bg
        body.addChild(bg);                                                   // Show bg

        vector<unsigned char> userImgVector(userImgData.begin(), userImgData.end()); // 用户头像
        Mat userImgMat = imdecode(userImgVector, IMREAD_COLOR);
        auto userImg = make_shared<Component::Image>(18.56,
                                                     180.98,
                                                     135.53,
                                                     135.53,
                                                     -160,
                                                     userImgMat);

        auto mask = make_shared<Component::ImageMask>(0, 0, 512, 512, 0, "p_mask.png", userImg); // Mask
        body.addChild(mask);                                                                     // Show mask

        // Renderer renderer(OutputTypePixmap, 512, 512, Renderer::PX, 72);
        Renderer renderer(OutputTypePng, 512, 512, Renderer::PX, 72);
        renderer.render(body.getSurface());

        auto fileNew = make_shared<InputFile>();
        fileNew->data = renderer.getDataString();
        fileNew->mimeType = "image/png";
        // api.sendPhoto(chatId, fileNew, "", 0, std::make_shared<GenericReply>(), "", true);
        api.sendSticker(chatId, fileNew, 0, std::make_shared<GenericReply>(), true);
    }
    else
    {
        api.sendMessage(chatId, "No Photos.", false, 0, std::make_shared<GenericReply>(), "", true);
    }
}

int main()
{
    cout << "ThrowItBot start!" << endl;

    // init
    if (0 != access("UsersData", 0))
    {
        if (mkdir("UsersData", S_IRWXU) == -1)
        {
            cerr << "error: " << endl;
            exit(1);
        }
    }

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        cout << message->chat->username << ": " << message->text << endl;

        if (message->forwardFrom) // 是转发的消息
        {
            throwIt(bot.getApi(), message->chat->id, message->forwardFrom);
            bot.getApi().sendMessage(message->chat->id, "<(ˉ^ˉ)>", false, 0, std::make_shared<GenericReply>(), "", true);
            return;
        }

        if (
            StringTools::startsWith(message->text, "/start") ||
            StringTools::startsWith(message->text, "/help") ||
            StringTools::startsWith(message->text, "/throw") ||
            false)
        {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Do you need /help ?", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "You can say /throw to throw youself!\nAnd you can forward someone's message for me to throw he.", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Do you need to be /throw ?", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        throwIt(bot.getApi(), message->chat->id, message->from);
        bot.getApi().sendMessage(message->chat->id, "( ﹁ ﹁ ) ", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        cout << "onInlineQuery" << endl;
    });

    bot.getEvents().onChosenInlineResult([&bot](ChosenInlineResult::Ptr chosenInlineResult) {
        cout << "onChosenInlineResult" << endl;
    });

    while (true)
    {
        try
        {
            cout << "Connecting to telegram API server" << endl;
            cout << "Bot username: " << bot.getApi().getMe()->username << endl;

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
    }

    return 0;
}