#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_

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

using std::cout;
using std::endl;

#include "Message.hpp"

class TcpClient {
public:
	TcpClient() {
		_sock = INVALID_SOCKET;
	}

	virtual ~TcpClient() {
		close();
	}

	// Initialize socket
	SOCKET init() {
#ifdef _WIN32
		// Start Win Sock 2.x
		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(version, &data);
#endif
		// Create socket
		if (isConnected()) {
			cout << "<client " << _sock << "> " << "Close old connection..." << endl;
			close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!isConnected()) {
			cout << "Create socket - Fail..." << endl;
		}
		else {
			cout << "<client " << _sock << "> " << "Create socket - Success..." << endl;
		}
		return _sock;
	}

	// Connect server
	int connect(const char *ip, unsigned short port) {
		if (!isConnected()) {
			init();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = ::connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			cout << "<client " << _sock << "> " << "Connect - Fail..." << endl;
		}
		else {
			//cout << "<client " << _sock << "> " << "Connect - Success..." << endl;
		}
		return ret;
	}

	// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;
		cout << "<client " << _sock << "> " << "Quit..." << endl;
#ifdef _WIN32
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		::close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}

	// Start client service
	bool start() {
		if (!isConnected())
		{
			cout << "<client " << _sock << "> " << "Start - Fail ..." << endl;
			return false;
		};

		// Select
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		timeval t = { 0, 0 };
		int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);

		if (ret < 0) {
			cout << "<client " << _sock << "> Select - Fail ..." << endl;
			close();
			return false;
		}

		// If socket is inside the set
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			// Handle request
			if (-1 == recv()) {
				cout << "<client " << _sock << "> " << "Process - Fail ..." << endl;
				close();
				return false;
			}
		}

		// Handle other services
		//cout << "Other services..." << endl;

		return true;
	}

	// If connected
	inline bool isConnected() {
		return _sock != INVALID_SOCKET;
	}

	// Size of the recieve buffer
#define RECV_BUFF_SIZE 10240

// Size of the message buffer
#define MSG_BUFF_SIZE 102400

	// Recieve Buffer (System Buffer)
	char _recvBuf[RECV_BUFF_SIZE] = {};

	// Message Buffer (Secondary Buffer)
	char _msgBuf[MSG_BUFF_SIZE] = {};

	// Last position of the message buffer
	int _lastPos = 0;

	// Receive data and unpack
	int recv() {
		// Receive data into the recieve buffer
		int recvlen = (int)::recv(_sock, _recvBuf, RECV_BUFF_SIZE, 0);

		if (recvlen <= 0) {
			cout << "<client " << _sock << "> " << "Disconnected..." << endl;
			return -1;
		}

		// Copy data into the message buffer
		memcpy(_msgBuf + _lastPos, _recvBuf, recvlen);
		_lastPos += recvlen;

		while (_lastPos >= sizeof(Header)) {
			// Get header
			Header *header = (Header *)_msgBuf;
			if (_lastPos >= header->length) {
				// Size of remaining messages
				int nSize = _lastPos - header->length;
				// Process message
				process(header);
				// Move remaining messages forward.
				memcpy(_msgBuf, _msgBuf + header->length, nSize);
				_lastPos = nSize;
			}
			else {
				break;
			}
		}
		return 0;
	}

	// Process data
	virtual int process(Header *msg) {
		switch (msg->cmd) {
		case CMD_LOGIN_RESULT:
		{
			LoginResult* _loginResult = (LoginResult *)msg;
			//cout << "<client " << _sock << "> " << "Recieve Message: " << _loginResult->cmd << " Data Length: " << _loginResult->length << " Result: " << _loginResult->result << endl;
			break;
		}
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* _logoutResult = (LogoutResult *)msg;
			//cout << "<client " << _sock << "> " << "Recieve Message: " << _logoutResult->cmd << " Data Length: " << _logoutResult->length << " Result: " << _logoutResult->result << endl;
			break;
		}
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* _userJoin = (NewUserJoin *)msg;
			//cout << "<client " << _sock << "> " << "Recieve Message: " << _userJoin->cmd << " Data Length: " << _userJoin->length << " New User: " << _userJoin->sock << endl;
			break;
		}
		case CMD_ERROR:
		{
			cout << "<client " << _sock << "> " << "Recieve Message: " << "ERROR" << " Data Length: " << msg->length << endl;
			break;
		}
		default:
		{
			cout << "<client " << _sock << "> " << "Recieve Message: " << "Unknown" << " Data Length: " << msg->length << endl;
		}
		}

		return 0;
	}

	// Send data
	int send(Header *_msg) {
		if (!isConnected() || _msg == NULL)
			return SOCKET_ERROR;
		return ::send(_sock, (const char *)_msg, _msg->length, 0);
	}

private:
	SOCKET _sock;
};

#endif // !_TcpClient_hpp_
