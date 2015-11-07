#ifndef SRC_ROUTER_H_
#define SRC_ROUTER_H_

#include <iostream>
#include "Logger.h"

class Router {
private:
	int queue_size;
	Logger logger;
public:
	Router(int qsize);
	virtual ~Router();
	void watch(std::string port);
};

#endif /* SRC_ROUTER_H_ */
