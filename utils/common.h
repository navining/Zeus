#ifndef _common_h_
#define _common_h_

#ifdef _WIN32
#define FD_SETSIZE 10240	// Size of fd_set
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
#include <sys/epoll.h>
#define SOCKET int
#define INVALID_SOCKET    (SOCKET)(~0)
#define SOCKET_ERROR        (-1)
#define EPOLL_ERROR        (-1)
typedef long unsigned int size_t;
#endif

// IO multiplexing mode
// SELECT, IOCP, EPOLL
#define IO_MODE SELECT

// Maximum number of events mornitored by epoll
#define EPOLL_SIZE 10240

// Size of the receive buffer
#define RECV_BUFF_SIZE 8192

// Size of the send buffer
#define SEND_BUFF_SIZE 10240

// Size of the byte stream buffer
#define STREAM_BUFF_SIZE 1024

// Heartbeat detection (in millisecond)
// Define a negative number to disable it
#define CLIENT_DEAD_TIME -1

// Interval to clear the send buffer (in millisecond)
// Define a negative number to disable it
#define CLIENT_SEND_TIME -1

// Maximum available connection
// Define a negative number to disable it
#define MAX_CLIENT -1

// Log warning level
// 0 - LOG_ERROR, LOG_PERROR
// 1 - LOG_INFO, LOG_ERROR, LOG_PERROR
// 2 - LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_PERROR
#define LOG_LEVEL 2

#ifndef _WIN32
void blockSignal();
#endif // !_WIN32


#include "Message.h"
#include "Timestamp.h"
#include "Log.h"

#endif // !_common_h_
