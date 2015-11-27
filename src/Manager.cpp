#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <pthread.h>

#include "Manager.h"
#include "Worker.h"
/*
 * manager - keeps track of workers, their current status, should handle zombies (number of workers comes from config)
 */

Manager::Manager() {
    this->logger = Logger("Manager");
    // TODO Move to config file and fetch directly from there
    int worker_count = 10;
    this->pool = (pthread_t*) calloc(worker_count, sizeof(pthread_t));
}

Manager::~Manager() {
    free(this->pool);
}

void *worker_runner(void *socket_fd);

void Manager::handle_request(int socket_fd) {
    this->logger.debug("Handling request via socket: " + std::to_string(socket_fd));

    // get worker, if available
    pthread_t worker_t = this->get_free_worker();

    // handle the requst in a new thread
    pthread_create(&worker_t, NULL, worker_runner, (void *) &socket_fd);

    return;
}

int Manager::get_free_worker() {
    // TODO look for a free worker
    return this->pool[0];
}

void *worker_runner(void *socket_fd) {
    int *socket = (int *) socket_fd;
    Worker worker(*socket);
    worker.handle_request();

    return NULL;
}
