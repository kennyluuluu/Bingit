#include <boost/bind.hpp>
#include <iostream>
#include "echo_handler.h"
#include <string>

echo_handler::echo_handler(boost::asio::ip::tcp::socket* socket, request req)
    : handler(socket, req)
{
    
}

echo_handler::~echo_handler() {

}

std::string echo_handler::get_response(size_t bytes_transferred, char* data_) 
{
    // generate status code and content type headers
    std::string status_line = "HTTP/1.1 200 OK\r\n";
    std::string headers = "Content-Type: text/plain\r\n";
    headers += "Content-Length: " + std::to_string(bytes_transferred) + "\r\n";
    // TODO: add more headers

    //must have empty line after headers
    headers += "\r\n";

    // combine status line with headers
    std::string status_and_headers = status_line + headers;

    // convert data buffer into c++ string
    std::string data(data_);

    // combine headers and response into the full echo response
    std::string response = status_and_headers + data;
    
    return response;
}