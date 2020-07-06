#ifndef _Log_hpp_
#define _Log_hpp_

#include "common.h"
#include "TaskHandler.h"
#include <stdio.h>
#include <chrono>

using std::chrono::system_clock;

class Log {
#define LOG_INFO(...) Log::info(__VA_ARGS__);

#define LOG_WARNING(...) Log::warning(__VA_ARGS__);

#define LOG_ERROR(...) Log::error(__VA_ARGS__);

#ifdef _DEBUG
#define LOG_DEBUG(...) Log::debug(__VA_ARGS__);
#else
#define LOG_DEBUG(...)
#endif

#define LOG_SETPATH(...) Log::setPath(__VA_ARGS__);
public:
	static Log& Instance();

	template<typename ...Args>
	static void info(const char *format, Args... args) {
		print("INFO", format, args...);
	}

	template<typename ...Args>
	static void warning(const char *format, Args... args) {
		print("WARNING", format, args...);
	}

	template<typename ...Args>
	static void error(const char *format, Args... args) {
		print("ERROR", format, args...);
	}

	template<typename ...Args>
	static void debug(const char *format, Args... args) {
		print("DEBUG", format, args...);
	}

	template<typename ...Args>
	static void print(const char *type, const char *format, Args... args) {
		Instance()._logTaskHandler.addTask([=]() {
			printf("[%s] ", type);
			printf(format, args...);

			FILE *pFile = Instance()._pFile;
			if (pFile != nullptr) {
				time_t now = system_clock::to_time_t(system_clock::now());
				std::tm *pNow = localtime(&now);
				fprintf(pFile, "[%d-%02d-%02d %02d:%02d:%02d] ", pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec);
				fprintf(Instance()._pFile, "[%s] ", type);
				fprintf(Instance()._pFile, format, args...);
				fflush(Instance()._pFile);
			}
		});
	}

	static void setPath(const char *path, const char *mode);
private:
	Log();

	~Log();

	Log(const Log &) = delete;
	Log &operator=(const Log &) = delete;
private:
	TaskHandler _logTaskHandler;
	FILE *_pFile;

	static void split(const char *whole_name, char *fname, char *ext);
};

#endif // !_Log_hpp_
