#define SERVER_MAIN

#include <iostream>

#include "TcpServer.hpp"
//#include "Allocator.hpp"

class MyServer : public TcpServer {
public:
	void onConnection(const TcpConnection& pClient) {
		TcpServer::onConnection(pClient);
	}

	void onDisconnection(const TcpConnection& pClient) {
		TcpServer::onDisconnection(pClient);
	}

	void onMessage(TcpSubserver *pServer, const TcpConnection& pClient, Message *msg) {
		TcpServer::onMessage(pServer, pClient, msg);
		switch (msg->cmd) {
		case CMD_TEST:	// Send back the test data (echo)
		{
			Test* _test = (Test *)msg;
			// Send
			Test *result = new Test();
			pServer->send(pClient, result);
			break;
		}
		default:
		{
			printf("<server %d> From: <client %d> Recieve Message: UNDIFINED\n", _sock, pClient->sockfd());
		}
		}

	}
private:
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

	MyServer server;
	server.init();
	server.bind(ip, port);
	server.listen(5);
	server.start(4);

    return 0;
}


