#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include "BasicHTTP.h"
#include "Config.h"
#include "DataHandler.h"
#include "Worker.h"

#define HTTP_REQUEST_LENGTH 8192

/*
 * worker - uses an appropriate handler to serve the request
 */

Worker::Worker(int socket_fd) {
    this->logger = new Logger("Worker");
    this->socket_fd = socket_fd;
}

Worker::~Worker() {
    // make sure to close the socket when we finish
    this->logger->info("Destructor called");
	close(this->socket_fd);
    delete(this->logger);
}

void Worker::handle_request() {
    char *request = (char *)calloc(HTTP_REQUEST_LENGTH, sizeof(char)); // standard limit of 8kb

    this->logger->info("Handling request via socket: " + std::to_string(this->socket_fd));

    // first read the request
    ssize_t request_size = recv(this->socket_fd, request, HTTP_REQUEST_LENGTH, 0);
    if(request_size < 0) {
        // TODO not sure if necessary
        if(errno == EINTR){
            this->logger->info("Recv() interrupted by signal");
            return;
        }
        // TODO replace with proper HTTP response
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while reading request: " + std::string(err ? err : "unknown error"));
    }

    // get the static file contents
    std::string req_str = std::string(request);

    // TODO replace with some actual request parser (to be part of BasicHTTP)
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

    DataHandler dHandler;
    resource data;
    int return_code = 200;
    if (space_count == 2) {
        std::string path = req_str.substr(prev_pos, pos-prev_pos);

        try {
            data = dHandler.read_resource(path);
        }
        catch (DataHandler::Unsupported &e) {
            this->logger->debug("Unsupported file while handling request: " + std::string(e.what()));
            return_code = 415;
            data = dHandler.get_error_file(return_code, std::string(e.what()));
        }
        catch (DataHandler::Exception &e) {
            this->logger->debug("Got exception while handling request: " + std::string(e.what()));
            return_code = 404;
            data = dHandler.get_error_file(return_code, path.substr(1));
        }
    }
    else {
        this->logger->debug("Could not parse request: " + req_str);
        return_code = 400;
        data = dHandler.get_error_file(return_code, req_str);
    }

    // TODO: move this to BasicHTTP handling
    // apend standard response headers, but only if it's static content
    if (data.type != "executable") {
        std::string headers =
            "HTTP/1.1 "+std::to_string(return_code)+" OK\n"
            "Server: HackTTP\n"
            "Connection: close\n"
            "Content-Type: "+data.type+"\n";
        ;

        if (data.type.find("image/") != std::string::npos) {
            headers += "Accept-Ranges: bytes\n";
            headers += "Content-Length: " + std::to_string(data.size) + "\n";
        }

        // now send the headers
        headers += "\n";
        send_msg((char *) headers.c_str(), headers.length());
    }

    // then the data
    send_msg(data.data, data.size);

    free(data.data);

    this->logger->debug("Request handling done");

    return;
}

void Worker::send_msg(char * msgc, long size) {
    if (send(this->socket_fd, msgc, size, 0) < 0) {
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while sending response to request: " + std::string(err ? err : "unknown error"));
    }
    return;
}
