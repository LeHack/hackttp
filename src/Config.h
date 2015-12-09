#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_
#include <iostream>
#include <exception>
#include <vector>
#include <signal.h>


class Config {
	private:
		// to be replaced with some dynamic map struct
		int queue_size = 10;
        std::string configString;
        std::string delimiter;
        std::string configRelevantString;
        std::string configLine;
        std::string key;
        size_t position;
        void loadConfigFileToMap();

public:
		Config();
		virtual ~Config();
		static int get_int_setting(std::string);
		static std::string get_str_setting(std::string);
};

using namespace std;
// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class ConfigException: public exception {
	std::string reason;

	public:
		ConfigException(std::string msg = "Unknown exception") {
			reason = msg;
		}
		virtual const char* what() const throw() {
			return reason.c_str();
		}
};

#endif /* SRC_CONFIG_H_ */
