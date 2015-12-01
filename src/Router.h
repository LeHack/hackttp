#ifndef SRC_ROUTER_H_
#define SRC_ROUTER_H_

#include "Exceptions.h"
#include "Logger.h"

class Router {
private:
	int queue_size;
	int listening_socket_fd;
	std::string port;
	struct addrinfo *addr;
	Logger *logger;
	int init_socket();

public:
	Router(int qsize, std::string port);
	virtual ~Router();
	void watch();

	class Exception: public BaseException {
	    public:
	        Exception(std::string msg = "Unknown router exception") {
	            reason = msg;
	        }
	};
};

#endif /* SRC_ROUTER_H_ */
