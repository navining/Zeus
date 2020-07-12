#ifndef _TcpSubserver_h_
#define _TcpSubserver_h_

#include <unordered_map>
#include <vector>

#include "common.h"
#include "Event.h"
#include "TaskHandler.h"
#include "Thread.h"

// Child thread responsible for handling messsages
class TcpSubserver
{
public:
	TcpSubserver(int id = 0, Event *pEvent = nullptr);

	~TcpSubserver();

	// Start the server
	void start();

	// Start server service
	void onRun(Thread & thread);

	// select
	// Return false if error
	bool select();

	// Handle message
	void onMessage(const TcpConnection& pClient, Message *msg);

	void onDisconnection(const TcpConnection& pClient);

	// Do extra things when the server is idle
	void onIdle();

	// Client socket response: handle read request
	void respondRead(fd_set &fdRead);

	// Client socket response: handle write request
	void respondWrite(fd_set &fdWrite);

	// Check if the client is alive
	void checkAlive();

	// Check if the send buffer is ready to be cleared
	// void checkSendBuffer();

	// Add new clients into the buffer
	void addClients(const TcpConnection& pClient);

	// Get number of clients in the current subserver
	size_t getClientCount();

	// Receive data
	int recv(const TcpConnection& pClient);

	// Close socket
	void close();

private:
	std::unordered_map<SOCKET, TcpConnection> _clients;
	std::vector<TcpConnection> _clientsBuf;	// Clients buffer
	std::mutex _mutex;	// Mutex for clients buffer
	Event *_pMain;	// Pointer to the main thread (for event callback)
	//TaskHandler _sendTaskHandler;	// Child thread for sending messages
	time_t _tCurrent;	// Current timestamp				
	fd_set _fdRead;	// A cache of fd_set
	bool _clientsChange;	// If the clients array changes
	SOCKET _maxSock;	// Record current max socket
	int _id;
	Thread _thread;
};
#endif // !_TcpSubserver_h_
