#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable : 4996)

#define MAX_STATION_NAME 100
#define MAX_STATIONS 1000

typedef struct SubwayEdge {
    int time;
    int distance;
    char line;
    struct SubwayEdge* next;
} SubwayEdge;

typedef struct Station {
    char name[MAX_STATION_NAME];
    char line;
    SubwayEdge* edge;
    struct Station* next;
} Station;

Station stations[MAX_STATIONS];
int stationCount = 0;

void loadCSV(const char* filename) {
    FILE* file = fopen("subway_line.csv", "r");
    if (!file) {
        printf("CSV 파일을 열 수 없습니다: %s\n", filename);
        return;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // 헤더 건너뜀

    while (fgets(buffer, sizeof(buffer), file)) {
        char* token = strtok(buffer, ",");
        if (token == NULL) continue;

        stations[stationCount].line = (char)atoi(token);

        token = strtok(NULL, ",\n");
        if (token == NULL) continue;

        strncpy(stations[stationCount].name, token, MAX_STATION_NAME - 1);
        stations[stationCount].name[MAX_STATION_NAME - 1] = '\0';

        stationCount++;
        if (stationCount >= MAX_STATIONS) {
            printf("최대 역 수를 초과했습니다.\n");
            break;
        }
    }

    fclose(file);
    printf("총 %d개의 역을 불러왔습니다.\n", stationCount);
}

void printStations() {
    printf("\n--- 지하철 역 목록 ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d호선 - %s\n", stations[i].line, stations[i].name);
    }
}

int main() {
    int choice;

    while (1) {
        system("cls");
        printf("\n\n\t\t지하철 길찾기 프로그램\n\n");
        printf("1. CSV 파일 불러오기\n");
        printf("2. 역 목록 출력\n");
        printf("3. 길찾기 (미구현)\n");
        printf("0. 프로그램 종료\n");
        printf("\n메뉴 선택 : ");
        scanf("%d", &choice);
        while (getchar() != '\n');

        switch (choice) {
        case 1:
            loadCSV("subway_line.csv");
            break;
        case 2:
            printStations();
            break;
        case 3:
            printf("길찾기 기능은 아직 구현되지 않았습니다.\n");
            break;
        case 0:
            exit(0);
        default:
            printf("잘못된 선택입니다.\n");
        }

        printf("\n\n\t\t");
        system("pause");
    }

    return 0;
}
