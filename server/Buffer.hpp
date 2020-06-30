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

	// Add data into the buffer
	// Retern false if the buffer is full
	bool add(const char * pData, int length) {
		if (_last + length > SEND_BUFF_SIZE) {
			return false;
		}

		// Copy into the buffer
		memcpy(_pBuf + _last, pData, length);
		_last += length;
		return true;
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
private:
	char* _pBuf = nullptr;	// Buffer
	int _last;	// Last position of the buffer
	int _size;
};

#endif // !_Buffer_hpp_
