#include "Log.h"

Log & Log::Instance() {
	static Log _log;
	return _log;
}

void Log::setPath(const char * path, const char * mode) {
	FILE *&pFile = Instance()._pFile;
	if (pFile != nullptr) {
		LOG_WARNING("Close old log file\n");
		fclose(pFile);
		pFile = nullptr;
	}

	pFile = fopen(path, mode);

	if (pFile == nullptr) {
		LOG_INFO("Set log path - Fail! PATH=%s, MODE=%s\n", path, mode);
	}
	else {
		LOG_INFO("Set log path - Success! PATH=%s, MODE=%s\n", path, mode);
	}
}

Log::Log() {
	_logTaskHandler.start();
}

Log::~Log() {
	_logTaskHandler.close();
}
