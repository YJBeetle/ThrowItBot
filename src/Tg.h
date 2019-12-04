#pragma once

#include <tgbot/tgbot.h>

#include "Global.h"
#include "StringCheck.h"

// 设置正在发送状态
bool sendChatActionUploadPhoto(const TgBot::Api &api, int64_t chatId); 

// 发一个消息
bool sendMessage(const TgBot::Api &api, int64_t chatId,
                 const std::string &message);

// 发一个贴纸
bool sendSticker(const TgBot::Api &api, int64_t chatId,
                 const std::string &stickerFileId);

inline std::string getUsername(TgBot::User::Ptr &user)
{
    return user->username.empty() ? "user" + std::to_string(user->id) : user->username;
}

inline bool checkSelf(const TgBot::Api &api, int64_t chatId,const std::string &username)
{
    if (lowercaseEq(username, botUsername))
    {
        sendMessage(api, chatId, "(┙>∧<)┙彡 ┻━┻"); // 不允许丢自己
        return true;
    }
    return false;
}

inline bool checkSelf(const TgBot::Api &api, int64_t chatId,uint64_t userId)
{
    if (userId == botId)
    {
        sendMessage(api, chatId, "(┙>∧<)┙彡 ┻━┻"); // 不允许丢自己
        return true;
    }
    return false;
}
