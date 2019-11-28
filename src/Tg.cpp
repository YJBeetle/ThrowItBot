#include "Tg.h"

#include "Log.h"
#include "Global.h"

using namespace std;
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
        LogE("sendChatActionUploadPhoto: TgBot::Api::sendChatAction: %s", e.what());
    }
}

void sendMessage(const Api &api, int64_t chatId,
                 const string &message)
{
    try
    {
        api.sendMessage(chatId, message, false, 0, make_shared<GenericReply>(), "", true);
    }
    catch (TgException &e)
    {
        LogE("sendMessage: TgBot::Api::sendMessage: %s", e.what());
    }
}
