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
	CmdLine cmdline;
	Config config;
    Logger logger(config.get_str_setting("log_path"), "Server");
    //"hello from server.cpp" message, to be removed
    logger.info(config.get_str_setting("config_test"));

    if(config.get_str_setting("start_full_server") == "true") {
        // starting the full server
        std::string port = config.get_str_setting("port");
        logger.info("Starting HackTTP at port: " + port);

        Router router(config.get_int_setting("queue_size"), port);
        router.watch();
    } else {
        logger.info("Running in test mode");
    }
	// cleanup
	logger.info("HackTTP shutting down");

	return 0;
}
