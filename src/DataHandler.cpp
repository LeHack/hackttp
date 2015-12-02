#include <unistd.h>
#include <cstring>
#include <fcntl.h>
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

char * DataHandler::read_resource(std::string path) {
    // prepend cwd() to path

    if (!verify_path(path))
        path = "/index.html";

    path = get_working_path() + path;
    this->logger->debug("Reading file at: " + path);

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        // TODO: replace with a proper HTTP CODE
        char * err = std::strerror(errno);
        throw DataHandler::FileNotFound("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
    }

    lseek(fd, 0, SEEK_END);
    long fsize = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);

    char * data = (char *) malloc(fsize + 1);
    read(fd, data, fsize);
    close(fd);
    data[fsize] = '\0';

    return data;
}

std::string DataHandler::get_working_path() {
   char temp[PATH_MAX];
   return ( getcwd(temp, PATH_MAX) ? std::string( temp ) : std::string("") );
}

char * DataHandler::get_error_file(int error_code, std::string param) {
    // start by reading the error template
    char * contents = read_resource("/errors/"+ std::to_string(error_code) +".html");
    // now prepare a place to write the filled template
    char * data = (char *) malloc(strlen(contents) + param.length() - 3);
    // and fill it
    sprintf(data, contents, param.c_str());
    return data;
}
