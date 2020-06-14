#include <iostream>

#include "TcpServer.hpp"

class MyServer : public TcpServer {
public:
	void onConnection(TcpSocket *pClient) {
		TcpServer::onConnection(pClient);
	}

	void onDisconnection(TcpSocket *pClient) {
		TcpServer::onDisconnection(pClient);
	}

	void onMessage(TcpSubserver *pServer, TcpSocket *pClient, Header *msg) {
		TcpServer::onMessage(pServer, pClient, msg);
		switch (msg->cmd) {
		case CMD_LOGIN:
		{
			Login* login = (Login *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
			// Judge username and password
			// Send
			LoginResult result;
			pClient->send(&result);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
			// Send
			LogoutResult result;
			pClient->send(&result);
			break;
		}
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


