#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_
#ifdef _WIN32
#define FD_SETSIZE	2048
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
#include <limits.h>
#define SOCKET int
#define INVALID_SOCKET    (SOCKET)(~0)
#define SOCKET_ERROR        (-1)
#endif

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "Message.hpp"
#include "common.h"

#define TCPSERVER_THREAD_COUNT 4

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

	int send(Header *msg) {
		if (msg != nullptr) {
			return ::send(_sockfd, (const char *)msg, msg->length, 0);
		}
		return SOCKET_ERROR;
	}
private:
	SOCKET _sockfd;	// socket fd_set			
	char _msgBuf[MSG_BUFF_SIZE];	// Message Buffer (Secondary Buffer)
	int _lastPos;	// Last position of the message buffer
};

// Interface for handling events
class Event {
public:
	// Client connect event
	virtual void onConnect(ClientSocket *pClient) = 0;
	// Client disconnect event
	virtual void onDisconnect(ClientSocket *pClient) = 0;
	// Recieve message event
	virtual void onMessage(ClientSocket *pClient, Header *header) = 0;
private:
};

// Child thread responsible for handling messsages
class MessageHandler : Event
{
public:
	MessageHandler(SOCKET sock = INVALID_SOCKET, Event *pEvent = nullptr) {
		_sock = sock;
		_pMain = pEvent;
	}

	~MessageHandler() {
		close();
	}

	void onConnect(ClientSocket *pClient) {}
	void onDisconnect(ClientSocket *pClient) {}

	// If connected
	inline bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	// Start server service
	bool onRun() {
		while (isRun()) {
			// Sleep if there's no clients
			if (getClientCount() == 0) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			// Add clients from the buffer to the vector
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuf) {
					_clients.push_back(pClient);
				}
			}
			_clientsBuf.clear();

			// Select
			fd_set fdRead;
			FD_ZERO(&fdRead);

			// Record curret max socket
			SOCKET maxSock = INVALID_SOCKET;

			// Put client sockets inside fd_set
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd()) {
					maxSock = _clients[n]->sockfd();
				}
			}

			int ret = select(maxSock + 1, &fdRead, 0, 0, 0);

			if (ret < 0) {
				printf("<server %d> Select - Fail...\n", _sock);
				close();
				return false;
			}

			// Client socket response: handle request
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == recv(_clients[n])) {
						// Client disconnected
						auto it = _clients.begin() + n;
						if (it != _clients.end()) {
							if (_pMain != nullptr) {
								_pMain->onDisconnect(_clients[n]);
							}
							delete _clients[n];
							_clients.erase(it);
						}
					}
				}
			}

			// Handle other services
		}
		return true;
	}

	// Recieve Buffer (System Buffer)
	char _recvBuf[RECV_BUFF_SIZE] = {};

	// Recieve data
	int recv(ClientSocket *pClient) {
		// Receive header
		int recvlen = (int)::recv(pClient->sockfd(), _recvBuf, RECV_BUFF_SIZE, 0);
		if (recvlen <= 0) {
			// printf("<server %d> <client %d> Disconnected...\n", _sock, pClient->sockfd());
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
				onMessage(pClient, header);
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

	// Send data
	int send(SOCKET _cli, Header *_msg) {
		if (!isRun() || _msg == NULL)
			return SOCKET_ERROR;
		return ::send(_cli, (const char *)_msg, _msg->length, 0);
	}

	// Handle message
	virtual void onMessage(ClientSocket *pClient, Header *msg) {
		if (_pMain != nullptr) {
			_pMain->onMessage(pClient, msg);
		}
		switch (msg->cmd) {
		case CMD_LOGIN:
		{
			Login* login = (Login *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _login->cmd << " Data length: " << _login->length << " Username: " << _login->username << " Password: " << _login->password << endl;
			// Judge username and password
			// Send
			LoginResult result;
			pClient->send(&result);
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout *)msg;
			//cout << "<server " << _sock << "> " << "From: " << "<client " << cli << "> " << "Command: " << _logout->cmd << " Data length: " << _logout->length << " Username: " << _logout->username << endl;
			// Send
			LogoutResult result;
			pClient->send(&result);
			break;
		}
		case CMD_TEST:	// Send back the test data (echo)
		{
			Test* _test = (Test *)msg;
			// Send
			Test result;
			pClient->send(&result);
			break;
		}
		default:
		{
			// cout << "<server " << _sock << "> " << "From: " << "<client " << _sock << "> " << "Recieve Message: " << "Unknown" << " Data Length: " << msg->length << endl;
		}
		}

	}

	// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;
		printf("<server %d> Quit...\n", _sock);
#ifdef _WIN32
		for (int i = (int)_clients.size() - 1; i >= 0; i--) {
			closesocket(_clients[i]->sockfd());
			delete _clients[i];
		}
		// Close Win Sock 2.x
		closesocket(_sock);
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

	// Add new clients into the buffer
	void addClients(ClientSocket* pClient) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuf.push_back(pClient);
	}

	// Start the server
	void start() {
		_thread = std::thread(std::mem_fn(&MessageHandler::onRun), this);
		_thread.detach();
	}

	// Get number of clients in the current handler
	size_t getClientCount() {
		return _clients.size() + _clientsBuf.size();
	}

private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	std::vector<ClientSocket*> _clientsBuf;	// Clients buffer
	std::mutex _mutex;	// Mutex for clients buffer
	std::thread _thread;
	Event *_pMain;	// Pointer to the main thread (for event callback)
};

// Main thread responsible for accepting connections
class TcpServer : public Event {
public:
	TcpServer() {
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
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
		if (isRun()) {
			printf("<server %d> Close old connection...\n", _sock);
			close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (!isRun()) {
			printf("Create socket - Fail...\n");
		}
		else {
			printf("<server %d> Create socket - Success...\n", _sock);
		}
		return _sock;
	}

	// Bind IP and port
	int bind(const char* ip, unsigned short port) {
		if (!isRun()) {
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
			printf("<server %d> Bind %d - Fail...\n", _sock, port);
		}
		else {
			printf("<server %d> Bind %d - Success...\n", _sock, port);
		}

		return ret;
	}

	// Listen to port
	int listen(int n) {
		int ret = ::listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("<server %d> Listen - Fail...\n", _sock);
		}
		else {
			printf("<server %d> Listen - Success...\n", _sock);
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
			printf("<server %d> Invaild client socket...\n", _sock);
		}
		else {
			onConnect(new ClientSocket(cli));
		}

		return cli;
	}

	// Add new client into the buffer with least clients
	virtual void onConnect(ClientSocket* pClient) {
		int minCount = INT_MAX;
		MessageHandler *minHandler = nullptr;
		for (auto handler : _handlers) {
			if (handler->getClientCount() < minCount) {
				minCount = handler->getClientCount();
				minHandler = handler;
			}
		}
		if (minHandler == nullptr) {
			return;
		}
		minHandler->addClients(pClient);
		_clientCount++;
		// cout << "<server " << _sock << "> " << "New connection: " << "<client " << cli << "> " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;
	}

	// Start child threads
	void start() {
		for (int i = 0; i < TCPSERVER_THREAD_COUNT; i++) {
			MessageHandler *handler = new MessageHandler(_sock, this);
			_handlers.push_back(handler);
			handler->start();
		}
	}

	// Start server service
	bool onRun() {
		if (!isRun())
		{
			printf("<server %d> Start - Fail...\n", _sock);
			return false;
		};

		// Select
		fd_set fdRead;
		FD_ZERO(&fdRead);
		// Put server sockets inside fd_set
		FD_SET(_sock, &fdRead);

		// Timeval
		timeval t = { 0, 10 };

		int ret = select(_sock + 1, &fdRead, 0, 0, &t);

		if (ret < 0) {
			printf("<server %d> Select - Fail...\n", _sock);
			close();
			return false;
		}

		// Server socket response: accept connection
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			accept();
		}

		// Handle other services here...
		// Benchmark
		benchmark();

		return true;
	}

	// Benchmark
	void benchmark() {
		double t1 = _time.getElapsedSecond();
		if (t1 >= 1.0) {
			printf("<server %d> Time: %f Threads: %d Clients: %d Packages: %d\n", _sock, t1, TCPSERVER_THREAD_COUNT, (int)_clientCount, _recvCount);
			_recvCount = 0;
			_time.update();
		}
	}

	virtual void onDisconnect(ClientSocket *pClient) {
		_clientCount--;
	}

	virtual void onMessage(ClientSocket *pClient, Header *header) {
		_recvCount++;
	}

	// If connected
	inline bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;
		printf("<server %d> Quit...\n", _sock);
#ifdef _WIN32
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		::close(_sock);
#endif
		_sock = INVALID_SOCKET;
		_handlers.clear();
	}

private:
	SOCKET _sock;
	std::vector<MessageHandler*> _handlers;
	Timestamp _time;
	std::atomic_int _recvCount;	// Number of packages
	std::atomic_int _clientCount;	// Number of clients
};

#endif // !_TcpServer_hpp_
