#ifndef _Task_hpp_
#include <thread>
#include <mutex>
#include <list>

// Type of task
class Task {
public:
	Task() {

	}

	virtual ~Task() {

	}

	// Run the task
	virtual void run() {

	}
private:

};

// Class handling the task
class TaskHandler {
public:
	void addTask(Task* task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}

	void start() {
		std::thread t(std::mem_fun(&TaskHandler::onRun), this);
		t.detach();
	}
protected:
	void onRun() {
		while (true) {
			if (!_tasksBuf.empty()) {
				std::lock_guard<std::mutex> lock(_mutex);
				// Get task from the buffer and put into the list
				for (auto task : _tasksBuf) {
					_tasks.push_back(task);
				}
				_tasksBuf.clear();
			}

			if (_tasks.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			for (auto task : _tasks) {
				task->run();
				delete task;
			}

			_tasks.clear();
		}
	}
private:
	std::list<Task *> _tasks;	// List storing tasks
	std::list<Task *> _tasksBuf;	// List for buffering
	std::mutex _mutex;
};
#endif // !_Task_hpp__
