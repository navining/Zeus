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
		printf("<client %d> Close old connection...\n", _pClient->sockfd());
		close();
	}
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET) {
		printf("Create socket - Fail...\n");
	}
	else {
		// printf("<client %d> Create socket - Success...\n", _sock);
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
		printf("<client %d> Connect - Fail...\n", _pClient->sockfd());
	}
	else {
		isConnect = true;
		// printf("<client %d> Connect - Success...\n", _pClient->sockfd());
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
		printf("<client %d> Start - Fail...\n", _pClient->sockfd());
		return false;
	};

	// Select
	fd_set fdRead;
	FD_ZERO(&fdRead);
	FD_SET(_pClient->sockfd(), &fdRead);
	timeval t = { 0, 0 };
	int ret = select(_pClient->sockfd() + 1, &fdRead, NULL, NULL, &t);

	if (ret < 0) {
		printf("<client %d> Select - Fail...\n", _pClient->sockfd());
		close();
		return false;
	}

	// If socket is inside the set
	if (FD_ISSET(_pClient->sockfd(), &fdRead)) {
		FD_CLR(_pClient->sockfd(), &fdRead);
		// Handle request
		if (-1 == recv()) {
			printf("<client %d> Process - Fail...\n", _pClient->sockfd());
			close();
			return false;
		}
	}

	// Handle other services
	//cout << "Other services..." << endl;

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

// Process data

int TcpClient::onMessage(Message * msg) {
	switch (msg->cmd) {
	case CMD_TEST:
	{
		Test* test = (Test *)msg;
		// printf("<client %d> Receive Message: Test\n", _pClient->sockfd());
		break;
	}
	case CMD_ERROR:
	{
		printf("<client %d> Receive Message: ERROR\n", _pClient->sockfd());
		break;
	}
	default:
	{
		printf("<client %d> Receive Message: UNDIFINED\n", _pClient->sockfd());
	}
	}

	return 0;
}

// Send data

int TcpClient::send(Message * _msg) {
	return _pClient->send(_msg);
}
