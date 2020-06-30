#ifndef _TcpConnection_hpp_
#define _TcpConnection_hpp_

#include "common.h"
#include "ObjectPool.hpp"
#include "Buffer.hpp"

class TcpSocket : public ObjectPool<TcpSocket, 10000> {
public:
	TcpSocket(SOCKET sockfd = INVALID_SOCKET) 
		: _sendBuf(SEND_BUFF_SIZE), _recvBuf(RECV_BUFF_SIZE) {
		_sockfd = sockfd;
		reset_tHeartbeat();
		reset_tSendBuf();
	}

	~TcpSocket() {
		close();
	}

	SOCKET sockfd() {
		return _sockfd;
	}

	// Receive message into the receive buffer
	int recv() {
		return _recvBuf.recv(_sockfd);
	}

	// Whether there is a complete message inside the buffer
	bool hasMessage() {
		if (_recvBuf.last() >= sizeof(Message)) {
			// Get header
			Message *header = (Message *)_recvBuf.data();
			if (_recvBuf.last() >= header->length) {
				return true;
			}
		}
		return false;
	}

	// Return the first message in the buffer
	// Must be called after check with hasMessage()
	Message* getMessage() {
		return (Message *)_recvBuf.data();
	}

	// Pop and return the first message in the buffer
	// Must be called after check with hasMessage()
	Message* popMessage() {
		Message *msg = getMessage();
		_recvBuf.pop(msg->length);
		return msg;
	}

	// Send message (put into the send buffer)
	int send(Message *msg) {
		int ret = SOCKET_ERROR;
		int sendLength = msg->length;
		const char *sendData = (const char *)msg;

		while (true) {
			if (_sendBuf.push(sendData, sendLength)) {
				return sendLength;
			}
			else {
				// Data reaches the limitation of send buffer
				int copyLength = _sendBuf.size() - _sendBuf.last();
				_sendBuf.push(sendData, copyLength);
				sendLength -= copyLength;
				// Send the entire buffer
				ret = sendAll();
				if (SOCKET_ERROR == ret) {
					return ret;
				}
			}
		}

		return ret;
	}

	// Clear the buffer (send everything out)
	int sendAll() {
		reset_tSendBuf();
		return _sendBuf.send(_sockfd);
	}

	// Close socket
	void close() {
		if (_sockfd == INVALID_SOCKET) return;
#ifdef _WIN32
		closesocket(_sockfd);
#else
		::close(_sockfd);
#endif
		_sockfd = INVALID_SOCKET;
	}

	// Reset timeclock for heartbeat checking
	void reset_tHeartbeat() {
		_tHeartbeat = 0;
	}

	// Reset timeclock for send checking
	void reset_tSendBuf() {
		_tSendBuf = 0;
	}

	bool isAlive(time_t t) {
		_tHeartbeat += t;
		return CLIENT_DEAD_TIME < 0 || _tHeartbeat < CLIENT_DEAD_TIME;
	}

	bool canSend(time_t t) {
		_tSendBuf += t;
		return CLIENT_SEND_TIME > 0 && _tSendBuf > CLIENT_SEND_TIME;
	}

private:
	SOCKET _sockfd;	// socket fd_set			
	Buffer _recvBuf;	// Receive buffer
	Buffer _sendBuf;	// Send buffer
	time_t _tHeartbeat;	// For heartbeat detection
	time_t _tSendBuf;	// For clearing send buffer
};

typedef std::shared_ptr<TcpSocket> TcpConnection;

#endif // !_TcpConnection_hpp_


