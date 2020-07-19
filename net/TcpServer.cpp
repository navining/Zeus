#include "TcpServer.h"

TcpServer::TcpServer() {
	Network::Init();
	_sock = INVALID_SOCKET;
	thread_count = 0;
	_clientCount = 0;
}

TcpServer::~TcpServer() {
	close();
}

// Initialize socket

int TcpServer::init() {
	// Create socket
	if (isRun()) {
		LOG_WARNING("<server %d> Close old connection...\n", _sock);
		close();
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!isRun()) {
		LOG_ERROR("Create socket - Fail...\n");
	}
	else {
		LOG_INFO("<server %d> Create socket - Success...\n", _sock);
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
		LOG_INFO("<server %d> IP - %s\n", _sock, ip);
	}
	else {
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		_sin.sin_addr.s_addr = INADDR_ANY;
#endif
		LOG_INFO("<server %d> IP - ANY\n", _sock);
	}

	int ret = ::bind(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		LOG_ERROR("<server %d> Bind %d - Fail...\n", _sock, port);
	}
	else {
		LOG_INFO("<server %d> Bind %d - Success...\n", _sock, port);
	}

	return ret;
}

// Listen to port

int TcpServer::listen(int n) {
	int ret = ::listen(_sock, n);
	if (SOCKET_ERROR == ret) {
		LOG_ERROR("<server %d> Listen - Fail...\n", _sock);
	}
	else {
		LOG_INFO("<server %d> Listen - Success...\n", _sock);
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
		LOG_ERROR("<server %d> Invaild client socket...\n", _sock);
		return INVALID_SOCKET;
	}

	if (MAX_CLIENT >= 0 && _clientCount > MAX_CLIENT) {
#ifdef _WIN32
		::closesocket(cli);
#else
		::close(cli);
#endif
		LOG_WARNING("Reach maximum connection limit: %d\n", MAX_CLIENT);
		return INVALID_SOCKET;
	}

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

	_clientCount++;
	onConnection(pClient);

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
		LOG_ERROR("<server %d> Start - Fail...\n", _sock);
		return;
	};

#if IO_MODE == EPOLL
  _epoll.create(1);
  _epoll.ctl(EPOLL_CTL_ADD, _sock, EPOLLIN);
#endif

	while (thread.isRun()) {

		// IO multiplexing
		if (!multiplex()) {
			LOG_ERROR("IO multiplexing - Fail...\n");
			thread.exit();
			return;
		}

		// Handle other services here...
		// Benchmark
		onIdle();
	}
}

bool TcpServer::select()
{
	fd_set fdRead;
	FD_ZERO(&fdRead);
	// Put server sockets inside fd_set
	FD_SET(_sock, &fdRead);

	// Timeval
	timeval t = { 0, 1 };

	int ret = ::select(_sock + 1, &fdRead, 0, 0, &t);

	if (ret < 0) {
		LOG_ERROR("<server %d> Select - Fail...\n", _sock);
		return false;
	}

	// Server socket response: accept connection
	if (FD_ISSET(_sock, &fdRead)) {
		FD_CLR(_sock, &fdRead);
		accept();
	}

	return true;
}

bool TcpServer::epoll()
{
  #if IO_MODE == EPOLL
  int ret = _epoll.wait(1);

  if (ret < 0) {
		LOG_ERROR("<server %d> Epoll - Fail...\n", _sock);
		return false;
	}

  for (int i = 0; i < ret; i++) {
	  if (_epoll.events()[i].events & EPOLLIN) {
    	// Server socket response: accept connection
		  accept();
	  }
  }

  return true;
  #endif

	return false;
}

bool TcpServer::iocp()
{
	return false;
}

void TcpServer::onIdle() {

}

void TcpServer::onDisconnection(const TcpConnection & pClient) {

}

void TcpServer::onMessage(const TcpConnection & pClient, Message * msg) {

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
	LOG_INFO("<server %d> Quit...\n", _sock);

	_sock = INVALID_SOCKET;
}

int TcpServer::clientCount()
{
	return _clientCount;
}

