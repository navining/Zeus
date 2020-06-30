#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_

#include <iostream>

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>

#include "common.h"
#include "TcpConnection.hpp"
#include "TcpSubserver.hpp"
#include "TaskHandler.hpp"
#include "Event.h"

#define TCPSERVER_THREAD_COUNT 1

// Main thread responsible for accepting connections
class TcpServer : public Event {
public:
	TcpServer() {
		_sock = INVALID_SOCKET;
		_msgCount = 0;
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
#else
		blockSignal();
#endif // !_WIN32

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
			// Add new client into the buffer with least clients
			int minCount = INT_MAX;
			TcpSubserver *minHandler = nullptr;
			for (auto subserver : _subservers) {
				if (subserver->getClientCount() < minCount) {
					minCount = subserver->getClientCount();
					minHandler = subserver;
				}
			}
			if (minHandler == nullptr) {
				return INVALID_SOCKET;
			}

			TcpConnection pClient(new TcpSocket(cli));
			minHandler->addClients(pClient);

			onConnection(pClient);
		}

		return cli;
	}


	virtual void onConnection(const TcpConnection& pClient) {
		_clientCount++;
		// cout << "<server " << _sock << "> " << "New connection: " << "<client " << cli << "> " << inet_ntoa(clientAddr.sin_addr) << "-" << clientAddr.sin_port << endl;
	}

	// Start child threads
	void start(int numOfThreads = TCPSERVER_THREAD_COUNT) {
		for (int i = 0; i < numOfThreads; i++) {
			TcpSubserver *subserver = new TcpSubserver(i, this);
			_subservers.push_back(subserver);
			subserver->start();
		}
		_thread.start(
			EMPTY_THREAD_FUNC,		// onStart
			[this](Thread & thread) {	// onRun
				onRun(thread);
			},
			EMPTY_THREAD_FUNC	// onClose
		);
	}

	// Start server service
	void onRun(Thread & thread) {
		if (!isRun())
		{
			printf("<server %d> Start - Fail...\n", _sock);
			return;
		};

		while (thread.isRun()) {

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
				thread.exit();
				return;
			}

			// Server socket response: accept connection
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				accept();
			}

			// Handle other services here...
			// Benchmark
			benchmark();

		}
	}

	// Benchmark
	void benchmark() {
		double t1 = _time.getElapsedSecond();
		if (t1 >= 1.0) {
			printf("<server %d> Time: %f Threads: %d Clients: %d Packages: %d\n", _sock, t1, (int)_subservers.size(), (int)_clientCount, (int)_msgCount);
			_msgCount = 0;
			_time.update();
		}
	}

	virtual void onDisconnection(const TcpConnection& pClient) {
		_clientCount--;
	}

	virtual void onMessage(TcpSubserver *pServer, const TcpConnection& pClient, Message *header) {
		_msgCount++;
	}

	// If connected
	inline bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	// Close socket
	void close() {
		if (_sock == INVALID_SOCKET) return;

		for (TcpSubserver *server : _subservers) {
			delete server;
		}
		_subservers.clear();

		_thread.close();

#ifdef _WIN32
		// Close Win Sock 2.x
		closesocket(_sock);
		WSACleanup();
#else
		::close(_sock);
#endif
		printf("<server %d> Quit...\n", _sock);

		_sock = INVALID_SOCKET;
	}

private:
	std::vector<TcpSubserver *> _subservers;
	Timestamp _time;
	Thread _thread;
protected:
	std::atomic_int _msgCount;	// Number of messages
	std::atomic_int _clientCount;	// Number of clients
	SOCKET _sock;
};

#endif // !_TcpServer_hpp_
