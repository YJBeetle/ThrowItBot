#include "ThrowIt.h"

#include "Log.h"
#include "Global.h"

using namespace std;
using namespace cv;
using namespace TgBot;
using namespace ArtRobot;

void throwImage(const Api &api, int64_t chatId,
                const string &__username,
                const string &__title,
                const string &__ImgData)
{ // 向指定的chatId丢一张图
    LogV("throwImage: %s", __username.c_str());
    api.sendChatAction(chatId, "upload_photo"); // 设置正在发送

    auto body = Component::Group("body");                                      // body
    auto bg = make_shared<Component::Image>("bg", 0, 0, 512, 512, 0, "p.png"); // bg
    body.addChild(bg);                                                         // Show bg

    vector<unsigned char> userImgVector(__ImgData.begin(), __ImgData.end()); // 图片转为vector
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

    string username = __username;
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
        api.createNewStickerSet(chatId, stickerName, __title, stickerFile->fileId, "🙃");
    }

    // api.sendMessage(chatId, "https://t.me/addstickers/" + stickerName, false, 0, std::make_shared<GenericReply>(), "", true); // 发送一个贴纸地址

    auto stickerSet = api.getStickerSet(stickerName);
    auto fileId = stickerSet->stickers[0]->fileId;
    api.sendSticker(chatId, fileId, 0, std::make_shared<GenericReply>(), true); // 发送一个贴纸

    usersData.set(username, fileId);
}
