#include "TaskHandler.h"

TaskHandler::TaskHandler(int id) : _id(id) {}

// Put task into the buffer

void TaskHandler::addTask(Task task) {
	std::lock_guard<std::mutex> lock(_mutex);
	_tasksBuf.push_back(task);
}

// Start the thread

void TaskHandler::start() {
	_thread.start(
		EMPTY_THREAD_FUNC,	// start
		[this](Thread & thread) {	// run
		onRun(thread);
	},
		EMPTY_THREAD_FUNC);	// close
}

void TaskHandler::close() {
	_thread.close();
	//LOG::INFO("<TaskHandler %d> Quit...\n", _id);
}

// Run the task

void TaskHandler::onRun(Thread & thread) {
	while (thread.isRun()) {
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

	// Execute remaining tasks in the buffer
	for (auto &task : _tasksBuf) {
		task();
	}
}
