#ifndef SRC_MANAGER_H_
#define SRC_MANAGER_H_

#include "Exceptions.h"
#include "Logger.h"

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

	class Exception: public BaseException {
	    public:
	        Exception(std::string msg = "Unknown manager exception") {
	            reason = msg;
	        }
	};
};

#endif /* SRC_MANAGER_H_ */
