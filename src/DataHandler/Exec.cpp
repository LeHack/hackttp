#include "../DataHandler.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>

/*
 * handlers - currently two are planned: static file handler, cgi script handler
 * 1. cgi handler - fires given script and returns it's output
 * 2. static resource handler - should be used by static file handler for caching
 * 		already retrieved files (up to some configured limit, then gracefully rotate the cache)
 * 		this memory should be shared between all workers/static handlers
 */

DataHandler::Exec::Exec() {
    this->logger = new Logger("DataHandler::Exec");
}

DataHandler::Exec::~Exec() {
    delete(this->logger);
}

DataHandler::resource DataHandler::Exec::run_command(std::string args[]) {
    int comms[2];
    // start with a small 1k buffer
    int buf_pos = 0, buf_max = 1024, buf_blocks = 1;
    DataHandler::resource output;
    output.data = (char *) malloc(buf_blocks * buf_max * sizeof(char));

    if (pipe(comms) < 0) {
        char * err = std::strerror(errno);
        throw DataHandler::Exception("Cannot create pipe: " + std::string(err ? err : "unknown error"));
    }

    int pid = fork();
    if (pid == 0) {
        // redirect STDOUT to our comms
        if (dup2(comms[1], STDOUT_FILENO) == -1) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception("Cannot redirect STDOUT to pipe: " + std::string(err ? err : "unknown error"));
        }

        // same with STDERR
        if (dup2(comms[1], STDERR_FILENO) == -1) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception("Cannot redirect STDERR to pipe: " + std::string(err ? err : "unknown error"));
        }

        // now close the pipe on our side
        close(comms[0]);

        // now run the target
        // TODO: Fix me! I'm ugly!
        if (execl(args[0].c_str(), args[0].c_str(), args[1].c_str(), (char *) 0) < 0) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception(
                "Cannot exec '" + std::string(args[0]) + "' due to: " + std::string(err ? err : "unknown error")
            );
        }
    }
    else if (pid > 0) {
        close(comms[1]);

        // Just a char by char read here, you can change it accordingly
        char rchar;
        while (read(comms[0], &rchar, 1) == 1) {
            output.data[buf_pos++] = rchar;
            if (buf_pos >= buf_max){
                // extend the buffer by another 1k block
                buf_blocks++;
                // reset the curent position
                buf_pos = 0;
                // allocate more memory
                output.data = (char *) realloc(output.data, buf_blocks * buf_max * sizeof(char));
                if (output.data == NULL) {
                    char * err = std::strerror(errno);
                    throw DataHandler::Exception(
                        "Error while reading output from: " + std::string(args[0]) +", "
                        + "because of: " + std::string(err ? err : "unknown error")
                    );
                }
            }
        }
        // add NULL termination
        output.data[buf_pos] = '\0';
        output.size = (buf_blocks - 1) * buf_max + buf_pos + 1;

        close(comms[0]);
    }
    else {
        // fork failed
        char * err = std::strerror(errno);
        close(comms[0]);
        close(comms[1]);
        throw DataHandler::Exception("Cannot fork: " + std::string(err ? err : "unknown error"));
    }

    if (strstr(output.data, "DataHandler::Exception") != NULL) {
        throw DataHandler::Exception("Fork returned an exception: " + std::string(output.data));
    }

    return output;
}
