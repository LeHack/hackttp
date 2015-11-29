#ifndef SRC_WORKER_H_
#define SRC_WORKER_H_

#include "Exceptions.h"
#include "Logger.h"

class Worker {
private:
    int socket_fd;
    Logger logger;
    void send_msg(char * msg);

public:
	Worker(int socket_fd);
	virtual ~Worker();
	void handle_request();
	char * read_static_file(std::string path);

    class Exception: public BaseException {
        public:
            Exception(std::string msg = "Unknown worker exception") {
                this->reason = msg;
            }
    };
    class FileNotFound: public Exception {
        public:
            FileNotFound(std::string msg = "Could not find requested file") {
                this->reason = msg;
            }
    };
};

#endif /* SRC_WORKER_H_ */
