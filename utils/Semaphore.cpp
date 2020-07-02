#include "Semaphore.h"

// Block current thread

void Semaphore::wait() {
	std::unique_lock<std::mutex> lock(_mutex);
	if (--_wait < 0) {
		_cv.wait(lock, [this]()->bool {
			return _wakeup > 0;
		});
		--_wakeup;
	}
}

// Wake up target thread

void Semaphore::wakeup() {
	std::unique_lock<std::mutex> lock(_mutex);
	if (++_wait <= 0) {
		++_wakeup;
		_cv.notify_one();
	}

}
