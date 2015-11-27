#ifndef SRC_ROUTER_H_
#define SRC_ROUTER_H_
#include <cstring>
#include <iostream>

#include "Logger.h"

class Router {
private:
	int queue_size;
	int listening_socket_fd;
	std::string port;
	struct addrinfo *addr;
	Logger logger;
	int init_socket();

public:
	Router(int qsize, std::string port);
	virtual ~Router();
	void watch();
};

using namespace std;
// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class RouterException: public exception {
    std::string reason;

    public:
        RouterException(std::string msg = "Unknown exception") {
            reason = msg;
        }
        virtual const char* what() const throw() {
            return reason.c_str();
        }
};

#endif /* SRC_ROUTER_H_ */
