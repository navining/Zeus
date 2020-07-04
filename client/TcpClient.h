#ifndef _TcpClient_h_
#define _TcpClient_h_

#include "common.h"
#include "TcpConnection.h"

class TcpClient {
public:
	TcpClient();

	virtual ~TcpClient();

	// Initialize socket
	SOCKET init();

	// Connect server
	int connect(const char *ip, unsigned short port);

	// Close socket
	void close();

	// Start client service
	bool onRun();

	// If connected
	bool isRun();

	// Receive data and unpack
	int recv();

	// Process data
	virtual int onMessage(Message *msg);

	// Send data
	int send(Message *_msg);

private:
	bool isConnect;
	// Receive Buffer (System Buffer)
	char _recvBuf[RECV_BUFF_SIZE] = {};
	// Message Buffer (Secondary Buffer)
	char _msgBuf[RECV_BUFF_SIZE * 5] = {};
	// Last position of the message buffer
	int _lastPos = 0;
	TcpSocket *_pClient;
};

#endif // !_TcpClient_h_
