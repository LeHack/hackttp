#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include "Common.h"
#include "globals.h"

class Config {
	private:
		int queue_size;
        static std::string configString;
        static std::string delimiter;
        static std::string configRelevantString;
        static std::string configLine;
        static std::string key;
        static size_t position;
        static void loadConfigFileToMap();
	    static void printMapContents();

public:
		Config();
		virtual ~Config();
		static int get_int_setting(std::string);
		static std::string get_str_setting(std::string);
};

class ConfigException: public BaseException {
    public:
        ConfigException(std::string msg = "Unknown config exception") {
            this->reason = msg;
        }
};

#endif /* SRC_CONFIG_H_ */
