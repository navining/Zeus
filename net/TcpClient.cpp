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
		LOG_INFO("<client %d> Close old connection...\n", _pClient->sockfd());
		close();
	}
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET) {
		LOG_INFO("Create socket - Fail...\n");
	}
	else {
		// LOG::INFO("<client %d> Create socket - Success...\n", _sock);
		_pClient = new TcpSocket(_sock);
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
		LOG_INFO("<client %d> Connect - Fail...\n", _pClient->sockfd());
	}
	else {
		isConnect = true;
		// LOG::INFO("<client %d> Connect - Success...\n", _pClient->sockfd());
	}
	return ret;
}

// Close socket

void TcpClient::close() {
	if (_pClient == nullptr) return;
	delete _pClient;
	_pClient = nullptr;
	isConnect = false;
}

// Start client service

bool TcpClient::onRun() {
	if (!isRun())
	{
		LOG_INFO("<client %d> Start - Fail...\n", _pClient->sockfd());
		return false;
	};

	// Select
	fd_set fdRead;
	FD_ZERO(&fdRead);
	FD_SET(_pClient->sockfd(), &fdRead);

	fd_set fdWrite;
	FD_ZERO(&fdWrite);

	int ret = 0;

	timeval t = { 0, 1 };

	if (_pClient->isSendEmpty()) {
		int ret = select(_pClient->sockfd() + 1, &fdRead, NULL, NULL, &t);
	}
	else {
		// Only check fdWrite if there's something to be sent
		FD_SET(_pClient->sockfd(), &fdWrite);
		int ret = select(_pClient->sockfd() + 1, &fdRead, &fdWrite, NULL, &t);
	}

	if (ret < 0) {
		LOG_INFO("<client %d> Select - Fail...\n", _pClient->sockfd());
		close();
		return false;
	}

	// If socket is inside the set
	if (FD_ISSET(_pClient->sockfd(), &fdRead)) {
		// Handle request
		if (-1 == recv()) {
			LOG_INFO("<client %d> Read - Fail...\n", _pClient->sockfd());
			close();
			return false;
		}
	}

	if (FD_ISSET(_pClient->sockfd(), &fdWrite)) {
		// Handle request
		if (-1 == _pClient->sendAll()) {
			LOG_INFO("<client %d> Write - Fail...\n", _pClient->sockfd());
			close();
			return false;
		}
	}

	// Handle other services
	onIdle();

	return true;
}

// If connected

bool TcpClient::isRun() {
	return _pClient != nullptr && isConnect;
}


// Receive data and unpack

int TcpClient::recv() {
	int ret = _pClient->recv();
	if (ret <= 0) {
		return ret;
	}

	while (_pClient->hasMessage()) {
		// Pop one message from the client buffer
		Message *msg = _pClient->popMessage();
		// Process message
		onMessage(msg);
	}

	return ret;
}

void TcpClient::onMessage(Message * msg)
{

}

void TcpClient::onIdle()
{

}

// Send data

int TcpClient::send(Message * _msg) {
	return _pClient->send(_msg);
}

int TcpClient::sendAll() {
	return _pClient->sendAll();
}
