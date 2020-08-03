#ifndef _IO_h_
#define _IO_h_

#define SELECT 1
#define IOCP 2
#define EPOLL 3

#include "Select.h"
#ifdef __linux__
#include "Epoll.h"
#endif

#include "common.h"

// An interface for IO-multiplexing
class IO {
public:
	// Do IO multiplexing according to IO_MODE defined in common.h
	bool multiplex();
protected:
	virtual bool select() = 0;
#ifdef _WIN32
	virtual bool iocp() = 0;
#endif
#ifdef __linux__
	virtual bool epoll() = 0;
#endif
#if IO_MODE == EPOLL
	Epoll _epoll;
#elif IO_MODE == SELECT
	Select _select;
#endif
};

#endif // !_IO_h_
