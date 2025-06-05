#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <time.h>

int main() {
    time_t now;
    struct tm* local;
    struct tm* utc;

    time(&now);
    local = localtime(&now);  // 한국 시간 (KST)

    printf("한국 시간 (KST): %d-%02d-%02d %02d:%02d:%02d\n",
        local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec);
    return 0;
}
