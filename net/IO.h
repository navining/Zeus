#ifndef _IO_h_
#define _IO_h_

#define SELECT 1
#define IOCP 2
#define EPOLL 3

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
	virtual bool iocp() = 0;
	virtual bool epoll() = 0;

#if IO_MODE == EPOLL
  Epoll _epoll;
#endif
};

#endif // !_IO_h_
