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

#define TCPSERVER_THREAD_COUNT 1

// Main thread responsible for accepting connections
class TcpServer : public Event {
public:
	TcpServer();

	virtual ~TcpServer();

	virtual void onConnection(const TcpConnection& pClient);

	virtual void onDisconnection(const TcpConnection& pClient);

	virtual void onMessage(const TcpConnection& pClient, Message *msg);

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

	bool select();

	// Close socket
	void close();

	int clientCount();

private:
	std::vector<TcpSubserver *> _subservers;
	Thread _thread;
	std::atomic_int _clientCount;	// Number of clients
protected:
	SOCKET _sock;
	Timestamp _time;
	int thread_count;
};

#endif // !_TcpServer_h_
