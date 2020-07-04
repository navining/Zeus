#define SERVER_MAIN

#include <iostream>
#include <thread>
#include <atomic>
#include "TcpServer.h"
//#include "Allocator.hpp"

class MyServer : public TcpServer {
public:
	MyServer() {
		_clientCount = 0;
		_msgCount = 0;
	}

	void onConnection(const TcpConnection& pClient) {
		_clientCount++;
	}

	void onDisconnection(const TcpConnection& pClient) {
		_clientCount--;
	}

	void onMessage(TcpSubserver *pServer, const TcpConnection& pClient, Message *msg) {
		_msgCount++;
		switch (msg->cmd) {
		case CMD_TEST:	// Send back the test data (echo)
		{
			Test* _test = (Test *)msg;
			// Send
			Test result;
			pClient->send(&result);
			break;
		}
		default:
		{
			printf("<server %d> From: <client %d> Receive Message: UNDIFINED\n", _sock, pClient->sockfd());
		}
		}

	}

	void onIdle() {
		// Benchmark
		double t1 = _time.getElapsedSecond();
		if (t1 >= 1.0) {
			LOG::INFO("<server %d> Time: %f Threads: %d Clients: %d Packages: %d\n", _sock, t1, thread_count, (int)_clientCount, (int)_msgCount);
			_msgCount = 0;
			_time.update();
		}
	}
private:
	std::atomic_int _msgCount;	// Number of messages
	std::atomic_int _clientCount;	// Number of clients
};

int main(int argc, char* argv[]) {
	const char *ip;
	u_short port;
	if (argc == 1) {
		ip = NULL;
		port = 4567;
	}
	else if (argc == 2) {
		ip = NULL;
		port = atoi(argv[1]);
	}
	else if (argc == 3) {
		ip = argv[1];
		port = atoi(argv[2]);
	}
	else {
		std::cout << "Invalid Input!" << std::endl;
		return -1;
	}

	LOG::setPath("zeus-server.log", "w");
	MyServer server;
	server.init();
	server.bind(ip, port);
	server.listen(5);
	server.start(1);
	

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.close();
			break;
		}
		else {
			printf("Invalid input!\n");
		}
	}

    return 0;
}
