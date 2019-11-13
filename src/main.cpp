#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include "ArtRobot/ArtRobot.h"
#include "ArtRobot/ArtRobot.h"

using namespace std;
using namespace cv;
using namespace TgBot;
using namespace ArtRobot;

const char UserImgSearchStr[] = "<img class=\"tgme_page_photo_image\" src=\"";
const int UserImgSearchStrLen = sizeof(UserImgSearchStr) - 1;

string botUsername;
unordered_map<string, string> usersData;

void readUsersData()
{
    ifstream in("UsersData.txt");
    if (in)
    {
        string username;
        string fileId;
        while (getline(in, username) && getline(in, fileId))
            usersData[username] = fileId;
    }
}

void saveUsersData()
{
    ofstream out("UsersData.txt");
    for (auto &user : usersData)
    {
        out << user.first << endl;
        out << user.second << endl;
    }
}

void throwItImage(const Api &api, int64_t chatId, string username, const string &title, const string &userImgData)
{
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

    transform(username.begin(), username.end(), username.begin(), ::tolower); // 用户名转小写
    string stickerName = username + "_by_" + botUsername;                     // 贴纸名字
    auto stickerFile = api.uploadStickerFile(chatId, fileNew);                // 上传贴纸
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
        api.createNewStickerSet(chatId, stickerName, title, stickerFile->fileId, "🙃");
    }

    // api.sendMessage(chatId, "https://t.me/addstickers/" + stickerName, false, 0, std::make_shared<GenericReply>(), "", true); // 发送一个贴纸地址

    auto stickerSet = api.getStickerSet(stickerName);
    auto fileId = stickerSet->stickers[0]->fileId;
    api.sendSticker(chatId, fileId, 0, std::make_shared<GenericReply>(), true); // 发送一个贴纸

    usersData[username] = fileId;
    saveUsersData();
}

void throwIt(const Api &api, int64_t chatId, User::Ptr user)
{
    cout << "Throw: " << user->username << endl;

    api.sendChatAction(chatId, "upload_photo"); // 设置正在发送

    auto userPhotosInfo = api.getUserProfilePhotos(user->id);

    if (userPhotosInfo->totalCount) // 照片数不为0
    {
        auto &userPhotosInfoFirst = userPhotosInfo->photos[0];
        auto userImgPath = api.getFile(userPhotosInfoFirst[userPhotosInfoFirst.size() - 1]->fileId); // 取用最大的图片
        auto userImgData = api.downloadFile(userImgPath->filePath);                                  // 图像数据（maybe jpg）

        string username = user->username.empty() ? "user" + to_string(user->id) : user->username;
        string title = user->username.empty() ? "Throw" : "Throw @" + user->username;
        throwItImage(api, chatId, username, title, userImgData);
    }
    else
    {
        api.sendMessage(chatId, "No Photos.", false, 0, std::make_shared<GenericReply>(), "", true);
    }
}

string searchFileIdByUsername(const Api &api, const string &username)
{
    auto s = usersData.find(username);
    if (s != usersData.end())
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
                usersData[username] = fileId;
                saveUsersData();
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
    readUsersData();

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
                    throwIt(bot.getApi(), message->chat->id, message->forwardFrom);
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
            string username = message->text.c_str() + 1;
            transform(username.begin(), username.end(), username.begin(), ::tolower);

            CurlHttpClient curl;
            string url = "https://t.me/" + username;
            vector<HttpReqArg> args;
            string html = curl.makeRequest(url, args);

            auto startpos = html.find(UserImgSearchStr) + UserImgSearchStrLen;
            auto endpos = html.find_first_of("\"", startpos);
            string imgurl = html.substr(startpos, endpos - startpos);

            string img = curl.makeRequest(imgurl, args);

            try
            {
                bot.getApi().sendMessage(message->chat->id, "Throw " + username + "(Not yet developed)", false, 0, std::make_shared<GenericReply>(), "", true);
            }
            catch (TgException &e)
            {
                cerr << "sendMessage error: " << e.what() << endl;
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
            bot.getApi().sendMessage(message->chat->id, "You can say /throw to throw youself!\nAnd you can forward someone's message for me to throw him/her.", false, 0, std::make_shared<GenericReply>(), "", true);
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
            throwIt(bot.getApi(), message->chat->id, message->from);
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
            if (!pushStickerToResultByUsername(bot.getApi(), results, query.c_str() + 1))
            {
                auto text = make_shared<InputTextMessageContent>();
                text->messageText = "@" + botUsername;
                auto result = make_shared<InlineQueryResultArticle>();
                result->title = "No username found";
                result->id = "nouser";
                result->inputMessageContent = text;
                results.push_back(result);
            }
        }
        else
        {
            string username = inlineQuery->from->username.empty() ? "user" + to_string(inlineQuery->from->id) : inlineQuery->from->username;
            transform(username.begin(), username.end(), username.begin(), ::tolower);
            pushStickerToResultByUsername(bot.getApi(), results, username); // 查询者自己的username
            if (!pushStickerToResultByUsername(bot.getApi(), results, query))
            {
                int i = 0;
                for (auto user : usersData)
                {
                    if (user.first == username)
                        continue;
                    if (user.first.find(query) != string::npos)
                    {
                        auto result = make_shared<InlineQueryResultCachedSticker>();
                        result->id = user.first;
                        result->stickerFileId = user.second;
                        results.push_back(result);
                    }
                    if (i++ >= 20)
                        break;
                }
            }
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
