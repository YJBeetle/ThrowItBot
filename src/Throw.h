#pragma once

#include <tgbot/tgbot.h>
#include <opencv2/opencv.hpp>
#include <ArtRobot/ArtRobot.h>

bool throwByImage(const TgBot::Api &api, int64_t chatId,
                  const std::string &__username,
                  const std::string &__title,
                  const std::string &__userImgData,
                  int32_t ownerId);

bool throwByUserId(const TgBot::Api &api, int64_t chatId,
                   TgBot::User::Ptr user,
                   int32_t ownerId);

bool throwByUsername(const TgBot::Api &api, int64_t chatId,
                     const std::string &__username,
                     int32_t ownerId);
