#include <iostream>

#include "TcpServer.hpp"

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
		cout << "Invalid Input!";
		return -1;
	}

	TcpServer server;
	server.init();
	server.bind(ip, port);
	server.listen(5);

	while (server.isConnected()) {
		server.start();
	}

    getchar();
    return 0;
}


