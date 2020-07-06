#include "MemoryPool.h"
#include <stdlib.h>
#include <assert.h>

MemoryPool::MemoryPool(int blockSize, int blockCount) {
	_pBuf = nullptr;
	_pHead = nullptr;
	_blockSize = blockSize;
	_blockCount = blockCount;
}

MemoryPool::~MemoryPool() {
	if (_pBuf != nullptr) {
		::free(_pBuf);
	}
}

// Allocate memory

void * MemoryPool::alloc(size_t size) {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_pBuf == nullptr) {
		init();
	}

	MemoryBlock *block = nullptr;
	if (_pHead == nullptr) {
		// No extra space - allocate from the system
		LOG_INFO("WARNING: Memory pool exceed limit: size = %d\n", (int)size);
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

void MemoryPool::free(void * p) {
	MemoryBlock *block = (MemoryBlock *)((char *)p - sizeof(MemoryBlock));
	assert(1 == block->refCount);
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (--block->refCount != 0) {
			return;
		}
	}
	if (block->inPool) {
		// Return back to the pool
		std::lock_guard<std::mutex> lock(_mutex);
		block->next = _pHead;
		_pHead = block;
	}
	else {
		::free(block);
	}
}

// Initialize memory pool

void MemoryPool::init() {
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
		MemoryBlock *cur = (MemoryBlock *)((char *)prev + _blockSize + sizeof(MemoryBlock));
		prev->next = cur;
		cur->inPool = true;
		cur->id = n;
		cur->refCount = 0;
		cur->pool = this;
		prev = cur;
	}

	prev->next = nullptr;
}

Memory & Memory::Instance() {
	static Memory instance;
	return instance;
}

// Allocate memory

void * Memory::alloc(size_t size) {
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

void Memory::free(void * p) {
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

void Memory::addRef(void * p) {
	MemoryBlock *block = (MemoryBlock *)((char *)p - sizeof(MemoryBlock));
	block->refCount++;
}

// Initialize the mapping array

void Memory::init(int begin, int end, MemoryPool * pool) {
	for (int i = begin; i <= end; i++) {
		_pool[i] = pool;
	}
}
