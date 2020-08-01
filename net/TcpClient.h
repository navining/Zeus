#ifndef _TcpClient_h_
#define _TcpClient_h_

#include "common.h"
#include "TcpConnection.h"
#include "IO.h"
class TcpClient : public IO {
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

	bool epoll();

	bool iocp();

	// If connected
	bool isRun();

	// Receive data and unpack
	int recv();

	// Process messages in each client buffer
	void process();

	// Process data
	virtual void onMessage(Stream *msg);

	// Handle other services
	virtual void onIdle();

	// Send data
	int send(Message *_msg);

	int send(Stream * _msg);

	int sendAll();

private:
	bool isConnect;

protected:
	TcpConnection _pClient;
};

#endif // !_TcpClient_h_
