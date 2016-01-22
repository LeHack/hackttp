#include "Config.h"
#include "Logger.h"

#include <fcntl.h>
#include <map>
#include <unistd.h>
#include <vector>

#define CONFIG_SIZE 500

/*
 * config - loads config file on demand, keeps the config in shared RO memory for fast access
 */

std::map<string, string> configMap;

Logger *logger;

Config* Config::getInstance() {
    static Config configSingleton;
    return &configSingleton;
}

Config::Config() {
    queue_size = 10;
    loadConfigFileToMap();

}



void Config::loadConfigFileToMap(){
    configMap.clear();
    std::string configString;
    std::string delimiter;
    std::string configRelevantString;
    std::string configLine;
    std::string key;
    size_t position;

    int configFileDescriptor;
    // TODO: Should take name of config file from constructor param (passed from CmdLine or some default)
    if ((configFileDescriptor = open("./config", O_RDONLY, S_IRUSR)) == -1) {
        throw ConfigException("Cannot open config file!");
    }

    char configFileContents[CONFIG_SIZE];
    if (read(configFileDescriptor, &configFileContents, CONFIG_SIZE) == -1) {
        throw ConfigException("Cannot read config file!");
    }
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

void Config::printMapContents(){
    logger->debug("Currently loaded Config:");
    for (const auto &p : configMap) {
        logger->debug("[" + p.first + "] = " + p.second);
    }
}

Config::~Config() {
    delete(logger);
}

void Config::reload_on_usr1() {
    while (isSigusr1Received) {
        confMutex.lock();
        if (!isSigusr1Received) break;
        Config::loadConfigFileToMap();
        Config::printMapContents();
        isSigusr1Received = false;
        confMutex.unlock();
    }
}

int Config::get_int_setting(std::string setting_name) {
    reload_on_usr1();
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
    reload_on_usr1();
    std::string returnString;
    try{
        returnString = configMap.at(setting_name);
    } catch (out_of_range) {
        throw ConfigException("Unknown string option passed: " + setting_name);
    }
    return returnString;
}
