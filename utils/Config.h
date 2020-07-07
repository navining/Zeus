#ifndef _Config_h_
#define _Config_h_

#include <string>
#include <unordered_map>

class Config {
private:
	Config() {}
	~Config() {}
public:
	static Config& Instance();

	static void Init(int argc, char* args[]);

	const char * parseStr(const char * arg, const char * default = "");

	int parseInt(const char * arg, int default = 0);

private:
	std::string _path;
	std::unordered_map<std::string, std::string> _cmd;

	void split(char *cmd);
};

#endif // !_Config_h_
