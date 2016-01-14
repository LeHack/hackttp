//
// Created by atticus on 1/14/16.
//

#ifndef HACKTTP_LOGGINGSINGLETON_H
#define HACKTTP_LOGGINGSINGLETON_H


class LoggingSingleton {
public:
    static LoggingSingleton* GetInstance();
    void log(int logFileDescriptor, const char* cFullMessage);
private:
    LoggingSingleton();
    static LoggingSingleton* pSingleton;
};


#endif //HACKTTP_LOGGINGSINGLETON_H
