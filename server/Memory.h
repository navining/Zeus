#ifndef _Memory_h_
#define _Memory_h_
#include <stdlib.h>
#include <assert.h>
#include <mutex>
#include "common.h"


#define MAX_MEMORY_SIZE 128

class MemoryPool;

// Memory Block
class MemoryBlock {
public:
	int id;	// ID of current block
	int refCount; // Reference count
	MemoryPool *pool; // Current memory pool
	MemoryBlock *next;	// Next block
	bool inPool;	// Whether in the pool or not
private:
	char c[3];	// Memory alignment
};

// Memory Pool
//		------------------------
//		| Block |     Data     |
//		------------------------
//		| Block |     Data     |
//		------------------------
//		| Block |     Data     |
//		------------------------
//		         ......
//		------------------------
//		| Block |     Data     |
//		------------------------
class MemoryPool {
public:
	MemoryPool(int blockSize, int blockCount);

	~MemoryPool();

	// Allocate memory
	void *alloc(size_t size);

	// Free memory
	void free(void *p);

	// Initialize memory pool
	void init();
private:
	char *_pBuf;	// Address of the memory buffer
	MemoryBlock *_pHead;	// Pointing to the first empty block
	size_t _blockSize;	// Size of memory block
	size_t _blockCount;	// Number of memory block
	std::mutex _mutex;
};

// Memory Management (Singleton)
class Memory {
public:
	static Memory & Instance();

	// Allocate memory
	void *alloc(size_t size);

	// Free memory
	void free(void *p);

	// Add reference count
	void addRef(void *p);

private:
	Memory() {
		init(1, 16, &_pool_16);
		init(17, 64, &_pool_64);
		init(65, 128, &_pool_128);
		// init(129, 256, &_pool_256);
		// init(257, 512, &_pool_512);
		// init(513, 1024, &_pool_1024);
	};
	Memory(const Memory &) = delete;
	Memory& operator=(const Memory &) = delete;

	// Initialize the mapping array
	void init(int begin, int end, MemoryPool *pool);

private:
	MemoryPool _pool_16 = { 16, 8000000 };
	MemoryPool _pool_64 = { 64, 4000000 };
	MemoryPool _pool_128 = { 128, 4000000 };
	// MemoryPool _pool_256 = { 256, 10000 };
	// MemoryPool _pool_512 = { 512, 10000 };
	// MemoryPool _pool_1024 = { 1024, 10000 };
	MemoryPool* _pool[MAX_MEMORY_SIZE + 1];	// Mapping array for memory pool
};

#endif // !_Memory_h_