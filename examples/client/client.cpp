#ifdef _WIN32
#endif

#include <iostream>
#include <thread>
#include "Timestamp.h"
#include "TcpClient.h"
#include <vector>
#include <atomic>
#include "Config.h"

class MyClient : public TcpClient {
public:
	void onMessage(Message *msg) {
		_msgCount++;
		switch (msg->cmd) {
		case CMD_TEST:
		{
			Test* test = (Test *)msg;
			// LOG::INFO("<client %d> Receive Message: Test\n", _pClient->sockfd());
			break;
		}
		case CMD_ERROR:
		{
			LOG_INFO("<client %d> Receive Message: ERROR\n", _pClient->sockfd());
			break;
		}
		default:
		{
			LOG_INFO("<client %d> Receive Message: UNDIFINED\n", _pClient->sockfd());
		}
		}
	}

public:
	std::atomic_int _msgCount;	// Number of messages
};


// Number of clients
int numOfClients;

// Number of threads
int numOfThreads;

// Server IP
const char *ip;

// Server port
u_short port;

// Array of clients
MyClient** clients;

// Message to be sent
Test msg;	// 100 Byte

// Number of messages sent by each client
int numOfMsg;

int msgCount = 0;

std::atomic_int clientsCount(0);
std::atomic_int readyCount(0);

void cmdThread(Thread &t) {
	while (t.isRun())
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			break;
		}
		else {
			LOG_INFO("Invalid input!\n");
		}
	}
}

void workerThread(Thread &t, int id) {
	LOG_INFO("thread<%d> start...\n", id);

	int count = numOfClients / numOfThreads;
	int begin = (id - 1) * count;
	int end = id * count;

	for (int i = begin; i < end; i++) {
		clients[i] = new MyClient();
	}

	for (int i = begin; i < end; i++) {
		clients[i]->connect(ip, port);
		clientsCount++;
	}

	readyCount++;
	LOG_INFO("thread<%d> connected...\n", id);

	while (readyCount < numOfThreads) {
		Thread::sleep(10);
	}

	while (t.isRun()) {
		for (int n = 0; n < numOfMsg; n++) {
			for (int i = begin; i < end; i++) {
				if (clients[i]->isRun())
					clients[i]->send(&msg);
				if (clients[i]->isRun())
					clients[i]->onRun();
			}
		}
	}

	for (int n = begin; n < end; n++)
	{
		delete clients[n];
	}

	LOG_INFO("thread<%d> exit..\n", id);
}

int main(int argc, char* argv[]) {
	Config::Init(argc, argv);
	ip = Config::Instance().parseStr("IP", "127.0.0.1");
	port = Config::Instance().parseInt("PORT", 4567);
	numOfClients = Config::Instance().parseInt("CLIENT", 1000);
	numOfThreads = Config::Instance().parseInt("THREAD", 2);
	numOfMsg = Config::Instance().parseInt("MSG", 100);

	clients = new MyClient*[numOfClients];

	Thread cmd;
	cmd.start(
		EMPTY_THREAD_FUNC,
		std::function<void(Thread &)>(cmdThread),
		EMPTY_THREAD_FUNC
	);

	LOG_SETPATH("zeus-client.log", "w");

	LOG_INFO("IP: %s\n", ip);
	LOG_INFO("Port: %d\n", port);
	LOG_INFO("Number of clients: %d\n", numOfClients);
	LOG_INFO("Number of threads: %d\n", numOfThreads);
	LOG_INFO("Number of packages per client: %d\n", numOfMsg);
	LOG_INFO("Size per package: %d Bytes\n", (int)sizeof(msg));

	std::vector<Thread> threads(numOfThreads);
	for (int n = 0; n < numOfThreads; n++)
	{
		threads[n].start(
			EMPTY_THREAD_FUNC,
			[n](Thread& p) {
				workerThread(p, n + 1);
			},
			EMPTY_THREAD_FUNC
			);
	}

	Timestamp _time;

	while (clientsCount < numOfClients) {
		Thread::sleep(10);
	}

	while (cmd.isRun()) {
		// Benchmark
		double t1 = _time.getElapsedSecond();
		if (t1 >= 1.0) {

			for (int i = 0; i < numOfClients; i++) {
				msgCount += clients[i]->_msgCount;
				clients[i]->_msgCount = 0;
			}

			LOG_INFO("Time: %f Threads: %d Clients: %d Packages: %d\n", t1, numOfThreads, (int)clientsCount, msgCount);
			msgCount = 0;
			_time.update();
		}
	}

	for (auto &t : threads) {
		t.close();
	}

	delete[] clients;
	cmd.close();
	return 0;
}
