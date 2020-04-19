#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};

struct Header {
	short cmd;
	short length;
};

struct Login : public Header {
	Login() {
		length = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char username[32];
	char password[32];
};

struct LoginResult : public Header {
	LoginResult() {
		length = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public Header {
	Logout() {
		length = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char username[32];
};

struct LogoutResult : public Header {
	LogoutResult() {
		length = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

vector<SOCKET> g_clients;

int processor(SOCKET _cli) {

	// Buffer
	char recvBuf[1024] = {};
	// Recv
	int recvlen = recv(_cli, recvBuf, sizeof(Header), 0);
	Header *_header = (Header *)recvBuf;
	if (recvlen <= 0) {
		cout << "Client quits" << endl;
		return -1;
	}

	switch (_header->cmd) {
	case CMD_LOGIN:
	{
		recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
		Login* _login = (Login *)recvBuf;
		cout << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
		// Judge username and password
		// Send
		LoginResult _result;
		send(_cli, (char *)&_result, sizeof(LoginResult), 0);
		break;
	}
	case CMD_LOGOUT:
	{
		recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);
		Logout* _logout = (Logout *)recvBuf;
		cout << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
		// Send
		LogoutResult _result;
		send(_cli, (char *)&_result, sizeof(LogoutResult), 0);
		break;
	}
	default:
	{
		Header _header = { 0, CMD_ERROR };
		send(_cli, (char *)&_header, sizeof(Header), 0);
	}
	}

	return 0;
}

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

	while (true) {
		// Select
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExcept);

		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			FD_SET(g_clients[n], &fdRead);
		}

		timeval t = {0, 0};
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

		if (ret < 0) {
			cout << "Select quits" << endl;
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			// Accept
			sockaddr_in clientAddr = {};
			int addrlen = sizeof(sockaddr_in);
			SOCKET _cli = accept(_sock, (sockaddr *)&clientAddr, &addrlen);
			if (INVALID_SOCKET == _cli) {
				cout << "Invaild client socket" << endl;
			}
			g_clients.push_back(_cli);
			cout << "New client: " << inet_ntoa(clientAddr.sin_addr) << endl;
		}

		// Handle request
		for (size_t n = 0; n < fdRead.fd_count; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto it = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (it != g_clients.end()) {
					g_clients.erase(it);
				}
			}
		}
	}

	// Close
	for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
		closesocket(g_clients[n]);
	}
	closesocket(_sock);

	WSACleanup();
	getchar();
	return 0;
}

