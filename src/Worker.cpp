#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "Worker.h"
#include "BasicHTTP.h"
#include "Config.h"

#define HTTP_REQUEST_LENGTH 8192

/*
 * worker - uses an appropriate handler to serve the request
 */

Worker::Worker(int socket_fd) {
    this->logger = Logger(Config::get_str_setting("log_path"), "Worker");
    this->socket_fd = socket_fd;
}

Worker::~Worker() {
    // make sure to close the socket when we finish
	close(this->socket_fd);
}

std::string get_working_path();

void Worker::handle_request() {
    char *request = (char *)calloc(HTTP_REQUEST_LENGTH, sizeof(char)); // standard limit of 8kb

    this->logger.info("Handling request via socket: " + std::to_string(this->socket_fd));

    // first read the request
    if(recv(this->socket_fd, request, HTTP_REQUEST_LENGTH, 0) <= 0) {
        // TODO replace with proper HTTP response
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while reading request: " + std::string(err ? err : "unknown error"));
    }

    // get the static file contents
    std::string req_str = std::string(request);
    int space_count = 0;
    size_t pos = 0, prev_pos = 0;
    while (space_count < 2) {
        prev_pos = pos + 1;
        if ((pos = req_str.find(" ", prev_pos)) != std::string::npos) {
            space_count++;
        }
        else
            break;
    }

    char * data;
    if (space_count == 2) {
        std::string file_path = req_str.substr(prev_pos, pos-prev_pos);
        // add simple CGI handling

        try {
            data = read_static_file(file_path);
        }
        catch (Worker::FileNotFound &e) {
            this->logger.debug("Got exception while handling request: " + std::string(e.what()));
            char * contents = read_static_file("/errors/404.html");
            data = (char *) malloc(strlen(contents) + file_path.length() - 3);
            sprintf(data, contents, file_path.substr(1).c_str());
        }
    }
    else {
        data = (char*) std::string("<h1>Error, could not parse request</h1><pre>" + req_str + "</pre>").c_str();
    }

    // apend standard response headers
    // TODO: move this to BasicHTTP handling
    char * headers = (char *) std::string(
        "HTTP/1.x 200 OK\nServer: HackTTP\nConnection: close\nContent-Type: text/html; charset=UTF-8\n\n"
    ).c_str();

    // now first send the headers
    send_msg(headers);

    // then the data
    send_msg(data);

    free(data);

    this->logger.debug("Request handling done");

    return;
}

void Worker::send_msg(char * msgc) {
    if (send(this->socket_fd, msgc, strlen(msgc), 0) < 0) {
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while sending response to request: " + std::string(err ? err : "unknown error"));
    }
    return;
}

char * Worker::read_static_file(std::string path) {
    // prepend cwd() to path
    if (path == "/")
        path = "/index.html";

    path = get_working_path() + path;
    this->logger.debug("Reading file at: " + path);

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        // TODO: replace with a proper HTTP CODE
        char * err = std::strerror(errno);
        throw Worker::FileNotFound("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
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

std::string get_working_path() {
   char temp[PATH_MAX];
   return ( getcwd(temp, PATH_MAX) ? std::string( temp ) : std::string("") );
}
