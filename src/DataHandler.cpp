#include "DataHandler.h"

#include <linux/limits.h>
#include <unistd.h>

/*
 * handlers - currently two are planned: static file handler, cgi script handler
 * 1. cgi handler - fires given script and returns it's output
 * 2. static resource handler - should be used by static file handler for caching
 * 		already retrieved files (up to some configured limit, then gracefully rotate the cache)
 * 		this memory should be shared between all workers/static handlers
 */

DataHandler::DataHandler(string client) {
    this->logger = new Logger("DataHandler");
    this->logger->set_postfix(client);
}

DataHandler::~DataHandler() {
    delete(this->logger);
}

bool DataHandler::verify_path(std::string path) {
    bool path_ok = false;

    // verify that the path is not all slashes
    // there's a very nice uncaught exception when we pass '//+' to open()
    for(char c : path) {
        if (c != '/') {
            path_ok = true;
            break;
        }
    }

    return path_ok;
}

bool is(char * mime, std::string type) {
    return (strstr(mime, type.c_str()) != NULL);
}

DataHandler::resource DataHandler::read_resource(std::string path) {
    return DataHandler::read_resource(path, "");
}
DataHandler::resource DataHandler::read_resource(std::string path, std::string cookies) {
    return DataHandler::read_resource(path, cookies, NULL);
}
DataHandler::resource DataHandler::read_resource(std::string path, std::string cookies, DataHandler::resource * data) {
    // prepend cwd() to path

    if (!verify_path(path))
        path = "/index.html";

    std::string cwd = get_working_path();
    path = cwd + path;
    this->logger->debug("Checking resource at: " + path);

    // check mime type of resource
    DataHandler::Exec runner;
    std::string args[2] = { "/usr/bin/file", path };
    DataHandler::resource file_mime = runner.run_command(args);
    char * mime = file_mime.data;
    std::string ext = path.substr(path.length()-3);
    for(char c : ext)
        c = std::toupper(c);

    DataHandler::resource output;
    // now check for known mime types
    if (is(mime, "executable")) {
        // run the script, pass the data
        std::string args[2] = { path, "" }; // TODO: Fix me too!
        DataHandler::resource script_output = runner.run_command(args, data, cookies);
        output.data = script_output.data;
        output.size = script_output.size;
        output.type = "executable";
    }
    // TODO: Move this definition to some more reasonable place
    else if (is(mime, "HTML"))                       { output.type = "text/html; charset=UTF-8";  }
    else if (is(mime, "ASCII") && ext == "CSS")      { output.type = "text/css";                  }
    else if (is(mime, "ERROR") || is(mime, "ASCII")) { output.type = "text/plain; charset=UTF-8"; }
    else if (is(mime, "JPEG"))                       { output.type = "image/jpeg";                }
    else if (is(mime, "PNG"))                        { output.type = "image/png";                 }
    else if (is(mime, "MS Windows icon"))            { output.type = "image/vnd.microsoft.icon";  }

    if (output.type.length() > 0 && output.type != "executable") {
        DataHandler::Static getter;
        DataHandler::resource f = getter.get_file(path);
        output.data = f.data;
        output.size = f.size;
    }
    else if (output.type.length() == 0){
        std::string error_str = "Unsupported mime type: " + std::string(mime);
        // drop 'local' part of path
        size_t pos = 0;
        while ((pos = error_str.find(cwd)) != std::string::npos )
            error_str.erase(pos, cwd.length());

        throw DataHandler::Unsupported(error_str);
    }

    return output;
}

std::string DataHandler::get_working_path() {
   char temp[PATH_MAX];
   return ( getcwd(temp, PATH_MAX) ? std::string( temp ) : std::string("") );
}

DataHandler::resource DataHandler::get_error_file(int error_code, std::string param) {
    // start by reading the error template
    DataHandler::resource output = DataHandler::read_resource("/errors/"+ std::to_string(error_code) +".html");
    // now prepare a place to write the filled template
    long new_size = output.size + param.length() - 3;
    char * data = (char *) malloc((new_size+1) * sizeof(char));
    // and fill it
    sprintf(data, output.data, param.c_str());

    // free old structure
    free(output.data);
    // replace with new pointer
    output.data = data;
    output.size = new_size;

    return output;
}
