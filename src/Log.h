#pragma once

#define LogV(...)                                                                                                      \
    printf("\33[1;47;30mVERBOSE\33[0;40;37m %s:\33[33m%d:\33[32m%s:\33[0m \33[37m", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);                                                                                               \
    printf("\33[0m\n");
#define LogD(...)                                                                                                      \
    printf("\33[1;47;30m DEBUG \33[0;40;37m %s:\33[33m%d:\33[32m%s:\33[0m \33[37m", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);                                                                                               \
    printf("\33[0m\n");
#define LogI(...)                                                                                                   \
    printf("\33[1;44m INFO  \33[0;40;37m %s:\33[33m%d:\33[32m%s:\33[0m \33[34m", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);                                                                                            \
    printf("\33[0m\n");
#define LogW(...)                                                                                                   \
    printf("\33[1;43m WARN  \33[0;40;37m %s:\33[33m%d:\33[32m%s:\33[0m \33[33m", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);                                                                                            \
    printf("\33[0m\n");
#define LogE(...)                                                                                                   \
    printf("\33[1;41m ERROR \33[0;40;37m %s:\33[33m%d:\33[32m%s:\33[0m \33[31m", __FILE__, __LINE__, __FUNCTION__); \
    printf(__VA_ARGS__);                                                                                            \
    printf("\33[0m\n");
