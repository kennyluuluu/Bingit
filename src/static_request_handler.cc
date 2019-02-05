#include <boost/bind.hpp>
#include <iostream>
#include "static_request_handler.h"
#include <string>
#include <fstream>

static_request_handler::static_request_handler(boost::asio::ip::tcp::socket* socket, request req)
    : request_handler(socket, req)
{
    
}

static_request_handler::~static_request_handler()
{

}

std::string static_request_handler::get_response(size_t bytes_transferred, char* data_) {
    // TODO: Implement static request handling
    std::string path = this->get_request().path;
    std::string response;
    std::ifstream static_file(path.c_str());
    if (static_file.good()) {
        // file found
        std::string status_line = "HTTP/1.1 200 OK\r\n";
        std::string headers = "Content-Type: text/plain\r\n";
        headers += "Content-Length: " + std::to_string(bytes_transferred) + "\r\n";
        headers += "\r\n";
        std::string status_and_headers = status_line + headers;
        response = status_and_headers;
        
        // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << static_file.rdbuf();
        response += buffer.str();
    }
    else {
        // file not found
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }
    static_file.close();
    return response;

}