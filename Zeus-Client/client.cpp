#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum CMD {
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};

struct Header {
	short cmd;
	short length;
};

struct Login {
	char username[32];
	char password[32];
};

struct LoginResult {
	int result;
};

struct Logout {
	char username[32];
};

struct LogoutResult {
	int result;
};

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

	while (true) {
		// Handle request
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "quit")) {
			cout << "Quit" << endl;
			break;
		} else if (0 == strcmp(cmdBuf, "login")) {
			Header _header = { CMD_LOGIN, sizeof(Login) };
			Login _login = {"navi", "123456"};
			// Send
			send(_sock, (char *)&_header, sizeof(Header), 0);
			send(_sock, (char *)&_login, sizeof(Login), 0);
			// Recv
			Header _resultH = {};
			LoginResult _result = {};
			recv(_sock, (char *)&_resultH, sizeof(Header), 0);
			recv(_sock, (char *)&_result, sizeof(LoginResult), 0);
			cout << "Login result: " << _result.result << endl;
		} else if (0 == strcmp(cmdBuf, "logout")) {
			Header _header = { CMD_LOGOUT, sizeof(Logout) };
			Logout _logout = { "navi"};
			// Send
			send(_sock, (char *)&_header, sizeof(Header), 0);
			send(_sock, (char *)&_logout, sizeof(Logout), 0);
			// Recv
			Header _resultH = {};
			LogoutResult _result = {};
			recv(_sock, (char *)&_resultH, sizeof(Header), 0);
			recv(_sock, (char *)&_result, sizeof(LogoutResult), 0);
			cout << "Logout result: " << _result.result << endl;
		}
		else {
			cout << "Invaild input" << endl;
		}
	}


	// Close
	closesocket(_sock);

	WSACleanup();

	getchar();
	return 0;
}

