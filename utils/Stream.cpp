#include "Stream.h"

Stream::Stream(int size) {
	_size = size;
	_pBuf = new char[_size];
	_flag = true;
}

// Construct a stream from outside
// flag: whether the memory is freed within Stream or not

Stream::Stream(char * pBuf, int size, bool flag) {
	_size = size;
	_pBuf = pBuf;
	_flag = flag;
}

Stream::~Stream() {
	if (_flag)
		delete[] _pBuf;
}

const char * Stream::data()
{
	return _pBuf;
}

int Stream::size()
{
	return _write - _read;
}

int8_t Stream::readInt8() {
	int8_t n = 0;
	read(n);
	return n;
}

int16_t Stream::readInt16() {
	int16_t n = 0;
	read(n);
	return n;
}

int32_t Stream::readInt32() {
	int32_t n = 0;
	read(n);
	return n;
}

float Stream::readFloat() {
	float n = 0;
	read(n);
	return n;
}

double Stream::readDouble() {
	double n = 0;
	read(n);
	return n;
}

bool Stream::writeInt8(int8_t n) {
	return write(n);
}

bool Stream::writeInt16(int16_t n) {
	return write(n);
}

bool Stream::writeInt32(int32_t n) {
	return write(n);
}

bool Stream::writeFloat(float n) {
	return write(n);
}

bool Stream::writeDouble(double n) {
	return write(n);
}

int32_t Stream::getArraySize()
{
	int32_t n = 0;
	peek(n);
	return n;
}
