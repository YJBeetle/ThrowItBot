#include "ThrowIt.h"

#include "Log.h"
#include "Global.h"

using namespace std;
using namespace cv;
using namespace TgBot;

// 用ArtRobot绘制一张丢的图像
shared_ptr<ArtRobot::Component::Base> drawThrowImage(const string &__imgData)
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

// 向指定的chatId丢一张图
void throwImage(const Api &api, int64_t chatId,
                const string &__username,
                const string &__title,
                const string &__imgData)
{
    LogV("throwImage: %s", __username.c_str());

    api.sendChatAction(chatId, "upload_photo"); // 设置正在发送

    auto body = drawThrowImage(__imgData); // 绘制图像

    ArtRobot::Renderer renderer(ArtRobot::OutputTypePng, 512, 512, ArtRobot::Renderer::PX, 72); // 渲染png
    renderer.render(body->getSurface());

    auto stickerPngFile = make_shared<InputFile>(); // 待上传的PNG文件
    stickerPngFile->data = renderer.getDataString();
    stickerPngFile->mimeType = "image/png";

    string username = __username;
    transform(username.begin(), username.end(), username.begin(), ::tolower); // 用户名转小写
    string stickerName = username + "_by_" + botUsername;                     // 贴纸名字
    auto stickerFile = api.uploadStickerFile(chatId, stickerPngFile);         // 上传贴纸
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
