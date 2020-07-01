#ifndef _TaskHandler_h_
#define _TaskHandler_h_

#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include "Thread.hpp"

// Class handling the task (consumer)
class TaskHandler {
	typedef std::function<void()> Task;
public:
	TaskHandler(int id = 0);

	// Put task into the buffer
	void addTask(Task task);

	// Start the thread
	void start();

	void close();
protected:
	// Run the task
	void onRun(Thread &thread);
private:
	std::list<Task> _tasks;	// List storing tasks
	std::list<Task> _tasksBuf;	// List for buffering
	std::mutex _mutex;
	Thread _thread;
	int _id;
};
#endif // !_TaskHandler_h_
