#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <mutex>
#include "Common.h"
#include "globals.h"
#include "Logger.h"

class Config {
	private:
		int queue_size;
        std::string configString;
        std::string delimiter;
        std::string configRelevantString;
        std::string configLine;
        std::string key;
        size_t position;
        void loadConfigFileToMap();
	    void printMapContents();
        Config();
        static Config* configSingleton;
        static bool isConfigLoggerCreated;
        std::mutex confMutex;


public:
        static Config* getInstance();
		virtual ~Config();
		int get_int_setting(std::string);
		std::string get_str_setting(std::string);
};

class ConfigException: public BaseException {
    public:
        ConfigException(std::string msg = "Unknown config exception") {
            this->reason = msg;
        }
};

#endif /* SRC_CONFIG_H_ */
