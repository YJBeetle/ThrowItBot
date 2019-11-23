#pragma once

#define LogV(...) printf("\33[1;47;30mVERBOSE\33[0m \33[37m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogD(...) printf("\33[1;47;30m DEBUG \33[0m \33[37m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogI(...) printf("\33[1;44;30m INFO  \33[0m \33[34m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogW(...) printf("\33[1;43;30m WARN  \33[0m \33[33m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogE(...) printf("\33[1;41;30m ERROR \33[0m \33[31m"); printf(__VA_ARGS__); printf("\33[0m\n");
