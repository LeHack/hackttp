//
// Created by atticus on 1/14/16.
//

#ifndef HACKTTP_LOGGINGSINGLETON_H
#define HACKTTP_LOGGINGSINGLETON_H

#include <mutex>
#include <unistd.h>
#include "Logger.h"

class LoggingSingleton {
    private:
        std::mutex logMutex;
        LoggingSingleton();
        static LoggingSingleton* pSingleton;

    public:
        static LoggingSingleton* GetInstance();
        void log(int logFileDescriptor, const char* cFullMessage);

    class Exception: public BaseException {
        public:
            Exception(std::string msg = "Unknown logger singleton exception") {
                this->reason = msg;
            }
    };
};


#endif //HACKTTP_LOGGINGSINGLETON_H
