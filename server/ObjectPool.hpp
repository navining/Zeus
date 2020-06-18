#ifndef _ObjectPool_hpp_
#include <stdlib.h>

#ifndef _WIN32
typedef long unsigned int size_t;
#endif

template<typename T>
class ObjectPool {
public:
	void *operator new(size_t size) {
		return malloc(size);
	}

	void operator delete(void *p) noexcept {
		free(p);
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

#endif // !_ObjectPool_hpp_
