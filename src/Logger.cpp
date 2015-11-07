#include "Logger.h"

/*
  * Simple logging utility with:
 *  1. logging levels: quiet, warnings, info, debug
 *  2. output to stderr/file
 *  3. log entry namespacing
 */

// global
std::string log_path;

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
}

Logger::~Logger() {
	// TODO Auto-generated destructor stub
}

void Logger::_log(std::string msg, int level) {
	if (this->current_log_level >= level) {
		// add timestamp
		std::cout << "[" << this->class_name << "] " << msg << std::endl;
	}
}
