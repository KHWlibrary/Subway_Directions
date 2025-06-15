#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

#pragma warning(disable : 4996)

#define MAX_STATION_NAME 100
#define MAX_STATIONS 1000
#define TRANSFER_PENALTY 3

// ---------------------- 구조체 정의 ----------------------

typedef struct SubwayEdge {
    int destIndex;
    float time;
    float distance;
    int line;
    struct SubwayEdge* next;
} SubwayEdge;

typedef struct Station {
    char name[MAX_STATION_NAME];
    SubwayEdge* edge;
} Station;

Station stations[MAX_STATIONS];
int stationCount = 0;

// ---------------------- 공통 유틸 함수 ----------------------

void trim(char* str) {
    str[strcspn(str, "\r\n")] = 0;
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) *end-- = '\0';
    if (start != str) memmove(str, start, strlen(start) + 1);
}

int getStationIndexByName(const char* name) {
    for (int i = 0; i < stationCount; i++) {
        if (strcmp(stations[i].name, name) == 0)
            return i;
    }
    return -1;
}

void addEdge(int from, int to, float time, float distance, int line) {
    SubwayEdge* edge = (SubwayEdge*)malloc(sizeof(SubwayEdge));
    edge->destIndex = to;
    edge->time = time;
    edge->distance = distance;
    edge->line = line;
    edge->next = stations[from].edge;
    stations[from].edge = edge;
}

int calculateFare(float distance) {
    int fare = 1400;
    if (distance > 10.0f) {
        int extra = distance - 10.0f;
        int blocks = (int)((extra + 4.999f) / 5.0f);
        fare += blocks * 100;
    }
    return fare;
}

void appendToCSV(const char* filename, int line, const char* from, const char* to, float distance, float time) {
    FILE* file = fopen(filename, "a");
    if (!file) {
        printf("CSV 파일에 쓰는 데 실패했습니다: %s\n", filename);
        return;
    }
    fprintf(file, "%d,%s,%s,%.2f,%.2f\n", line, from, to, distance, time);
    fclose(file);
}

// ---------------------- CSV 불러오기 ----------------------

void loadCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("CSV 파일을 열 수 없습니다: %s\n", filename);
        return;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);

    while (fgets(buffer, sizeof(buffer), file)) {
        char* token = strtok(buffer, ",");
        if (!token) continue;
        int line = atoi(token);

        char* name1 = strtok(NULL, ",");
        char* name2 = strtok(NULL, ",");
        char* d_str = strtok(NULL, ",");
        char* t_str = strtok(NULL, ",");
        if (!name1 || !name2 || !d_str || !t_str) continue;

        trim(name1); trim(name2);
        float distance = atof(d_str);
        float time = atof(t_str);

        int fromIndex = getStationIndexByName(name1);
        if (fromIndex == -1) {
            memset(&stations[stationCount], 0, sizeof(Station));
            strncpy(stations[stationCount].name, name1, MAX_STATION_NAME - 1);
            fromIndex = stationCount++;
        }

        int toIndex = getStationIndexByName(name2);
        if (toIndex == -1) {
            memset(&stations[stationCount], 0, sizeof(Station));
            strncpy(stations[stationCount].name, name2, MAX_STATION_NAME - 1);
            toIndex = stationCount++;
        }

        addEdge(fromIndex, toIndex, time, distance, line);
        addEdge(toIndex, fromIndex, time, distance, line);
    }

    fclose(file);
    printf("총 %d개의 역을 불러왔습니다.\n", stationCount);
}

// ---------------------- 기능 구현 ----------------------

void printStations() {
    printf("\n--- 지하철 역 목록 ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d - %s\n", i, stations[i].name);
    }
}

void findPath(const char* startName, const char* endName, int mode) {
    int start = getStationIndexByName(startName);
    int end = getStationIndexByName(endName);
    if (start == -1 || end == -1) {
        printf("입력한 역이 존재하지 않습니다.\n");
        return;
    }

    float cost[MAX_STATIONS];
    float dist[MAX_STATIONS];
    int prev[MAX_STATIONS];
    int visited[MAX_STATIONS] = { 0 };
    int prevLine[MAX_STATIONS];

    for (int i = 0; i < stationCount; i++) {
        cost[i] = INT_MAX;
        prev[i] = -1;
        prevLine[i] = 0;
        dist[i] = 0.0f;
    }

    cost[start] = 0;

    for (int i = 0; i < stationCount; i++) {
        float minCost = INT_MAX;
        int u = -1;
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
            int v = e->destIndex;
            float weight = (mode == 1) ? e->time : (mode == 2) ? e->distance : (float)calculateFare(dist[u] + e->distance);
            if (prevLine[u] != 0 && prevLine[u] != e->line)
                weight += TRANSFER_PENALTY;

            if (!visited[v] && cost[u] + weight < cost[v]) {
                cost[v] = cost[u] + weight;
                prev[v] = u;
                prevLine[v] = e->line;
                dist[v] = dist[u] + e->distance;
            }
            e = e->next;
        }
    }

    if (cost[end] == INT_MAX) {
        printf("경로를 찾을 수 없습니다.\n");
        return;
    }

    printf("경로: ");
    int path[MAX_STATIONS], count = 0;
    for (int v = end; v != -1; v = prev[v]) path[count++] = v;
    int lastLine = prevLine[path[count - 1]];
    for (int i = count - 1; i >= 0; i--) {
        int curr = path[i];
        printf("%s", stations[curr].name);
        if (i != 0) {
            int next = path[i - 1];
            int edgeLine = prevLine[next];
            if (edgeLine != lastLine) {
                printf(" (환승: %d호선)", edgeLine);
                lastLine = edgeLine;
            }
            printf(" -> ");
        }
    }
    printf("\n");

    float totalDist = dist[end];
    if (mode == 1)
        printf("총 소요 시간: %.1f 분, 거리: %.1f km\n", cost[end], totalDist);
    else if (mode == 2)
        printf("총 거리: %.1f km\n", cost[end]);
    else if (mode == 3)
        printf("총 거리: %.1f km, 총 요금: %d원\n", totalDist, calculateFare(totalDist));
}

void addLineInteractive() {
    char from[MAX_STATION_NAME], to[MAX_STATION_NAME];
    float distance, time;
    int line;
    printf("출발역 이름: "); fgets(from, sizeof(from), stdin); trim(from);
    printf("도착역 이름: "); fgets(to, sizeof(to), stdin); trim(to);
    printf("거리 (km): "); scanf("%f", &distance);
    printf("시간 (분): "); scanf("%f", &time);
    printf("호선 번호: "); scanf("%d", &line);
    while (getchar() != '\n');

    int fromIdx = getStationIndexByName(from);
    if (fromIdx == -1) {
        memset(&stations[stationCount], 0, sizeof(Station));
        strncpy(stations[stationCount].name, from, MAX_STATION_NAME - 1);
        fromIdx = stationCount++;
    }

    int toIdx = getStationIndexByName(to);
    if (toIdx == -1) {
        memset(&stations[stationCount], 0, sizeof(Station));
        strncpy(stations[stationCount].name, to, MAX_STATION_NAME - 1);
        toIdx = stationCount++;
    }

    addEdge(fromIdx, toIdx, time, distance, line);
    addEdge(toIdx, fromIdx, time, distance, line);
    appendToCSV("subway_line.csv", line, from, to, distance, time);
    printf("호선이 추가되었습니다.\n");
}
void deleteLineInteractive() {
    int targetLine;
    printf("삭제할 호선 번호: ");
    if (scanf("%d", &targetLine) != 1) {
        printf("잘못된 입력입니다.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    int deletedCount = 0;

    // 각 역에 연결된 간선 중에서 해당 호선인 간선을 제거
    for (int i = 0; i < stationCount; i++) {
        SubwayEdge** edgePtr = &stations[i].edge;
        while (*edgePtr) {
            if ((*edgePtr)->line == targetLine) {
                SubwayEdge* temp = *edgePtr;
                *edgePtr = (*edgePtr)->next;
                free(temp);
                deletedCount++;
            }
            else {
                edgePtr = &(*edgePtr)->next;
            }
        }
    }

    if (deletedCount > 0) {
        printf("%d호선의 간선 %d개가 삭제되었습니다.\n", targetLine, deletedCount);
    }
    else {
        printf("해당 호선은 존재하지 않거나 연결된 간선이 없습니다.\n");
    }
}

void addStationInteractive() {
    char name[MAX_STATION_NAME];
    printf("추가할 역 이름: ");
    fgets(name, sizeof(name), stdin);
    trim(name);

    if (getStationIndexByName(name) != -1) {
        printf("이미 존재하는 역입니다.\n");
        return;
    }

    if (stationCount >= MAX_STATIONS) {
        printf("더 이상 역을 추가할 수 없습니다.\n");
        return;
    }

    memset(&stations[stationCount], 0, sizeof(Station));
    strncpy(stations[stationCount].name, name, MAX_STATION_NAME - 1);
    stationCount++;

    printf("역 '%s'가 추가되었습니다.\n", name);
}

void deleteStationInteractive() {
    char name[MAX_STATION_NAME];
    printf("삭제할 역 이름: ");
    fgets(name, sizeof(name), stdin);
    trim(name);

    int target = getStationIndexByName(name);
    if (target == -1) {
        printf("해당 역은 존재하지 않습니다.\n");
        return;
    }

    // 다른 역들에 연결된 해당 역으로의 간선 삭제
    for (int i = 0; i < stationCount; i++) {
        if (i == target) continue;
        SubwayEdge** edgePtr = &stations[i].edge;
        while (*edgePtr) {
            if ((*edgePtr)->destIndex == target) {
                SubwayEdge* temp = *edgePtr;
                *edgePtr = (*edgePtr)->next;
                free(temp);
            }
            else {
                // 삭제된 역 이후 인덱스면 하나씩 감소시켜야 함
                if ((*edgePtr)->destIndex > target) {
                    (*edgePtr)->destIndex--;
                }
                edgePtr = &(*edgePtr)->next;
            }
        }
    }

    // 본인 간선 해제
    SubwayEdge* edge = stations[target].edge;
    while (edge) {
        SubwayEdge* temp = edge;
        edge = edge->next;
        free(temp);
    }

    // 역 배열에서 삭제 
    for (int i = target; i < stationCount - 1; i++) {
        stations[i] = stations[i + 1];
    }
    stationCount--;

    printf("역 '%s'가 삭제되었습니다.\n", name);
}

// ---------------------- 메인 함수 ----------------------

int main() {
    int choice;

    while (1) {
        system("cls");  
        printf("\n\n\t\t지하철 길찾기 프로그램\n\n");
        printf("1. CSV 파일 불러오기\n");
        printf("2. 역 목록 출력\n");
        printf("3. 길찾기\n");
        printf("4. 호선 추가(기존역 가능, 새로운 역 만들기 가능)\n");
        printf("5. 호선 삭제\n");
        printf("6. 역 추가\n");
        printf("7. 역 삭제\n");
        printf("0. 프로그램 종료\n");
        printf("\n메뉴 선택 : ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
        case 1:
            loadCSV("subway_line.csv");
            break;
        case 2:
            printStations();
            break;
        case 3: {
            time_t now;
            struct tm* local;
            time(&now);
            local = localtime(&now);

            if (local->tm_hour >= 1 && local->tm_hour < 5) {
                printf("\n현재 시각은 %02d:%02d입니다. 지하철 운행 시간이 아닙니다.\n", local->tm_hour, local->tm_min);
                break;
            }

            char start[MAX_STATION_NAME], end[MAX_STATION_NAME];
            int mode;
            printf("출발역 이름: "); fgets(start, sizeof(start), stdin); trim(start);
            printf("도착역 이름: "); fgets(end, sizeof(end), stdin); trim(end);
            printf("1. 최소 시간 경로\n2. 최단 거리 경로\n3. 최소 요금 경로\n선택: ");
            if (scanf("%d", &mode) != 1 || mode < 1 || mode > 3) {
                printf("잘못된 입력입니다.\n");
                while (getchar() != '\n');
                break;
            }
            while (getchar() != '\n');
            findPath(start, end, mode);
            break;
        }
        case 4:
            addLineInteractive();
            break;
        case 5:
            deleteLineInteractive();
            break;
        case 6:
            addStationInteractive();
            break;
        case 7:
            deleteStationInteractive();
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
