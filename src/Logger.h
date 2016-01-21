#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_
#include "Config.h"

#define QUIET 0
#define WARNINGS 1
#define INFO 2
#define DEBUG 3

class Logger {
protected:
    int current_log_level = DEBUG;

private:
	string class_name;
	string addit_postfix;
	string fullMessage;
	string fullDateTimestamp;
	string log_path;
	int logFileDescriptor;
    bool isLoggingToFileEnabled;

public:
	Logger() {}
	Logger(string name);
	virtual ~Logger();
	void set_postfix(string postfix);

	void _log(std::string msg, int level);
	void warn(std::string msg)  { this->_log(msg, WARNINGS); }
	void info(std::string msg)  { this->_log(msg, INFO); 	 }
	void debug(std::string msg) { this->_log(msg, DEBUG);	 }

	class Exception: public BaseException {
	    public:
	        Exception(std::string msg = "Unknown logger exception") {
	            this->reason = msg;
	        }
	};
};

#endif /* SRC_LOGGER_H_ */
