#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#ifndef _WIN32
typedef long unsigned int size_t;
#endif

void *operator new(size_t size);

void *operator new[](size_t size);

void operator delete(void *p) noexcept;

void operator delete[](void *p) noexcept;

void *mem_alloc(size_t size);

void mem_free(void *p);

#endif // !_ALLOCATOR_H_
