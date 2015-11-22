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
    loadConfigFileToMap();
}
    //TODO: podzielić na prepareVariables() i reloadConfig() - ta druga przyda się później
void Config::loadConfigFileToMap(){
    int configFileDescriptor = open("./config", O_RDONLY);
    char configFileContents[CONFIG_SIZE];
    read(configFileDescriptor, &configFileContents, CONFIG_SIZE);
    close(configFileDescriptor);

    //Removes everything after ; in config file
    configString = std::string(configFileContents);
    delimiter = ";";
    configRelevantString = configString.substr(0, configString.find(delimiter));

    //Splits the file into lines
    vector<string> vectorOfLines;
    delimiter = "\n";
    while ((position = configRelevantString.find(delimiter)) != std::string::npos) {
        configLine = configRelevantString.substr(0, position);
        vectorOfLines.push_back(configLine);
        configRelevantString.erase(0, position + delimiter.length());
    }

    //Splits the lines into key/value pairs
    delimiter = "=";
    for (std::vector<string>::const_iterator i = vectorOfLines.begin(); i != vectorOfLines.end(); ++i){
        configLine = *i;
        while ((position = configLine.find(delimiter)) != std::string::npos) {
            key = configLine.substr(0, position);
            configLine.erase(0, position + delimiter.length());
            configMap.insert(pair<string, string>(key, configLine));
        }
    }
}

Config::~Config() {
	// TODO Auto-generated destructor stub
}


int Config::get_int_setting(std::string setting_name) {
	int returnInt = -1;
    try {
        returnInt = std::stoi(configMap.at(setting_name));
    } catch (out_of_range){
        throw ConfigException("Unknown int option passed: " + setting_name);
	} catch (invalid_argument){
        throw ConfigException("Error parsing config to int, check config file syntax at " + setting_name);
    }
	return returnInt;
}


std::string Config::get_str_setting(std::string setting_name) {
    try{
        returnString = configMap.at(setting_name);
    } catch (out_of_range) {
        throw ConfigException("Unknown string option passed: " + setting_name);
    }
    return returnString;
}
