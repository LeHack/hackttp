#include "BasicHTTP.h"
#include "DataHandler.h"
#include "Worker.h"

#include <sys/socket.h>
#include <unistd.h>

#define HTTP_REQUEST_LENGTH 8192

/*
 * worker - uses an appropriate handler to serve the request
 */

Worker::Worker(string client, int socket_fd) {
    this->logger = new Logger("Worker");
    this->logger->set_postfix(client);
    this->socket_fd = socket_fd;
    this->client = client;
}

Worker::~Worker() {
    // make sure to close the socket when we finish
	close(this->socket_fd);
    delete(this->logger);
}

void Worker::handle_request() {
    char *request = (char *)calloc(HTTP_REQUEST_LENGTH, sizeof(char)); // standard limit of 8kb

    this->logger->debug("Handling request via socket: " + std::to_string(this->socket_fd));

    int return_code = HTTP_OK;
    BasicHTTP *httpHandler = (BasicHTTP *)   new BasicHTTP(this->client);
    BasicHTTP::request req;
    DataHandler *dHandler  = (DataHandler *) new DataHandler(this->client);
    DataHandler::resource data;

    // first read the request
    ssize_t request_size = recv(this->socket_fd, request, HTTP_REQUEST_LENGTH, 0);
    if(request_size < 0) {
        // TODO not sure if necessary
        if(errno == EINTR){
            this->logger->info("Recv() interrupted by signal");
            free(request);
            delete(httpHandler);
            delete(dHandler);
            return;
        }
        return_code = HTTP_BAD_REQUEST;
        data = dHandler->get_error_file(return_code, "Empty request received");
    }
    else {
        std::string req_str = std::string(request);
        if (req_str.length() == 0) {
            this->logger->warn("Empty request received, ignoring");
            free(request);
            delete(httpHandler);
            delete(dHandler);
            return;
        }

        req = httpHandler->parse_request(req_str);

        if (req.valid) {
            try {
                data = dHandler->read_resource(req.uri, req.cookies, &req.data);
            }
            catch (DataHandler::Unsupported &e) {
                this->logger->debug("Unsupported file while handling request: " + std::string(e.what()));
                return_code = HTTP_UNSUP_MEDIA_TYPE;
                data = dHandler->get_error_file(return_code, std::string(e.what()));
            }
            catch (DataHandler::Exec::PermissionDenied &e) {
                this->logger->debug("Permission denied while handling request: " + std::string(e.what()));
                return_code = HTTP_FORBIDDEN;
                data = dHandler->get_error_file(return_code, std::string(e.what()));
            }
            catch (DataHandler::Exception &e) {
                this->logger->debug("Got exception while handling request: " + std::string(e.what()));
                return_code = HTTP_NOT_FOUND;
                data = dHandler->get_error_file(return_code, req.uri.substr(1));
            }
        }
        else {
            this->logger->debug("Could not parse request: " + req_str);
            return_code = HTTP_BAD_REQUEST;
            data = dHandler->get_error_file(return_code, req_str);
        }

        // finally free the allocated memory
        if (req.data.data) {
            free(req.data.data);
        }
    }

    if (req.valid) {
        this->logger->info("\"" + req.method + " " + req.uri + " " + req.http_version + "\" " + to_string(return_code) + " " + to_string(data.size));
    }
    else {
        this->logger->info("Could not parse request, " + to_string(return_code));
    }

    BasicHTTP::response resp = httpHandler->render_headers(return_code, data);
    if (resp.has_headers) {
        send_msg((char *) resp.headers.c_str(), resp.headers.length());
    }

    // then the data
    send_msg(data.data, data.size);

    free(data.data);
    free(request);
    delete(httpHandler);
    delete(dHandler);

    this->logger->debug("Request handling done");

    return;
}

void Worker::send_msg(char * msgc, long size) {
    if (send(this->socket_fd, msgc, size, 0) < 0) {
        char * err = std::strerror(errno);
        throw Worker::Exception("Error while sending response to request: " + std::string(err ? err : "unknown error"));
    }
    return;
}
