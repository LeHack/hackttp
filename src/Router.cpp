#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "Router.h"

/*
 * router - watches the designated port and for launches a new worker for every connection
 * (when all workers are busy, waits some time before returning an error - "how long" should be kept in the config)
 * Handles *some* of the HTTP errors
 */

Router::Router(int qsize) {
	this->queue_size = qsize;
	this->logger = Logger("Router");
}

Router::~Router() {
	// TODO Auto-generated destructor stub
}

void *newSocketFunction(void *socket_fd);
void Router::watch(std::string port) {
	this->logger.debug("starting with port" + port);

    struct sockaddr_storage incoming_connection_info;
    socklen_t addr_size;
    struct addrinfo listening_socket_description, *results;
    int listening_socket_fd, new_socket_fd;

    // first, load up address structs with getaddrinfo():
    memset(&listening_socket_description, 0, sizeof listening_socket_description);

    listening_socket_description.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    listening_socket_description.ai_socktype = SOCK_STREAM;
    listening_socket_description.ai_flags = AI_PASSIVE;     // fill in my IP for me
    getaddrinfo(NULL, port.c_str(), &listening_socket_description, &results);

    // make a socket, bind it, and listen on it:
    listening_socket_fd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    bind(listening_socket_fd, results->ai_addr, results->ai_addrlen);

    listen(listening_socket_fd, this->queue_size);

    addr_size = sizeof incoming_connection_info;
    new_socket_fd = accept(listening_socket_fd, (struct sockaddr *) &incoming_connection_info, &addr_size);
    this->logger.info("handling connection");

    pthread_t t1;
    this->logger.debug("creating new thread");
    pthread_create(&t1, NULL, newSocketFunction, (void *) &new_socket_fd);
    pthread_join(t1, NULL);

    this->logger.debug("closing connection");
    freeaddrinfo(results);
    close(listening_socket_fd);
    close(new_socket_fd);

    this->logger.debug("done");
}

// Possibly this code will be moved and divided to manager/worker
// that's why I'm leaving this out of the Router:: class
void *newSocketFunction(void *socket_fd_ptr){
	int *socket_fd = (int *)socket_fd_ptr;
    char reply[100], *msg;

    std::string string = "Welcome to HACKttp: " + std::to_string(*socket_fd) + "\nYour message: ";
    //msg = "Welcome to HACKttp " + (char) socket_fd;
    msg = (char *) string.c_str();
    send(*socket_fd, msg, strlen(msg), 0);

    std::string response;
    while(1){
        memset(&reply, 0, sizeof reply);
        if(recv(*socket_fd, reply, 100, 0) <= 0) break;
        response.append("HackTTP echo: ");
        response.append(reply);
		response.append("\nYour message: ");
        msg = (char *)response.c_str();
        send(*socket_fd, msg, strlen(msg), 0);
        //reply[99] = '\0';
        printf("Sent back: %s", response.c_str());
        response.clear();
    }

    return NULL;
}
