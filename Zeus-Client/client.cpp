#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);

	// Create socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		cout << "Create socket - Fail" << endl;
	}
	else {
		cout << "Create socket - Success" << endl;
	}

	// Connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		cout << "Connect - Fail" << endl;
	}
	else {
		cout << "Connect - Success" << endl;
	}

	// Recv
	char recvBuf[256] = {};
	int recvlen = recv(_sock, recvBuf, 256, 0);
	if (recvlen > 0) {
		cout << recvBuf << endl;
	}

	// Close
	closesocket(_sock);

	WSACleanup();

	getchar();
	return 0;
}

