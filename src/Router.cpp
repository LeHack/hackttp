#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>
#include <chrono>

#include "Router.h"
#include "Manager.h"

/*
 * router - watches the designated port and for launches a new worker for every connection
 * (when all workers are busy, waits some time before returning an error - "how long" should be kept in the config)
 * Handles *some* of the HTTP errors
 */

Router::Router(int qsize, std::string port) {
	this->queue_size = qsize;
	this->logger = Logger("Router");
	this->port = port;
    // initialize the socket
    this->init_socket();
}

Router::~Router() {
    // close our socket
    close(this->listening_socket_fd);
    // free the address
    freeaddrinfo(addr);
}

void Router::watch() {
	socklen_t addr_size;
    struct sockaddr_storage incoming_connection_info;
    int handling_socket;

    this->logger.debug("watching port: " + this->port);

    // create a manager object to handle different threads
    Manager manager;

    // we work until we're told to stop working
    while (true) {
        // listen for new connections
        listen(this->listening_socket_fd, this->queue_size);

        addr_size = sizeof incoming_connection_info;
        handling_socket = accept(listening_socket_fd, (struct sockaddr *) &incoming_connection_info, &addr_size);
        this->logger.info("Handling incoming connection from: ");
        // this will create a new worker to work with
        // TODO: handle too many workers exception
        manager.handle_request(handling_socket);

        // sleep a little
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }
}

int Router::init_socket() {
    struct addrinfo listening_socket_description;

    // first, load up address structs with getaddrinfo():
    // TODO error handling (what if there's no memory available)
    memset(&listening_socket_description, 0, sizeof listening_socket_description);

    listening_socket_description.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    listening_socket_description.ai_socktype = SOCK_STREAM;
    listening_socket_description.ai_flags = AI_PASSIVE;     // fill in my IP for me
    getaddrinfo(NULL, this->port.c_str(), &listening_socket_description, &this->addr);

    // TODO error handling (what if we cannot open the socket)
    int listening_socket = socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol);

    // TODO error handling (what if we can't bind)
    bind(listening_socket, this->addr->ai_addr, this->addr->ai_addrlen);

    return listening_socket;
}
