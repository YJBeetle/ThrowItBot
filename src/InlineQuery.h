#pragma once

#include <tgbot/tgbot.h>

void pushStickerToResultByUsername(const TgBot::Api &api,
                                   std::vector<TgBot::InlineQueryResult::Ptr> &results,
                                   const std::string &username);

void pushStickerToResultByUsernameFuzzy(const TgBot::Api &api,
                                   std::vector<TgBot::InlineQueryResult::Ptr> &results,
                                   const std::string &keywords);
