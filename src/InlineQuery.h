#pragma once

#include <tgbot/tgbot.h>

bool pushStickerToResultByUsername(const TgBot::Api &api,
                                   std::vector<TgBot::InlineQueryResult::Ptr> &results,
                                   const std::string &username);
