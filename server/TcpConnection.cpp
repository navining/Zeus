#include "TcpConnection.h"

TcpSocket::TcpSocket(SOCKET sockfd)
	: _sendBuf(SEND_BUFF_SIZE), _recvBuf(RECV_BUFF_SIZE) {
	_sockfd = sockfd;
	reset_tHeartbeat();
	reset_tSendBuf();
}

TcpSocket::~TcpSocket() {
	close();
}

SOCKET TcpSocket::sockfd() {
	return _sockfd;
}

// Receive message into the receive buffer

int TcpSocket::recv() {
	return _recvBuf.recv(_sockfd);
}

// Whether there is a complete message inside the buffer

bool TcpSocket::hasMessage() {
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

Message * TcpSocket::getMessage() {
	return (Message *)_recvBuf.data();
}

// Pop and return the first message in the buffer
// Must be called after check with hasMessage()

Message * TcpSocket::popMessage() {
	Message *msg = getMessage();
	_recvBuf.pop(msg->length);
	return msg;
}

// Wheter the send buffer is empty

bool TcpSocket::isSendEmpty() {
	return _sendBuf.empty();
}

// Send message (put into the send buffer)

int TcpSocket::send(Message * msg) {
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

int TcpSocket::sendAll() {
	reset_tSendBuf();
	return _sendBuf.send(_sockfd);
}

// Close socket

void TcpSocket::close() {
	if (_sockfd == INVALID_SOCKET) return;
#ifdef _WIN32
	closesocket(_sockfd);
#else
	::close(_sockfd);
#endif
	_sockfd = INVALID_SOCKET;
}

// Reset timeclock for heartbeat checking

void TcpSocket::reset_tHeartbeat() {
	_tHeartbeat = 0;
}

// Reset timeclock for send checking

void TcpSocket::reset_tSendBuf() {
	_tSendBuf = 0;
}

bool TcpSocket::isAlive(time_t t) {
	_tHeartbeat += t;
	return CLIENT_DEAD_TIME < 0 || _tHeartbeat < CLIENT_DEAD_TIME;
}

bool TcpSocket::canSend(time_t t) {
	_tSendBuf += t;
	return CLIENT_SEND_TIME > 0 && _tSendBuf > CLIENT_SEND_TIME;
}
