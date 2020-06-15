#ifndef _Memory_hpp_
#define _Memory_hpp_
#include <stdlib.h>

// Memory Block
class MemoryBlock {

};

// Memory Pool
class MemoryPool {

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
		return malloc(size);
	}

	// Free memory
	void free(void *p) {
		::free(p);
	}
private:
	Memory() {};
	Memory(const Memory &) = delete;
	Memory& operator=(const Memory &) = delete;
};

#endif // !_Memory_hpp_
