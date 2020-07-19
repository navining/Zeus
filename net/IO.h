#ifndef _IO_h_
#define _IO_h_

#define SELECT 1
#define IOCP 2
#define EPOLL 3

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
};

#endif // !_IO_h_
