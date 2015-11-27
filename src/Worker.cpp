#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>

#include "Worker.h"
#include "BasicHTTP.h"

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

void Worker::handle_request() {
    char reply[100], *msg;

    this->logger.info("Handling request via socket: " + std::to_string(this->socket_fd));
    // Send welcome message
    std::string string = "Welcome to HACKttp: " + std::to_string(this->socket_fd) + "\nYour message: ";
    msg = (char *) string.c_str();
    send(this->socket_fd, msg, strlen(msg), 0);

    std::string response;
    while(true){
        memset(&reply, 0, sizeof reply);
        if(recv(this->socket_fd, reply, 100, 0) <= 0)
            break;
        response.append("HackTTP echo: ");
        response.append(reply);
        response.append("\nYour message: ");
        msg = (char *)response.c_str();
        send(this->socket_fd, msg, strlen(msg), 0);
        //reply[99] = '\0';
        printf("Sent back: %s", response.c_str());
        response.clear();
    }
    this->logger.debug("Reqest handling done");

    return;
}
