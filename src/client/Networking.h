//Copyright (C) Nacho Fuentes Talens

#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include "winsock2.h"
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#include "MainMenuScene.h"

class Networking {
public:
	struct position {
		int X;
		int Y;
	};

	struct client {
		int id;
		sockaddr_in ip;
		position pos;
		std::string name;
		int lives;
		int team;
		int rotation;
	};

	static Networking& Instance() {
		static Networking instance;
		return instance;
	}

	//winsock data
	WSADATA wsa;
	SOCKET udp_socket;
	sockaddr_in ip;
	sockaddr_in ips;

	vector<thread> threads;
	static client owning_player;
	static vector<client> players;

	void ReceiveData();
	void RegisterUser(std::string name, std::string pass);
	void LoginUser(std::string name, std::string pass, int id);
	void UpdateOwnerData();
	void Damage(int affected);

private:
	Networking();
	Networking(Networking const &m);
	~Networking() {
		for (unsigned int i = 0; i < threads.size(); ++i) {
			threads[i].detach();
		}
	};
	Networking operator=(const Networking&);
};

#endif