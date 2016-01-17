#include "../DataHandler.h"

#include <fcntl.h>
#include <unistd.h>

/*
 * handlers - currently two are planned: static file handler, cgi script handler
 * 1. cgi handler - fires given script and returns it's output
 * 2. static resource handler - should be used by static file handler for caching
 * 		already retrieved files (up to some configured limit, then gracefully rotate the cache)
 * 		this memory should be shared between all workers/static handlers
 */

DataHandler::Static::Static() {
    this->logger = new Logger("DataHandler::Static");
}

DataHandler::Static::~Static() {
    delete(this->logger);
}

DataHandler::resource DataHandler::Static::get_file(std::string path) {
    this->logger->debug("Read file: " + path);
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        char * err = std::strerror(errno);
        throw DataHandler::FileNotFound("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
    }

    long fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    DataHandler::resource data;
    data.data = (char *) malloc(fsize+1);
    if (read(fd, data.data, fsize) == -1) {
        char * err = std::strerror(errno);
        throw DataHandler::Exception("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
    }
    close(fd);

    data.data[fsize] = '\0';
    data.size = fsize;

    return data;
}
