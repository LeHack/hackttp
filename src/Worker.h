#ifndef SRC_WORKER_H_
#define SRC_WORKER_H_

#include "Logger.h"

class Worker {
private:
    int socket_fd;
    Logger *logger;
    void send_msg(char * msg, long size);

public:
	Worker(int socket_fd);
	virtual ~Worker();
	void handle_request();

    class Exception: public BaseException {
        public:
            Exception(std::string msg = "Unknown worker exception") {
                this->reason = msg;
            }
    };
};

#endif /* SRC_WORKER_H_ */
