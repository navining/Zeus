#ifndef _Semaphore_h_
#define _Semaphore_h_

#include <condition_variable>

class Semaphore {
public:
	// Block current thread
	void wait();

	// Wake up target thread
	void wakeup();
private:
	std::mutex _mutex;
	std::condition_variable _cv;
	int _wait = 0;
	int _wakeup = 0;
};

#endif // !_Semaphore_h_
