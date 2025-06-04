#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#pragma warning(disable : 4996)

#define MAX_STATION_NAME 100
#define MAX_STATIONS 1000
#define TRANSFER_PENALTY 3  // �߰� ȯ�� 3�� �ð� �г�Ƽ

//---------------- ����ü ���� ----------------
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

//---------------- ���� ���� ----------------
Station stations[MAX_STATIONS];
int stationCount = 0;

//----------------  �� �̸����� �ε��� ã�� ----------------
int getStationIndexByName(const char* name) {
    for (int i = 0; i < stationCount; i++) {
        if (strcmp(stations[i].name, name) == 0)
            return i;
    }
    return -1;
}

//---------------- ���� �߰� �Լ� ----------------
void addEdge(int from, int to, int time, int distance, char line) {
    SubwayEdge* edge = (SubwayEdge*)malloc(sizeof(SubwayEdge));
    edge->destIndext = to;
    edge->time = time;
    edge->distance = distance;
    edge->line = line;
    edge->next = stations[from].edge;
    stations[from].edge = edge;
}

//---------------- CSV ���� �ε� ----------------
void loadCSV(const char* filename) {
    FILE* file = fopen("subway_line.csv", "r");
    if (!file) {
        printf("CSV ������ �� �� �����ϴ�: %s\n", filename);
        return;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // ��� �ǳʶ�

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
        addEdge(toIndex, fromIndex, time, distance, line); // �����
    }

    fclose(file);
    printf("�� %d���� ���� �ҷ��Խ��ϴ�.\n", stationCount);
}

//---------------- �� ��� ��� ----------------
void printStations() {
    printf("\n--- ����ö �� ��� ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%d - %s\n", i, stations[i].name);
    }
}

//---------------- ��� Ž�� (���ͽ�Ʈ�� ���) ----------------
void findPath(const char* startName, const char* endName, int mode) {
    int start = getStationIndexByName(startName);
    int end = getStationIndexByName(endName);
    if (start == -1 || end == -1) {
        printf("�Է��� ���� �������� �ʽ��ϴ�.\n");
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
        printf("��θ� ã�� �� �����ϴ�.\n");
        return;
    }

    // ��� ���
    printf("\n�ּ� %s: %d\n", (mode == 1) ? "�Ÿ�(Km)" : "�ð�(��)", cost[end]);
    printf("���: ");
    int path[MAX_STATIONS], count = 0;

    for (int v = end; v != -1; v = prev[v])
        path[count++] = v;

    for (int i = count - 1; i >= 0; i--) {
        printf("%s", stations[path[i]].name);
        if (i != 0) printf("->");
    }
    printf("\n");
}

//---------------- ���� �Լ� ----------------
int main() {
    int choice;

    while (1) {
        system("cls");  // Windows ȯ�� ����
        printf("\n\n\t\t����ö ��ã�� ���α׷�\n\n");
        printf("1. CSV ���� �ҷ�����\n");
        printf("2. �� ��� ���\n");
        printf("3. ��ã��\n");
        printf("0. ���α׷� ����\n");
        printf("\n�޴� ���� : ");
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

            printf("��߿� �̸�: ");
            fgets(start, sizeof(start), stdin);
            start[strcspn(start, "\n")] = '\0';

            printf("������ �̸�: ");
            fgets(end, sizeof(end), stdin);
            end[strcspn(end, "\n")] = '\0';

            printf("1. �ִ� �ð� ���\n");
            printf("2. �ִ� �Ÿ� ���\n");
            printf("����: ");
            scanf("%d", &mode);
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
        system("pause");  // Windows ����
    }

    return 0;
}
