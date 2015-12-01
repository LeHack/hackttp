#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <pthread.h>
#include <mutex>

#include "Manager.h"
#include "Worker.h"
#include "Config.h"
/*
 * manager - keeps track of workers, their current status, should handle zombies (number of workers comes from config)
 */

// global
pthread_mutex_t * mutex_pool; // separate locks for each thread

Manager::Manager() {
    this->logger = new Logger(Config::get_str_setting("log_path"), "Manager");
    this->worker_count = Config::get_int_setting("worker_count");
    this->pool = (pthread_t*)       calloc(worker_count, sizeof(pthread_t));
    mutex_pool = (pthread_mutex_t*) calloc(worker_count, sizeof(pthread_mutex_t));
    for (int i = 0; i < this->worker_count; i++) {
        pthread_mutex_init(&mutex_pool[i], NULL);
    }
}

Manager::~Manager() {
    free(this->pool);
    delete(this->logger);
}

void *worker_runner(void *socket_fd);

void Manager::handle_request(int socket_fd) {
    this->logger->debug("Creating new thread for request at socket: " + std::to_string(socket_fd));

    // get worker, if available
    int worker_ind = this->get_free_worker_index();

    // handle the requst in a new thread
    int * params = (int*) malloc(2*sizeof(int));
    params[0] = worker_ind;
    params[1] = socket_fd;
    if (pthread_create(&this->pool[worker_ind], NULL, worker_runner, (void *)params) < 0) {
        char * err = std::strerror(errno);
        throw Manager::Exception("Error creating thread: " + std::string(err ? err : "unknown error"));
    }

    return;
}

int Manager::get_free_worker_index() {
    int free_worker = -1;
    for (int i=0; i < this->worker_count; i++) {
        // try locking the mutex, if it doesn't lock,
        if (!pthread_mutex_trylock(&mutex_pool[i])) {
            free_worker = i;
            break;
        }
    }

    if (free_worker < 0)
        throw Manager::Exception("Max workers exceeded, no free worker available");

    this->logger->debug("Using worker: " + std::to_string(free_worker));
    return free_worker;
}

void *worker_runner(void *arg) {
    int *params = (int *) arg;
    int worker_ind = params[0];
    int socket_fd  = params[1];

    // start handling the request
    Worker worker(socket_fd);
    worker.handle_request();

    // now we have to notify the manager, that we finished by unlocking our mutex
    pthread_mutex_unlock(&mutex_pool[worker_ind]);
    pthread_exit(0);
}

