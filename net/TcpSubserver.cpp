#include "TcpSubserver.h"

TcpSubserver::TcpSubserver(int id, Event * pEvent) : _sendTaskHandler(id) {
	_id = id;
	_pMain = pEvent;
	_tCurrent = Time::getCurrentTimeInMilliSec();;
}

TcpSubserver::~TcpSubserver() {
	close();
}

// Start server service

void TcpSubserver::onRun(Thread & thread) {
	_clientsChange = false;
	while (thread.isRun()) {
		// Sleep if there's no clients
		if (getClientCount() == 0) {
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			_tCurrent = Time::getCurrentTimeInMilliSec();
			continue;
		}

		// Add clients from the buffer to the vector
		if (_clientsBuf.size() > 0) {
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuf) {
					_clients[pClient->sockfd()] = pClient;
				}
			}
			_clientsBuf.clear();
			_clientsChange = true;
		}

		// Select
		fd_set fdRead;	// Set of sockets
		FD_ZERO(&fdRead);
		fd_set fdWrite;
		FD_ZERO(&fdWrite);

		if (_clientsChange) {
			_maxSock = INVALID_SOCKET;

			// Put client sockets inside fd_set
			for (auto it : _clients) {
				FD_SET(it.second->sockfd(), &fdRead);
				if (_maxSock < it.second->sockfd()) {
					_maxSock = it.second->sockfd();
				}
			}

			// Cache _fdRead
			memcpy(&_fdRead, &fdRead, sizeof(fd_set));
			_clientsChange = false;
		}
		else {
			// Use cached fdRead
			memcpy(&fdRead, &_fdRead, sizeof(fd_set));
		}

		memcpy(&fdWrite, &_fdRead, sizeof(fd_set));

		timeval t{ 0, 1 };
		int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);

		if (ret < 0) {
			LOG::INFO("<subserver %d> Select - Fail...\n", _id);
			thread.exit();
			return;
		}

		respondRead(fdRead);
		respondWrite(fdWrite);

		// Do other things here...
		onIdle();

		// Update current timestamp
		_tCurrent = Time::getCurrentTimeInMilliSec();
	}
}

// Client socket response: handle request

void TcpSubserver::respondRead(fd_set & fdRead) {
#ifdef _WIN32
	for (int n = 0; n < fdRead.fd_count; n++) {
		auto it = _clients.find(fdRead.fd_array[n]);
		if (it == _clients.end()) continue;
		const TcpConnection& pClient = it->second;
		if (-1 == recv(pClient)) {
			// Client disconnected
			onDisconnection(pClient);
			_clients.erase(pClient->sockfd());
			
		}
	}
#else
	for (auto it = _clients.begin(); it != _clients.end();) {
		if (FD_ISSET(it->second->sockfd(), &fdRead)) {
			if (-1 == recv(it->second)) {
				// Client disconnected
				onDisconnection(it->second);
				it = _clients.erase(it);
			}
			else {
				++it;
			}
		}
	}
#endif
}

void TcpSubserver::respondWrite(fd_set & fdWrite) {
#ifdef _WIN32
	for (int n = 0; n < fdWrite.fd_count; n++) {
		auto it = _clients.find(fdWrite.fd_array[n]);
		if (it == _clients.end()) continue;
		const TcpConnection& pClient = it->second;
		if (-1 == pClient->sendAll()) {
			// Client disconnected
			onDisconnection(pClient);
			_clients.erase(pClient->sockfd());
		}
	}
#else
	for (auto it = _clients.begin(); it != _clients.end();) {
		if (FD_ISSET(it->second->sockfd(), &fdWrite)) {
			if (-1 == recv(it->second)) {
				// Client disconnected
				onDisconnection(it->second);
				it = _clients.erase(it);
			}
			else {
				++it;
			}
		}
	}
#endif
}

// Check if the client is alive

void TcpSubserver::checkAlive() {
	time_t current = Time::getCurrentTimeInMilliSec();
	time_t dt = current - _tCurrent;
	for (auto it = _clients.begin(); it != _clients.end();) {
		if (!it->second->isAlive(dt)) {
			// Client disconnected
			onDisconnection(it->second);
			it = _clients.erase(it);
		}
		else {
			++it;
		}
	}
}

// Check if the send buffer is ready to be cleared

void TcpSubserver::checkSendBuffer() {
	time_t current = Time::getCurrentTimeInMilliSec();
	time_t dt = current - _tCurrent;
	_tCurrent = current;
	for (auto it = _clients.begin(); it != _clients.end(); ++it) {
		const TcpConnection &pClient = it->second;
		if (pClient->canSend(dt)) {
			// Add a task to clear the client buffer (send everything out)
			_sendTaskHandler.addTask([=]()->void {
				pClient->sendAll();
			});
		}
	}
}

// Receive data

int TcpSubserver::recv(const TcpConnection & pClient) {

	// Use each buffer of the client directly, no need to copy here
	int ret = pClient->recv();
	if (ret <= 0) {
		return ret;
	}

	while (pClient->hasMessage()) {
		// Pop one message from the client buffer
		Message *msg = pClient->popMessage();
		// Process message
		onMessage(pClient, msg);
	}

	return 0;
}

// Handle message

void TcpSubserver::onMessage(const TcpConnection & pClient, Message * msg) {
	pClient->reset_tHeartbeat();
	if (_pMain != nullptr) {
		_pMain->onMessage(this, pClient, msg);
	}
}

void TcpSubserver::onDisconnection(const TcpConnection & pClient)
{
	if (_pMain != nullptr) {
		_pMain->onDisconnection(pClient);
	}
	_clientsChange = true;
}

void TcpSubserver::onIdle()
{
	checkAlive();
	checkSendBuffer();
}

// Close socket

void TcpSubserver::close() {
	_sendTaskHandler.close();
	_thread.close();
	LOG::INFO("<subserver %d> Quit...\n", _id);
}

// Add new clients into the buffer

void TcpSubserver::addClients(const TcpConnection & pClient) {
	std::lock_guard<std::mutex> lock(_mutex);
	_clientsBuf.push_back(pClient);
}

// Start the server

void TcpSubserver::start() {
	_thread.start(
		EMPTY_THREAD_FUNC,	// onStart
		[this](Thread & thread) {		// onRun
		onRun(thread);
	},
		EMPTY_THREAD_FUNC
		);
	_sendTaskHandler.start();
}

// Get number of clients in the current subserver

size_t TcpSubserver::getClientCount() {
	return _clients.size() + _clientsBuf.size();
}

// Send message to the client

void TcpSubserver::send(const TcpConnection & pClient, Message * header) {
	_sendTaskHandler.addTask([=]() {
		pClient->send(header);
		delete header;
	});
}
