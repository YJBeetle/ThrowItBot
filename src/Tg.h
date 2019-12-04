#pragma once

#include <tgbot/tgbot.h>

// 设置正在发送状态
bool sendChatActionUploadPhoto(const TgBot::Api &api, int64_t chatId); 

// 发一个消息
bool sendMessage(const TgBot::Api &api, int64_t chatId,
                 const std::string &message);

// 发一个贴纸
bool sendSticker(const TgBot::Api &api, int64_t chatId,
                 const std::string &stickerFileId);
