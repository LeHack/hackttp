#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>
#include "Config.h"
#define CONFIG_SIZE 500
/*
 * config - loads config file on demand, keeps the config in shared RO memory for fast access
 */

std::map<string, string> configMap;

Config::Config() {
	// TODO Auto-generated constructor stub
    int configFileDescriptor = open("./config", O_RDONLY);
    char configFileContents[CONFIG_SIZE];
    read(configFileDescriptor, &configFileContents, CONFIG_SIZE);

    //Obcina nieistotną końcówkę configa
    std::string configString(configFileContents);
    std::string delimiter = ";";
    std::string configRelevantString = configString.substr(0, configString.find(delimiter));

    //Dzieli na linie
    delimiter = "\n";
    size_t pos;
    std::string token;
    vector<string> tokens;
    while ((pos = configRelevantString.find(delimiter)) != std::string::npos) {
        token = configRelevantString.substr(0, pos);
        tokens.push_back(token);
        configRelevantString.erase(0, pos + delimiter.length());
    }

    //Dzieli linie na 2 wartości, które wrzuca do mapy
    std::string tmpString;
    size_t pos2;
    delimiter = "=";
    for (std::vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i){
        tmpString = *i;
        while ((pos2 = tmpString.find(delimiter)) != std::string::npos) {
            token = tmpString.substr(0, pos2);
            tmpString.erase(0, pos2 + delimiter.length());
            configMap.insert(pair<string, string>(token, tmpString));
        }
    }
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
    //Dodać obsługę błędów!
    return configMap.at(setting_name);
}
