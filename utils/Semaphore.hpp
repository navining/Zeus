#ifndef _Semaphore_hpp_
#define _Semaphore_hpp_

#include <condition_variable>

class Semaphore {
public:
	// Block current thread
	void wait() {
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0) {
			_cv.wait(lock, [this]()->bool {
				return _wakeup > 0; 
			});
			--_wakeup;
		}
	}

	// Wake up target thread
	void wakeup() {
		std::unique_lock<std::mutex> lock(_mutex);
		if (++_wait <= 0) {
			++_wakeup;
			_cv.notify_one();
		}

	}
private:
	std::mutex _mutex;
	std::condition_variable _cv;
	int _wait = 0;
	int _wakeup = 0;
};

#endif // !_Semaphore_hpp_
