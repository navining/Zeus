#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

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
		Header _header = {};
		// Recv
		int recvlen = recv(_cli, (char *)&_header, sizeof(Header), 0);
		if (recvlen <= 0) {
			cout << "Client quits" << endl;
			break;
		}
		
		// Handle request
		switch (_header.cmd) {
		case CMD_LOGIN:
		{
			Login _login;
			recv(_cli, (char *)&_login + sizeof(Header), sizeof(Login) - sizeof(Header), 0);
			cout << "Command: "<< _login.cmd << "Data length: " << _login.length << " Username: " << _login.username << " Password: " << _login.password << endl;
			// Judge username and password
			// Send
			LoginResult _result;
			send(_cli, (char *)&_result, sizeof(LoginResult), 0);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout _logout;
			recv(_cli, (char *)&_logout + sizeof(Header), sizeof(Logout) - sizeof(Header), 0);
			cout << "Command: " << _logout.cmd << " Data length: " << _logout.length << " Username: " << _logout.username << endl;
			// Send
			LogoutResult _result;
			send(_cli, (char *)&_result, sizeof(LogoutResult), 0);
			break;
		}
		default:
		{
			_header.cmd = CMD_ERROR;
			_header.cmd = 0;
			send(_cli, (char *)&_header, sizeof(Header), 0);
		}
		}
	}

	// Close
	closesocket(_sock);

	WSACleanup();
	getchar();
	return 0;
}

