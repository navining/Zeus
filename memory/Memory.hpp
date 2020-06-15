#ifndef _Memory_hpp_
#define _Memory_hpp_
#include <stdlib.h>
#include <assert.h>

#define MAX_MEMORY_SIZE 64

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
	MemoryPool(int blockSize, int blockCount) {
		_pBuf = nullptr;
		_pHead = nullptr;
		_blockSize = blockSize;
		_blockCount = blockCount;
	}

	~MemoryPool() {
		if (_pBuf != nullptr) {
			::free(_pBuf);
		}
	}
	
	// Allocate memory
	void *alloc(size_t size) {
		if (_pBuf == nullptr) {
			init();
		}

		MemoryBlock *block = nullptr;
		if (_pHead == nullptr) {
			// No extra space - allocate from the system
			block = (MemoryBlock *)malloc(size + sizeof(MemoryBlock));
			block->inPool = false;
			block->id = -1;
			block->refCount = 1;
			block->pool = nullptr;
			block->next = nullptr;
		}
		else {
			block = _pHead;
			_pHead = _pHead->next;
			assert(0 == block->refCount);
			block->refCount = 1;
		}

		return (char *)block + sizeof(MemoryBlock);
	}

	// Free memory
	// This function only frees memory allocated from the memory pool
	// The memory allocated from system (ie. no enough space in the pool) should be freed by the user himself!
	void free(void *p) {
		MemoryBlock *block = (MemoryBlock *)((char *)p - sizeof(MemoryBlock));
		assert(1 == block->refCount);
		if (--block->refCount != 0) {
			return;
		}
		if (block->inPool) {
			// Return back to the pool
			block->next = _pHead;
			_pHead = block;
		}
		else {
			::free(block);
		}
	}

	// Initialize memory pool
	void init() {
		assert(nullptr == _pBuf);
		if (_pBuf != nullptr) return;

		size_t size = (_blockSize + sizeof(MemoryBlock)) * _blockCount;
		// Allocate memory
		_pBuf = (char *)malloc(size);

		// Initialize memory blocks
		_pHead = (MemoryBlock *)_pBuf;
		_pHead->inPool = true;
		_pHead->id = 0;
		_pHead->refCount = 0;
		_pHead->pool = this;
		_pHead->next = nullptr;

		MemoryBlock *prev = _pHead;
		for (size_t n = 1; n < _blockCount; n++) {
			MemoryBlock *cur = (MemoryBlock *)(prev + _blockSize + sizeof(MemoryBlock));
			prev->next = cur;
			cur->inPool = true;
			cur->id = n;
			cur->refCount = 0;
			cur->pool = this;
			prev = cur;
		}

		prev->next = nullptr;
	}
private:
	char *_pBuf;	// Address of the memory buffer
	MemoryBlock *_pHead;	// Pointing to the first empty block
	size_t _blockSize;	// Size of memory block
	size_t _blockCount;	// Number of memory block
};

// Memory Management (Singleton)
class Memory {
public:
	static Memory & Instance() {
		static Memory instance;
		return instance;
	}

	// Allocate memory
	void *alloc(size_t size) {
		if (size <= MAX_MEMORY_SIZE) {
			return _pool[size]->alloc(size);
		}
		else {
			// Exceed max memory size - allocate from the system
			MemoryBlock *block = (MemoryBlock *)malloc(size + sizeof(MemoryBlock));
			block->inPool = false;
			block->id = -1;
			block->refCount = 1;
			block->pool = nullptr;
			block->next = nullptr;
			return (char*)block + sizeof(MemoryBlock);
		}
		return nullptr;
	}

	// Free memory
	void free(void *p) {
		MemoryBlock *block = (MemoryBlock *)((char *)p - sizeof(MemoryBlock));
		if (block->inPool) {
			block->pool->free(p);
		}
		else {
			if (--block->refCount == 0)
				::free(block);
		}
	}

	// Add reference count
	void addRef(void *p) {
		MemoryBlock *block = (MemoryBlock *)((char *)p - sizeof(MemoryBlock));
		block->refCount++;
	}

private:
	Memory() {
		init(1, 64, &_pool_64);
	};
	Memory(const Memory &) = delete;
	Memory& operator=(const Memory &) = delete;

	// Initialize the mapping array
	void init(int begin, int end, MemoryPool *pool) {
		for (int i = begin; i <= end; i++) {
			_pool[i] = pool;
		}
	}

private:
	MemoryPool _pool_64 = { 64, 10 };
	MemoryPool* _pool[MAX_MEMORY_SIZE + 1];	// Mapping array for memory pool
};

#endif // !_Memory_hpp_
