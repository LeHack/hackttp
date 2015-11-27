#ifndef SRC_MANAGER_H_
#define SRC_MANAGER_H_

#include "Logger.h"
#include "Worker.h"

class Manager {
private:
    pthread_t * pool; // keeps a pool of available workers
    Logger logger;
    int get_free_worker();

public:
	Manager();
	virtual ~Manager();
	void handle_request(int socket_fd);
};

#endif /* SRC_MANAGER_H_ */
