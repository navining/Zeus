#include "FDSet.h"

#define FD_SIZE 10240

FDSet::FDSet() {
	_fdCount = FD_SIZE;
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

void FDSet::set(SOCKET sock) {
#ifndef _WIN32
	if (sock >= FD_SIZE) {
		LOG_ERROR("Socket fd exceed limit: %d\n", FD_SIZE);
		return;
	}
#endif // _WIN32
	FD_SET(sock, _fdset);
}

void FDSet::clear(SOCKET sock) {
	FD_CLR(sock, _fdset);
}

void FDSet::zero() {
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

void FDSet::copy(const FDSet & other) {
	memcpy(_fdset, other.fdset(), other.fdSize());
}
