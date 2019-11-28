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
        LogW("sendChatActionUploadPhoto: sendChatAction error");
    }
}
