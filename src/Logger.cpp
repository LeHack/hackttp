#include "Logger.h"
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <string.h>

/*
  * Simple logging utility with:
 *  1. logging levels: quiet, warnings, info, debug
 *  2. output to stderr/file
 *  3. log entry namespacing
 */

// global
std::string log_path;
std::time_t epochTimestamp;
int logFileDescriptor;
std::mutex mutex;

//Po co jest LoggerBase?

LoggerBase::LoggerBase(std::string path) {
	log_path = path;
}

LoggerBase::~LoggerBase() {
	// TODO Auto-generated destructor stub
}

Logger::Logger(std::string name) {
	this->class_name = name;
}

Logger::Logger(std::string path, std::string name) : LoggerBase(path) {
    std::string loggerStatus;
    const char* cLoggerStatus;

    this->class_name = name;
    logFileDescriptor = open(log_path.c_str(), O_WRONLY | O_APPEND);

    if(logFileDescriptor < 0){
        loggerStatus = "[" + this->class_name + "]" + " Error opening logfile, disabling logging to file\nError: " + strerror(errno) + "\n";
        cLoggerStatus = loggerStatus.c_str();
        write(2, cLoggerStatus, strlen(cLoggerStatus));
        isLoggingToFileEnabled = false;
    } else {
        loggerStatus = "[" + this->class_name + "]" + " Logfile is open, logging to file enabled\n";
        cLoggerStatus = loggerStatus.c_str();
        write(1, cLoggerStatus, strlen(cLoggerStatus));
    }
}

Logger::~Logger() {
    close(logFileDescriptor);
}

void Logger::_log(std::string msg, int level) {
    if(this->current_log_level >= level) {
        mutex.lock();
        epochTimestamp = std::time(nullptr);
        fullDateTimestamp = std::asctime(std::localtime(&epochTimestamp));
        fullMessage = "[" +
                      this->class_name + " " +
                      fullDateTimestamp.substr(0, fullDateTimestamp.size() - 1) + "] " +
                      msg + "\n";
        if(!isLoggingToFileEnabled){
            fullMessage = "WARNING: Logging to file disabled " + fullMessage;
        }

        const char* cFullMessage = fullMessage.c_str();

        write(1, cFullMessage, strlen(cFullMessage));
        if (isLoggingToFileEnabled) {
            write(logFileDescriptor, cFullMessage, strlen(cFullMessage));
        }
        mutex.unlock();
    }
}
