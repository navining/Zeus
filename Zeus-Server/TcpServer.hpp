#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET    (SOCKET)(~0)
#define SOCKET_ERROR        (-1)
#endif

#include <iostream>
#include <vector>
using std::vector;
using std::cout;
using std::endl;

#include "Message.hpp"

class TcpServer {
public:
	TcpServer() {
		_sock = INVALID_SOCKET;
	}

	virtual ~TcpServer() {
		close();
	}

	// Initialize socket
	int init() {
#ifdef _WIN32
		// Start Win Sock 2.x
		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(version, &data);
#endif
		// Create socket
		if (isConnected()) {
			cout << "<server " << _sock << "> " << "Close old connection..." << endl;
			close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!isConnected()) {
			cout << "Create socket - Fail..." << endl;
		}
		else {
			cout << "Create socket - Success..." << endl;
		}
		return _sock;
	}

	// Bind IP and port
	int bind(const char* ip, unsigned short port) {
		if (!isConnected()) {
			init();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		if (ip != NULL) {
#ifdef _WIN32
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
			_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		}
		else {
#ifdef _WIN32
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
			_sin.sin_addr.s_addr = INADDR_ANY;
#endif
		}
		int ret = ::bind(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			cout << "<server " << _sock << "> " << "Bind " << port << " - Fail..." << endl;
		}
		else {
			cout << "<server " << _sock << "> " << "Bind " << port << " - Success..." << endl;
		}

		return ret;
	}

	// Listen to port
	int listen(int n) {
		int ret = ::listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			cout << "<server " << _sock << "> " << "Listen - Fail..." << endl;
		}
		else {
			cout << "<server " << _sock << "> " << "Listen - Success..." << endl;
		}

		return ret;

	}

	// Accept client connection
	SOCKET accept() {
		// Accept
		sockaddr_in clientAddr = {};
		int addrlen = sizeof(sockaddr_in);
#ifdef _WIN32
		SOCKET _cli = ::accept(_sock, (sockaddr *)&clientAddr, &addrlen);
#else
		SOCKET _cli = ::accept(_sock, (sockaddr *)&clientAddr, (socklen_t *)&addrlen);
#endif
		if (INVALID_SOCKET == _cli) {
			cout << "<server " << _sock << "> " << "Invaild client socket..." << endl;
		}
		else {
			// Broadcast
			NewUserJoin userJoin;
			userJoin.sock = _cli;
			broadcast(&userJoin);
		}

		g_clients.push_back(_cli);
		cout << "<server " << _sock << "> " << "New connection: " << "<client " << _cli << "> " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;

		return _cli;
	}

	// Start server service
	bool start() {
		if (!isConnected())
		{
			cout << "<server " << _sock << "> " << "Start - Fail ..." << endl;
			return false;
		};

		// Select
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		// Put server sockets inside fd_set
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExcept);

		// Record curret max socket
		SOCKET maxSock = _sock;

		// Put client sockets inside fd_set
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			FD_SET(g_clients[n], &fdRead);
			if (maxSock < g_clients[n]) {
				maxSock = g_clients[n];
			}
		}

		timeval t = { 0, 0 };
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);

		if (ret < 0) {
			cout << "<server " << _sock << "> Select - Fail ..." << endl;
			close();
			return false;
		}

		// Server socket response: accept connection
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			accept();
		}

		// Client socket response: handle request
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			if (FD_ISSET(g_clients[n], &fdRead)) {
				if (-1 == recv(g_clients[n])) {
					auto it = g_clients.begin() + n;
					if (it != g_clients.end()) {
						g_clients.erase(it);
					}
				}
			}
		}

		// Handle other services
		//cout << "Other services..." << endl;

		return true;
	}


	int recv(SOCKET _cli) {
		// Buffer
		char recvBuf[1024] = {};
		// Receive header
		int recvlen = (int)::recv(_cli, recvBuf, sizeof(Header), 0);
		Header *_header = (Header *)recvBuf;
		if (recvlen <= 0) {
			cout << "<server " << _sock << "> " << "<client " << _cli << "> " << "Disconnected..." << endl;
			return -1;
		}
		// Receive body
		::recv(_cli, recvBuf + sizeof(Header), _header->length - sizeof(Header), 0);

		// Process data
		int ret = process(_cli, _header);
		return ret;
	}

	virtual int process(SOCKET _cli, Header *_msg) {
		switch (_msg->cmd) {
		case CMD_LOGIN:
		{
			Login* _login = (Login *)_msg;
			cout << "<server " << _sock << "> " << "From: " << "<client " << _cli << "> " << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
			// Judge username and password
			// Send
			LoginResult _result;
			send(_cli, &_result);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* _logout = (Logout *)_msg;
			cout << "<server " << _sock << "> " << "From: " << "<client " << _cli << "> " << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
			// Send
			LogoutResult _result;
			send(_cli, &_result);
			break;
		}
		default:
		{
			Header _result = { 0, CMD_ERROR };
			send(_cli, &_result);
		}
		}

		return 0;
	}

	// Send data
	int send(SOCKET _cli, Header *_msg) {
		if (!isConnected() || _msg == NULL)
			return SOCKET_ERROR;
		return ::send(_cli, (const char *)_msg, _msg->length, 0);
	}

	// Broadcast data
	void broadcast(Header *_msg) {
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			send(g_clients[n], _msg);
		}
	}

	// If connected
	inline bool isConnected() {
		return _sock != INVALID_SOCKET;
	}

	/// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;
		cout << "<server " << _sock << "> " << "Quit..." << endl;
#ifdef _WIN32
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		::close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}

private:
	SOCKET _sock;
	vector<SOCKET> g_clients;
};

#endif // !_TcpServer_hpp_
