#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include "ArtRobot/ArtRobot.h"
#include "ArtRobot/ArtRobot.h"

using namespace std;
using namespace cv;
using namespace TgBot;
using namespace ArtRobot;

string botUsername;

unordered_map<string, string> usersData;
string testFileId = "BQADBQADkgADR8RIVu_hHfP6s9kdFgQ"; // png
// string testFileId = "BQADBQADfQADR8RQVpgZ-15uRDTlFgQ"; // webp

// void readUsersData()
// {
//     ifstream in("UsersData.txt");
//     if (in)
//     {
//         string username;
//         string fileId;
//         while (getline(in, username) && getline(in, fileId))
//             usersData[username] = fileId;
//     }
// }

// void saveUsersData()
// {
//     ofstream out("UsersData.txt");
//     for (auto &user : usersData)
//     {
//         out << user.first << endl;
//         out << user.second << endl;
//     }
// }

void throwIt(const Api &api, int64_t chatId, User::Ptr user)
{
    api.sendChatAction(chatId, "upload_photo"); // 设置正在发送

    auto userPhotosInfo = api.getUserProfilePhotos(user->id);

    if (userPhotosInfo->totalCount) // 照片数不为0
    {
        auto &userPhotosInfoFirst = userPhotosInfo->photos[0];
        auto userImgPath = api.getFile(userPhotosInfoFirst[userPhotosInfoFirst.size() - 1]->fileId); // 取用最大的图片
        auto userImgData = api.downloadFile(userImgPath->filePath);                                  // 图像数据（maybe jpg）

        auto body = Component::Group("body");                                      // body
        auto bg = make_shared<Component::Image>("bg", 0, 0, 512, 512, 0, "p.png"); // bg
        body.addChild(bg);                                                         // Show bg

        vector<unsigned char> userImgVector(userImgData.begin(), userImgData.end()); // 用户头像
        Mat userImgMat = imdecode(userImgVector, IMREAD_COLOR);
        auto userImg = make_shared<Component::Image>("userimg", 18.56,
                                                     180.98,
                                                     135.53,
                                                     135.53,
                                                     -160,
                                                     userImgMat);
        auto mask = make_shared<Component::ImageMask>("mask", 0, 0, 512, 512, 0, "p_mask.png", userImg); // Mask
        body.addChild(mask);                                                                             // Show mask

        Renderer renderer(OutputTypePng, 512, 512, Renderer::PX, 72); // 渲染png
        renderer.render(body.getSurface());
        auto fileNew = make_shared<InputFile>();
        fileNew->data = renderer.getDataString();
        fileNew->mimeType = "image/png";

        // 先丢一个
        api.sendSticker(chatId, fileNew, 0, std::make_shared<GenericReply>(), true);

        string username = user->username.empty() ? "user" + to_string(user->id) : user->username;
        string stickerName = username + "_by_" + botUsername;      // 贴纸名字
        auto stickerFile = api.uploadStickerFile(chatId, fileNew); // 上传贴纸
        try
        {
            // 如果存在则删除贴纸包内贴纸
            auto stickerSet = api.getStickerSet(stickerName);
            // 删除贴纸
            for (auto sticker : stickerSet->stickers)
                api.deleteStickerFromSet(sticker->fileId);
            api.addStickerToSet(chatId, stickerName, stickerFile->fileId, "🙃");
        }
        catch (TgException &e)
        {
            // 没有找到贴纸 创建
            api.createNewStickerSet(chatId, stickerName, user->username.empty() ? "Throw" : "Throw @" + user->username, stickerFile->fileId, "🙃");
        }

        api.sendMessage(chatId, "https://t.me/addstickers/" + stickerName, false, 0, std::make_shared<GenericReply>(), "", true);

        // cout << stickerFile << endl;
        // cout << stickerFile->fileId << endl;
        // testFileId = stickerFile->fileId;
        // api.sendSticker(chatId, stickerFile->fileId, 0, std::make_shared<GenericReply>(), true);
        // api.sendPhoto(chatId, fileNew, "", 0, std::make_shared<GenericReply>(), "", true);
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
    // readUsersData();

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        cout << "Message: " << message->chat->username << ": " << message->text << endl;

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
        bot.getApi().sendMessage(message->chat->id, "You can say /throw to throw youself!\nAnd you can forward someone's message for me to throw him/her.", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Do you need to be /throw ?", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("throw", [&bot](Message::Ptr message) {
        throwIt(bot.getApi(), message->chat->id, message->from);
        bot.getApi().sendMessage(message->chat->id, "( ﹁ ﹁ ) ", false, 0, std::make_shared<GenericReply>(), "", true);
    });

    bot.getEvents().onCommand("test", [&bot](Message::Ptr message) {
        bot.getApi().sendSticker(message->chat->id, testFileId, 0, std::make_shared<GenericReply>(), true);
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        cout << "InlineQuery: " << inlineQuery->from->username << ": " << inlineQuery->query << endl;

        // 贴图
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = "123456";
        result->stickerFileId = testFileId;
        cout << result->stickerFileId << endl;

        // // 文本
        // auto text = make_shared<InputTextMessageContent>();
        // text->messageText = "aabbcc";
        // auto result = make_shared<InlineQueryResultArticle>();
        // result->title = "xxx";
        // result->id = "1234567";
        // result->inputMessageContent = text;

        // results
        vector<InlineQueryResult::Ptr> results;
        results.push_back(result);

        cout << (result) << endl;
        TgTypeParser tgTypeParser;
        cout << tgTypeParser.parseArray<InlineQueryResult>(&TgTypeParser::parseInlineQueryResult, results) << endl;

        bot.getApi().answerInlineQuery(inlineQuery->id, results);

        cout << "onInlineQuery" << endl;
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
