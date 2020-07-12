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

	// Select
	bool select();

	// If connected
	bool isRun();

	// Receive data and unpack
	int recv();

	// Process messages in each client buffer
	void process();

	// Process data
	virtual void onMessage(Message *msg);

	// Handle other services
	virtual void onIdle();

	// Send data
	int send(Message *_msg);

	int sendAll();

private:
	bool isConnect;

protected:
	TcpConnection _pClient;
};

#endif // !_TcpClient_h_
