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

	// If connected
	bool isRun();

	// Process data
	virtual void onMessage(Stream *msg);

	// Handle other services
	virtual void onIdle();

	// Send data with Message
	int send(Message *_msg);

	// Send data with Stream
	int send(Stream * _msg);

private:
	// Select
	bool select();

	bool epoll();

	bool iocp();

	// Receive data and unpack
	int recv();

	// Process messages in each client buffer
	void process();

	// Send all data in the buffer immediately
	int sendAll();

private:
	bool isConnect;

protected:
	TcpConnection _pClient;
};

#endif // !_TcpClient_h_
