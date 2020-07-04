#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <thread>
#include "Timestamp.h"
#include "TcpClient.h"

using std::thread;

class MyClient : public TcpClient {
public:
	int onMessage(Message *msg) {
		switch (msg->cmd) {
		case CMD_TEST:
		{
			Test* test = (Test *)msg;
			// LOG::INFO("<client %d> Receive Message: Test\n", _pClient->sockfd());
			break;
		}
		case CMD_ERROR:
		{
			LOG::INFO("<client %d> Receive Message: ERROR\n", _pClient->sockfd());
			break;
		}
		default:
		{
			LOG::INFO("<client %d> Receive Message: UNDIFINED\n", _pClient->sockfd());
		}
		}

		return 0;
	}
};


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
Test data;	// 1000 Byte


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
			LOG::INFO("Invalid input!\n");
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
	LOG::INFO("thread<%d> start...\n", id);
	int count = numOfClients / numOfThreads;
	int begin = (id - 1) * count;
	int end = id * count;

	for (int i = begin; i < end; i++) {
		clients[i] = new MyClient();
	}

	for (int i = begin; i < end; i++) {
		clients[i]->connect(ip, port);
	}
	LOG::INFO("thread<%d> connected...\n", id);

	std::thread t1(recvThread, begin, end);

	while (g_isRun) {
		//std::chrono::milliseconds t(100);
		//std::this_thread::sleep_for(t);
		for (int i = begin; i < end; i++) {
			clients[i]->send(&data);
		}
	}

	t1.join();

	for (int n = begin; n < end; n++)
	{
		//clients[n]->close();
		delete clients[n];
	}
	LOG::INFO("thread<%d> exit..\n", id);
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

	LOG::INFO("Number of clients: %d\nThreads: %d\n", numOfClients, numOfThreads);
	LOG::INFO("Size per package: %d Bytes\n", (int)sizeof(data));
	LOG::INFO("----------------------------------------------\n");
	for (int i = 0; i < numOfThreads; i++) {
		thread t(sendThread, i + 1);
		t.detach();
	}

	while (true) {

	}

	return 0;
}
