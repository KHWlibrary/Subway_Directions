/*
*  지하철 길찾기 프로그램
*  서울 주요 역 1~4호선 (Seoul's major stations on lines 1-4)
* 1. 최소 시간 경로 (Minimum time path)
* 2. 최단 거리 경로 (Shortest distance path)
* 3. 최소 요금 경로 (Minimum fare path)
* 를 구현하였습니다. 
* 추가로 새벽 1시부터 5시 사이에 프로그램을 실행하면 작동되지 않고 
* 현재 시간을 알려주고 지하철 운행 시간이 아님을 알려주었습니다.
* 
* 최단경로 알고리즘으로는 Dijkstra를 사용하였습니다.
* 노선 추가, 노선 삭제, 역 추가, 역 삭제는 CSV 파일에서 입력 및 삭제할 수 있습니다.
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

// 지하철 간선(경로) 정보 저장 구조체
typedef struct SubwayEdge {
	int destIndex;
	float time;
	float distance;
	int line;
	struct SubwayEdge* next;
} SubwayEdge;

// 지하철 역 정보를 저장하는 구조체
typedef struct Station {
	char name[MAX_STATION_NAME];
	SubwayEdge* edge;
} Station;

// 전역 역 목록 배열과 현재 역 수
Station stations[MAX_STATIONS];
int stationCount = 0;

// 문자열 앞뒤 공백 제거 함수
void trim(char* str) {
	str[strcspn(str, "\r\n")] = 0;
	char* start = str;
	while (isspace((unsigned char)*start)) start++;

	char* end = start + strlen(start) - 1;
	while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

	if (start != str) memmove(str, start, strlen(start) + 1);
}

// 역 이름으로 인덱스를 찾아주는 함수
int getStationIndexByName(const char* name) {
	for (int i = 0; i < stationCount; i++) {
		if (strcmp(stations[i].name, name) == 0)
			return i;
	}
	return -1;
}

// 간선 추가 함수
void addEdge(int from, int to, float time, float distance, int line) {
	SubwayEdge* edge = (SubwayEdge*)malloc(sizeof(SubwayEdge));
	edge->destIndex = to;
	edge->time = time;
	edge->distance = distance;
	edge->line = line;
	edge->next = stations[from].edge;
	stations[from].edge = edge;
}

// CSV 파일 불러오기
void loadCSV(const char* filename) {
	FILE* file = fopen("subway_line.csv", "r");
	if (!file) {
		printf("CSV 파일을 열 수 없습니다: %s\n", filename); // Original: "CSV 파일을 열 수 없습니다: %s\n" -> Good
		return;
	}
	// 첫 줄(헤더) 건너뛰기
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
	printf("총 %d개의 역을 불러왔습니다.\n", stationCount); // Original: "총 %d개의 역을 불러왔습니다.\n" -> Good
}

// 거리 기반으로 요금 계산하는 함수
int calculateFare(float distance) {
	int fare = 1400;
	if (distance > 10.0f) {
		int extra = distance - 10.0f;
		int blocks = (int)((extra + 0.0001f + 4.999f) / 5.0f);
		fare += blocks * 100;
	}
	return fare;
}

// 지하철 모든 역 출력
void printStations() {
	printf("\n--- 지하철 역 목록 ---\n"); // Original: "\n--- 지하철 역 목록 ---\n" -> Good
	for (int i = 0; i < stationCount; i++) {
		printf("%d - %s\n", i, stations[i].name);
	}
}

// Dijkstra 알고리즘으로 경로 탐색 함수
void findPath(const char* startName, const char* endName, int mode) {
	int start = getStationIndexByName(startName);
	int end = getStationIndexByName(endName);
	if (start == -1 || end == -1) {
		printf("입력한 역이 존재하지 않습니다.\n"); // Original: "입력한 역이 존재하지 않습니다.\n" -> Good
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
			else if (mode == 3) {   // 최소 요금
				float newDistance = dist[u] + e->distance;
				int newFare = calculateFare(newDistance);
				weight = (float)newFare;
			}

			// 환승 시 가중 패널티 적용
			if (prevLine[u] != 0 && prevLine[u] != e->line)
				weight += TRANSFER_PENALTY;

			// 최적 경로 갱신
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
		printf("경로를 찾을 수 없습니다.\n"); // Original: "경로를 찾을 수 없습니다.\n" -> Good
		return;
	}

	printf("경로: "); // Original: "경로: " -> Good
	int path[MAX_STATIONS], count = 0;
	for (int v = end; v != -1; v = prev[v])
		path[count++] = v;
	int lastLine = prevLine[path[count - 1]];
	for (int i = count - 1; i >= 0; i--) {
		int curr = path[i];
		printf("%s", stations[curr].name);
		if (i != 0) {
			int next = path[i - 1];
			int edgeLine = prevLine[next];
			if (edgeLine != lastLine) {
				printf(" (환승: %d호선)", edgeLine); // Original: " (환승: %d호선)" -> Good
				lastLine = edgeLine;
			}
			printf(" -> ");
		}
	}
	printf("\n");

	float totalDist = dist[end];

	if (mode == 1) {
		printf("총 소요 시간: %.1f분\n", cost[end]); // Original: "총 소요 시간: %.1f 분\n" -> "분" without space
		printf("총 거리: %.1fkm\n", totalDist); // Original: "총 거리: %.1f km\n" -> "km" without space
	}
	else if (mode == 2) {
		printf("총 거리: %.1fkm\n", cost[end]); // Original: "총 거리: %.1f km\n" -> "km" without space
	}
	else if (mode == 3) {
		int totalFare = calculateFare(totalDist);
		printf("총 거리: %.1fkm, 총 요금: %d원\n", totalDist, totalFare); // Original: "총 거리: %.1f km, 총 요금: %d원\n" -> "km" without space
	}
}
// 역 추가 함수
void addStationInteractive() {
	char name[MAX_STATION_NAME];
	printf("추가할 역 이름: "); // Original: "추가할 역 이름: " -> Good
	fgets(name, sizeof(name), stdin);
	trim(name);
	if (getStationIndexByName(name) != -1) {
		printf("이미 존재하는 역입니다.\n"); // Original: "이미 존재하는 역입니다.\n" -> Good
		return;
	}
	if (stationCount >= MAX_STATIONS) {
		printf("더 이상 역을 추가할 수 없습니다.\n"); // Original: "더 이상 역을 추가할 수 없습니다.\n" -> Good
		return;
	}
	memset(&stations[stationCount], 0, sizeof(Station));
	strncpy(stations[stationCount].name, name, MAX_STATION_NAME - 1);
	stationCount++;
	appendStationToCSV("subway_line.csv", name);
	printf("역 '%s'가 추가되었습니다.\n", name); // Original: "역 '%s'가 추가되었습니다.\n" -> Good
}

// 간선 추가 + CSV 저장
void addLineInteractive() {
	char from[MAX_STATION_NAME], to[MAX_STATION_NAME];
	float distance, time;
	int line;
	printf("출발역 이름: "); // Original: "출발역 이름: " -> Good
	fgets(from, sizeof(from), stdin); trim(from);
	printf("도착역 이름: "); // Original: "도착역 이름: " -> Good
	fgets(to, sizeof(to), stdin); trim(to);
	printf("거리 (km): "); // Original: "거리 (km): " -> Good
	scanf("%f", &distance);
	printf("시간 (분): "); // Original: "시간 (분): " -> Good
	scanf("%f", &time);
	printf("호선 번호: "); // Original: "호선 번호: " -> Good
	scanf("%d", &line);
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
	appendToCSV("subway_line.csv", line, from, to, distance, time); // CSV 저장
	printf("호선이 추가되었습니다.\n"); // Original: "호선이 추가되었습니다.\n" -> Good
}
// CSV에 노선 추가 (Note: This function seems to be for adding a new station entry, not a line between two stations)
void appendStationToCSV(const char* filename, const char* name) {
	FILE* file = fopen(filename, "a");
	if (!file) {
		printf("CSV 파일을 열 수 없습니다: %s\n", filename); // Original: "CSV 파일을 열 수 없습니다: %s\n" -> Good
		return;
	}
	// 빈 호선으로 저장하거나, 고정 포맷 사용
	fprintf(file, "0,%s,%s,0.0,0.0\n", name, name);
	fclose(file);
}
// CSV에 노선, 역, 거리, 시간 추가
void appendToCSV(const char* filename, int line, const char* from, const char* to, float distance, float time) {
	FILE* file = fopen(filename, "a");
	if (!file) {
		printf("CSV 파일을 열 수 없습니다: %s\n", filename); // Original: "CSV 파일을 열 수 없습니다: %s\n" -> Good
		return;
	}
	fprintf(file, "%d,%s,%s,%.1f,%.1f\n", line, from, to, distance, time);
	fclose(file);
}


// 역 삭제
void deleteStationInteractive() {
	char name[MAX_STATION_NAME];
	printf("삭제할 역 이름: "); // Original: "삭제할 역 이름: " -> Good
	fgets(name, sizeof(name), stdin);
	trim(name);
	int target = getStationIndexByName(name);
	if (target == -1) {
		printf("해당 역은 존재하지 않습니다.\n"); // Original: "해당 역은 존재하지 않습니다.\n" -> Good
		return;
	}
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
	for (int i = 0; i < stationCount; i++) {
		SubwayEdge* e = stations[i].edge;
		while (e) {
			if (e->destIndex > target) {
				e->destIndex--;
			}
			e = e->next;
		}
	}
	printf("역 '%s'가 삭제되었습니다.\n", name); // Original: "역 '%s'가 삭제되었습니다.\n" -> Good
}


int main() {
	int choice;

	while (1) {
		system("cls");
		printf("\n\n\t\t지하철 길찾기 프로그램\n\n"); // Original: "\n\n\t\t지하철 길찾기 프로그램\n\n" -> Good
		printf("1. CSV 파일 불러오기\n"); // Original: "1. CSV 파일 불러오기\n" -> Good
		printf("2. 역 목록 출력\n"); // Original: "2. 역 목록 출력\n" -> Good
		printf("3. 길찾기\n"); // Original: "3. 길찾기\n" -> Good
		printf("4. 노선 추가\n"); // Original: "4. 호선 추가\n" -> '호선' is usually for the line itself, '노선' for a segment.
		printf("5. 역 삭제\n"); // Original: "5. 역 삭제\n" -> Good
		printf("0. 프로그램 종료\n"); // Original: "0. 프로그램 종료\n" -> Good
		printf("\n메뉴 선택 : "); // Original: "\n메뉴 선택 : " -> Good
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
				printf("\n현재 시각은 %02d:%02d입니다. 지하철 운행 시간이 아닙니다.\n", local->tm_hour, local->tm_min); // Original: "현재 시각은 %02d:%02d입니다. 지하철 운행 시간이 아닙니다.\n" -> Good
				break;
			}

			char start[MAX_STATION_NAME], end[MAX_STATION_NAME];
			int mode;

			printf("출발역 이름: "); // Original: "출발역 이름: " -> Good
			fgets(start, sizeof(start), stdin);
			trim(start);

			printf("도착역 이름: "); // Original: "도착역 이름: " -> Good
			fgets(end, sizeof(end), stdin);
			trim(end);

			printf("1. 최소 시간 경로\n"); // Original: "1. 최소 시간 경로\n" -> Good
			printf("2. 최단 거리 경로\n"); // Original: "2. 최단 거리 경로\n" -> Good
			printf("3. 최소 요금 경로\n");  // Original: "3. 최소 요금 경로\n" -> Good
			printf("선택: "); // Original: "선택: " -> Good
			if (scanf("%d", &mode) != 1 || (mode < 1 || mode > 3)) {
				printf("잘못된 입력입니다.\n"); // Original: "잘못된 입력입니다.\n" -> Good
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
			deleteStationInteractive();
			break;
		case 0:
			exit(0);
		default:
			printf("잘못된 선택입니다.\n"); // Original: "잘못된 선택입니다.\n" -> Good
		}

		printf("\n\n\t\t");
		system("pause");
	}

	return 0;
}