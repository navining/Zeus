#ifndef _TcpSubserver_hpp_
#define _TcpSubserver_hpp_

#include <unordered_map>
#include <vector>

#include "common.h"
#include "Event.h"
#include "Task.hpp"

// Child thread responsible for handling messsages
class TcpSubserver
{
public:
	TcpSubserver(int id, Event *pEvent = nullptr) {
		_id = id;
		_pMain = pEvent;
		_tCurrent = Time::getCurrentTimeInMilliSec();;
	}

	~TcpSubserver() {
		close();
	}

	// Start server service
	void onRun() {
		_clientsChange = false;
		while (_isRun) {
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

			timeval t{ 0, 1 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);

			if (ret < 0) {
				printf("<subserver %d> Select - Fail...\n", _id);
				close();
				return;
			}

			respond(fdRead);

			// Do other things here...
			checkAlive();
			checkSendBuffer();

			// Update current timestamp
			_tCurrent = Time::getCurrentTimeInMilliSec();
		}
	}

	// Client socket response: handle request
	void respond(fd_set &fdRead) {
#ifdef _WIN32
		for (int n = 0; n < fdRead.fd_count; n++) {
			const TcpConnection& pClient = _clients[fdRead.fd_array[n]];
			if (-1 == recv(pClient)) {
				// Client disconnected
				if (_pMain != nullptr) {
					_pMain->onDisconnection(pClient);
				}
				_clients.erase(pClient->sockfd());
				_clientsChange = true;
			}
		}
#else
		std::vector<TcpConnection > disconnected;
		for (auto it : _clients) {
			if (FD_ISSET(it.second->sockfd(), &fdRead)) {
				if (-1 == recv(it.second)) {
					// Client disconnected
					if (_pMain != nullptr) {
						_pMain->onDisconnection(it.second);
					}

					disconnected.push_back(it.second);
					_clientsChange = true;
				}
			}
		}

		// Delete disconnected clients
		for (TcpConnection pClient : disconnected) {
			_clients.erase(pClient->sockfd());
		}
#endif
	}

	// Check if the client is alive
	void checkAlive() {
		time_t current = Time::getCurrentTimeInMilliSec();
		time_t dt = current - _tCurrent;
		for (auto it = _clients.begin(); it != _clients.end();) {
			if (!it->second->isAlive(dt)) {
				// Client disconnected
				if (_pMain != nullptr) {
					_pMain->onDisconnection(it->second);
				}
				it = _clients.erase(it);
				_clientsChange = true;
			}
			else {
				++it;
			}
		}
	}

	// Check if the send buffer is ready to be cleared
	void checkSendBuffer() {
		time_t current = Time::getCurrentTimeInMilliSec();
		time_t dt = current - _tCurrent;
		_tCurrent = current;
		for (auto it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second->canSend(dt)) {
				// Add a task to clear the client buffer
				// TODO: Have bugs here! Need a way to check iterator validity
				_sendTaskHandler.addTask( [=]()->void {
					it->second->clearBuffer();
					it->second->resetSendBuf();
				});
			}
		}
	}

	/// Recieve Buffer (System Buffer)
	/// char _recvBuf[RECV_BUFF_SIZE] = {};

	// Recieve data
	int recv(const TcpConnection& pClient) {
		/// Receive header into system buffer first
		/// int recvlen = (int)::recv(pClient->sockfd(), _recvBuf, RECV_BUFF_SIZE, 0);
		// Use each buffer of the client directly, no need to copy here
		int recvlen = (int)::recv(pClient->sockfd(), pClient->msgBuf() + pClient->getLastPos(), RECV_BUFF_SIZE - pClient->getLastPos(), 0);
		if (recvlen <= 0) {
			// printf("<server %d> <client %d> Disconnected...\n", _sock, pClient->sockfd());
			return -1;
		}
	
		/// Copy data from the system buffer into the message buffer
		/// memcpy(pClient->msgBuf() + pClient->getLastPos(), _recvBuf, recvlen);
		pClient->setLastPos(pClient->getLastPos() + recvlen);

		while (pClient->getLastPos() >= sizeof(Message)) {
			// Get header
			Message *header = (Message *)pClient->msgBuf();
			if (pClient->getLastPos() >= header->length) {
				// Size of remaining messages
				int nSize = pClient->getLastPos() - header->length;
				// Process message
				onMessage(pClient, header);
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
	void onMessage(const TcpConnection& pClient, Message *msg) {
		pClient->resetHeartbeat();
		if (_pMain != nullptr) {
			_pMain->onMessage(this, pClient, msg);
		}
	}

	// Close socket
	void close() {
		printf("<subserver %d> Quit...\n", _id);
		_clients.clear();
		_clientsBuf.clear();
		_sendTaskHandler.close();
	}

	// Add new clients into the buffer
	void addClients(const TcpConnection& pClient) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuf.push_back(pClient);
	}

	// Start the server
	void start() {
		if (_isRun) return;
		_isRun = true;
		_thread = std::thread(std::mem_fun(&TcpSubserver::onRun), this);
		_thread.detach();
		_sendTaskHandler.start();
	}

	// Get number of clients in the current subserver
	size_t getClientCount() {
		return _clients.size() + _clientsBuf.size();
	}

	// Send message to the client
	void send(const TcpConnection& pClient, Message *header) {
		_sendTaskHandler.addTask([=]() {
			pClient->send(header);
			delete header;
		});
	}

private:
	std::unordered_map<SOCKET, TcpConnection> _clients;
	std::vector<TcpConnection> _clientsBuf;	// Clients buffer
	std::mutex _mutex;	// Mutex for clients buffer
	std::thread _thread;
	Event *_pMain;	// Pointer to the main thread (for event callback)
	TaskHandler _sendTaskHandler;	// Child thread for sending messages
	time_t _tCurrent;	// Current timestamp				
	fd_set _fdRead;	// A cache of fd_set
	bool _clientsChange;	// If the clients array changes
	SOCKET _maxSock;	// Record current max socket
	bool _isRun = false;
	int _id;
};
#endif // !_TcpSubserver_hpp_
