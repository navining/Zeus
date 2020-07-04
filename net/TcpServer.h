#ifndef _TcpServer_h_
#define _TcpServer_h_

#include <iostream>

#include <mutex>
#include <atomic>
#include <vector>

#include "common.h"
#include "TcpConnection.h"
#include "TcpSubserver.h"
#include "Event.h"
#include "Network.h"

#define TCPSERVER_THREAD_COUNT 1

// Main thread responsible for accepting connections
class TcpServer : public Event {
public:
	TcpServer();

	virtual ~TcpServer();

	virtual void onConnection(const TcpConnection& pClient);

	virtual void onDisconnection(const TcpConnection& pClient);

	virtual void onMessage(TcpSubserver *pServer, const TcpConnection& pClient, Message *header);

	virtual void onIdle();

	// Initialize socket
	int init();

	// Bind IP and port
	int bind(const char* ip, unsigned short port);

	// Listen to port
	int listen(int n);

	// Accept client connection
	SOCKET accept();

	// Start child threads
	void start(int numOfThreads = TCPSERVER_THREAD_COUNT);

	// Start server service
	void onRun(Thread & thread);

	// If connected
	bool isRun();

	// Close socket
	void close();

private:
	std::vector<TcpSubserver *> _subservers;
	Thread _thread;
protected:
	SOCKET _sock;
	Timestamp _time;
	int thread_count;
};

#endif // !_TcpServer_h_
