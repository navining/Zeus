#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <thread>

#include "TcpClient.hpp"

using std::thread;

void cmdThread(TcpClient* client) {
	char cmdBuf[256] = {};
	while (true) {
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "quit")) {
			client->close();
			return;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login _login;
			strcpy(_login.username, "Navi");
			strcpy(_login.password, "123456");
			client->send(&_login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout _logout;
			strcpy(_logout.username, "Navi");
			client->send(&_logout);
		}
		else {
			cout << "Invalid input!" << endl;
		}
	}
}


int main(int argc, char* argv[]) {
	const char *ip;
	u_short port;
	if (argc == 1) {
		ip = "127.0.0.1";
		port = 4567;
	}
	else if (argc == 3) {
		ip = argv[1];
		port = atoi(argv[2]);
	}
	else {
		cout << "Invalid Input!" << endl;
		return -1;
	}

	const int numOfClients = 100;
	TcpClient* clients[numOfClients];
	for (int i = 0; i < numOfClients; i++) {
		clients[i] = new TcpClient();
		clients[i]->init();
		clients[i]->connect(ip, port);
	}

	// New thread
	//thread _cmd(test, &client);
	//_cmd.detach();

	Login login;
	strcpy(login.username, "Navi");
	strcpy(login.password, "123456");
	while (true) {
		for (int i = 0; i < numOfClients; i++) {
			clients[i]->send(&login);
		}
	}

	return 0;
}

