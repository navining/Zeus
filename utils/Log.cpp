#include "Log.h"

LOG & LOG::Instance() {
	static LOG _log;
	return _log;
}

void LOG::setPath(const char * path, const char * mode) {
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

LOG::LOG() {
	_logTaskHandler.start();
}

LOG::~LOG() {
	_logTaskHandler.close();
}
