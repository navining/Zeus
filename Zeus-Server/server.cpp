#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
	WORD version = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(version, &data);

	// Create socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		cout << "Create socket - Fail" << endl;
	}
	else {
		cout << "Create socket - Success" << endl;
	}

	// Bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	int ret = bind(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		cout << "Bind - Fail" << endl;
	}
	else {
		cout << "Bind - Success" << endl;
	}

	// Listen
	ret = listen(_sock, 5);
	if (SOCKET_ERROR == ret) {
		cout << "Listen - Fail" << endl;
	}
	else {
		cout << "Listen - Success" << endl;
	}

	// Accept
	sockaddr_in clientAddr = {};
	int addrlen = sizeof(sockaddr_in);
	SOCKET _cli = accept(_sock, (sockaddr *)&clientAddr, &addrlen);
	if (INVALID_SOCKET == _cli) {
		cout << "Invaild client socket" << endl;
	}
	cout << "New client: " << inet_ntoa(clientAddr.sin_addr) << endl;

	while (true) {
		// Recv
		char recvBuf[128] = {};
		int recvlen = recv(_cli, recvBuf, 128, 0);
		if (recvlen <= 0) {
			cout << "Client quits" << endl;
			break;
		}
		cout << "Recieve command: " << recvBuf << endl;
		// Handle request
		char *msgBuf;
		if (0 == strcmp(recvBuf, "name")) {
			msgBuf = "Navi";
		}
		else if (0 == strcmp(recvBuf, "age")) {
			msgBuf = "21";
		}
		else {
			msgBuf = "???";
		}

		// Send
		send(_cli, msgBuf, strlen(msgBuf) + 1, 0);
	}

	// Close
	closesocket(_sock);

	WSACleanup();
	getchar();
	return 0;
}

