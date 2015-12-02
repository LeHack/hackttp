#ifndef SRC_DATAHANDLER_H_
#define SRC_DATAHANDLER_H_

#include "Exceptions.h"
#include "Logger.h"

class DataHandler {
private:
    Logger *logger;
    std::string get_working_path();
    bool verify_path(std::string path);

public:
	DataHandler();
	virtual ~DataHandler();
    char * read_resource(std::string path);
    char * get_error_file(int error_code, std::string param);

    class Exception: public BaseException {
        public:
            Exception(std::string msg = "Unknown data handler exception") {
                this->reason = msg;
            }
    };
    class FileNotFound: public Exception {
        public:
            FileNotFound(std::string msg = "Could not find requested file") {
                this->reason = msg;
            }
    };
};

#endif /* SRC_DATAHANDLER_H_ */
