#include "Log.h"
#include <string>

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
	
	char filename[128] = {};
	char ext[32] = {};
	split(path, filename, ext);

	char file[256] = {};

	time_t now = system_clock::to_time_t(system_clock::now());
	std::tm *pNow = localtime(&now);
	sprintf(file, "%s_%d_%d_%d_%d_%d_%d%s", filename, pNow->tm_year + 1900, pNow->tm_mon + 1, pNow->tm_mday, pNow->tm_hour, pNow->tm_min, pNow->tm_sec, ext);
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

void Log::split(const char *whole_name, char *fname, char *ext)
{
	char *index = (char *)whole_name;

	while (*index != '\0') {
		if (*index == '.') break;
		index++;
	}

	if (*index == '.') {
		strcpy(ext, index);
		snprintf(fname, index - whole_name + 1, "%s", whole_name);
	}
	else {
		*ext = '\0';
		strcpy(fname, whole_name);
	}
}
