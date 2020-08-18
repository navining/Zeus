#include "TcpClient.h"
#include <stdio.h>
#include "Network.h"

TcpClient::TcpClient() {
	isConnect = false;
}

TcpClient::~TcpClient() {
	close();
}

// Initialize socket

SOCKET TcpClient::init() {
	Network::Init();

	// Create socket
	if (isRun()) {
		LOG_WARNING("<client %d> Close old connection...\n", _pClient->sockfd());
		close();
	}
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET) {
		LOG_PERROR("Create socket - Fail...\n");
	}
	else {
		// LOG::INFO("<client %d> Create socket - Success...\n", _sock);
		_pClient = std::make_shared<TcpSocket>(_sock);
	}
	return _sock;
}

// Connect server

int TcpClient::connect(const char * ip, unsigned short port) {
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
	int ret = ::connect(_pClient->sockfd(), (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		LOG_PERROR("<client %d> Connect - Fail...\n", _pClient->sockfd());
	}
	else {
		isConnect = true;
		// LOG::INFO("<client %d> Connect - Success...\n", _pClient->sockfd());
#if IO_MODE == EPOLL
		_epoll.ctl(EPOLL_CTL_ADD, _pClient->sockfd(), EPOLLIN);
#endif
	}
	return ret;
}

// Close socket

void TcpClient::close() {
	isConnect = false;
}

// Start client service

bool TcpClient::onRun() {
	if (!isRun())
	{
		LOG_PERROR("<client %d> Start - Fail...\n", _pClient->sockfd());
		return false;
	};

	// IO-mutiplex
	if (!multiplex()) {
		close();
		return false;
	}

	// Process messages
	process();

	// Handle other services
	onIdle();

	return true;
}

bool TcpClient::select() {
	fd_set fdRead;
	FD_ZERO(&fdRead);
	FD_SET(_pClient->sockfd(), &fdRead);

	fd_set fdWrite;
	FD_ZERO(&fdWrite);

	int ret = 0;

	timeval t = { 0, 0 };

	if (_pClient->isSendEmpty()) {
		int ret = ::select(_pClient->sockfd() + 1, &fdRead, NULL, NULL, &t);
	}
	else {
		// Only check fdWrite if there's something to be sent
		FD_SET(_pClient->sockfd(), &fdWrite);
		int ret = ::select(_pClient->sockfd() + 1, &fdRead, &fdWrite, NULL, &t);
	}

	if (ret < 0) {
		if (errno == EINTR) {
			LOG_WARNING("<client %d> Select - Interrupted\n", _pClient->sockfd());
			return true;
		}
		LOG_PERROR("<client %d> Select - Fail...\n", _pClient->sockfd());
		return false;
	}

	// If socket is inside the set
	if (FD_ISSET(_pClient->sockfd(), &fdRead)) {
		// Handle request
		if (SOCKET_ERROR == recv()) {
			LOG_ERROR("<client %d> Read - Fail...\n", _pClient->sockfd());
			return false;
		}
	}

	if (FD_ISSET(_pClient->sockfd(), &fdWrite)) {
		// Handle request
		if (SOCKET_ERROR == sendAll()) {
			LOG_ERROR("<client %d> Write - Fail...\n", _pClient->sockfd());
			return false;
		}
	}

	return true;
}

bool TcpClient::epoll()
{
#if IO_MODE == EPOLL
	// Only mornitor writtable sockets
	if (!_pClient->isSendEmpty()) {
		_epoll.ctl(EPOLL_CTL_MOD, _pClient->sockfd(), EPOLLIN | EPOLLOUT);
	}
	else {
		_epoll.ctl(EPOLL_CTL_MOD, _pClient->sockfd(), EPOLLIN);
	}

	int ret = _epoll.wait(1);
	if (ret < 0) {
		LOG_PERROR("<client %d> Epoll - Fail...\n", _pClient->sockfd());
		return false;
	}

	if (_epoll.events()[0].events & EPOLLIN) {
		if (SOCKET_ERROR == recv()) {
			LOG_ERROR("<client %d> Read - Fail...\n", _pClient->sockfd());
			return false;
		}
	}

	if (_epoll.events()[0].events & EPOLLOUT) {
		if (SOCKET_ERROR == sendAll()) {
			// Client disconnected
			LOG_ERROR("<client %d> Write - Fail...\n", _pClient->sockfd());
			return false;
		}
	}
	return true;
#endif

	return false;
}

bool TcpClient::iocp()
{
	return false;
}

// If connected

bool TcpClient::isRun() {
	return _pClient != nullptr && isConnect;
}


// Receive data and unpack

int TcpClient::recv() {
	return _pClient->recv();
}

void TcpClient::process()
{
	while (_pClient->hasMessage()) {
		// Pop one message from the client buffer
		Message *msg = _pClient->popMessage();
		// Convert into stream
		Stream stream(msg);
		// Process message
		onMessage(&stream);
	}
}



void TcpClient::onMessage(Stream * msg)
{

}

void TcpClient::onIdle()
{

}

// Send data

int TcpClient::send(Message * _msg) {
	return _pClient->send(_msg);
}

int TcpClient::send(Stream * _msg) {
	return _pClient->send(_msg);
}

int TcpClient::sendAll() {
	return _pClient->sendAll();
}
