#include "Throw.h"

#include "Log.h"
#include "Global.h"

using namespace std;
using namespace cv;
using namespace TgBot;

// 设置正在发送
void sendChatActionUploadPhoto(const Api &api, int64_t chatId)
{
    try
    {
        api.sendChatAction(chatId, "upload_photo");
    }
    catch (TgException &e)
    {
        LogW("sendChatActionUploadPhoto: sendChatAction error");
    }
}

// ArtRobot的实际绘制函数
shared_ptr<ArtRobot::Component::Base> drawImage(const string &__imgData)
{
    vector<unsigned char> imgVector(__imgData.begin(), __imgData.end()); // 图片转为vector
    Mat imgMat = imdecode(imgVector, IMREAD_COLOR);                      // 图片转为Mat

    // 开始ArtRobot绘图

    auto bg = make_shared<ArtRobot::Component::Image>("bg", 0, 0, 512, 512, 0, "p.png"); // bg

    auto img = make_shared<ArtRobot::Component::Image>("img", // img
                                                       18.56, 180.98,
                                                       135.53, 135.53,
                                                       -160,
                                                       imgMat);

    auto mask = make_shared<ArtRobot::Component::ImageMask>("mask", 0, 0, 512, 512, 0, "p_mask.png", img); // Mask

    auto body = make_shared<ArtRobot::Component::Group>("body"); // body
    body->addChild(bg);
    body->addChild(mask);
    return body;
}

// throwByImage
void throwByImage(const Api &api, int64_t chatId,
                const string &__username,
                const string &__title,
                const string &__imgData)
{
    string username = __username;
    transform(username.begin(), username.end(), username.begin(), ::tolower); // 用户名转小写
    string stickerName = username + "_by_" + botUsername;                     // 贴纸名字

    LogV("throwByImage: %s", username.c_str());

    sendChatActionUploadPhoto(api, chatId); // 设置正在发送

    auto body = drawImage(__imgData); // 绘制图像

    ArtRobot::Renderer renderer(ArtRobot::OutputTypePng, 512, 512, ArtRobot::Renderer::PX, 72); // 渲染png
    renderer.render(body->getSurface());

    auto stickerPngFile = make_shared<InputFile>(); // 待上传的PNG文件
    stickerPngFile->data = renderer.getDataString();
    stickerPngFile->mimeType = "image/png";

    File::Ptr stickerFile;
    try
    {
        stickerFile = api.uploadStickerFile(chatId, stickerPngFile); // 上传贴纸
    }
    catch (TgException &e)
    {
        LogE("throwByImage: uploadStickerFile error");
        return;
    }

    StickerSet::Ptr stickerSet;
    try
    {
        stickerSet = api.getStickerSet(stickerName); // 尝试获取贴纸包
    }
    catch (TgException &e)
    {
        LogI("throwByImage: getStickerSet error, no sticker, create it.");
    }

    if (stickerSet)
    { // 存在贴纸包
        try
        {
            api.addStickerToSet(chatId, stickerName, stickerFile->fileId, "🙃"); // 添加贴纸到贴纸包
        }
        catch (TgException &e)
        {
            LogE("throwByImage: addStickerToSet error");
            return;
        }
        for (auto sticker : stickerSet->stickers)
            try
            {
                api.deleteStickerFromSet(sticker->fileId); // 删除所有其他贴纸
            }
            catch (TgException &e)
            {
                LogW("throwByImage: deleteStickerFromSet error");
            }
    }
    else
    { // 没有找到贴纸 创建
        try
        {
            api.createNewStickerSet(chatId, stickerName, __title, stickerFile->fileId, "🙃"); // 创建贴纸包并添加第一个贴纸
        }
        catch (TgException &e)
        {
            LogE("throwByImage: createNewStickerSet error");
            return;
        }
    }

    string stickerFileId;
    try
    {
        stickerFileId = api.getStickerSet(stickerName)->stickers[0]->fileId;
    }
    catch (TgException &e)
    {
        LogE("throwByImage: getStickerSet error");
        return;
    }

    usersData.set(username, stickerFileId);

    try
    {
        // api.sendMessage(chatId, "https://t.me/addstickers/" + stickerName, false, 0, std::make_shared<GenericReply>(), "", true); // 发送贴纸地址
        api.sendSticker(chatId, stickerFileId, 0, std::make_shared<GenericReply>(), true); // 发送一个贴纸
    }
    catch (TgException &e)
    {
        LogE("throwByImage: sendSticker error");
        return;
    }
}

// 丢一个用户（聊天者本人或转发消息时才可获得）
void throwByUserId(const Api &api, int64_t chatId,
               User::Ptr user)
{
    LogV("throwByUserId: %s %d", user->username.c_str(), user->id);

    sendChatActionUploadPhoto(api, chatId); // 设置正在发送

    UserProfilePhotos::Ptr userPhotosInfo;

    try
    {
        userPhotosInfo = api.getUserProfilePhotos(user->id);
    }
    catch (TgException &e)
    {
        LogE("throwByUserId: getUserProfilePhotos error");
        return;
    }

    if (userPhotosInfo && userPhotosInfo->totalCount) // 照片数不为0
    {
        string userPhotosData;
        try
        {
            auto &userPhotosInfoFirst = userPhotosInfo->photos[0];
            auto userPhotosPath = api.getFile(userPhotosInfoFirst[userPhotosInfoFirst.size() - 1]->fileId); // 取用最大的图片
            userPhotosData = api.downloadFile(userPhotosPath->filePath);                                    // 图像数据（maybe jpg）
        }
        catch (TgException &e)
        {
            LogE("throwByUserId: Get user photo error.");
            return;
        }
        string username = user->username.empty() ? "user" + to_string(user->id) : user->username;
        string title = user->username.empty() ? "Throw" : "Throw @" + user->username;
        throwByImage(api, chatId, username, title, userPhotosData);
    }
    else
    {
        try
        {
            api.sendMessage(chatId, "No Photos.", false, 0, std::make_shared<GenericReply>(), "", true);
        }
        catch (TgException &e)
        {
            LogE("throwByUserId: sendMessage error");
            return;
        }
    }
}
