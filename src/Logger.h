#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <iostream>

enum LogLevel { QUIET, WARNINGS, INFO, DEBUG };

class LoggerBase {
protected:
	int current_log_level = INFO;
public:
	LoggerBase() {}
	LoggerBase(std::string path);
	virtual ~LoggerBase();

	virtual void _log(std::string msg, int level) {}
	virtual void info(std::string msg) {}
};

class Logger : public LoggerBase {
private:
	std::string class_name;
public:
	Logger() {}
	Logger(std::string name);
	Logger(std::string path, std::string name);
	virtual ~Logger();

	void _log(std::string msg, int level);
	void warn(std::string msg)  { this->_log(msg, WARNINGS); }
	void info(std::string msg)  { this->_log(msg, INFO); 	 }
	void debug(std::string msg) { this->_log(msg, DEBUG);	 }
};

#endif /* SRC_LOGGER_H_ */
