//
// Created by atticus on 1/14/16.
//

#include "LoggingSingleton.h"

LoggingSingleton::LoggingSingleton() {}
LoggingSingleton* LoggingSingleton::GetInstance() {
    static LoggingSingleton pSingleton;
    return &pSingleton;
}

void LoggingSingleton::log(int logFileDescriptor, const char* cFullMessage){
    logMutex.lock();
    int result = write(logFileDescriptor, cFullMessage, strlen(cFullMessage));
    // first close the mutex
    logMutex.unlock();
    // then handle any errors
    if (result < 0) {
        throw LoggingSingleton::Exception("Cannot write to given file descriptor: " + std::to_string(logFileDescriptor));
    }
}

