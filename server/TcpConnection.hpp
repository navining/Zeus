#ifndef _TcpConnection_hpp_
#define _TcpConnection_hpp_

#include "common.h"
#include "ObjectPool.hpp"
#include "Buffer.hpp"

class TcpSocket : public ObjectPool<TcpSocket, 10000> {
public:
	TcpSocket(SOCKET sockfd = INVALID_SOCKET) : _sendBuf(SEND_BUFF_SIZE) {
		_sockfd = sockfd;
		memset(_msgBuf, 0, RECV_BUFF_SIZE);
		_msgLastPos = 0;
		reset_tHeartbeat();
		reset_tSendBuf();
	}

	~TcpSocket() {
		close();
	}

	SOCKET sockfd() {
		return _sockfd;
	}

	char* msgBuf() {
		return _msgBuf;
	}

	int getLastPos() {
		return _msgLastPos;
	}

	void setLastPos(int pos) {
		_msgLastPos = pos;
	}

	// Send message (put into the send buffer)
	int send(Message *msg) {
		int ret = SOCKET_ERROR;
		int sendLength = msg->length;
		const char *sendData = (const char *)msg;

		while (true) {
			if (_sendBuf.add(sendData, sendLength)) {
				return sendLength;
			}
			else {
				// Data reaches the limitation of send buffer
				int copyLength = _sendBuf.size() - _sendBuf.last();
				_sendBuf.add(sendData, copyLength);
				sendLength -= copyLength;
				// Send the entire buffer
				ret = _sendBuf.send(_sockfd);
				reset_tSendBuf();
				if (SOCKET_ERROR == ret) {
					return ret;
				}
			}
		}

		return ret;
	}

	// Clear the buffer (send everything out)
	int clearBuffer() {
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

	void reset_tHeartbeat() {
		_tHeartbeat = 0;
	}

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
	char _msgBuf[RECV_BUFF_SIZE];	// Message Buffer
	int _msgLastPos;	// Last position of the message buffer
	Buffer _sendBuf;	// Send buffer
	time_t _tHeartbeat;	// For heartbeat detection
	time_t _tSendBuf;	// For clearing send buffer
};

typedef std::shared_ptr<TcpSocket> TcpConnection;

#endif // !_TcpConnection_hpp_


