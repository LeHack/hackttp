#include "BasicHTTP.h"

/*
 * BasicHTTP - implements a simple way to create serializable HTTP reponses that can be returned by worker
 */

BasicHTTP::BasicHTTP(string client) {
    this->logger = new Logger("BasicHTTP");
    this->logger->set_postfix(client);
}

BasicHTTP::~BasicHTTP() {
    delete(this->logger);
}

bool is_valid(BasicHTTP::request req, std::string http_ver) {
    // if all tests pass, return true
    return (
        (http_ver == "HTTP/1.0" || http_ver == "HTTP/1.1")
        && (req.method == "GET" || req.method == "POST")
        && req.uri.length() > 0
    );
}

BasicHTTP::request BasicHTTP::parse_request(std::string req_str) {
    this->logger->debug("Full request: " + req_str);
    request req;
    if (req_str.length() < 3) {
        req.valid = false;
        return req;
    }

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
        prev_pos = pos + 2;
    }
    this->logger->debug("Method: " + req.method + ", HTTP version string: " + http_ver + ", URI: " + req.uri);

    // now end with checking if it's actually valid
    req.valid = is_valid(req, http_ver);

    // find and extract cookies, if available
    if (req.valid) {
        req.cookies = BasicHTTP::fetch_cookies(req_str);

        if (req.method == "GET") {
            handle_get_method(&req);
        }
        if (req.method == "POST") {
            handle_post_method(&req, req_str);
        }
    }

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

std::string BasicHTTP::fetch_cookies(std::string req_str) {
    size_t pos = 0, eol = 0;

    std::string cookies = "";
    // to extract the POST params we first need to find it and it's length
    if ((pos = req_str.find("Cookie: ")) != std::string::npos) {
        if ((eol = req_str.find("\r\n", pos)) != std::string::npos) {
            cookies = req_str.substr(pos + 8, eol - pos - 8);
            this->logger->debug("Cookies: " + cookies);
        }
    }

    return cookies;
}

void BasicHTTP::handle_get_method(BasicHTTP::request * req) {
    size_t pos = 0;

    req->data.type = req->method;
    if ((pos = req->uri.find("?", 0)) != std::string::npos) {
        req->data.size = req->uri.length() - pos + 1;
        req->data.data = (char *) malloc((req->data.size+1) * sizeof(char));
        std::string params = req->uri.substr(pos+1);
        for (int i = 0; i < req->data.size; i++)
            req->data.data[i] = params[i];
        req->uri  = req->uri.substr(0, pos);
    }
    else {
        req->data.size = 0;
        req->data.data = (char *) malloc((req->data.size+1) * sizeof(char));
        req->data.data[0] = '\0';
    }
}

void BasicHTTP::handle_post_method(BasicHTTP::request * req, std::string req_str) {
    size_t pos = 0, prev_pos = 0;

    // to extract the POST params we first need to find it and it's length
    if ((pos = req_str.find("Content-Length:", prev_pos)) != std::string::npos) {
        size_t eol = 0;
        int content_length = 0;
        if ((eol = req_str.find("\r\n", pos)) != std::string::npos) {
            content_length = std::stoi(req_str.substr(pos + 16, eol - pos + 16));
            this->logger->debug("Content-Length: " + std::to_string(content_length));
        }
        // don't read anything, if content data was 0, or wasn't defined at all
        if (content_length > 0 && (pos = req_str.find("\r\n\r\n", eol)) != std::string::npos) {
            req->data.type = req->method;
            req->data.size = content_length;
            req->data.data = (char *) malloc((req->data.size + 1)* sizeof(char));
            std::string data = req_str.substr(pos+4);
            for (int i = 0; i < req->data.size; i++)
                req->data.data[i] = data[i];
            req->data.data[req->data.size] = '\0';
        }
    }
}
