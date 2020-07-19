#include "Buffer.h"

Buffer::Buffer(int size) {
	_size = size;
	_pBuf = new char[_size];
	memset(_pBuf, 0, _size);
	_last = 0;
}

Buffer::~Buffer() {
	if (_pBuf != nullptr) {
		delete[] _pBuf;
		_pBuf = nullptr;
	}
}

char * Buffer::data() {
	return _pBuf;
}

// Add data into the buffer
// Retern false if the buffer is full

bool Buffer::push(const char * pData, int length) {
	if (_last + length > SEND_BUFF_SIZE) {
		return false;
	}

	// Copy into the buffer
	memcpy(_pBuf + _last, pData, length);
	_last += length;
	return true;
}

// Pop data from the buffer

void Buffer::pop(int length) {
	if (_last - length >= 0) {
		memcpy(_pBuf, _pBuf + length, _last - length);
		_last -= length;
	}
}

// Send the entire buffer to the client

int Buffer::send(SOCKET client) {
	int ret = 0;
	if (_last > 0) {
		ret = ::send(client, _pBuf, _last, 0);
		if (ret <= 0) return SOCKET_ERROR;
		if (ret == _last) {
			clear();
		}
		else {
			// Fail to send the entire buffer - still has remaining data
			memcpy(_pBuf, _pBuf + ret, _last - ret);
			_last -= ret;
		}
	}
	return ret;
}

// Receive data into the buffer
// Return the length of received data

int Buffer::recv(SOCKET client) {
	if (_size - _last <= 0) return 0;

	int recvlen = (int)::recv(client, _pBuf + _last, _size - _last, 0);
	if (recvlen <= 0) {
		return SOCKET_ERROR;
	}

	_last += recvlen;
	return recvlen;
}

// Clear the buffer

void Buffer::clear() {
	_last = 0;
}

int Buffer::size() {
	return _size;
}

int Buffer::last() {
	return _last;
}

bool Buffer::empty() {
	return _last == 0;
}
