#include "Network.h"

Network::Network() {
#ifdef _WIN32
	// Start Win Sock 2.x
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);
#else
	blockSignal();
#endif // !_WIN32
}

Network::~Network() {
#ifdef _WIN32
	WSACleanup();
#endif
}

void Network::Init() {
	static Network _network;
}
