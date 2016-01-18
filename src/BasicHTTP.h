#ifndef SRC_BASICHTTP_H_
#define SRC_BASICHTTP_H_

#include "DataHandler.h"
#include "Logger.h"

// Currently returned HTTP CODES
#define HTTP_OK                 200
#define HTTP_BAD_REQUEST        400
#define HTTP_FORBIDDEN          403
#define HTTP_NOT_FOUND          404
#define HTTP_UNSUP_MEDIA_TYPE   415
#define HTTP_INTERNAL_SRV_ERROR 500
#define HTTP_NOT_IMPLEMENTED    501

class BasicHTTP {
public:
    struct request {
        string method;
        string uri;
        string cookies;
        DataHandler::resource data;
        bool valid;
        string http_version;
    };
    struct response {
        string headers;
        bool has_headers;
    };

private:
    Logger *logger;
    std::string fetch_cookies(string req_str);
    void handle_get_method(request * req);
    void handle_post_method(BasicHTTP::request * req, string req_str);

public:
    BasicHTTP(string client);
	virtual ~BasicHTTP();
	request  parse_request(string req_str);
	response render_headers(int code, DataHandler::resource rsrc);

    class Exception: public BaseException {
        public:
            Exception(string msg = "Unknown basic http exception") {
                this->reason = msg;
            }
    };
};

#endif /* SRC_BASICHTTP_H_ */
