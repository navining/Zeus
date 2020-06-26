#ifndef _Task_hpp_
#define _Task_hpp_

#include <thread>
#include <mutex>
#include <list>
#include <memory>
#include <functional>


// Class handling the task (consumer)
class TaskHandler {
	typedef std::function<void()> Task;
public:
	// Put task into the buffer
	void addTask(Task task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}

	// Start the thread
	void start() {
		_isRun = true;
		std::thread t(std::mem_fun(&TaskHandler::onRun), this);
		t.detach();
	}

	void close() {
		_isRun = false;
	}
protected:
	// Run the task
	void onRun() {
		while (_isRun) {
			if (!_tasksBuf.empty()) {
				std::lock_guard<std::mutex> lock(_mutex);
				// Get task from the buffer and put into the list
				for (auto &task : _tasksBuf) {
					_tasks.push_back(task);
				}
				_tasksBuf.clear();
			}

			// Sleep if there's no task to do
			if (_tasks.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			for (auto &task : _tasks) {
				task();
			}

			_tasks.clear();
		}
	}
private:
	std::list<Task> _tasks;	// List storing tasks
	std::list<Task> _tasksBuf;	// List for buffering
	std::mutex _mutex;
	bool _isRun = false;
};
#endif // !_Task_hpp__
