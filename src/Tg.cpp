#include "Tg.h"

#include "Log.h"
#include "Global.h"

using namespace std;
using namespace TgBot;

// 设置正在发送
bool sendChatActionUploadPhoto(const Api &api, int64_t chatId)
{
    try
    {
        api.sendChatAction(chatId, "upload_photo");
    }
    catch (TgException &e)
    {
        LogE("sendChatActionUploadPhoto: TgBot::Api::sendChatAction: %s", e.what());
        return false;
    }
    return true;
}

bool sendMessage(const Api &api, int64_t chatId,
                 const string &message)
{
    try
    {
        api.sendMessage(chatId, message, false, 0, make_shared<GenericReply>(), "", true);
    }
    catch (TgException &e)
    {
        LogE("sendMessage: TgBot::Api::sendMessage: %s", e.what());
        return false;
    }
    return true;
}

bool sendSticker(const Api &api, int64_t chatId,
                 const string &stickerFileId)
{
    try
    {
        // api.sendMessage(chatId, "https://t.me/addstickers/" + stickerName, false, 0, std::make_shared<GenericReply>(), "", true); // 发送贴纸地址
        // api.sendSticker(chatId, stickerFileId, 0, std::make_shared<GenericReply>(), true); // 发送一个贴纸
        api.sendSticker(chatId, stickerFileId); // 发送一个贴纸
    }
    catch (TgException &e)
    {
        LogE("sendSticker: TgBot::Api::sendSticker: %s", e.what());
        return false;
    }
    return true;
}
