#include "BasicHTTP.h"

/*
 * BasicHTTP - implements a simple way to create serializable HTTP reponses that can be returned by worker
 */

BasicHTTP::BasicHTTP() {
    this->logger = new Logger("BasicHTTP");
}

BasicHTTP::~BasicHTTP() {
    delete(this->logger);
}

bool is_valid(BasicHTTP::request req, std::string http_ver) {
    // if all tests pass, return true
    return (
        (http_ver == "HTTP/1.0" || http_ver == "HTTP/1.1")
        && req.method == "GET" // || POST
        && req.uri.length() > 0
    );
}

BasicHTTP::request BasicHTTP::parse_request(std::string req_str) {
    request req;
    // From http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5
    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    size_t pos = 0, prev_pos = 0;

    // first read the method
    if ((pos = req_str.find(" ", prev_pos)) != std::string::npos) {
        req.method = req_str.substr(prev_pos, pos-prev_pos);
        prev_pos = pos + 1;
    }

    // the the uri
    if ((pos = req_str.find(" ", prev_pos)) != std::string::npos) {
        req.uri = req_str.substr(prev_pos, pos-prev_pos);
        prev_pos = pos + 1;
    }

    // finally the HTTP version
    std::string http_ver;
    if ((pos = req_str.find("\r\n", prev_pos)) != std::string::npos) {
        http_ver = req_str.substr(prev_pos, pos-prev_pos);
        prev_pos = pos + 1;
    }
    this->logger->info("Method: " + req.method + ", HTTP version string: " + http_ver + ", URI: " + req.uri);

    // now end with checking if it's actually valid
    req.valid  = is_valid(req, http_ver);

    return req;
}

BasicHTTP::response BasicHTTP::render_headers(int code, DataHandler::resource rsrc) {
    response resp;

    // by default no headers are created
    resp.has_headers = false;
    if (rsrc.type != "executable") {
        resp.headers =
            "HTTP/1.1 "+std::to_string(code)+" OK\n"
            "Server: HackTTP\n"
            "Connection: close\n"
            "Content-Type: "+rsrc.type+"\n";
        ;

        if (rsrc.type.find("image/") != std::string::npos) {
            resp.headers += "Accept-Ranges: bytes\n";
            resp.headers += "Content-Length: " + std::to_string(rsrc.size) + "\n";
        }

        // now send the headers
        resp.headers += "\n";
        resp.has_headers = true;
    }

    return resp;
}
