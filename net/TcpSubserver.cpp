#include "TcpSubserver.h"

TcpSubserver::TcpSubserver(int id, Event * pEvent) /*: _sendTaskHandler(id)*/ {
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
		// Update current timestamp
		_tCurrent = Time::getCurrentTimeInMilliSec();

		// Sleep if there's no clients
		if (getClientCount() == 0) {
			Thread::sleep(1);
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

		// IO-multiplexing
		if (!select()) {
			thread.exit();
		}

		// Process messages
		process();

		// Do other things here...
		onIdle();

		
	}
}

// Select

bool TcpSubserver::select() {
	_fdRead.zero();

	if (_clientsChange) {
		_maxSock = INVALID_SOCKET;

		// Put client sockets inside fd_set
		for (auto it : _clients) {
			_fdRead.set(it.second->sockfd());
			if (_maxSock < it.second->sockfd()) {
				_maxSock = it.second->sockfd();
			}
		}

		// Cache _fdRead
		_fdRead_cached = _fdRead;
		_clientsChange = false;
	}
	else {
		// Use cached fdRead
		_fdRead = _fdRead_cached;
	}

	_fdWrite = _fdRead;

	timeval t{ 0, 1 };
	int ret = ::select(_maxSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);

	if (ret < 0) {
		LOG_ERROR("<subserver %d> Select - Fail...\n", _id);
		return false;
	}

	respondRead();
	respondWrite();

	return true;
}

// Client socket response: handle request

void TcpSubserver::respondRead() {
#ifdef _WIN32
	for (int n = 0; n < _fdRead.fdset()->fd_count; n++) {
		auto it = _clients.find(_fdRead.fdset()->fd_array[n]);
		if (it == _clients.end()) continue;
		const TcpConnection& pClient = it->second;
		if (SOCKET_ERROR == recv(pClient)) {
			// Client disconnected
			onDisconnection(pClient);
			_clients.erase(pClient->sockfd());
			
		}
	}
#else
	for (auto it = _clients.begin(); it != _clients.end();) {
		if (_fdRead.isset(it->second->sockfd()) {
			if (SOCKET_ERROR == recv(it->second)) {
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

void TcpSubserver::respondWrite() {
#ifdef _WIN32
	for (int n = 0; n < _fdWrite.fdset()->fd_count; n++) {
		auto it = _clients.find(_fdWrite.fdset()->fd_array[n]);
		if (it == _clients.end()) continue;
		const TcpConnection& pClient = it->second;
		if (SOCKET_ERROR == pClient->sendAll()) {
			// Client disconnected
			onDisconnection(pClient);
			_clients.erase(pClient->sockfd());
		}
	}
#else
	for (auto it = _clients.begin(); it != _clients.end();) {
		if (_fdWrite.isset(it->second->sockfd()) {
			if (SOCKET_ERROR == recv(it->second)) {
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
/*
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
*/

// Receive data

int TcpSubserver::recv(const TcpConnection & pClient) {

	// Use each buffer of the client directly, no need to copy here
	int ret = pClient->recv();

	if (ret <= 0) {
		return ret;
	}

	return 0;
}

void TcpSubserver::process() {
	for (auto it : _clients) {
		TcpConnection & pClient = it.second;
		if (pClient->hasMessage()) {
			// Pop one message from the client buffer
			Message *msg = pClient->popMessage();
			// Process message
			onMessage(pClient, msg);
		}
	}
}

// Handle message

void TcpSubserver::onMessage(const TcpConnection & pClient, Message * msg) {
	pClient->reset_tHeartbeat();
	if (_pMain != nullptr) {
		_pMain->onMessage(pClient, msg);
	}
}

void TcpSubserver::onDisconnection(const TcpConnection & pClient)
{
	_clientsChange = true;
	if (_pMain != nullptr) {
		_pMain->onDisconnection(pClient);
	}
}

void TcpSubserver::onIdle()
{
	checkAlive();
	// checkSendBuffer();
}

// Close socket

void TcpSubserver::close() {
	// _sendTaskHandler.close();
	_thread.close();
	LOG_INFO("<subserver %d> Quit...\n", _id);
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
	// _sendTaskHandler.start();
}

// Get number of clients in the current subserver

size_t TcpSubserver::getClientCount() {
	return _clients.size() + _clientsBuf.size();
}

