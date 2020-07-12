#ifndef _FDSet_h_
#define _FDSet_h_

#include "common.h"

class FDSet {
public:
	FDSet();

	~FDSet();

	void set(SOCKET sock);

	void clear(SOCKET sock);

	void zero();

	int isset(SOCKET sock);

	fd_set *fdset() const;

	size_t fdSize() const;

	int fdCount() const;

	// Warning: assume same size here
	FDSet(const FDSet &other);
	
	// Warning: assume same size here
	FDSet& operator=(const FDSet &other);
private:
	fd_set *_fdset = nullptr;
	size_t _fdSize = 0;
	int _fdCount = 0;
};

#endif // _FDSet_h_