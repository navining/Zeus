#include "Thread.h"

void Thread::start(ThreadFunc onStart, ThreadFunc onRun, ThreadFunc onClose) {
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

void Thread::close() {
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_isRun) return;
	_isRun = false;

	_semaphore.wait();	// Wait till onRun() finishes
}

// Exit the thread. This function should be called by current thread

void Thread::exit() {
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_isRun) return;
	_isRun = false;
}

bool Thread::isRun() const {
	return _isRun;
}

void Thread::run() {
	_onStart(*this);
	_onRun(*this);
	_onClose(*this);

	_semaphore.wakeup();	// Go on close()
}
