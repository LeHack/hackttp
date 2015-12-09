//
// Created by atticus on 12/9/15.
//

#include <string.h>
#include <signal.h>
#include <iostream>
#include "SignalHandler.h"
#include "globals.h"

bool isSigintRecieved;
bool isSigusr1Recieved;

void SignalHandler::sigintHandler(int sig, siginfo_t *siginfo, void *context){
    std::cout << "SignalHandler: Signal recieved - SIGINT" << std::endl;
    isSigintRecieved = true;
}

void SignalHandler::sigusr1Handler(int sig, siginfo_t *siginfo, void *context){
    std::cout << "SignalHandler: Signal recieved - SIGUSR1, Config will be reloaded on next use." << std::endl;
    isSigusr1Recieved = true;
}

SignalHandler::~SignalHandler() {

}

SignalHandler::SignalHandler() {
    isSigintRecieved = false;
    isSigusr1Recieved = false;

    struct sigaction sigintStruct;
    memset (&sigintStruct, '\0', sizeof(sigintStruct));
    /* Use the sa_sigaction field because the handles has two additional parameters */
    sigintStruct.sa_sigaction = &sigintHandler;
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    sigintStruct.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &sigintStruct, NULL) < 0) {
        std::cout <<"Signal error" <<std::endl;
    }

    struct sigaction sigusr1Struct;
    memset (&sigusr1Struct, '\0', sizeof(sigusr1Struct));
    /* Use the sa_sigaction field because the handles has two additional parameters */
    sigusr1Struct.sa_sigaction = &sigusr1Handler;
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    sigusr1Struct.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGUSR1, &sigusr1Struct, NULL) < 0) {
        std::cout <<"Signal error" <<std::endl;
    }

}
