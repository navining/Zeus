#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <thread>
#include "Timestamp.h"
#include "TcpClient.hpp"

using std::thread;

// Number of clients
const int numOfClients = 1000;

// Number of threads
const int numOfThreads = 2;

// Server IP
const char *ip;

// Server port
u_short port;

// Array of clients
TcpClient* clients[numOfClients];

// Data to be sent
Test data[10];	// 100*10Byte


bool g_isRun = true;

void cmdThread() {
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_isRun = false;
			break;
		}
		else {
			printf("Invalid input!\n");
		}
	}
}

void recvThread(int begin, int end)
{
	while (g_isRun)
	{
		for (int n = begin; n < end; n++)
		{
			clients[n]->onRun();
		}
	}
}

void sendThread(int id) {
	printf("thread<%d> start...\n", id);
	int count = numOfClients / numOfThreads;
	int begin = (id - 1) * count;
	int end = id * count;

	for (int i = begin; i < end; i++) {
		clients[i] = new TcpClient();
	}

	for (int i = begin; i < end; i++) {
		clients[i]->connect(ip, port);
	}
	printf("thread<%d> connected...\n", id);

	std::thread t1(recvThread, begin, end);
	t1.detach();

	while (g_isRun) {
		for (int i = begin; i < end; i++) {
			clients[i]->send(data, 10 * sizeof(Test));
		}
	}

	for (int n = begin; n < end; n++)
	{
		clients[n]->close();
		delete clients[n];
	}

	printf("thread<%d> exit..\n", id);
}

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

	thread t1(cmdThread);
	t1.detach();

	printf("Number of clients: %d\nThreads: %d\n", numOfClients, numOfThreads);
	printf("Size per package: %d Bytes\n", (int)sizeof(data));
	printf("----------------------------------------------\n");
	for (int i = 0; i < numOfThreads; i++) {
		thread t(sendThread, i + 1);
		t.detach();
	}

	while (g_isRun) {

	}

	return 0;
}
