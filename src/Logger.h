#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_
#include <string>
#include "Config.h"
#define QUIET 0
#define WARNINGS 1
#define INFO 2
#define DEBUG 3

class Logger {
protected:
    int current_log_level = DEBUG;

private:
	std::string class_name;
	std::string fullMessage;
	std::string fullDateTimestamp;
    bool isLoggingToFileEnabled = true;

public:
	Logger() {}
	Logger(std::string path, std::string name);
	// delegation is fun
	Logger(std::string name) : Logger(Config::get_str_setting("log_path"), name) {};
	virtual ~Logger();

	void _log(std::string msg, int level);
	void warn(std::string msg)  { this->_log(msg, WARNINGS); }
	void info(std::string msg)  { this->_log(msg, INFO); 	 }
	void debug(std::string msg) { this->_log(msg, DEBUG);	 }
};

#endif /* SRC_LOGGER_H_ */
