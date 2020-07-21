#include "Select.h"

#define FD_SIZE 10240

Select::Select() {
	_fdCount = FD_SIZE;
#ifdef _WIN32
	_fdSize = sizeof(u_int) + sizeof(SOCKET) * _fdCount;
#else
	_fdSize = _fdCount / (8 * sizeof(long));
#endif // _WIN32

	_fdset = (fd_set *) new char[_fdSize];
	memset(_fdset, 0, _fdSize);
}

Select::~Select() {
	if (_fdset != nullptr) {
		delete[] _fdset;
		_fdset = nullptr;
	}
}

void Select::set(SOCKET sock) {
#ifndef _WIN32
	if (sock >= FD_SIZE) {
		LOG_ERROR("Socket fd exceed limit: %d\n", FD_SIZE);
		return;
	}
#endif // _WIN32
	FD_SET(sock, _fdset);
}

void Select::clear(SOCKET sock) {
	FD_CLR(sock, _fdset);
}

void Select::zero() {
#ifdef _WIN32
	FD_ZERO(_fdset);
#else
	memset(_fdset, 0, _fdSize);
#endif // _WIN32
}

int Select::isset(SOCKET sock) {
	return FD_ISSET(sock, _fdset);
}

fd_set * Select::fdset() const {
	return _fdset;
}

size_t Select::fdSize() const {
	return _fdSize;
}

int Select::fdCount() const {
	return _fdCount;
}

// Warning: assume same size here

void Select::copy(const Select & other) {
	memcpy(_fdset, other.fdset(), other.fdSize());
}
