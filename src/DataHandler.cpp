#include "DataHandler.h"

/*
 * handlers - currently two are planned: static file handler, cgi script handler
 * 1. cgi handler - fires given script and returns it's output
 * 2. static resource handler - should be used by static file handler for caching
 * 		already retrieved files (up to some configured limit, then gracefully rotate the cache)
 * 		this memory should be shared between all workers/static handlers
 */

DataHandler::DataHandler() {

}

DataHandler::~DataHandler() {
}

