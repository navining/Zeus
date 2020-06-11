#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <thread>

#include "TcpClient.hpp"

using std::thread;

// Number of clients
const int numOfClients = 10000;

// Number of threads
const int numOfThreads = 4;

// Server IP
const char *ip;

// Server port
u_short port;

// Array of clients
TcpClient* clients[numOfClients];

void IOThread(int id) {
	int count = numOfClients / numOfThreads;
	int begin = (id - 1) * count;
	int end = id * count;

	for (int i = begin; i < end; i++) {
		clients[i] = new TcpClient();
	}

	for (int i = begin; i < end; i++) {
		clients[i]->connect("127.0.0.1", 4567);
	}

	// Data to be sent
	Test data;	// 100Byte

	while (true) {
		for (int i = begin; i < end; i++) {
			clients[i]->send(&data);
			clients[i]->onRun();
		}
	}
}

/*void cmdThread(TcpClient* client) {
char cmdBuf[256] = {};
while (true) {
int ret = scanf("%s", cmdBuf);
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
}*/

int main(int argc, char* argv[]) {
	if (argc == 1) {
		ip = "127.0.0.1";
		port = 4567;
	}
	else if (argc == 3) {
		ip = argv[1];
		port = atoi(argv[2]);
	}
	else {
		std::cout << "Invalid Input!" << std::endl;
		return -1;
	}

	printf("Number of clients: %d\nThreads: %d\n", numOfClients, numOfThreads);
	printf("Size per package: %d Bytes\n", sizeof(Test));
	for (int i = 0; i < numOfThreads; i++) {
		thread t(IOThread, i + 1);
		t.detach();
	}

	getchar();

	return 0;
}