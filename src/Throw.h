#pragma once

#include <tgbot/tgbot.h>
#include <opencv2/opencv.hpp>
#include <ArtRobot/ArtRobot.h>

void throwByImage(const TgBot::Api &api, int64_t chatId,
                const std::string &__username,
                const std::string &__title,
                const std::string &__userImgData);

void throwByUserId(const TgBot::Api &api, int64_t chatId,
               TgBot::User::Ptr user);
