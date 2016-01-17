#include "Logger.h"
#include "LoggingSingleton.h"
#include <ctime>
#include <fcntl.h>
#include <map>
#include <string.h>
#include <unistd.h>

/*
  * Simple logging utility with:
 *  1. logging levels: quiet, warnings, info, debug
 *  2. output to stderr/file
 *  3. log entry namespacing
 */

// global
std::time_t epochTimestamp;

Logger::Logger(string path, string name) {
    isLoggingToFileEnabled = true;
    std::map<string, int > map;
    map.insert(pair<string, int>("QUIET", 0));
    map.insert(pair<string, int>("WARNINGS", 1));
    map.insert(pair<string, int>("INFO", 2));
    map.insert(pair<string, int>("DEBUG", 3));
    std::string loggerStatus;
    const char* cLoggerStatus;
    this->current_log_level = map.at(Config::get_str_setting("current_log_level"));
    this->class_name = name;
    log_path = path;
    logFileDescriptor = open(log_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );

    if(logFileDescriptor < 0){
        loggerStatus = "[" + this->class_name + "]" + " Error opening logfile at "+log_path+", disabling logging to file\nError: " + strerror(errno) + "\n";
        cLoggerStatus = loggerStatus.c_str();
        if (write(STDERR_FILENO, cLoggerStatus, strlen(cLoggerStatus)) == -1) {
            throw Logger::Exception("Cannot write to STDERR.");
        }
        isLoggingToFileEnabled = false;
    }
}

Logger::~Logger() {
    close(logFileDescriptor);
}

void Logger::set_postfix(string postfix) {
    addit_postfix = postfix;
}

void Logger::_log(string msg, int level) {
    if(this->current_log_level >= level) {
        epochTimestamp = std::time(nullptr);
        fullDateTimestamp = std::asctime(std::localtime(&epochTimestamp));
        fullMessage = "[" + fullDateTimestamp.substr(0, fullDateTimestamp.size() - 1) + "] " +
                      "[:" + this->class_name + "] ";
        if (addit_postfix.length() > 0) {
            fullMessage += "[" + addit_postfix + "] ";
        }
        fullMessage += msg + "\n";
        if(!isLoggingToFileEnabled){
            fullMessage = "WARNING: Logging to file disabled " + fullMessage;
        }

        const char* cFullMessage = fullMessage.c_str();

        // fetch a LoggingSingleton instance when it's actually needed
        LoggingSingleton *logWriter = LoggingSingleton::GetInstance();
        logWriter->log(STDOUT_FILENO, cFullMessage);
        if (isLoggingToFileEnabled) {
            try {
                logWriter->log(logFileDescriptor, cFullMessage);
            }
            catch (LoggingSingleton::Exception &e) {
                const char* errMessage = std::string("Error while writing to logfile: " + std::string(e.what()) + "\n").c_str();
                logWriter->log(STDOUT_FILENO, errMessage);
            }
        }
    }
}
