#ifndef _LOG_HPP_
#define _LOG_HPP_

#include "common.h"
#include <stdio.h>

class LOG {
public:
	static LOG& Instance() {
		static LOG _log;
		return _log;
	}

	static void INFO(const char *msg) {
		printf("%s", msg);
	}

	template<typename ...Args>
	static void INFO(const char *format, Args... args) {
		printf(format, args...);
	}
private:
	LOG() {}
	LOG(const LOG &) = delete;
	LOG &operator=(const LOG &) = delete;
};

#endif // !_LOG_HPP_
