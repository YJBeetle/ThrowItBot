#pragma once

#include <tgbot/tgbot.h>

void sendChatActionUploadPhoto(const TgBot::Api &api, int64_t chatId);

void sendMessage(const TgBot::Api &api, int64_t chatId,
                 const std::string &message);
