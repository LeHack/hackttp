#include "Logger.h"
#include <iostream>
#include <fstream>
#include <ctime>

/*
  * Simple logging utility with:
 *  1. logging levels: quiet, warnings, info, debug
 *  2. output to stderr/file
 *  3. log entry namespacing
 */

// global
std::string log_path;
std::fstream logFile;
std::time_t epochTimestamp;

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
	this->class_name = name;
    logFile.open("./log", std::fstream::out | std::fstream::app);

    if(logFile.is_open()){
        std::cout << "Logfile is open for " << this->class_name <<", logging to file enabled" << std::endl;
    } else {
        std::cout << "Error opening logfile for " << this->class_name << ", disabling logging to file" <<std::endl;
        isLoggingToFileEnabled = false;
    }
}

Logger::~Logger() {
    //Otaczać ifami?
    logFile.close();
}

void Logger::_log(std::string msg, int level) {
    epochTimestamp = std::time(nullptr);
    fullDateTimestamp = std::asctime(std::localtime(&epochTimestamp));
    fullMessage = "[" +
                  this->class_name + " " +
                  fullDateTimestamp.substr(0, fullDateTimestamp.size()-1) + "] " +
                  msg + "\n";

	if (this->current_log_level >= level) {
        std::cout << fullMessage;
        if (isLoggingToFileEnabled) {
            logFile << fullMessage;
            logFile.flush();
        }
    }
}
