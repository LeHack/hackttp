#ifndef SRC_WORKER_H_
#define SRC_WORKER_H_

#include <cstring>
#include "Logger.h"

class Worker {
private:
    int socket_fd;
    Logger logger;
    void send_msg(std::string msg);

public:
	Worker(int socket_fd);
	virtual ~Worker();
	void handle_request();
};

using namespace std;
// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class WorkerException: public exception {
    std::string reason;

    public:
        WorkerException(std::string msg = "Unknown exception") {
            reason = msg;
        }
        virtual const char* what() const throw() {
            return reason.c_str();
        }
};

#endif /* SRC_WORKER_H_ */
