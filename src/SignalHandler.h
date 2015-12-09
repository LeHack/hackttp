//
// Created by atticus on 12/9/15.
//

#ifndef HACKTTP_SIGNALHANDLER_H
#define HACKTTP_SIGNALHANDLER_H

#include <signal.h>

class SignalHandler{
public:
    virtual ~SignalHandler();
    SignalHandler();
    static void sigintHandler(int, siginfo_t *, void *);
    static void sigusr1Handler(int, siginfo_t *, void *);
};

#endif //HACKTTP_SIGNALHANDLER_H
