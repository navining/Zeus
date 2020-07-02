#ifndef _Event_h_
#define _Event_h_

#include "TcpConnection.h"

class TcpSubserver;

// Interface for handling events
class Event {
public:
	// Client connect event
	virtual void onConnection(const TcpConnection& pClient) = 0;
	// Client disconnect event
	virtual void onDisconnection(const TcpConnection& pClient) = 0;
	// Receive message event
	virtual void onMessage(TcpSubserver *pServer, const TcpConnection& pClient, Message *header) = 0;	// TODO: Try to eliminate pointer to subserver.
private:
};

#endif // !_Event_h_
