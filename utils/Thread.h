#ifndef _Thread_h_
#define _Thread_h_

#include "Semaphore.h"

class Thread;

typedef std::function<void(Thread &)> ThreadFunc;

#define EMPTY_THREAD_FUNC [](Thread &) {}

class Thread {
public:
	void start(ThreadFunc onStart = EMPTY_THREAD_FUNC, ThreadFunc onRun = EMPTY_THREAD_FUNC, ThreadFunc onClose = EMPTY_THREAD_FUNC);

	// Close the thread. This function should be called from other threads (otherwise will cause dead lock)
	void close();

	// Exit the thread. This function should be called by current thread
	void exit();

	bool isRun() const;
	
protected:
	void run();
private:
	bool _isRun = false;	// Whether the thread is running
	Semaphore _semaphore;	// Control the exit

	ThreadFunc _onStart;
	ThreadFunc _onRun;
	ThreadFunc _onClose;

	std::mutex _mutex;
};

#endif // !_Thread_h_
