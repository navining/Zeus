#ifndef _TcpSubserver_h_
#define _TcpSubserver_h_

#include <unordered_map>
#include <vector>

#include "common.h"
#include "Event.h"
#include "TaskHandler.h"
#include "Thread.h"
#include "IO.h"

// Child thread responsible for handling messsages
class TcpSubserver : public Event, public IO
{
public:
	TcpSubserver(int id = 0, Event *pEvent = nullptr);

	~TcpSubserver();

	// Add new clients into the buffer
	void addClients(const TcpConnection& pClient);

	// Get number of clients in the current subserver
	size_t getClientCount();

	// Start the subserver
	void start();

	// Close subserver
	void close();

private:
	bool select();

	bool epoll();

	bool iocp();

	void onMessage(const TcpConnection& pClient, Stream *msg);

	void onDisconnection(const TcpConnection& pClient);

	void onConnection(const TcpConnection& pClient);

	// Do extra things when the server is idle
	void onIdle();

	// Client socket response: handle read request
	void respondRead(fd_set &fdRead);

	// Client socket response: handle write request
	void respondWrite(fd_set &fdWrite);

	// Check if the client is alive
	void checkAlive();

	// Receive data
	int recv(const TcpConnection& pClient);

	// Send data
	int send(const TcpConnection& pClient);

	// Process messages in each client buffer
	void process();

	// Start server service
	void onRun(Thread & thread);

private:
	std::unordered_map<SOCKET, TcpConnection> _clients;	// Map the sockfd to client connections
	std::vector<TcpConnection> _clientsBuf;	// Clients buffer
	std::mutex _mutex;	// Mutex for clients buffer
	Event *_pMain;	// Pointer to the main thread (for event callback)
	//TaskHandler _sendTaskHandler;	// Child thread for sending messages
	time_t _tCurrent;	// Current timestamp				
	fd_set _fdRead;	// A cache of fd_set
	bool _clientsChange;	// If the clients array changes
	SOCKET _maxSock;	// Record current max socket
	int _id;	// id of the subserver
	Thread _thread;	// current thread
};
#endif // !_TcpSubserver_h_
