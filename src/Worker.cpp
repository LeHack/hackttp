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

#define HTTP_REQUEST_LENGTH 8192

/*
 * worker - uses an appropriate handler to serve the request
 */

Worker::Worker(int socket_fd) {
    this->logger = Logger("Worker");
    this->socket_fd = socket_fd;
}

Worker::~Worker() {
    // make sure to close the socket when we finish
	close(this->socket_fd);
}

//void read_static_file(std::string path, char * data);
std::string get_working_path();

void Worker::handle_request() {
    char *request = (char *)calloc(HTTP_REQUEST_LENGTH, sizeof(char)); // standard limit of 8kb

    this->logger.info("Handling request via socket: " + std::to_string(this->socket_fd));

    // first read the request
    if(recv(this->socket_fd, request, HTTP_REQUEST_LENGTH, 0) <= 0) {
        // TODO replace with proper HTTP response
        char * err = std::strerror(errno);
        throw WorkerException("Error while reading request: " + std::string(err ? err : "unknown error"));
    }

    // get the static file contents
    std::string req_str = std::string(request);
    int space_count = 0;
    size_t pos = 0, prev_pos = 0;
    while (space_count < 2) {
        prev_pos = pos;
        if ((pos = req_str.find(" ", prev_pos+1)) != std::string::npos) {
            space_count++;
        }
        else
            break;
    }

    char * data;
    if (space_count == 2) {
        std::string file_path = req_str.substr(prev_pos, pos-prev_pos);
//        data = "Contents of file at: " + file_path + "\nUsing request: " + req_str;
        if (file_path.compare("/"))
            file_path = "/index.txt";

        file_path = get_working_path() + file_path;
        int fd = open(file_path.c_str(), O_RDONLY);
        if (fd < 0) {
            // TODO: replace with a proper HTTP CODE
            char * err = std::strerror(errno);
            throw WorkerException("Error while reading file contents at " + file_path + ": " + std::string(err ? err : "unknown error"));
        }

        this->logger.debug("Opened file: " + file_path);

        long fsize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        this->logger.debug("File size: " + std::to_string(fsize));
        data = (char *) malloc(fsize+1);
        this->logger.debug("Memory assigned");
        read(fd, &data, fsize);
        this->logger.debug("Data read");
        close(fd);
        // add null termination
        this->logger.debug("... and null-terminated");
        data[fsize] = '\0';

//        this->logger.debug("File data: " + std::string(data));
//        read_static_file(file_path, data);
    }
    else {
        data = (char*) std::string("Error, could not parse request").c_str();
    }

//    data.append("Welcome to HACKttp: " + std::to_string(this->socket_fd) + "<br/>Your message: ");
//    data.append(file_path);
//    send_msg(data);

    // now answer
//    printf("Sending data: %s\n", data);
    if (send(this->socket_fd, data, strlen(data), 0) < 0) {
        char * err = std::strerror(errno);
        throw WorkerException("Error while reading request: " + std::string(err ? err : "unknown error"));
    }
    free(data);

    this->logger.debug("Reqest handling done");

    return;
}

void Worker::send_msg(std::string msg) {
    char *msgc = (char *) msg.c_str();
    send(this->socket_fd, msgc, strlen(msgc), 0);
    return;
}

//void read_static_file(std::string path, char * data) {
//    // prepend cwd() to path
//    path = get_working_path() + path;
//    int fd = open(path.c_str(), O_RDONLY);
//    if (fd < 0) {
//        // TODO: replace with a proper HTTP CODE
//        char * err = std::strerror(errno);
//        throw WorkerException("Error while reading file contents at " + path + ": " + std::string(err ? err : "unknown error"));
//    }
//
//    lseek(fd, 0, SEEK_END);
//    long fsize = lseek(fd, 0, SEEK_CUR);
//    lseek(fd, 0, SEEK_SET);
//
//    data = (char *) malloc(fsize + 1);
//    read(fd, &data, fsize);
//    close(fd);
//
//    return;
//}

std::string get_working_path() {
   char temp[PATH_MAX];
   return ( getcwd(temp, PATH_MAX) ? std::string( temp ) : std::string("") );
}
