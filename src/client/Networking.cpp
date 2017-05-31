//Copyright (C) 2016 Nacho Fuentes Talens
#include "Networking.h"
#include "MainMenuScene.h"

Networking::client Networking::owning_player;
std::vector<Networking::client> Networking::players;

Networking::Networking() {
	char buffer[250];
	memset(buffer, 0, 250);
	int bytes = 0;
	//WINSOCK
	WSAStartup(MAKEWORD(2, 0), &wsa);
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ip.sin_family = AF_INET;
	ip.sin_port = htons(8888);
	ip.sin_addr.s_addr = inet_addr("255.255.255.255");
	sprintf(buffer, "RG_DISCOVER");
	int valor = 1;
	if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&valor, sizeof(valor))) {
		printf("Broadcast socket error");
		return;
	}
	sendto(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR *)&ip, sizeof(ip));
	valor = 0;
	if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&valor, sizeof(valor))) {
		printf("undo Broadcast socket error");
		return;
	}
	//	ip.sin_addr.s_addr = inet_addr("127.0.0.1");
	threads.push_back(std::thread(&Networking::ReceiveData, this));
}

void Networking::RegisterUser(std::string name, std::string pass) {
	char buffer[250];
	memset(buffer, 0, 250);
	strcpy(buffer, "RG_REGISTER:");
	strcat(buffer, name.c_str());
	strcat(buffer, ":");
	strcat(buffer, pass.c_str());
	sendto(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR *)&ip, sizeof(ip));
}

void Networking::LoginUser(std::string name, std::string pass, int id) {
	char buffer[250];
	char cId[10];
	memset(buffer, 0, 250);
	memset(buffer, 0, 10);
	itoa(id, cId, 10);
	strcpy(buffer, "RG_LOGIN:");
	strcat(buffer, name.c_str());
	strcat(buffer, ":");
	strcat(buffer, pass.c_str());
	strcat(buffer, ":");
	strcat(buffer, cId);
	sendto(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR *)&ip, sizeof(ip));
}

void Networking::ReceiveData() {
	char* ptr;
	int size = sizeof(ip);
	char buffer[250];
	char answer[250];
	char cID[10];
	int bytes = 0;
	while (1) {
		memset(buffer, 0, 250);
		memset(answer, 0, 250);
		memset(cID, 0, 10);
		bytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR*)&ips, &size);
		if (bytes > 0) {
			ptr = strtok(buffer, ":");
			if (strcmp(ptr, "RG_IDENTIFY") == 0) {
				/*ptr = strtok(NULL, ":");
				UserDefault::getInstance()->setIntegerForKey("Id", atoi(ptr));*/
				char tmp_addr[40];
				memset(tmp_addr, 0, 40);
				sprintf(tmp_addr, "%s", inet_ntoa(ips.sin_addr));
				ip.sin_addr.s_addr = inet_addr(tmp_addr);
			}
			else {
				if (strcmp(ptr, "RG_LOGIN") == 0) {
					ptr = strtok(NULL, ":");
					if (strcmp(ptr, "OK") == 0) {
						//RG_LOGIN:OK:ID:TEAM:POSX:POSY:LIVES:NAME
						//Login Successful, retrieve data and start game
						//UserDefault::getInstance()->setIntegerForKey("Id", atoi(ptr));
						//UserDefault::getInstance()->setIntegerForKey("Team", atoi(ptr));
						//UserDefault::getInstance()->setIntegerForKey("PosX", atoi(ptr));
						//UserDefault::getInstance()->setIntegerForKey("PosY", atoi(ptr));
						//UserDefault::getInstance()->setIntegerForKey("Lives", atoi(ptr));
						ptr = strtok(NULL, ":");
						owning_player.id = atoi(ptr);
						ptr = strtok(NULL, ":");
						owning_player.team = atoi(ptr);
						ptr = strtok(NULL, ":");
						owning_player.pos.X = atoi(ptr);
						ptr = strtok(NULL, ":");
						owning_player.pos.Y = atoi(ptr);
						ptr = strtok(NULL, ":");
						owning_player.lives = atoi(ptr);
						ptr = strtok(NULL, ":");
						owning_player.name = ptr;
						MainMenu::UpdateFeedbackLabel("Login Successful");
					}
					else {
						//Register Error
						MainMenu::UpdateFeedbackLabel("Login Error");
					}
				}
				else {
					if (strcmp(ptr, "RG_REGISTER") == 0) {
						ptr = strtok(NULL, ":");
						if (strcmp(ptr, "OK") == 0) {
							//Registered Successfully
							MainMenu::UpdateFeedbackLabel("Successfully Registered");
						}
						else {
							//Register Error
							MainMenu::UpdateFeedbackLabel("Register Error");
						}
					}
					else {
						//RG_NEWPLAYER:OK:ID:TEAM:POSX:POSY:LIVES:NAME
						if (strcmp(ptr, "RG_NEWPLAYER") == 0) {
							client tmp_client;
							ptr = strtok(NULL, ":");
							tmp_client.id = atoi(ptr);
							ptr = strtok(NULL, ":");
							tmp_client.team = atoi(ptr);
							ptr = strtok(NULL, ":");
							tmp_client.pos.X = atoi(ptr);
							ptr = strtok(NULL, ":");
							tmp_client.pos.Y = atoi(ptr);
							ptr = strtok(NULL, ":");
							tmp_client.lives = atoi(ptr);
							ptr = strtok(NULL, ":");
							tmp_client.name = ptr;
							players.push_back(tmp_client);
						}
						else {
							if (strcmp(ptr, "RG_UPDATE") == 0) {
								int tmp_ID;
								ptr = strtok(NULL, ":");
								tmp_ID = atoi(ptr);
								for (unsigned int i = 0; i < players.size(); ++i) {
									if (players[i].id == tmp_ID) {
										ptr = strtok(NULL, ":");
										players[i].pos.X = atoi(ptr);
										ptr = strtok(NULL, ":");
										players[i].pos.Y = atoi(ptr);
										ptr = strtok(NULL, ":");
										players[i].lives = atoi(ptr);
										ptr = strtok(NULL, ":");
										players[i].rotation = atoi(ptr);
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void Networking::UpdateOwnerData() {
	char answer[250];
	char cID[10];
	char cTeam[10];
	char cPosX[10];
	char cPosY[10];
	char cLives[10];
	char cRotation[10];
	memset(answer, 0, 250);
	memset(cID, 0, 10);
	memset(cTeam, 0, 10);
	memset(cPosX, 0, 10);
	memset(cPosY, 0, 10);
	memset(cLives, 0, 10);
	memset(cRotation, 0, 10);
	_itoa(owning_player.id, cID, 10);
	_itoa(owning_player.team, cTeam, 10);
	_itoa(owning_player.pos.X, cPosX, 10);
	_itoa(owning_player.pos.Y, cPosY, 10);
	_itoa(owning_player.lives, cLives, 10);
	_itoa(owning_player.rotation, cRotation, 10);
	strcpy(answer, "RG_UPDATE:");
	strcat(answer, cID);
	strcat(answer, ":");
	strcat(answer, cPosX);
	strcat(answer, ":");
	strcat(answer, cPosY);
	strcat(answer, ":");
	strcat(answer, cLives);
	strcat(answer, ":");
	strcat(answer, cRotation);
	sendto(udp_socket, answer, sizeof(answer), 0, (SOCKADDR*)&ip, sizeof(ip));
}

void Networking::Damage(int affected) {
	char answer[250];
	memset(answer, 0, 250);
	char cID[10];
	char cIDEnemy[10];
	memset(cID, 0, 10);
	memset(cIDEnemy, 0, 10);
	_itoa(owning_player.id, cID, 10);
	_itoa(affected, cIDEnemy, 10);
	strcpy(answer, "RG_DAMAGE:");
}
//loged_clients[i]