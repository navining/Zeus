#ifndef _Select_h_
#define _Select_h_

#include "common.h"

class Select {
public:
	Select();

	~Select();

	void set(SOCKET sock);

	void clear(SOCKET sock);

	void zero();

	int isset(SOCKET sock);

	fd_set *fdset() const;

	size_t fdSize() const;

	int fdCount() const;

	// Warning: assume same size here
	void copy(const Select &other);

private:
	fd_set *_fdset = nullptr;
	size_t _fdSize = 0;
	int _fdCount = 0;
};

#endif // _Select_h_