#include <iostream>
#include "CmdLine.h"
#include "Config.h"
#include "Router.h"
#include "Logger.h"

/*
 * "main" - should use other modules for everything, that is:
 *	- read cmd line options
 *	- read config file
 *	- bind the router tho the receiving port and give it control
 *	- when router finishes, log and shutdown
 */

int main() {
	// Init basic classes
	CmdLine cmdline();
	Config config();
	Logger logger();
	Router router();

	// starting watching the incoming port using the router


	// cleanup

	return 0;
}
