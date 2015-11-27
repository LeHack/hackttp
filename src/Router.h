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

#endif /* SRC_ROUTER_H_ */
