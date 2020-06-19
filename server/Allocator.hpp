#ifdef SERVER_MAIN
#ifndef _ALLOCATOR_HPP_
#define _ALLOCATOR_HPP_

#include "Memory.h"

void *operator new(size_t size) {
	return Memory::Instance().alloc(size);
}

void *operator new[](size_t size) {
	return Memory::Instance().alloc(size);
}

void operator delete(void *p) {
	Memory::Instance().free(p);
}

void operator delete[](void *p) {
	Memory::Instance().free(p);
}

void *mem_alloc(size_t size) {
	return malloc(size);
}

void mem_free(void * p) {
	free(p);
}
#endif // !_ALLOCATOR_HPP_
#endif