#include <iostream>

#include "TcpServer.hpp"

class MyServer : public TcpServer {
public:
	virtual void onConnection(TcpSocket *pClient) {
		_clientCount++;
	}

	virtual void onDisconnection(TcpSocket *pClient) {
		_clientCount--;
	}

	virtual void onMessage(TcpSocket *pClient, Header *msg) {
		_msgCount++;
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
			//Test result;
			//pClient->send(&result);
			break;
		}
		default:
		{
			// cout << "<server " << _sock << "> " << "From: " << "<client " << _sock << "> " << "Recieve Message: " << "Unknown" << " Data Length: " << msg->length << endl;
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

	TcpServer server;
	server.init();
	server.bind(ip, port);
	server.listen(5);
	server.start(4);

	while (server.isRun()) {
		server.onRun();
	}

    getchar();
    return 0;
}


