//
// Created by atticus on 12/9/15.
//
#include "SignalHandler.h"

bool isSigintReceived;
bool isSigusr1Received;

Logger *SignalHandler::logger;
SignalHandler::SignalHandler() {
    isSigintReceived = false;
    isSigusr1Received = false;
    logger = new Logger("SignalHandler");

    struct sigaction sigintStruct;
    memset (&sigintStruct, '\0', sizeof(sigintStruct));
    /* Use the sa_sigaction field because the handles has two additional parameters */
    sigintStruct.sa_sigaction = &sigintHandler;
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    sigintStruct.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &sigintStruct, NULL) < 0) {
        logger->warn("Signal error");
    }

    struct sigaction sigusr1Struct;
    memset (&sigusr1Struct, '\0', sizeof(sigusr1Struct));
    /* Use the sa_sigaction field because the handles has two additional parameters */
    sigusr1Struct.sa_sigaction = &sigusr1Handler;
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    sigusr1Struct.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGUSR1, &sigusr1Struct, NULL) < 0) {
        logger->warn("Signal error");
    }
    logger->debug("Ready");
}

SignalHandler::~SignalHandler() {
    delete(logger);
}

void SignalHandler::sigintHandler(int sig, siginfo_t *siginfo, void *context){
    logger->info("SignalHandler: Signal recieved - SIGINT, Shutting down");
    isSigintReceived = true;
}

void SignalHandler::sigusr1Handler(int sig, siginfo_t *siginfo, void *context){
    logger->info("SignalHandler: Signal recieved - SIGUSR1, Config will be reloaded on next use.");
    isSigusr1Received = true;
}
