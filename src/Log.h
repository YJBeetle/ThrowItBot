#pragma once

#define LogV(...) printf("\33[1;47mVERBOSE\33[0m \33[37m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogD(...) printf("\33[1;47m DEBUG \33[0m \33[37m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogI(...) printf("\33[1;44m INFO  \33[0m \33[34m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogW(...) printf("\33[1;43m WARN  \33[0m \33[33m"); printf(__VA_ARGS__); printf("\33[0m\n");
#define LogE(...) printf("\33[1;41m ERROR \33[0m \33[31m"); printf(__VA_ARGS__); printf("\33[0m\n");
