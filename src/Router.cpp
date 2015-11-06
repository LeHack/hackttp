#include "Router.h"

/*
 * router - watches the designated port and for launches a new worker for every connection
 * (when all workers are busy, waits some time before returning an error - "how long" should be kept in the config)
 * Handles *some* of the HTTP errors
 */

Router::Router() {
	// TODO Auto-generated constructor stub

}

Router::~Router() {
	// TODO Auto-generated destructor stub
}

