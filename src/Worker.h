#ifndef SRC_WORKER_H_
#define SRC_WORKER_H_

#include "Logger.h"

class Worker {
private:
    int socket_fd;
    Logger logger;

public:
	Worker(int socket_fd);
	virtual ~Worker();
	void handle_request();
};

#endif /* SRC_WORKER_H_ */
