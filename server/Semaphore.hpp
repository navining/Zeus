#ifndef _Semaphore_hpp_
#define _Semaphore_hpp_

#include <chrono>
#include <thread>

class Semaphore {
public:
	void wait() {
		_wait = true;
		while (_wait) {
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
	}

	void wakeup() {
		_wait = false;
	}
private:
	bool _wait = false;
};

#endif // !_Semaphore_hpp_
