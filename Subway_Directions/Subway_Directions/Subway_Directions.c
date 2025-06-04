#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#pragma warning(disable : 4996)

#define MAX_STATION_NAME 100
#define MAX_STATIONS 1000
#define TRANSFER_PENALTY 3  // 추가 환승 3분 시간 패널티

//---------------- 구조체 정의 ----------------
typedef struct SubwayEdge {
    int destIndext;
    float time;
    float distance;
    char line;
    struct SubwayEdge* next;
} SubwayEdge;

typedef struct Station {
    char name[MAX_STATION_NAME];
    SubwayEdge* edge;
} Station;

//---------------- 전역 변수 ----------------
Station stations[MAX_STATIONS];
int stationCount = 0;

//----------------  역 이름으로 인덱스 찾기 ----------------
int getStationIndexByName(const char* name) {
    for (int i = 0; i < stationCount; i++) {
        if (strcmp(stations[i].name, name) == 0)
            return i;
    }
    return -1;
}

//---------------- 에지 추가 함수 ----------------
void addEdge(int from, int to, int time, int distance, char line) {
    SubwayEdge* edge = (SubwayEdge*)malloc(sizeof(SubwayEdge));
    edge->destIndext = to;
    edge->time = time;
    edge->distance = distance;
    edge->line = line;
    edge->next = stations[from].edge;
    stations[from].edge = edge;
}

//---------------- CSV 파일 로딩 ----------------
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
        if (!token) continue;

        int line = atoi(token);

        char* name1 = strtok(NULL, ",");
        char* name2 = strtok(NULL, ",");
        char* t_str = strtok(NULL, ",");
        char* d_str = strtok(NULL, ",");
        if (!name1 || !name2 || !t_str || !d_str) continue;

        int time = atoi(t_str);
        int distance = atoi(d_str);

        int fromIndex = getStationIndexByName(name1);
        if (fromIndex == -1) {
            strncpy(stations[stationCount].name, name1, MAX_STATION_NAME - 1);
            stations[stationCount].name[MAX_STATION_NAME - 1] = '\0';
            stations[stationCount].edge = NULL;
            fromIndex = stationCount++;
        }

        int toIndex = getStationIndexByName(name2);
        if (toIndex == -1) {
            strncpy(stations[stationCount].name, name2, MAX_STATION_NAME - 1);
            stations[stationCount].name[MAX_STATION_NAME - 1] = '\0';
            stations[stationCount].edge = NULL;
            toIndex = stationCount++;
        }

        addEdge(fromIndex, toIndex, time, distance, line);
        addEdge(toIndex, fromIndex, time, distance, line); // 양방향
    }

    fclose(file);
    printf("총 %d개의 역을 불러왔습니다.\n", stationCount);
}

//---------------- 역 목록 출력 ----------------
void printStations() {
    printf("\n--- 지하철 역 목록 ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d - %s\n", i, stations[i].name);
    }
}

//---------------- 경로 탐색 (다익스트라 기반) ----------------
void findPath(const char* startName, const char* endName, int mode) {
    int start = getStationIndexByName(startName);
    int end = getStationIndexByName(endName);
    if (start == -1 || end == -1) {
        printf("입력한 역이 존재하지 않습니다.\n");
        return;
    }

    int cost[MAX_STATIONS];
    int prev[MAX_STATIONS];
    int visited[MAX_STATIONS] = { 0 };
    char prevLine[MAX_STATIONS];

    for (int i = 0; i < stationCount; i++) {
        cost[i] = INT_MAX;
        prev[i] = -1;
        prevLine[i] = 0;
    }

    cost[start] = 0;

    for (int i = 0; i < stationCount; i++) {
        int minCost = INT_MAX, u = -1;
        for (int j = 0; j < stationCount; j++) {
            if (!visited[j] && cost[j] < minCost) {
                minCost = cost[j];
                u = j;
            }
        }
        if (u == -1) break;
        visited[u] = 1;

        SubwayEdge* e = stations[u].edge;
        while (e) {
            int v = e->destIndext;
            int weight = (mode == 1) ? e->distance : e->time;

            if (prevLine[u] != 0 && prevLine[u] != e->line)
                weight += TRANSFER_PENALTY;

            if (!visited[v] && cost[u] + weight < cost[v]) {
                cost[v] = cost[u] + weight;
                prev[v] = u;
                prevLine[v] = e->line;
            }
            e = e->next;
        }
    }

    if (cost[end] == INT_MAX) {
        printf("경로를 찾을 수 없습니다.\n");
        return;
    }

    // 결과 출력
    printf("\n최소 %s: %d\n", (mode == 1) ? "거리(Km)" : "시간(분)", cost[end]);
    printf("경로: ");
    int path[MAX_STATIONS], count = 0;

    for (int v = end; v != -1; v = prev[v])
        path[count++] = v;

    for (int i = count - 1; i >= 0; i--) {
        printf("%s", stations[path[i]].name);
        if (i != 0) printf("->");
    }
    printf("\n");
}

//---------------- 메인 함수 ----------------
int main() {
    int choice;

    while (1) {
        system("cls");  // Windows 환경 전용
        printf("\n\n\t\t지하철 길찾기 프로그램\n\n");
        printf("1. CSV 파일 불러오기\n");
        printf("2. 역 목록 출력\n");
        printf("3. 길찾기\n");
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
        case 3: {
            char start[MAX_STATION_NAME], end[MAX_STATION_NAME];
            int mode;

            printf("출발역 이름: ");
            fgets(start, sizeof(start), stdin);
            start[strcspn(start, "\n")] = '\0';

            printf("도착역 이름: ");
            fgets(end, sizeof(end), stdin);
            end[strcspn(end, "\n")] = '\0';

            printf("1. 최단 시간 경로\n");
            printf("2. 최단 거리 경로\n");
            printf("선택: ");
            scanf("%d", &mode);
            while (getchar() != '\n');

            findPath(start, end, mode);
            break;
        }
        case 0:
            exit(0);
        default:
            printf("잘못된 선택입니다.\n");
        }

        printf("\n\n\t\t");
        system("pause");  // Windows 전용
    }

    return 0;
}
