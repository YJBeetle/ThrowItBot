#pragma once

#include <tgbot/tgbot.h>

void pushStickerToResultByUsername(const TgBot::Api &api,
                                   std::vector<TgBot::InlineQueryResult::Ptr> &results,
                                   const std::string &__username);

void pushStickerToResultByUsernameFuzzy(const TgBot::Api &api,
                                   std::vector<TgBot::InlineQueryResult::Ptr> &results,
                                   const std::string &__keywords);

void pushClickToThrow(const TgBot::Api &api,
                      std::vector<TgBot::InlineQueryResult::Ptr> &results,
                      const std::string &__username);
