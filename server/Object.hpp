#ifndef _Object_hpp_
#define _Object_hpp_
#include <stdlib.h>
#include <mutex>
#include <assert.h>
#include "common.h"

#ifndef _WIN32
typedef long unsigned int size_t;
#endif


class ObjectBlock {
public:
	int id;	// ID of current block
	char refCount; // Reference count
	ObjectBlock *next;	// Next block
	bool inPool;	// Whether in the pool or not
private:
	char c[2];	// Memory alignment
};

//	Object Pool
//		------------------------
//		| Block |    Object    |
//		------------------------
//		| Block |    Object    |
//		------------------------
//		| Block |    Object    |
//		------------------------
//		         ......
//		------------------------
//		| Block |    Object    |
//		------------------------
template<typename T>
class ObjectPool {
public:
	ObjectPool(int size) {
		init(size);
	}

	~ObjectPool() {
		delete[] _pBuf;
	}

	// Create Object
	T* create() {
		std::lock_guard<std::mutex> lock(_mutex);

		ObjectBlock *block = nullptr;
		if (_pHead == nullptr) {
			// No extra space - allocate from the system
			block = (ObjectBlock *)new char[sizeof(T) + sizeof(ObjectBlock)];
			block->inPool = false;
			block->id = -1;
			block->refCount = 1;
			block->next = nullptr;
		}
		else {
			block = _pHead;
			_pHead = _pHead->next;
			assert(0 == block->refCount);
			block->refCount = 1;
		}

		PRINT("Create %lx, id = %d, size = %d\n", block, block->id, sizeof(T));
		return (T*)((char *)block + sizeof(ObjectBlock));
	}

	// Destroy object
	void destory(T *p) {
		ObjectBlock *block = (ObjectBlock *)((char *)p - sizeof(ObjectBlock));
		assert(1 == block->refCount);
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (--block->refCount != 0) {
				return;
			}
		}

		PRINT("Destory %lx, id = %d, size = %d\n", block, block->id, sizeof(T));
		if (block->inPool) {
			// Return back to the pool
			std::lock_guard<std::mutex> lock(_mutex);
			block->next = _pHead;
			_pHead = block;
		}
		else {
			delete block;
		}
	}

private:
	// Initialize object pool
	void init(int size) {
		assert(nullptr == _pBuf);
		if (_pBuf != nullptr) return;

		// Allocate memory
		size_t n = size * (sizeof(T) + sizeof(ObjectBlock));
		_pBuf = new char[n];

		// Initialize object blocks
		_pHead = (ObjectBlock *)_pBuf;
		_pHead->inPool = true;
		_pHead->id = 0;
		_pHead->refCount = 0;
		_pHead->next = nullptr;

		ObjectBlock *prev = _pHead;
		for (size_t n = 1; n < size; n++) {
			ObjectBlock *cur = (ObjectBlock *)((char *)prev + sizeof(T) + sizeof(ObjectBlock));
			prev->next = cur;
			cur->inPool = true;
			cur->id = n;
			cur->refCount = 0;
			prev = cur;
		}

		prev->next = nullptr;
	}

	ObjectBlock *_pHead;	// Pointing to the first empty block
	char *_pBuf;	// Address of the object buffer
	std::mutex _mutex;
};

#endif // !_Object_hpp_


// Object management (Singleton)
template<typename T, size_t SIZE = 10>
class Object {
public:
	static ObjectPool<T>& Pool() {
		static ObjectPool<T> pool(SIZE);
		return pool;
	}

	void *operator new(size_t size) {
		return Pool().create();
	}

	void operator delete(void *p) noexcept {
		Pool().destory((T *)p);
	}

	template<typename ...Args>
	static T* create(Args ... args) {
		T *obj = new T(args...);
		return obj;
	}

	static void destroy(T *obj) {
		delete obj;
	}
private:
};