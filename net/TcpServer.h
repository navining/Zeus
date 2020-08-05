#ifndef _TcpServer_h_
#define _TcpServer_h_

#include <iostream>

#include <mutex>
#include <vector>
#include <atomic>

#include "common.h"
#include "TcpConnection.h"
#include "TcpSubserver.h"
#include "Event.h"
#include "Network.h"
#include "IO.h"

#define TCPSERVER_THREAD_COUNT 1

// Main thread responsible for accepting connections
class TcpServer : public Event, public IO {
public:
	TcpServer();

	virtual ~TcpServer();

	virtual void onConnection(const TcpConnection& pClient);

	virtual void onDisconnection(const TcpConnection& pClient);

	virtual void onMessage(const TcpConnection& pClient, Stream *msg);

	virtual void onIdle();

	// Initialize socket
	int init();

	// Bind IP and port
	int bind(const char* ip, unsigned short port);

	// Listen to port
	int listen(int n);

	// Start TcpSubservers and TcpServer::onRun()
	void start(int numOfThreads = TCPSERVER_THREAD_COUNT);

	// If connected
	bool isRun();

	// Close the server
	void close();

	// Total number of clients
	int clientCount();

private:
	std::vector<TcpSubserver *> _subservers;	// Vector of subservers
	Thread _thread; // Current thread
	std::atomic_int _clientCount;	// Number of clients
	
	bool select();

	bool epoll();

	bool iocp();

	// Start server service
	void onRun(Thread & thread);

	// Accept client connection
	SOCKET accept();
protected:
	SOCKET _sock;
	Timestamp _time;
	int thread_count;
};

#endif // !_TcpServer_h_
