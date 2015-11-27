#ifndef SRC_MANAGER_H_
#define SRC_MANAGER_H_

#include "Logger.h"
#include "Worker.h"

class Manager {
private:
    int worker_count;
    pthread_t * pool; // keeps a pool of available workers
    Logger logger;
    int get_free_worker_index();

public:
	Manager();
	virtual ~Manager();
	void handle_request(int socket_fd);
};


using namespace std;
// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class ManagerException: public exception {
    std::string reason;

    public:
        ManagerException(std::string msg = "Unknown exception") {
            reason = msg;
        }
        virtual const char* what() const throw() {
            return reason.c_str();
        }
};

#endif /* SRC_MANAGER_H_ */
