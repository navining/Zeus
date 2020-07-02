#ifndef _Log_hpp_
#define _Log_hpp_

#include "common.h"
#include "TaskHandler.h"
#include <stdio.h>
#include <chrono>

using std::chrono::system_clock;

class LOG {
public:
	static LOG& Instance();

	template<typename ...Args>
	static void INFO(const char *format, Args... args) {
		Instance()._logTaskHandler.addTask([=]() {
			printf("[INFO] ");
			printf(format, args...);

			FILE *pFile = Instance()._pFile;
			if (pFile != nullptr) {
				time_t now = system_clock::to_time_t(system_clock::now());
				std::tm *pNow = localtime(&now);
				fprintf(pFile, "[%d-%02d-%02d %02d:%02d:%02d] ", pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
				fprintf(Instance()._pFile, "[INFO] ");
				fprintf(Instance()._pFile, format, args...);
				fflush(Instance()._pFile);
			}
		});
	}

	static void setPath(const char *path, const char *mode);
private:
	LOG();

	~LOG();

	LOG(const LOG &) = delete;
	LOG &operator=(const LOG &) = delete;
private:
	TaskHandler _logTaskHandler;
	FILE *_pFile;
};

#endif // !_Log_hpp_
