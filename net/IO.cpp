#include "IO.h"

bool IO::multiplex()
{
#if IO_MODE == SELECT
	return select();
#elif IO_MODE == IOCP
	return iocp();
#elif IO_MODE == EPOLL
	return epoll();
#endif
}
