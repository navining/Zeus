#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_
#ifdef _WIN32
#define FD_SETSIZE	1024
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
#include "Timestamp.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
#ifndef MSG_BUFF_SIZE
#define MSG_BUFF_SIZE 102400
#endif

class ClientSocket {
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_msgBuf, 0, sizeof(_msgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd() {
		return _sockfd;
	}

	char* msgBuf() {
		return _msgBuf;
	}

	int getLastPos() {
		return _lastPos;
	}

	void setLastPos(int pos) {
		_lastPos = pos;
	}
private:
	SOCKET _sockfd;	// socket fd_set			
	char _msgBuf[MSG_BUFF_SIZE];	// Message Buffer (Secondary Buffer)
	int _lastPos;	// Last position of the message buffer
};

class TcpServer {
public:
	TcpServer() {
		_sock = INVALID_SOCKET;
		_recvCount = 0;
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
			cout << "<server " << _sock << "> " << "Create socket - Success..." << endl;
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
		SOCKET cli = ::accept(_sock, (sockaddr *)&clientAddr, &addrlen);
#else
		SOCKET cli = ::accept(_sock, (sockaddr *)&clientAddr, (socklen_t *)&addrlen);
#endif
		if (INVALID_SOCKET == cli) {
			cout << "<server " << _sock << "> " << "Invaild client socket..." << endl;
		}
		else {
			// Broadcast
			NewUserJoin userJoin;
			userJoin.sock = cli;
			broadcast(&userJoin);
		}

		_clients.push_back(new ClientSocket(cli));
		cout << "<server " << _sock << "> " << "New connection: " << "<client " << cli << "> " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;

		return cli;
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
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			FD_SET(_clients[n]->sockfd(), &fdRead);
			if (maxSock < _clients[n]->sockfd()) {
				maxSock = _clients[n]->sockfd();
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
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
				if (-1 == recv(_clients[n])) {
					auto it = _clients.begin() + n;
					if (it != _clients.end()) {
						delete _clients[n];
						_clients.erase(it);
					}
				}
			}
		}

		// Handle other services
		//cout << "Other services..." << endl;

		return true;
	}

	// Recieve Buffer (System Buffer)
	char _recvBuf[RECV_BUFF_SIZE] = {};

	int recv(ClientSocket *pClient) {
		// Receive header
		int recvlen = (int)::recv(pClient->sockfd(), _recvBuf, RECV_BUFF_SIZE, 0);
		if (recvlen <= 0) {
			cout << "<server " << _sock << "> " << "<client " << pClient->sockfd() << "> " << "Disconnected..." << endl;
			return -1;
		}

		// Copy data into the message buffer
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _recvBuf, recvlen);
		pClient->setLastPos(pClient->getLastPos() + recvlen);

		while (pClient->getLastPos() >= sizeof(Header)) {
			// Get header
			Header *header = (Header *)pClient->msgBuf();
			if (pClient->getLastPos() >= header->length) {
				// Size of remaining messages
				int nSize = pClient->getLastPos() - header->length;
				// Process message
				process(pClient->sockfd(), header);
				// Move remaining messages forward.
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->length, nSize);
				pClient->setLastPos(nSize);
			}
			else {
				break;
			}
		}
		return 0;
	}

	virtual int process(SOCKET cli, Header *msg) {
		// Benchmark
		_recvCount++;
		double t1 = _time.getElapsedSecond();
		if (_time.getElapsedSecond() >= 1.0) {
			printf("<server %d> Time: %f Clients: %d Packages: %d\n", _sock, t1, (int)_clients.size(), _recvCount);
			_recvCount = 0;
			_time.update();
		}
		
		switch (msg->cmd) {
		case CMD_LOGIN:
		{
			Login* _login = (Login *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
			// Judge username and password
			// Send
			//LoginResult _result;
			//send(cli, &_result);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* _logout = (Logout *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
			// Send
			//LogoutResult _result;
			//send(cli, &_result);
			break;
		}
		default:
		{
			cout << "<server " << _sock << "> " << "From: " << "<client " << _sock << "> " << "Recieve Message: " << "Unknown" << " Data Length: " << msg->length << endl;
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
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			send(_clients[n]->sockfd(), _msg);
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
		for (int i = (int)_clients.size() - 1; i >= 0; i--) {
			closesocket(_clients[i]->sockfd());
			delete _clients[i];
		}
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		for (int i = (int)_clients.size() - 1; i >= 0; i--) {
			::close(_clients[i]->sockfd());
			delete _clients[i];
		}
		::close(_sock);
#endif
		_sock = INVALID_SOCKET;
		_clients.clear();
	}

private:
	SOCKET _sock;
	vector<ClientSocket*> _clients;
	Timestamp _time;
	int _recvCount;
};

#endif // !_TcpServer_hpp_
