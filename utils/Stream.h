#ifndef _Stream_h_
#define _Stream_h_

#include <cstdint>
#include "common.h"

// ---------------------------------------
// |          |            |             |
// |  readed  |  readable  |  writtable  |
// |          |            |             |
// ---------------------------------------
//            ^            ^             ^
//          _read        _write        _size

class Stream {
public:
	Stream(int size = STREAM_BUFF_SIZE);

	// Construct a stream from outside
	// flag: whether the memory is freed within Stream or not
	Stream(char *pBuf, int size, bool flag = false);

	~Stream();

	const char *data();

	int size();

	int8_t readInt8();

	int16_t readInt16();

	int32_t readInt32();

	float readFloat();

	double readDouble();

	bool writeInt8(int8_t n);

	bool writeInt16(int16_t n);

	bool writeInt32(int32_t n);

	bool writeFloat(float n);

	bool writeDouble(double n);

	template<typename T>
	bool writeArray(T *n, uint32_t size);

	template<typename T>
	uint32_t readArray(T *n, uint32_t srcSize);

	int32_t getArraySize();

private:
	char* _pBuf = nullptr;	// Buffer
	int _read = 0;	// Read position of the buffer
	int _write = 0;	// Write position of the buffer
	int _size = 0;
	bool _flag = false;

	template<typename T>
	bool write(T n);

	template<typename T>
	bool read(T &n);

	// Peek the first element in the buffer (without modifying _read)
	template<typename T>
	bool peek(T &n);
};


template<typename T>
bool Stream::write(T n) {
	size_t length = sizeof(T);
	if (_write + length > _size) {
		LOG_WARNING("Stream::write() - Buffer overflow\n");
		return false;
	}
	// Copy into the buffer
	memcpy(_pBuf + _write, &n, length);
	_write += length;
	return true;
}

template<typename T>
bool Stream::writeArray(T * n, uint32_t size)
{
	size_t length = sizeof(T) * size;
	if (_write + length + sizeof(uint32_t) > _size) {
		LOG_WARNING("Stream::writeArray() - Buffer overflow\n");
		return false;
	}
	// Write the size of array inside the buffer
	writeInt32(size);
	// Copy into the buffer
	memcpy(_pBuf + _write, n, length);
	_write += length;
	return true;
}

template<typename T>
bool Stream::read(T &n) {
	size_t length = sizeof(T);
	if (_read + length > _size) {
		LOG_WARNING("Stream::read() - Buffer overflow\n");
		return false;
	}
	// Copy from the buffer
	memcpy(&n, _pBuf + _read, length);
	_read += length;
	return true;
}

template<typename T>
bool Stream::peek(T &n) {
	size_t length = sizeof(T);
	if (_read + length > _size) {
		LOG_WARNING("Stream::peek() - Buffer overflow\n");
		return false;
	}
	// Copy from the buffer
	memcpy(&n, _pBuf + _read, length);
	return true;
}

template<typename T>
uint32_t Stream::readArray(T *n, uint32_t desSize) {
	uint32_t srcSize = 0;
	peek(srcSize);

	if (desSize < srcSize) {
		LOG_WARNING("Stream::readArray() - Array too small\n");
		return 0;
	}
	size_t length = srcSize * sizeof(T);
	if (_read + length + sizeof(uint32_t) > _size) {
		LOG_WARNING("Stream::readArray() - Buffer overflow\n");
		return 0;
	}
	// Copy from the buffer
	read(srcSize);
	memcpy(n, _pBuf + _read, length);
	_read += length;
	return srcSize;
}

#endif // !_Stream_h_