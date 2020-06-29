#ifndef _Thread_hpp_
#define _Thread_hpp_

#include "Semaphore.hpp"

class Thread;

typedef std::function<void(Thread &)> ThreadFunc;

#define EMPTY_THREAD_FUNC [](Thread &) {}

class Thread {
public:
	void start(ThreadFunc onStart = EMPTY_THREAD_FUNC, ThreadFunc onRun = EMPTY_THREAD_FUNC, ThreadFunc onClose = EMPTY_THREAD_FUNC) {
		if (_isRun) return;
		
		std::lock_guard<std::mutex> lock(_mutex);
		_isRun = true;

		_onStart = onStart;
		_onRun = onRun;
		_onClose = onClose;

		std::thread t(std::mem_fn(&Thread::run), this);
		t.detach();
	}

	// Close the thread. This function should be called from other threads (otherwise will cause dead lock)
	void close() {
		if (!_isRun) return;

		std::lock_guard<std::mutex> lock(_mutex);
		_isRun = false;

		_semaphore.wait();	// Wait till onRun() finishes
	}

	// Exit the thread. This function should be called by current thread
	void exit() {
		if (!_isRun) return;

		std::lock_guard<std::mutex> lock(_mutex);
		_isRun = false;
	}

	bool isRun() const {
		return _isRun;
	}
	
protected:
	void run() {
		_onStart(*this);
		_onRun(*this);
		_onClose(*this);

		_semaphore.wakeup();	// Go on close()
	}
private:
	bool _isRun = false;	// Whether the thread is running
	Semaphore _semaphore;	// Control the exit

	ThreadFunc _onStart;
	ThreadFunc _onRun;
	ThreadFunc _onClose;

	std::mutex _mutex;
};

#endif // !_Thread_hpp_
