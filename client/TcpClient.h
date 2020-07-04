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
	TcpSocket *_pClient;
};

#endif // !_TcpClient_h_
