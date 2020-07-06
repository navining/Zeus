#ifndef _common_h_
#define _common_h_

#ifdef _WIN32
#define FD_SETSIZE	4096	// Size of FD_SET
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#define SOCKET int
#define INVALID_SOCKET    (SOCKET)(~0)
#define SOCKET_ERROR        (-1)
typedef long unsigned int size_t;
#endif

#include "Message.h"
#include "Timestamp.h"
#include "Log.h"

// Size of the receive buffer
#define RECV_BUFF_SIZE 10240

// Size of the send buffer
#define SEND_BUFF_SIZE 102400

// Heartbeat detection (in millisecond)
// Define a negative number to disable it
#define CLIENT_DEAD_TIME -1

// Interval to clear the send buffer (in millisecond)
// Define a negative number to disable it
#define CLIENT_SEND_TIME -1

#ifndef _WIN32
void blockSignal();
#endif // !_WIN32


#endif // !_common_h_
