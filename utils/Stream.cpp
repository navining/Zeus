#include "Stream.h"

Stream::Stream(int size) {
	_size = size;
	_pBuf = new char[_size + OFFSET];
	_pBuf += OFFSET;
	_flag = true;
}

Stream::Stream(Message * msg)
{
	_size = msg->length - OFFSET;
	_write = _size;
	_pBuf = (char *)msg + OFFSET;
	_flag = false;
}


Stream::~Stream() {
	if (_flag)
		delete[] (_pBuf - OFFSET);
}

const char * Stream::data()
{
	return _pBuf;
}

Message * Stream::toMessage()
{
	Message *msg = (Message *)(_pBuf - OFFSET);
	msg->type = STREAM;
	msg->length = _write + OFFSET;
	return msg;
}

int Stream::size() {
	return _write;
}

int Stream::capacity() {
	return _size;
}

int Stream::readedSize()
{
	return _read;
}

int Stream::readableSize()
{
	return _write - _read;
}

int Stream::writableSize()
{
	return _size - _write;
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

std::string Stream::toString()
{
	char *a = new char[_write - OFFSET];
	readArray(a, _write - OFFSET);
	std::string s(a);
	delete[] a;
	return s;
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

bool Stream::writeString(const char * n, uint32_t len)
{
	return writeArray(n, len);
}

bool Stream::writeString(const char * n)
{
	return writeArray(n, strlen(n)+1);
}

bool Stream::writeString(std::string & n)
{
	return writeArray(n.c_str(), n.length()+1);
}

int32_t Stream::getArraySize()
{
	int32_t n = 0;
	peek(n);
	return n;
}
