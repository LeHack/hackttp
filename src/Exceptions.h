#ifndef SRC_EXCEPTIONS_H_
#define SRC_EXCEPTIONS_H_

#include <cxxabi.h>
#include <exception>
#include <iostream>
#include <typeinfo>

// Possibly we could also add a stack trace here:
// http://stackoverflow.com/questions/353180/how-do-i-find-the-name-of-the-calling-function
class BaseException: public std::exception {
    protected:
        std::string reason;
    public:
        BaseException(std::string msg = "Unknown exception") {
            this->reason = msg;
        }
        std::string name() const {
            int status;
            return abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
        }
        virtual const char* what() const throw() {
            return (name() + ": " + reason).c_str();
        }
        virtual ~BaseException() throw() {};
};

#endif /* SRC_EXCEPTIONS_H_ */
