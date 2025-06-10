/*
*  ����ö ��ã�� ���α׷�
*  ���� �ֿ� �� 1~4ȣ��
* 1. �ּ� �ð� ���
* 2. �ִ� �Ÿ� ���
* 3. �ּ� ��� ���
* �� �����Ͽ����ϴ�. 
* �߰��� ���� 1�ú��� 5�� ���̿� ���α׷��� �����ϸ� �۵� ���� �ʰ� 
* ���� �ð��� �˷��ְ� ����ö ����ð��� �ƴ��� �˷��־����ϴ�.
* 
* �ִܰ�ξ˰������δ� Dijkstar�� ����Ͽ����ϴ�
* �뼱 �߰�, �뼱 ����, �� �߰�, �� ������ csv ���Ͽ��� �Է� ���� �� �� �ֽ��ϴ�.
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

void loadCSV(const char* filename) {
    FILE* file = fopen("subway_line.csv", "r");
    if (!file) {
        printf("CSV ������ �� �� �����ϴ�: %s\n", filename);
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

        trim(name1);
        trim(name2);

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
    printf("�� %d���� ���� �ҷ��Խ��ϴ�.\n", stationCount);
}

int calculateFare(float distance) {
    int fare = 1400;
    if (distance > 10.0f) {
        int extra = distance - 10.0f;
        int blocks = (int)((extra + 0.0001f + 4.999f) / 5.0f);
        fare += blocks * 100;
    }
    return fare;
}

void printStations() {
    printf("\n--- ����ö �� ��� ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d - %s\n", i, stations[i].name);
    }
}

void findPath(const char* startName, const char* endName, int mode) {
    int start = getStationIndexByName(startName);
    int end = getStationIndexByName(endName);
    if (start == -1 || end == -1) {
        printf("�Է��� ���� �������� �ʽ��ϴ�.\n");
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
            float weight;

            if (mode == 1) {
                weight = e->time;
            }
            else if (mode == 2) {
                weight = e->distance;
            }
            else if (mode == 3) {
                float newDistance = dist[u] + e->distance;
                int newFare = calculateFare(newDistance);
                weight = (float)newFare;
            }

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
        printf("��θ� ã�� �� �����ϴ�.\n");
        return;
    }

    printf("���: ");
    int path[MAX_STATIONS], count = 0;
    for (int v = end; v != -1; v = prev[v])
        path[count++] = v;
    for (int i = count - 1; i >= 0; i--) {
        printf("%s", stations[path[i]].name);
        if (i != 0) printf("->");
    }
    printf("\n");

    float totalDist = dist[end];

    if (mode == 1) {
        printf("�� �ҿ� �ð�: %.1f ��\n", cost[end]);
        printf("�� �Ÿ�: %.1f km\n", totalDist);
    }
    else if (mode == 2) {
        printf("�� �Ÿ�: %.1f km\n", cost[end]);
    }
    else if (mode == 3) {
        int totalFare = calculateFare(totalDist);
        printf("�� �Ÿ�: %.1f km, �� ���: %d��\n", totalDist, totalFare);
    }
}

int main() {
    int choice;

    while (1) {
        system("cls");
        printf("\n\n\t\t����ö ��ã�� ���α׷�\n\n");
        printf("1. CSV ���� �ҷ�����\n");
        printf("2. �� ��� ���\n");
        printf("3. ��ã��\n");
        printf("0. ���α׷� ����\n");
        printf("\n�޴� ���� : ");
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
                printf("\n���� �ð��� %02d:%02d�Դϴ�. ����ö ���� �ð��� �ƴմϴ�.\n", local->tm_hour, local->tm_min);
                break;
            }

            char start[MAX_STATION_NAME], end[MAX_STATION_NAME];
            int mode;

            printf("��߿� �̸�: ");
            fgets(start, sizeof(start), stdin);
            trim(start);

            printf("������ �̸�: ");
            fgets(end, sizeof(end), stdin);
            trim(end);

            printf("1. �ּ� �ð� ���\n");
            printf("2. �ִ� �Ÿ� ���\n");
            printf("3. �ּ� ��� ���\n");  
            printf("����: ");
            if (scanf("%d", &mode) != 1 || (mode < 1 || mode > 3)) {  
                printf("�߸��� �Է��Դϴ�.\n");
                while (getchar() != '\n');
                break;
            }
            while (getchar() != '\n');

            findPath(start, end, mode);
            break;
        }
        case 0:
            exit(0);
        default:
            printf("�߸��� �����Դϴ�.\n");
        }

        printf("\n\n\t\t");
        system("pause");
    }

    return 0;
}
