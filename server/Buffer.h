#ifndef _Buffer_h_
#define _Buffer_h_
#include "common.h"

class Buffer {
public:
	Buffer(int size);

	~Buffer();

	char* data();

	// Add data into the buffer
	// Retern false if the buffer is full
	bool push(const char * pData, int length);

	// Pop data from the buffer
	void pop(int length);

	// Send the entire buffer to the client
	int send(SOCKET client);

	// Receive data into the buffer
	// Return the length of received data
	int recv(SOCKET client);

	// Clear the buffer
	void clear();

	int size();

	int last();

	bool empty();
private:
	char* _pBuf = nullptr;	// Buffer
	int _last;	// Last position of the buffer
	int _size;
};

#endif // !_Buffer_h_
