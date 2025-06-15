/*
*  지하철 길찾기 프로그램
*  서울 주요 역 1~4호선
* 1. 최소 시간 경로
* 2. 최단 거리 경로
* 3. 최소 요금 경로
* 4. 역/호선 추가
* 5. 호선 삭제
* 6. 역 삭제
* 0. 프로그램 종료
* 추가로 새벽 1시부터 5시 사이에 프로그램을 실행하면 작동 되지 않고
* 현재 시간을 알려주고 지하철 운행시간이 아님을 알려주었습니다.
*
* 최단경로알고리즘으로는 Dijkstar를 사용하였습니다
*/

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

// 문자열 앞뒤 공백 제거함수
void trim(char* str) {
    str[strcspn(str, "\r\n")] = 0;
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) *end-- = '\0';
    if (start != str) memmove(str, start, strlen(start) + 1);
}

//역 이름으로 찾는 함수
int getStationIndexByName(const char* name) {
    for (int i = 0; i < stationCount; i++) {
        if (strcmp(stations[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 간선 추가 
void addEdge(int from, int to, float time, float distance, int line) {
    SubwayEdge* edge = (SubwayEdge*)malloc(sizeof(SubwayEdge));
    edge->destIndex = to;
    edge->time = time;
    edge->distance = distance;
    edge->line = line;
    edge->next = stations[from].edge;
    stations[from].edge = edge;
}
// 요금 계산 함수
int calculateFare(float distance) {
    int fare = 1400;
    if (distance > 10.0f) {
        int extra = distance - 10.0f;
        int blocks = (int)((extra + 4.999f) / 5.0f);
        fare += blocks * 100;
    }
    return fare;
}
// csv에 간선정보 추가하기
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

// 지하철 전체역 출력
void printStations() {
    printf("\n--- 지하철 역 목록 ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d - %s\n", i, stations[i].name);
    }
}

// 길찾기 프로그램
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
        printf("소요 시간: %.1f 분, 거리: %.1f km\n", cost[end], totalDist);
    else if (mode == 2)
        printf("거리: %.1f km\n", totalDist);
    else if (mode == 3)
        printf("거리: %.1f km, 총 요금: %d원\n", totalDist, calculateFare(totalDist));
}

// 역/호선 추가 함수
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

// 호선 삭제 함수
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

    // 메모리에서 간선 제거
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

    // CSV에서 해당 호선 제거
    FILE* original = fopen("subway_line.csv", "r");
    FILE* temp = fopen("temp.csv", "w");
    if (!original || !temp) {
        printf("파일 처리 오류.\n");
        if (original) fclose(original);
        if (temp) fclose(temp);
        return;
    }

    char buffer[256];
    // 헤더 복사
    if (fgets(buffer, sizeof(buffer), original)) {
        fputs(buffer, temp);
    }

    while (fgets(buffer, sizeof(buffer), original)) {
        int line;
        char tempBuf[256];
        strncpy(tempBuf, buffer, sizeof(tempBuf));
        char* token = strtok(tempBuf, ",");
        if (!token) continue;
        line = atoi(token);
        if (line != targetLine) {
            fputs(buffer, temp);  // 다른 호선만 복사
        }
    }

    fclose(original);
    fclose(temp);

    // 원본 파일 덮어쓰기
    remove("subway_line.csv");
    rename("temp.csv", "subway_line.csv");

    if (deletedCount > 0) {
        printf("%d호선의 간선 %d개가 제거되었습니다.\n", targetLine, deletedCount);
    }
    else {
        printf("해당 호선은 존재하지 않거나 연결된 간선이 없습니다.\n");
    }
}

// 역 삭제
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

    // 🔧 삭제 전 연결된 역들 파악해서 자동 연결 시도
    int linkedIndices[10];
    float distances[10];
    float times[10];
    int lines[10];
    int linkCount = 0;

    SubwayEdge* e = stations[target].edge;
    while (e && linkCount < 10) {
        linkedIndices[linkCount] = e->destIndex;
        distances[linkCount] = e->distance;
        times[linkCount] = e->time;
        lines[linkCount] = e->line;
        linkCount++;
        e = e->next;
    }

    // 연결된 두 역 A, C가 있고 같은 호선이면 자동 연결
    if (linkCount == 2) {
        int a = linkedIndices[0];
        int c = linkedIndices[1];
        float totalDist = distances[0] + distances[1];
        float totalTime = times[0] + times[1];

        if (lines[0] == lines[1]) {
            addEdge(a, c, totalTime, totalDist, lines[0]);
            addEdge(c, a, totalTime, totalDist, lines[0]);
            appendToCSV("subway_line.csv", lines[0], stations[a].name, stations[c].name, totalDist, totalTime);
            printf("'%s' 삭제로 인해 '%s' ↔ '%s' 간선이 자동 추가되었습니다 (%.1fkm, %.1f분).\n",
                stations[target].name, stations[a].name, stations[c].name, totalDist, totalTime);
        }
    }

    // 1. 메모리에서 삭제 작업
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
                if ((*edgePtr)->destIndex > target) {
                    (*edgePtr)->destIndex--;
                }
                edgePtr = &(*edgePtr)->next;
            }
        }
    }

    SubwayEdge* edge = stations[target].edge;
    while (edge) {
        SubwayEdge* temp = edge;
        edge = edge->next;
        free(temp);
    }

    for (int i = target; i < stationCount - 1; i++) {
        stations[i] = stations[i + 1];
    }
    stationCount--;

    // 2. CSV에서 삭제
    FILE* original = fopen("subway_line.csv", "r");
    FILE* temp = fopen("temp.csv", "w");
    if (!original || !temp) {
        printf("CSV 파일 열기 실패\n");
        if (original) fclose(original);
        if (temp) fclose(temp);
        return;
    }

    char buffer[512];
    int deletedCSV = 0;
    int isFirstLine = 1;

    while (fgets(buffer, sizeof(buffer), original)) {
        if (isFirstLine) {
            fputs(buffer, temp);
            isFirstLine = 0;
            continue;
        }

        char bufCopy[512];
        strncpy(bufCopy, buffer, sizeof(bufCopy));
        bufCopy[sizeof(bufCopy) - 1] = '\0';

        char* token = strtok(bufCopy, ","); // 호선
        if (!token) continue;

        token = strtok(NULL, ","); // 출발역
        if (!token) continue;
        char from[MAX_STATION_NAME];
        strncpy(from, token, MAX_STATION_NAME); trim(from);

        token = strtok(NULL, ","); // 도착역
        if (!token) continue;
        char to[MAX_STATION_NAME];
        strncpy(to, token, MAX_STATION_NAME); trim(to);

        if (strcmp(from, name) == 0 || strcmp(to, name) == 0) {
            deletedCSV++;
            continue;
        }

        fputs(buffer, temp);
    }

    fclose(original);
    fclose(temp);
    remove("subway_line.csv");
    rename("temp.csv", "subway_line.csv");

    printf("역 '%s' 삭제되었습니다.\n", name, deletedCSV);
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
        printf("4. 역/호선 추가(기존역 가능, 새로운 역 만들기 가능)\n");
        printf("5. 호선 삭제\n");
        printf("6. 역 삭제\n");
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
