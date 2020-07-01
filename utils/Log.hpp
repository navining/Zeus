#ifndef _LOG_HPP_
#define _LOG_HPP_
#define _CRT_SECURE_NO_WARNINGS

#include "common.h"
#include "TaskHandler.h"
#include <stdio.h>

class LOG {
public:
	static LOG& Instance() {
		static LOG _log;
		return _log;
	}

	static void INFO(const char *msg) {
		printf("[INFO] %s", msg);

		if (Instance()._pFile == nullptr) {
			return;
		}

		fprintf(Instance()._pFile, "[INFO] %s", msg);
		fflush(Instance()._pFile);
	}

	template<typename ...Args>
	static void INFO(const char *format, Args... args) {
		printf("[INFO] ");
		printf(format, args...);

		if (Instance()._pFile == nullptr) {
			return;
		}

		fprintf(Instance()._pFile, "[INFO] ");
		fprintf(Instance()._pFile, format, args...);
		fflush(Instance()._pFile);
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
