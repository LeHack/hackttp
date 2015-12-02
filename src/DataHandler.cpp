#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>

#include "DataHandler.h"

/*
 * handlers - currently two are planned: static file handler, cgi script handler
 * 1. cgi handler - fires given script and returns it's output
 * 2. static resource handler - should be used by static file handler for caching
 * 		already retrieved files (up to some configured limit, then gracefully rotate the cache)
 * 		this memory should be shared between all workers/static handlers
 */

DataHandler::DataHandler() {
    this->logger = new Logger("DataHandler");
}

DataHandler::~DataHandler() {
    delete(this->logger);
}

bool DataHandler::verify_path(std::string path) {
    bool path_ok = false;

    // verify that the path is not all slashes
    // there's a very nice uncaught exception when we pass '//+' to open()
    for(char c : path) {
        if (c != '/') {
            path_ok = true;
            break;
        }
    }

    return path_ok;
}

bool is(char * mime, std::string type) {
    return (strstr(mime, type.c_str()) != NULL);
}

resource DataHandler::read_resource(std::string path) {
    // prepend cwd() to path

    if (!verify_path(path))
        path = "/index.html";

    std::string cwd = get_working_path();
    path = cwd + path;
    this->logger->debug("Checking resource at: " + path);

    // check mime type of resource
    std::string args[2] = { "/usr/bin/file", path };
    resource file_mime = run_command(args);
    char * mime = file_mime.data;

    resource output;
    // now check for known mime types
    if (is(mime, "executable")) {
        // run the script
        std::string args[2] = { path, "" }; // TODO: Fix me too!
        resource script_output = run_command(args);
        output.data = script_output.data;
        output.size = script_output.size;
        output.type = "executable";
    }
    // TODO: Move this definition to some more reasonable place
    else if (is(mime, "HTML"))                       { output.type = "text/html; charset=UTF-8";  }
    else if (is(mime, "ERROR") || is(mime, "ASCII")) { output.type = "text/plain; charset=UTF-8"; }
    else if (is(mime, "JPEG"))                       { output.type = "image/jpeg";                }
    else if (is(mime, "PNG"))                        { output.type = "image/png";                 }
    else if (is(mime, "MS Windows icon"))            { output.type = "image/vnd.microsoft.icon";  }

    if (output.type.length() > 0 && output.type != "executable") {
        resource f = get_file(path);
        output.data = f.data;
        output.size = f.size;
    }
    else if (output.type.length() == 0){
        std::string error_str = "Unsupported mime type: " + std::string(mime);
        // drop 'local' part of path
        size_t pos = 0;
        while ((pos = error_str.find(cwd)) != std::string::npos )
            error_str.erase(pos, cwd.length());

        throw DataHandler::Unsupported(error_str);
    }

    return output;
}

std::string DataHandler::get_working_path() {
   char temp[PATH_MAX];
   return ( getcwd(temp, PATH_MAX) ? std::string( temp ) : std::string("") );
}

resource DataHandler::get_file(std::string path) {
    this->logger->debug("Read file: " + path);
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        // TODO: replace with a proper HTTP CODE
        char * err = std::strerror(errno);
        throw DataHandler::FileNotFound("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
    }

    long fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    resource data;
    data.data = (char *) malloc(fsize+1);
    read(fd, data.data, fsize);
    close(fd);

    data.data[fsize] = '\0';
    data.size = fsize;

    return data;
}

resource DataHandler::get_error_file(int error_code, std::string param) {
    // start by reading the error template
    resource output = read_resource("/errors/"+ std::to_string(error_code) +".html");
    // now prepare a place to write the filled template
    long new_size = output.size + param.length() - 3;
    char * data = (char *) malloc(new_size * sizeof(char));
    // and fill it
    sprintf(data, output.data, param.c_str());

    // free old structure
    free(output.data);
    // replace with new pointer
    output.data = data;
    output.size = new_size;

    return output;
}

resource DataHandler::run_command(std::string args[]) {
    int comms[2];
    // start with a small 1k buffer
    int buf_pos = 0, buf_max = 1024, buf_blocks = 1;
    resource output;
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
