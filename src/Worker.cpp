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
	close(this->socket_fd);
    delete(this->logger);
}

void Worker::handle_request() {
    char *request = (char *)calloc(HTTP_REQUEST_LENGTH, sizeof(char)); // standard limit of 8kb

    this->logger->info("Handling request via socket: " + std::to_string(this->socket_fd));

    // first read the request
    ssize_t request_size = recv(this->socket_fd, request, HTTP_REQUEST_LENGTH, 0);
    if(request_size < 0) {
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

    DataHandler dHandler;
    char * data;
    int return_code = 200;
    if (space_count == 2) {
        std::string path = req_str.substr(prev_pos, pos-prev_pos);

        try {
            data = dHandler.read_resource(path);
        }
        catch (DataHandler::FileNotFound &e) {
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

    // apend standard response headers
    // TODO: move this to BasicHTTP handling
    char * headers = (char *) std::string(
        "HTTP/1.x "+std::to_string(return_code)+" OK\n"
        "Server: HackTTP\n"
        "Connection: close\n"
        "Content-Type: text/html; charset=UTF-8\n\n"
    ).c_str();

    // now first send the headers
    send_msg(headers);

    // then the data
    send_msg(data);

    free(data);

    this->logger->debug("Request handling done");

    return;
}

void Worker::send_msg(char * msgc) {
    if (send(this->socket_fd, msgc, strlen(msgc), 0) < 0) {
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while sending response to request: " + std::string(err ? err : "unknown error"));
    }
    return;
}
