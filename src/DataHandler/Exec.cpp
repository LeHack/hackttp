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
    return DataHandler::Exec::run_command(args, NULL);
}
DataHandler::resource DataHandler::Exec::run_command(std::string args[], DataHandler::resource * data) {
    return DataHandler::Exec::run_command(args, NULL, "");
}
DataHandler::resource DataHandler::Exec::run_command(std::string args[], DataHandler::resource * data, std::string cookies) {
    const int PIPE_READ = 0, PIPE_WRITE = 1;
    int comms_in[2];
    int comms_out[2];
    // start with a small 1k buffer
    int buf_pos = 0, buf_max = 1024, buf_blocks = 1;
    DataHandler::resource output;
    output.data = (char *) malloc(buf_blocks * buf_max * sizeof(char));

    if (pipe(comms_in) < 0) {
        char * err = std::strerror(errno);
        throw DataHandler::Exception("Cannot create pipe to exec: " + std::string(err ? err : "unknown error"));
    }
    if (pipe(comms_out) < 0) {
        close(comms_in[PIPE_READ]);
        close(comms_in[PIPE_WRITE]);
        char * err = std::strerror(errno);
        throw DataHandler::Exception("Cannot create pipe from exec: " + std::string(err ? err : "unknown error"));
    }

    int pid = fork();
    if (pid == 0) {
        int sin = dup(STDIN_FILENO), sout = dup(STDOUT_FILENO), serr = dup(STDERR_FILENO);

        // redirect STDIN to our comms
        if (dup2(comms_in[PIPE_READ], STDIN_FILENO) == -1) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception("Cannot redirect STDIN to pipe: " + std::string(err ? err : "unknown error"));
        }

        // redirect STDOUT to our comms
        if (dup2(comms_out[PIPE_WRITE], STDOUT_FILENO) == -1) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception("Cannot redirect STDOUT to pipe: " + std::string(err ? err : "unknown error"));
        }

        // same with STDERR
        if (dup2(comms_out[PIPE_WRITE], STDERR_FILENO) == -1) {
            char * err = std::strerror(errno);
            throw DataHandler::Exception("Cannot redirect STDERR to pipe: " + std::string(err ? err : "unknown error"));
        }

        // now close the pipes on our side
        close(comms_in[PIPE_WRITE]);
        close(comms_out[PIPE_READ]);

        // set the request method correctly
        if (data) {
            std::string req_method = "REQUEST_METHOD=" + data->type;
            putenv((char *)req_method.c_str());
            std::string req_size = "CONTENT_LENGTH=" + std::to_string(data->size + 1);
            putenv((char *)req_size.c_str());
            // don't forget to pass any GET data we might have got for the executed command
            if (data->type == "GET") {
                std::string req_data = "QUERY_STRING=" + std::string(data->data);
                putenv((char *)req_data.c_str());
            }
        }
        if (cookies.length() > 0) {
            std::string req_cookies = "COOKIE=" + cookies;
            putenv((char *)req_cookies.c_str());
        }

        // now run the target
        // TODO: Fix me! I'm ugly!
        if (execl(args[0].c_str(), args[0].c_str(), args[1].c_str(), (char *) 0) < 0) {
            char * err = std::strerror(errno);
            // close everything before leaving
            close(comms_in[PIPE_READ]);
            close(comms_out[PIPE_WRITE]);
            // and restore IN/OUT streams
            dup2(sin,  STDIN_FILENO);  close(sin);
            dup2(sout, STDOUT_FILENO); close(sout);
            dup2(serr, STDERR_FILENO); close(serr);
            // finally throw the exception
            throw DataHandler::Exec::PermissionDenied(
                "Cannot exec '" + std::string(args[0]) + "' due to: " + std::string(err ? err : "unknown error")
            );
        }
    }
    else if (pid > 0) {
        close(comms_in[PIPE_READ]);
        close(comms_out[PIPE_WRITE]);

        // Now first write what we have
        if (data && data->type == "POST") {
            write(comms_in[PIPE_WRITE], data->data, data->size + 1);
        }
        close(comms_in[PIPE_WRITE]);

        // Then listen what they have
        char rchar;
        // TODO: 1 char at a time is a tad slow, we could do better
        while (read(comms_out[PIPE_READ], &rchar, 1) == 1) {
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

        close(comms_out[PIPE_READ]);
    }
    else {
        // fork failed
        char * err = std::strerror(errno);
        close(comms_in[PIPE_READ]);
        close(comms_in[PIPE_WRITE]);
        close(comms_out[PIPE_READ]);
        close(comms_out[PIPE_WRITE]);
        throw DataHandler::Exception("Cannot fork: " + std::string(err ? err : "unknown error"));
    }

    if (strstr(output.data, "DataHandler::Exception") != NULL) {
        throw DataHandler::Exception("Fork returned an exception: " + std::string(output.data));
    }

    return output;
}
