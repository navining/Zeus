
#include <iostream>
#include <thread>
#include "Timestamp.h"
#include "TcpClient.h"
#include <vector>
#include <atomic>
#include "Config.h"

class MyClient;

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
Stream msg;

int numOfMsg;	// Number of messages sent by each client

time_t sleepTime; // How long each client will sleep after sending a message

std::atomic_int clientsCount(0);
std::atomic_int readyCount(0);
std::atomic_int sendCount(0);


class MyClient : public TcpClient {
public:
	void onMessage(Message *msg) {
		switch (msg->type) {
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

	int sendTest(Stream *test)
	{
		int ret = 0;
		if (_nSendCount > 0)
		{
			ret = send(test);
			if (SOCKET_ERROR != ret)
			{
				--_nSendCount;
			}
		}
		return ret;
	}

	bool checkSend(time_t dt)
	{
		_tRestTime += dt;
		if (_tRestTime >= sleepTime)
		{
			_tRestTime -= sleepTime;
			_nSendCount = numOfMsg;
		}
		return _nSendCount > 0;
	}
private:
	time_t _tRestTime = 0;
	int _nSendCount = 0;
};



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
		Thread::sleep(0);
	}

	for (int i = begin; i < end; i++) {
		if (SOCKET_ERROR == clients[i]->connect(ip, port)) break;
		clientsCount++;
		Thread::sleep(0);
	}

	readyCount++;
	LOG_INFO("thread<%d> connected...\n", id);

	while (readyCount < numOfThreads) {
		Thread::sleep(10);
	}

	auto t2 = Time::getCurrentTimeInMilliSec();
	auto t0 = t2;
	auto dt = t0;
	Timestamp tTime;

	while (t.isRun()) {

		t0 = Time::getCurrentTimeInMilliSec();
		dt = t0 - t2;
		t2 = t0;

		for (int n = 0; n < numOfMsg; n++) {
			for (int i = begin; i < end; i++) {
				if (clients[i]->isRun())
					if (clients[i]->sendTest(&msg) > 0)
						sendCount++;
			}
		}

		for (int i = begin; i < end; i++) {
			if (clients[i]->isRun()) {
				if (!clients[i]->onRun()) {
					clientsCount--;
					continue;
				}
				clients[i]->checkSend(dt);
			}
		}

		//Thread::sleep(1);
	}

	for (int n = begin; n < end; n++)
	{
		delete clients[n];
	}

	LOG_INFO("thread<%d> exit..\n", id);
	readyCount--;
}

int main(int argc, char* argv[]) {
	Config::Init(argc, argv);
	ip = Config::Instance().parseStr("IP", "127.0.0.1");
	port = Config::Instance().parseInt("PORT", 4567);
	numOfClients = Config::Instance().parseInt("CLIENT", 10000);
	numOfThreads = Config::Instance().parseInt("THREAD", 1);
	numOfMsg = Config::Instance().parseInt("MSG", 10);
	sleepTime = Config::Instance().parseInt("SLEEP", 100);

	clients = new MyClient*[numOfClients];

	msg.writeString("Hello World!\n");

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
	LOG_INFO("Size per package: %d Bytes\n", msg.size() + msg.OFFSET);

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

	while (readyCount < numOfThreads) {
		Thread::sleep(10);
	}

	while (cmd.isRun()) {
		// Benchmark
		double t1 = _time.getElapsedSecond();
		if (t1 >= 1.0) {
			LOG_INFO("Time: %f Threads: %d Clients: %d Send: %d\n", t1, numOfThreads, (int)clientsCount, (int)sendCount);
			sendCount = 0;
			_time.update();
		}

		Thread::sleep(1);
	}

	for (auto &t : threads) {
		t.close();
	}

	delete[] clients;
	cmd.close();
	return 0;
}
