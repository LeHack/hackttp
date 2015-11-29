#ifndef SRC_EXCEPTIONS_H_
#define SRC_EXCEPTIONS_H_

#include <exception>
#include <iostream>

// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class BaseException: public std::exception {
    protected:
        std::string reason;
    public:
        BaseException(std::string msg = "Unknown exception") {
            this->reason = msg;
        }
        virtual const char* what() const throw() {
            return reason.c_str();
        }
};

#endif /* SRC_EXCEPTIONS_H_ */
