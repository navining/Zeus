#include "Config.h"

Config & Config::Instance() {
	static Config _config;
	return _config;
}

void Config::Init(int argc, char * args[]) {
	Instance()._path = args[0];
	for (int i = 1; i < argc; i++) {
		Instance().split(args[i]);
	}
}

const char * Config::parseStr(const char * arg, const char * default) {
	auto it = _cmd.find(arg);
	if (it != _cmd.end() && it->second != "") {
		default = it->second.c_str();
	}
	return default;
}

int Config::parseInt(const char * arg, int default) {
	auto it = _cmd.find(arg);
	if (it != _cmd.end() && it->second != "") {
		default = atoi(it->second.c_str());
	}
	return default;
}

void Config::split(char * cmd) {
	char *val = strchr(cmd, '=');
	if (val != NULL) {
		*val = '\0';
		val++;
		_cmd[cmd] = val;
	}
	else {
		_cmd[cmd] = "";
	}
}
