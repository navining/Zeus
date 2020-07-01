#ifndef _Buffer_hpp_
#define _Buffer_hpp_
#include "common.h"

class Buffer {
public:
	Buffer(int size) {
		_size = size;
		_pBuf = new char[_size];
		memset(_pBuf, 0, _size);
		_last = 0;
	}

	~Buffer() {
		if (_pBuf != nullptr) {
			delete[] _pBuf;
			_pBuf = nullptr;
		}
	}

	char* data() {
		return _pBuf;
	}

	// Add data into the buffer
	// Retern false if the buffer is full
	bool push(const char * pData, int length) {
		if (_last + length > SEND_BUFF_SIZE) {
			return false;
		}

		// Copy into the buffer
		memcpy(_pBuf + _last, pData, length);
		_last += length;
		return true;
	}

	// Pop data from the buffer
	void pop(int length) {
		if (_last - length >= 0) {
			memcpy(_pBuf, _pBuf + length, _last - length);
			_last -= length;
		}
	}

	// Send the entire buffer to the client
	int send(SOCKET client) {
		int ret = SOCKET_ERROR;
		if (_last > 0) {
			ret = ::send(client, _pBuf, _last, 0);
			clear();
		}
		return ret;
	}

	// Receive data into the buffer
	// Return the length of received data
	int recv(SOCKET client) {
		if (_size - _last <= 0) return 0;

		int recvlen = (int)::recv(client, _pBuf + _last, _size - _last, 0);
		if (recvlen <= 0) {
			return recvlen;
		}

		_last += recvlen;
		return recvlen;
	}

	// Clear the buffer
	void clear() {
		_last = 0;
	}

	int size() {
		return _size;
	}

	int last() {
		return _last;
	}

	bool empty() {
		return _last == 0;
	}
private:
	char* _pBuf = nullptr;	// Buffer
	int _last;	// Last position of the buffer
	int _size;
};

#endif // !_Buffer_hpp_