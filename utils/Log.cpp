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
	
	char drive[32] = {};
	char dir[256] = {};
	char filename[256] = {};
	char ext[32] = {};
	_splitpath(path, drive, dir, filename, ext);

	char file[1024] = {};

	time_t now = system_clock::to_time_t(system_clock::now());
	std::tm *pNow = localtime(&now);
	sprintf(file, "%s%s%s_%d_%d_%d_%d_%d_%d%s", drive, dir, filename, pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec, ext);
	pFile = fopen(file, mode);

	if (pFile == nullptr) {
		LOG_ERROR("Set log path - Fail! PATH=%s, MODE=%s\n", path, mode);
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
