#ifndef _Log_hpp_
#define _Log_hpp_

#include "common.h"
#include "TaskHandler.h"
#include <stdio.h>
#include <chrono>

using std::chrono::system_clock;

class Log {
#ifdef LOG_LEVEL

#if LOG_LEVEL >= 1
#define LOG_INFO(...) Log::info(__VA_ARGS__);
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= 2
#define LOG_WARNING(...) Log::warning(__VA_ARGS__);
#else
#define LOG_WARNING(...)
#endif

#endif // LOG_LEVEL

#define LOG_ERROR(...) Log::error(__VA_ARGS__);

#define LOG_PERROR(...) Log::perror(__VA_ARGS__);

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
	static void perror(const char *format, Args... args) {
#ifdef _WIN32
		// Get errno in current thread
		DWORD err = GetLastError();
		// Get errmsg in taskHandler thread
		Instance()._logTaskHandler.addTask([=]() {
			char msg[256] = {};
			FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&msg, 256, NULL
			);
			printTask("ERROR", format, args...);
			printTask("ERROR", "<%d> %s", err, msg);
		});
#else
		int err = errno;
		Instance()._logTaskHandler.addTask([=]() {
			print("ERROR", format, args...);
			print("ERROR", "<%d> %s\n", err, strerror(err));
		});
#endif		
		}

	template<typename ...Args>
	static void debug(const char *format, Args... args) {
		print("DEBUG", format, args...);
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

	template<typename ...Args>
	static void print(const char *type, const char *format, Args... args) {
		Instance()._logTaskHandler.addTask([=]() {
			printTask(type, format, args...);
		});
	}

	template<typename ...Args>
	static void printTask(const char *type, const char *format, Args... args) {
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
	}
	};

#endif // !_Log_hpp_
