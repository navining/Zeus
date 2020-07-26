#ifndef _TcpConnection_h_
#define _TcpConnection_h_

#include "common.h"
#include "ObjectPool.h"
#include "Buffer.h"

class TcpSocket : public ObjectPool<TcpSocket, 10000> {
public:
	TcpSocket(SOCKET sockfd = INVALID_SOCKET);

	~TcpSocket();

	SOCKET sockfd();

	// Receive message into the receive buffer
	int recv();

	// Whether there is a complete message inside the buffer
	bool hasMessage();

	// Return the first message in the buffer
	// Must be called after check with hasMessage()
	Message* getMessage();

	// Pop and return the first message in the buffer
	// Must be called after check with hasMessage()
	Message* popMessage();

	// Wheter the send buffer is empty
	bool isSendEmpty();

	// Send message (put into the send buffer)
	int send(Message *msg);

	// Send message (byte stream)
	int send(Stream * stream);

	// Clear the buffer (send everything out)
	int sendAll();

	// Close socket
	void close();

	// Reset timeclock for heartbeat checking
	void reset_tHeartbeat();

	// Reset timeclock for send checking
	void reset_tSendBuf();

	bool isAlive(time_t t);

	bool canSend(time_t t);

private:
	SOCKET _sockfd;	// socket fd_set			
	Buffer _recvBuf;	// Receive buffer
	Buffer _sendBuf;	// Send buffer
	time_t _tHeartbeat;	// For heartbeat detection
	time_t _tSendBuf;	// For clearing send buffer
};

typedef std::shared_ptr<TcpSocket> TcpConnection;

#endif // !_TcpConnection_h_


