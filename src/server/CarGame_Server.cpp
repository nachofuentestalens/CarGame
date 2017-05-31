//Programmed by Nacho Fuentes Talens
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "winsock2.h"
#include <time.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

struct Timer {
	LARGE_INTEGER  Frequency;
	LARGE_INTEGER  StartingTime, EndingTime, ElapsedMicroseconds;
};

//Update Timer
Timer update_timer;
unsigned int uiTimeStep = 16;

static void StartTimer(Timer &timer) {
	QueryPerformanceFrequency(&timer.Frequency);
	QueryPerformanceCounter(&timer.StartingTime);
}

static double GetTime(Timer &timer) {
	QueryPerformanceCounter(&timer.EndingTime);
	timer.ElapsedMicroseconds.QuadPart = timer.EndingTime.QuadPart - timer.StartingTime.QuadPart;
	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//
	timer.ElapsedMicroseconds.QuadPart *= 1000000;
	double time = (double)(timer.ElapsedMicroseconds.QuadPart /= timer.Frequency.QuadPart);
	return time / 1000;
}

struct position {
	int X;
	int Y;
};

struct client {
	int id;
	sockaddr_in ip;
	position pos;
	std::string name;
	std::string password;
	int lives;
	int team;
	int rotation = 0;
};

//winsock2 data
WSADATA wsa;
SOCKET udp_socket;
//SOCKET tcp_socket;
struct sockaddr_in ip;
sockaddr_in ipc;
int size = sizeof(ip);
int latestId = 0;
std::vector<client> identified_clients;
std::vector<client> loged_clients;
//buffer
int bytes = 0;
const position kInitialPosition = { 100, 100 };
const int kInitialLives = 3;

int GetNextTeam() {
	int team1 = 0;
	int team2 = 0;
	int team3 = 0;
	for (int i = 0; i < loged_clients.size(); ++i) {
		if (loged_clients[i].team == 1) {
			++team1;
		}
		else {
			if (loged_clients[i].team == 2) {
				++team2;
			}
			else {
				++team3;
			}
		}
	}

	return team1 < team2 ? (team1 < team3 ? 1 : 3) : (team2 < team3 ? 2 : 3);
	return 1;
}

bool RegisterUser(std::string name, std::string password) {
	std::ofstream myfile("tmp.txt", std::ios::out | std::ios::app);
	myfile << std::endl << name << "  " << password << std::endl;
	myfile.close();
	return true;
}

bool UserExists(std::string name, std::string password) {
	std::string tmp_name, tmp_password;
	std::ifstream myfile("tmp.txt");
	bool found = false;
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			myfile >> tmp_name >> tmp_password;
			if (name == tmp_name && password == tmp_password)
			{
				found = true;
				break;
			}
		}
		myfile.close();
	}
	if (found) return true;
	return false;
}

void Communications() {
	char* ptr;
	char buffer[250];
	char answer[250];
	char newUser[250];
	char cID[10];
	memset(buffer, 0, 250);
	memset(answer, 0, 250);
	while (1) {
		bytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR*)&ipc, &size);
		if (bytes > 0) {
			//printf("Received Data");
			ptr = strtok(buffer, ":");
			if (strcmp(ptr, "RG_DISCOVER") == 0) {
				client tmp_client;
				tmp_client.id = latestId;
				tmp_client.ip = ipc;
				tmp_client.pos = kInitialPosition;
				tmp_client.lives = kInitialLives;
				strcpy(answer, "RG_IDENTIFY:");
				_itoa(tmp_client.id, cID, 10);
				strcat(answer, cID);
				sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&ipc, sizeof(ipc));
			}
			else {
				if (strcmp(ptr, "RG_LOGIN") == 0) {
					ptr = strtok(NULL, ":");
					std::string tmp_name(ptr);
					ptr = strtok(NULL, ":");
					std::string tmp_pass(ptr);
					ptr = strtok(NULL, ":");
					char cTeam[10];
					char cPosX[10];
					char cPosY[10];
					char cLives[10];
					memset(cID, 0, 10);
					memset(cTeam, 0, 10);
					memset(cPosX, 0, 10);
					memset(cPosY, 0, 10);
					memset(cLives, 0, 10);
					if (UserExists(tmp_name, tmp_pass)) {
						client tmp_client;
						tmp_client.id = latestId;
						_itoa(latestId, cID, 10);
						_itoa(GetNextTeam(), cTeam, 10);
						_itoa(kInitialPosition.X, cPosX, 10);
						_itoa(kInitialPosition.Y, cPosY, 10);
						_itoa(kInitialLives, cLives, 10);
						tmp_client.ip = ipc;
						tmp_client.pos = kInitialPosition;
						tmp_client.lives = kInitialLives;
						tmp_client.name = tmp_name;
						tmp_client.password = tmp_pass;
						tmp_client.team = GetNextTeam();
						loged_clients.push_back(tmp_client);
						strcpy(answer, "RG_LOGIN:OK:");
						strcat(answer, cID);
						strcat(answer, ":");
						strcat(answer, cTeam);
						strcat(answer, ":");
						strcat(answer, cPosX);
						strcat(answer, ":");
						strcat(answer, cPosY);
						strcat(answer, ":");
						strcat(answer, cLives);
						strcat(answer, ":");
						strcat(answer, tmp_name.c_str());
						memset(newUser, 0, 250);
						strcpy(newUser, "RG_NEWPLAYER:");
						strcat(newUser, answer + 12);
						for (unsigned int i = 0; i < loged_clients.size() - 1; ++i) {
							sendto(udp_socket, newUser, sizeof(newUser), 0, (SOCKADDR*)&loged_clients[i].ip, sizeof(loged_clients[i].ip));
						}
					}
					else {
						strcpy(answer, "RG_LOGIN:ERROR");
					}
					sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&ipc, sizeof(ipc));
					for (unsigned int j = 0; j < loged_clients.size() - 1; ++j) {
						memset(answer, 0, 250);
						memset(cID, 0, 10);
						memset(cTeam, 0, 10);
						memset(cPosX, 0, 10);
						memset(cPosY, 0, 10);
						memset(cLives, 0, 10);
						_itoa(loged_clients[j].id, cID, 10);
						_itoa(loged_clients[j].team, cTeam, 10);
						_itoa(loged_clients[j].pos.X, cPosX, 10);
						_itoa(loged_clients[j].pos.Y, cPosY, 10);
						_itoa(loged_clients[j].lives, cLives, 10);
						strcpy(answer, "RG_NEWPLAYER:");
						strcat(answer, cID);
						strcat(answer, ":");
						strcat(answer, cTeam);
						strcat(answer, ":");
						strcat(answer, cPosX);
						strcat(answer, ":");
						strcat(answer, cPosY);
						strcat(answer, ":");
						strcat(answer, cLives);
						strcat(answer, ":");
						strcat(answer, loged_clients[j].name.c_str());
						sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&ipc, sizeof(ipc));
					}
					latestId++;
				}
				else{
					if ((strcmp(ptr, "RG_REGISTER") == 0)) {
						ptr = strtok(NULL, ":");
						std::string tmp_name(ptr);
						ptr = strtok(NULL, ":");
						std::string tmp_pass(ptr);
						if (UserExists(tmp_name, tmp_pass)) {
							strcpy(answer, "RG_REGISTER:ERROR");
						}
						else {
							RegisterUser(tmp_name, tmp_pass);
							strcpy(answer, "RG_REGISTER:OK");
						}
						sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&ipc, sizeof(ipc));

					}
					else {
						if ((strcmp(ptr, "RG_UPDATE") == 0)) {
							ptr = strtok(NULL, ":");
							int tmp_ID = atoi(ptr);
							for (unsigned int j = 0; j < loged_clients.size(); ++j) {
								if (tmp_ID == loged_clients[j].id) {
									ptr = strtok(NULL, ":");
									loged_clients[j].pos.X = atoi(ptr);
									ptr = strtok(NULL, ":");
									loged_clients[j].pos.Y = atoi(ptr);
									ptr = strtok(NULL, ":");
									loged_clients[j].lives = atoi(ptr);
									ptr = strtok(NULL, ":");
									loged_clients[j].rotation = atoi(ptr);
								}
							}
						}
					}
				}
			}
		}
	}
}

void UpdateClients() {
	char answer[250];
	char cID[10];
	char cTeam[10];
	char cPosX[10];
	char cPosY[10];
	char cLives[10];
	char cRotation[10];
	for (unsigned int i = 0; i < loged_clients.size(); ++i) {
		for (unsigned int j = 0; j < loged_clients.size(); ++j) {
			if (loged_clients[i].id != loged_clients[j].id) {
				memset(answer, 0, 250);
				memset(cID, 0, 10);
				memset(cTeam, 0, 10);
				memset(cPosX, 0, 10);
				memset(cPosY, 0, 10);
				memset(cLives, 0, 10);
				memset(cRotation, 0, 10);
				_itoa(loged_clients[j].id, cID, 10);
				_itoa(loged_clients[j].team, cTeam, 10);
				_itoa(loged_clients[j].pos.X, cPosX, 10);
				_itoa(loged_clients[j].pos.Y, cPosY, 10);
				_itoa(loged_clients[j].lives, cLives, 10);
				_itoa(loged_clients[j].rotation, cRotation, 10);
				strcpy(answer, "RG_UPDATE:");
				strcat(answer, cID);
				//strcat(answer, ":");
				//strcat(answer, cTeam);
				strcat(answer, ":");
				strcat(answer, cPosX);
				strcat(answer, ":");
				strcat(answer, cPosY);
				strcat(answer, ":");
				strcat(answer, cLives);
				strcat(answer, ":");
				strcat(answer, cRotation);
				//strcat(answer, ":");
				//strcat(answer, loged_clients[j].name.c_str());
				sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&loged_clients[i].ip, sizeof(loged_clients[i].ip));
			}
			//loged_clients[i]
		}
	}
}

void Update() {
	StartTimer(update_timer);
	double CurrentTime = GetTime(update_timer);
	while (1) {
		double accumTime = GetTime(update_timer) - CurrentTime;

		while (accumTime >= uiTimeStep) {
			CurrentTime += uiTimeStep;
			accumTime = GetTime(update_timer) - CurrentTime;
			UpdateClients();
		}
	}
}

int main(int argc, char** argv) {

	WSAStartup(MAKEWORD(2, 0), &wsa);
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ip.sin_family = AF_INET;
	ip.sin_addr.s_addr = inet_addr("0.0.0.0");
	ip.sin_port = htons(8888);
	//check bind
	if (bind(udp_socket, (SOCKADDR*)&ip, sizeof(ip))) {
		printf("bind error");
		return 1;
	}

	std::thread t1(Communications);
	std::thread t2(Update);

	while (1) {
		//Console
		char command[50];
		bool valid_command = false;
		memset(command, 0, 50);
		printf("\nCommand-> ");
		fgets(command, 50, stdin);
		if (strcmp(command, "details\n") == 0) {
			valid_command = true;
			if (loged_clients.size() == 0) printf("No connections, Can't display information");
			else{
				printf("Detailed information of all connected clients:");
				for (unsigned int i = 0; i < loged_clients.size(); ++i) {
					printf("\nId: %d, IP: %s, Name: %s, Position: X%d,Y%d, Lives: %d, Team: %d, Rotation: %d",
						loged_clients[i].id, inet_ntoa(loged_clients[i].ip.sin_addr), loged_clients[i].name.c_str(), loged_clients[i].pos.X,
						loged_clients[i].pos.Y, loged_clients[i].lives, loged_clients[i].team, loged_clients[i].rotation);
				}
			}
		}
		if (strcmp(command, "basic\n") == 0) {
			valid_command = true;
			if (loged_clients.size() == 0) printf("No connections, Can't display information");
			else{
				printf("Basic information of all connected clients:");
				for (unsigned int i = 0; i < loged_clients.size(); ++i) {
					printf("\nName: %s, Lives: %d, Team: %d", loged_clients[i].name.c_str(), loged_clients[i].lives, loged_clients[i].team);
				}
			}
		}
		if (strcmp(command, "cls\n") == 0) {
			valid_command = true;
			system("cls");
		}
		if (strcmp(command, "connections\n") == 0) {
			valid_command = true;
			printf("Current connections: %d", loged_clients.size());
		}
		if (strcmp(command, "help\n") == 0) {
			valid_command = true;
			printf("Command list:"
				"\ndetails: Displays detailed information of all connected clients."
				"\nbasic: Displays basic information of all connected clients."
				"\nconnections: Displays the number of users currently logged in."
				"\ncls: Clears the screen."
				"\nhelp: Displays the command list.");

		}
		if (!valid_command) printf("Command not valid, type \"help\" for command list.");
	}

	t1.detach();

	return 0;
}
