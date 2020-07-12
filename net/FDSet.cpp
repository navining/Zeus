#include "FDSet.h"

FDSet::FDSet(int fdCount) {
	_fdCount = fdCount <= MAX_FD_SIZE ? fdCount : MAX_FD_SIZE;
#ifdef _WIN32
	_fdSize = sizeof(u_int) + sizeof(SOCKET) * _fdCount;
#else
	_fdSize = _fdCount / (8 * sizeof(long));
#endif // _WIN32

	_fdset = (fd_set *) new char[_fdSize];
	memset(_fdset, 0, _fdSize);
}

FDSet::~FDSet() {
	if (_fdset != nullptr) {
		delete[] _fdset;
		_fdset = nullptr;
	}
}

inline void FDSet::set(SOCKET sock) {
	FD_SET(sock, _fdset);
}

inline void FDSet::clear(SOCKET sock) {
	FD_CLR(sock, _fdset);
}

inline void FDSet::zero() {
#ifdef _WIN32
	FD_ZERO(_fdset);
#else
	memset(_fdset, 0, _fdSize);
#endif // _WIN32
}

int FDSet::isset(SOCKET sock) {
	return FD_ISSET(sock, _fdset);
}

fd_set * FDSet::fdset() const {
	return _fdset;
}

size_t FDSet::fdSize() const {
	return _fdSize;
}

int FDSet::fdCount() const {
	return _fdCount;
}

// Warning: assume same size here

FDSet::FDSet(const FDSet & other) {
	memcpy(_fdset, other.fdset(), other.fdSize());
}

FDSet& FDSet::operator=(const FDSet &other) {
	memcpy(_fdset, other.fdset(), other.fdSize());
	return *this;
}
