#pragma once

#include <tgbot/tgbot.h>

void pushStickerOnInlineQuery(const TgBot::Api &api,
                              std::vector<TgBot::InlineQueryResult::Ptr> &results,
                              const std::string &query);
