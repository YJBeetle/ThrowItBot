#pragma once

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include <ArtRobot/ArtRobot.h>

void throwImage(const TgBot::Api &api, int64_t chatId,
                const std::string &__username,
                const std::string &__title,
                const std::string &__userImgData);

void throwUser(const TgBot::Api &api, int64_t chatId,
               TgBot::User::Ptr user);
