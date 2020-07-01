#ifndef _LOG_HPP_
#define _LOG_HPP_

#include "common.h"
#include "TaskHandler.h"
#include <stdio.h>
#include <chrono>

using std::chrono::system_clock;

class LOG {
public:
	static LOG& Instance() {
		static LOG _log;
		return _log;
	}

	static void INFO(const char *msg) {
		Instance()._logTaskHandler.addTask([=]() {
			printf("[INFO] %s", msg);

			FILE *pFile = Instance()._pFile;
			if (pFile != nullptr) {
				time_t now = system_clock::to_time_t(system_clock::now());
				std::tm *pNow = gmtime(&now);
				fprintf(pFile, "[%d/%d/%d %d:%d:%d] ", pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
				fprintf(pFile, "[INFO] %s", msg);
				fflush(pFile);
			}
		});
	}

	template<typename ...Args>
	static void INFO(const char *format, Args... args) {
		Instance()._logTaskHandler.addTask([=]() {
			printf("[INFO] ");
			printf(format, args...);

			FILE *pFile = Instance()._pFile;
			if (pFile != nullptr) {
				time_t now = system_clock::to_time_t(system_clock::now());
				std::tm *pNow = gmtime(&now);
				fprintf(pFile, "[%d/%d/%d %d:%d:%d] ", pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
				fprintf(Instance()._pFile, "[INFO] ");
				fprintf(Instance()._pFile, format, args...);
				fflush(Instance()._pFile);
			}
		});
	}

	static void setPath(const char *path, const char *mode) {
		FILE *&pFile = Instance()._pFile;
		if (pFile != nullptr) {
			LOG::INFO("Close old log file\n");
			fclose(pFile);
			pFile = nullptr;
		}

		pFile = fopen(path, mode);

		if (pFile == nullptr) {
			LOG::INFO("Set log path - Fail! PATH=%s, MODE=%s\n", path, mode);
		}
		else {
			LOG::INFO("Set log path - Success! PATH=%s, MODE=%s\n", path, mode);
		}
	}
private:
	LOG() {
		_logTaskHandler.start();
	}

	~LOG() {
		_logTaskHandler.close();
	}

	LOG(const LOG &) = delete;
	LOG &operator=(const LOG &) = delete;
private:
	TaskHandler _logTaskHandler;
	FILE *_pFile;
};

#endif // !_LOG_HPP_
