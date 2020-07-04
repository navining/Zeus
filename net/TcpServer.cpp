#include "TcpServer.h"

TcpServer::TcpServer() {
	Network::Init();
	_sock = INVALID_SOCKET;
	thread_count = 0;
}

TcpServer::~TcpServer() {
	close();
}

// Initialize socket

int TcpServer::init() {
	// Create socket
	if (isRun()) {
		LOG::INFO("<server %d> Close old connection...\n", _sock);
		close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!isRun()) {
		LOG::INFO("Create socket - Fail...\n");
	}
	else {
		LOG::INFO("<server %d> Create socket - Success...\n", _sock);
	}
	return _sock;
}

// Bind IP and port

int TcpServer::bind(const char * ip, unsigned short port) {
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
		LOG::INFO("<server %d> Bind %d - Fail...\n", _sock, port);
	}
	else {
		LOG::INFO("<server %d> Bind %d - Success...\n", _sock, port);
	}

	return ret;
}

// Listen to port

int TcpServer::listen(int n) {
	int ret = ::listen(_sock, n);
	if (SOCKET_ERROR == ret) {
		LOG::INFO("<server %d> Listen - Fail...\n", _sock);
	}
	else {
		LOG::INFO("<server %d> Listen - Success...\n", _sock);
	}

	return ret;

}

// Accept client connection

SOCKET TcpServer::accept() {
	// Accept
	sockaddr_in clientAddr = {};
	int addrlen = sizeof(sockaddr_in);
#ifdef _WIN32
	SOCKET cli = ::accept(_sock, (sockaddr *)&clientAddr, &addrlen);
#else
	SOCKET cli = ::accept(_sock, (sockaddr *)&clientAddr, (socklen_t *)&addrlen);
#endif
	if (INVALID_SOCKET == cli) {
		LOG::INFO("<server %d> Invaild client socket...\n", _sock);
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

void TcpServer::onConnection(const TcpConnection & pClient) {

}

// Start child threads

void TcpServer::start(int numOfThreads) {
	thread_count = numOfThreads;
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

void TcpServer::onRun(Thread & thread) {
	if (!isRun())
	{
		LOG::INFO("<server %d> Start - Fail...\n", _sock);
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
			LOG::INFO("<server %d> Select - Fail...\n", _sock);
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
		onIdle();
	}
}

void TcpServer::onIdle() {

}

void TcpServer::onDisconnection(const TcpConnection & pClient) {
	
}

void TcpServer::onMessage(TcpSubserver * pServer, const TcpConnection & pClient, Message * header) {
	
}

// If connected

bool TcpServer::isRun() {
	return _sock != INVALID_SOCKET;
}

// Close socket

void TcpServer::close() {
	if (_sock == INVALID_SOCKET) return;

	for (TcpSubserver *server : _subservers) {
		delete server;
	}
	_subservers.clear();

	_thread.close();

#ifdef _WIN32
	// Close Win Sock 2.x
	closesocket(_sock);
#else
	::close(_sock);
#endif
	LOG::INFO("<server %d> Quit...\n", _sock);

	_sock = INVALID_SOCKET;
}

