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
#if IO_MODE == EPOLL
  _epoll.create();
#endif
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
#if IO_MODE == EPOLL
          _epoll.ctl(EPOLL_CTL_ADD, pClient->sockfd(), EPOLLIN);
#endif
				}
			}
			_clientsBuf.clear();
			_clientsChange = true;
		}
		// IO-multiplexing
		if (!multiplex()) {
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

	// Only put writtable sockets in fdWrite
	bool needWrite = false;
	for (auto it : _clients) {
		if (!it.second->isSendEmpty()) {
			needWrite = true;
			FD_SET(it.second->sockfd(), &fdWrite);
		}
	}

	timeval t{ 0, 1 };
	int ret = 0;
	if (needWrite) {
		ret = ::select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
	}
	else {
		ret = ::select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
	}

	if (ret < 0) {
    if (errno == EINTR) {
      LOG_WARNING("<subserver %d> Select - Interrupted\n", _id);
      return true;
    }
		LOG_PERROR("<subserver %d> Select - Fail...\n", _id);
		return false;
	}

	respondRead(fdRead);
	respondWrite(fdWrite);

	return true;
}

bool TcpSubserver::epoll()
{
#if IO_MODE == EPOLL
  // Only mornitor writtable sockets
	bool needWrite = false;
	for (auto it : _clients) {
		if (!it.second->isSendEmpty()) {
			needWrite = true;
      _epoll.ctl(EPOLL_CTL_MOD, it.second->sockfd(), EPOLLIN | EPOLLOUT);
		} else {
      _epoll.ctl(EPOLL_CTL_MOD, it.second->sockfd(), EPOLLIN);
    }
	}

  int ret = _epoll.wait(1);
  if (ret < 0) {
		LOG_PERROR("<subserver %d> Epoll - Fail...\n", _id);
		return false;
	}

  for (int i = 0; i < ret; i++) {
    int sockfd = _epoll.events()[i].data.fd;
    auto it = _clients.find(sockfd);
    if (it == _clients.end()) continue;

	  if (_epoll.events()[i].events & EPOLLIN) {
      if (SOCKET_ERROR == recv(it->second)) {
				// Client disconnected
				onDisconnection(it->second);
				_clients.erase(it);
				continue;
			}
	  }

    if (_epoll.events()[i].events & EPOLLOUT) {
      if (SOCKET_ERROR == it->second->sendAll()) {
				// Client disconnected
				onDisconnection(it->second);
				_clients.erase(it);
				continue;
			}
	  }
  }

  return true;
#endif

	return false;
}

bool TcpSubserver::iocp()
{
	return false;
}

// Client socket response: handle request

void TcpSubserver::respondRead(fd_set & fdRead) {
#ifdef _WIN32
	for (int n = 0; n < fdRead.fd_count; n++) {
		auto it = _clients.find(fdRead.fd_array[n]);
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
		if (FD_ISSET(it->second->sockfd(), &fdRead)) {
			if (SOCKET_ERROR == recv(it->second)) {
				// Client disconnected
				onDisconnection(it->second);
				it = _clients.erase(it);
        continue;
			}
		}
    ++it;
	}
#endif
}

void TcpSubserver::respondWrite(fd_set & fdWrite) {
#ifdef _WIN32
	for (int n = 0; n < fdWrite.fd_count; n++) {
		auto it = _clients.find(fdWrite.fd_array[n]);
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
		if (FD_ISSET(it->second->sockfd(), &fdWrite)) {
			if (SOCKET_ERROR == recv(it->second)) {
				// Client disconnected
				onDisconnection(it->second);
				it = _clients.erase(it);
        continue;
			}
		}
    ++it;
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

// Receive data

int TcpSubserver::recv(const TcpConnection & pClient) {

	// Use each buffer of the client directly, no need to copy here
	int ret = pClient->recv();

	if (ret < 0) {
		return SOCKET_ERROR;
	}

	return 0;
}

void TcpSubserver::process() {
	for (auto it : _clients) {
		TcpConnection & pClient = it.second;
		if (pClient->hasMessage()) {
			// Pop one message from the client buffer
			Message *msg = pClient->popMessage();
			// Convert into stream
			Stream stream(msg);
			// Process message
			onMessage(pClient, &stream);
		}
	}
}

// Handle message

void TcpSubserver::onMessage(const TcpConnection & pClient, Stream * msg) {
	pClient->reset_tHeartbeat();
	if (_pMain != nullptr) {
		_pMain->onMessage(pClient, msg);
	}
}

void TcpSubserver::onDisconnection(const TcpConnection & pClient)
{
	_clientsChange = true;
#if IO_MODE == EPOLL
  _epoll.ctl(EPOLL_CTL_DEL, pClient->sockfd(), 0);
#endif
	if (_pMain != nullptr) {
		_pMain->onDisconnection(pClient);
	}
}

void TcpSubserver::onConnection(const TcpConnection & pClient)
{
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

