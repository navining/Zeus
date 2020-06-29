#ifndef _Thread_hpp_
#define _Thread_hpp_

#include "Semaphore.hpp"

class Thread;

typedef std::function<void(const Thread &)> ThreadFunc;


class Thread {
public:
	void start(ThreadFunc onStart = ThreadFunc(), ThreadFunc onRun = ThreadFunc(), ThreadFunc onClose = ThreadFunc()) {
		if (_isRun) return;
		_isRun = true;

		_onStart = onStart;
		_onRun = onRun;
		_onClose = onClose;

		std::thread t(std::mem_fn(&Thread::run), this);
		t.detach();
	}

	void close() {
		if (_isRun) return;
		_isRun = false;

		_semaphore.wait();	// Wait till onRun() finishes
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
};

#endif // !_Thread_hpp_
