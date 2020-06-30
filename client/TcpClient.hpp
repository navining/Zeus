#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_


#include "common.h"
#include <iostream>
#include "Message.hpp"


class TcpClient {
public:
	TcpClient() {
		_sock = INVALID_SOCKET;
		isConnect = false;
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
		if (isRun()) {
			printf("<client %d> Close old connection...\n", _sock);
			close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET) {
			printf("Create socket - Fail...\n");
		}
		else {
			// printf("<client %d> Create socket - Success...\n", _sock);
		}
		return _sock;
	}

	// Connect server
	int connect(const char *ip, unsigned short port) {
		if (!isRun()) {
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
			printf("<client %d> Connect - Fail...\n", _sock);
		}
		else {
			isConnect = true;
			// printf("<client %d> Connect - Success...\n", _sock);
		}
		return ret;
	}

	// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;
		//printf("<client %d> Quit...\n", _sock);
#ifdef _WIN32
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		::close(_sock);
#endif
		_sock = INVALID_SOCKET;
		isConnect = false;
	}

	// Start client service
	bool onRun() {
		if (!isRun())
		{
			printf("<client %d> Start - Fail...\n", _sock);
			return false;
		};

		// Select
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		timeval t = { 0, 0 };
		int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);

		if (ret < 0) {
			printf("<client %d> Select - Fail...\n", _sock);
			close();
			return false;
		}

		// If socket is inside the set
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			// Handle request
			if (-1 == recv()) {
				printf("<client %d> Process - Fail...\n", _sock);
				close();
				return false;
			}
		}

		// Handle other services
		//cout << "Other services..." << endl;

		return true;
	}

	// If connected
	inline bool isRun() {
		return _sock != INVALID_SOCKET && isConnect;
	}

	// Receive Buffer (System Buffer)
	char _recvBuf[RECV_BUFF_SIZE] = {};

	// Message Buffer (Secondary Buffer)
	char _msgBuf[RECV_BUFF_SIZE * 5] = {};

	// Last position of the message buffer
	int _lastPos = 0;

	// Receive data and unpack
	int recv() {
		// Receive data into the receive buffer
		int recvlen = (int)::recv(_sock, _recvBuf, RECV_BUFF_SIZE, 0);

		if (recvlen <= 0) {
			printf("<client %d> Disconnected...\n", _sock);
			return -1;
		}

		// Copy data into the message buffer
		memcpy(_msgBuf + _lastPos, _recvBuf, recvlen);
		_lastPos += recvlen;

		while (_lastPos >= sizeof(Message)) {
			// Get header
			Message *header = (Message *)_msgBuf;
			if (_lastPos >= header->length) {
				// Size of remaining messages
				int nSize = _lastPos - header->length;
				// Process message
				onMessage(header);
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
	virtual int onMessage(Message *msg) {
		switch (msg->cmd) {
		case CMD_TEST:
		{
			Test* test = (Test *)msg;
			// printf("<client %d> Receive Message: Test\n", _sock);
			break;
		}
		case CMD_ERROR:
		{
			printf("<client %d> Receive Message: ERROR\n", _sock);
			break;
		}
		default:
		{
			printf("<client %d> Receive Message: UNDIFINED\n", _sock);
		}
		}

		return 0;
	}

	// Send data
	int send(Message *_msg, int length) {
		if (!isRun() || _msg == NULL)
			return SOCKET_ERROR;
		int ret = ::send(_sock, (const char *)_msg, length, 0);
		if (SOCKET_ERROR == ret) {
			close();
		}
		return ret;
	}

private:
	SOCKET _sock;
	bool isConnect;
};

#endif // !_TcpClient_hpp_
