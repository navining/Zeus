#ifndef _Thread_h_
#define _Thread_h_

#include "Semaphore.h"
#include <functional>

class Thread;

typedef std::function<void(Thread &)> ThreadFunc;

#define EMPTY_THREAD_FUNC [](Thread &) {}

class Thread {
public:
	// Start the thread
	// Three function objects can be passed in (use EMPTY_THREAD_FUNC as default)
	void start(ThreadFunc onStart = EMPTY_THREAD_FUNC, ThreadFunc onRun = EMPTY_THREAD_FUNC, ThreadFunc onClose = EMPTY_THREAD_FUNC);

	// Close the thread. This function should be called from other threads (otherwise will cause dead lock)
	void close();

	// Exit the thread. This function should be called by current thread
	void exit();

	// Whether the thread is running
	bool isRun() const;
	
	// Sleep the thread
	static void sleep(time_t t);
protected:
	// The actual funtion of the thread, shouldn't be called directly, use start() instead
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
