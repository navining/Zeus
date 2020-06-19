#ifndef _Task_hpp_
#include <thread>
#include <mutex>
#include <list>
#include <memory>

// The task in producer-consumer pattern
class Task {
public:
	Task() {

	}

	virtual ~Task() {

	}

	// Run the task
	virtual int run() = 0;
private:

};

typedef std::shared_ptr<Task> TaskPtr;

// Class handling the task (consumer)
class TaskHandler {
public:
	// Put task into the buffer
	void addTask(TaskPtr& task) {
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}

	// Start the thread
	void start() {
		std::thread t(std::mem_fun(&TaskHandler::onRun), this);
		t.detach();
	}
protected:
	// Run the task
	void onRun() {
		while (true) {
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
				task->run();
			}

			_tasks.clear();
		}
	}
private:
	std::list<TaskPtr> _tasks;	// List storing tasks
	std::list<TaskPtr> _tasksBuf;	// List for buffering
	std::mutex _mutex;
};
#endif // !_Task_hpp__
