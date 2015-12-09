//
// Created by atticus on 12/9/15.
//

#include <string.h>
#include <signal.h>
#include <iostream>
#include "SignalHandler.h"
#include "globals.h"

void SignalHandler::signalHandler(int sig, siginfo_t *siginfo, void *context){
    std::cout << "Signal recieved: SignalHandler" << std::endl;
    isSigintRecieved = true;
}

SignalHandler::~SignalHandler() {

}

SignalHandler::SignalHandler() {

    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    /* Use the sa_sigaction field because the handles has two additional parameters */
    act.sa_sigaction = &signalHandler;
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        std::cout <<"Signal error" <<std::endl;
    }
}
