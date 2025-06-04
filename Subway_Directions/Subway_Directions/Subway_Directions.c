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
        printf("CSV ������ �� �� �����ϴ�: %s\n", filename);
        return;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // ��� �ǳʶ�

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
            printf("�ִ� �� ���� �ʰ��߽��ϴ�.\n");
            break;
        }
    }

    fclose(file);
    printf("�� %d���� ���� �ҷ��Խ��ϴ�.\n", stationCount);
}

void printStations() {
    printf("\n--- ����ö �� ��� ---\n");
    for (int i = 0; i < stationCount; i++) {
        printf("%dȣ�� - %s\n", stations[i].line, stations[i].name);
    }
}

int main() {
    int choice;

    while (1) {
        system("cls");
        printf("\n\n\t\t����ö ��ã�� ���α׷�\n\n");
        printf("1. CSV ���� �ҷ�����\n");
        printf("2. �� ��� ���\n");
        printf("3. ��ã�� (�̱���)\n");
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
        case 3:
            printf("��ã�� ����� ���� �������� �ʾҽ��ϴ�.\n");
            break;
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
