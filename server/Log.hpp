#ifndef _LOG_HPP_
#define _LOG_HPP_

class Log {
public:
	static Log& Instance() {
		static Log _log;
		return _log;
	}
private:
	Log() {}
	Log(const Log &) = delete;
	Log &operator=(const Log &) = delete;
};

#endif // !_LOG_HPP_
