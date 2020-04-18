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
			Login _login;
			strcpy(_login.username, "navi");
			strcpy(_login.password, "123456");
			// Send
			send(_sock, (char *)&_login, sizeof(Login), 0);
			// Recv
			LoginResult _result = {};
			recv(_sock, (char *)&_result, sizeof(LoginResult), 0);
			cout << "Login result: " << _result.result << endl;
		} else if (0 == strcmp(cmdBuf, "logout")) {
			Logout _logout;
			strcpy(_logout.username, "navi");
			// Send
			send(_sock, (char *)&_logout, sizeof(Logout), 0);
			// Recv
			LogoutResult _result = {};
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

