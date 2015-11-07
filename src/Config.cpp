#include "Config.h"

/*
 * config - loads config file on demand, keeps the config in shared RO memory for fast access
 */

Config::Config() {
	// TODO Auto-generated constructor stub

}

Config::~Config() {
	// TODO Auto-generated destructor stub
}


int Config::get_int_setting(std::string setting_name) {
	int value = -1;
	if (setting_name == "queue_size") {
		value = this->queue_size;
	}
	else {
		throw ConfigException("Unknown int option passed: " + setting_name);
	}
	return value;
}


std::string Config::get_str_setting(std::string setting_name) {
	std::string value;
	if (setting_name == "log_path") {
		value = this->log_path;
	}
	else if (setting_name == "port") {
		value = this->port;
	}
	else {
		throw ConfigException("Unknown string option passed: " + setting_name);
	}
	return value;
}
