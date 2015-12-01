#include "Logger.h"
#include "Config.h"
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <mutex>

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
std::mutex mut;

Logger::Logger(std::string path, std::string name) {
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
    logFileDescriptor = open(log_path.c_str(), O_WRONLY | O_APPEND | O_CREAT );

    if(logFileDescriptor < 0){
        loggerStatus = "[" + this->class_name + "]" + " Error opening logfile at "+log_path+", disabling logging to file\nError: " + strerror(errno) + "\n";
        cLoggerStatus = loggerStatus.c_str();
        write(2, cLoggerStatus, strlen(cLoggerStatus));
        isLoggingToFileEnabled = false;
    }
}

Logger::~Logger() {
    close(logFileDescriptor);
}

void Logger::_log(std::string msg, int level) {
    if(this->current_log_level >= level) {
        mut.lock();
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
        mut.unlock();
    }
}
