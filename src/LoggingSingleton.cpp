//
// Created by atticus on 1/14/16.
//

#include <unistd.h>
#include <string.h>
#include <mutex>
#include <iostream>
#include "LoggingSingleton.h"

std::mutex mutex;

LoggingSingleton::LoggingSingleton()
{
    std::cout << "Creating LoggingSingleton instance" <<std::endl;
}

LoggingSingleton* LoggingSingleton::GetInstance()
{
    static LoggingSingleton pSingleton;
    return &pSingleton;
}

void LoggingSingleton::log(int logFileDescriptor, const char* cFullMessage){
    mutex.lock();
        write(logFileDescriptor, cFullMessage, strlen(cFullMessage));
    mutex.unlock();
}

