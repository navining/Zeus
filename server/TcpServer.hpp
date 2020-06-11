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
#include "Timestamp.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
#ifndef MSG_BUFF_SIZE
#define MSG_BUFF_SIZE 102400
#endif
#ifndef THREAD_COUNT
#define THREAD_COUNT 4
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

class Event {
public:
	// Client disconnect event
	virtual void onDisconnect(ClientSocket *pClient) = 0;
private:
};

// Child thread responsible for handling messsages
class MessageHandler
{
public:
	MessageHandler(SOCKET sock = INVALID_SOCKET, Event *pEvent = nullptr) {
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
		_pEvent = pEvent;
	}

	~MessageHandler() {
		close();
	}

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
			SOCKET maxSock = INVALID_SOCKET;

			// Put client sockets inside fd_set
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd()) {
					maxSock = _clients[n]->sockfd();
				}
			}

			// Timeval
			timeval t = { 0, 0 };

			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);

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
							if (_pEvent != nullptr) {
								_pEvent->onDisconnect(_clients[n]);
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
				onMessage(pClient->sockfd(), header);
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

	// Handle message
	virtual int onMessage(SOCKET cli, Header *msg) {
		_recvCount++;
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
			// cout << "<server " << _sock << "> " << "From: " << "<client " << _sock << "> " << "Recieve Message: " << "Unknown" << " Data Length: " << msg->length << endl;
		}
		}

		return 0;
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

	// Add new clients into the buffer
	void addClient(ClientSocket* pClient) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuf.push_back(pClient);
	}

	// Start the server
	void start() {
		_pThread = new std::thread(std::mem_fun(&MessageHandler::onRun), this);
		_pThread->detach();
	}

	// Get number of clients in the current handler
	size_t getClientCount() {
		return _clients.size() + _clientsBuf.size();
	}

private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	std::vector<ClientSocket*> _clientsBuf;	// Clients buffer
	std::mutex _mutex;
	std::thread *_pThread;
	Event *_pEvent;
public:
	std::atomic_int _recvCount;
};

// Main thread responsible for accepting connections
class TcpServer : public Event {
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
			addClient(new ClientSocket(cli));
			// cout << "<server " << _sock << "> " << "New connection: " << "<client " << cli << "> " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;
		}

		return cli;
	}

	// Add new client into the buffer with least clients
	void addClient(ClientSocket* pClient) {
		_clients.push_back(pClient);

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
		minHandler->addClient(pClient);
	}

	// Start child threads
	void start() {
		for (int i = 0; i < THREAD_COUNT; i++) {
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
		fd_set fdWrite;
		fd_set fdExcept;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		// Put server sockets inside fd_set
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExcept);

		// Timeval
		timeval t = { 0, 10 };

		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);

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
			int recvCount = 0;
			for (auto handler : _handlers) {
				recvCount += handler->_recvCount;
				handler->_recvCount = 0;
			}
			printf("<server %d> Time: %f Threads: %d Clients: %d Packages: %d\n", _sock, t1, THREAD_COUNT, (int)_clients.size(), recvCount);
			_time.update();
		}
	}

	// Send data
	int send(SOCKET _cli, Header *_msg) {
		if (!isRun() || _msg == NULL)
			return SOCKET_ERROR;
		return ::send(_cli, (const char *)_msg, _msg->length, 0);
	}

	// Broadcast data
	void broadcast(Header *_msg) {
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			send(_clients[n]->sockfd(), _msg);
		}
	}

	void onDisconnect(ClientSocket *pClient) {
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			if (_clients[n] == pClient) {
				auto it = _clients.begin() + n;
				if (it != _clients.end())
					_clients.erase(it);
			}
		}
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
	std::vector<ClientSocket*> _clients;
	std::vector<MessageHandler*> _handlers;
	Timestamp _time;
};

#endif // !_TcpServer_hpp_
