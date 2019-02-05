#include <boost/bind.hpp>
#include <iostream>
#include "static_request_handler.h"
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>

static_request_handler::static_request_handler(boost::asio::ip::tcp::socket* socket, request req)
    : request_handler(socket, req)
{
    
}

static_request_handler::~static_request_handler()
{

}

std::string static_request_handler::get_response(size_t bytes_transferred, char* data_) {
    std::string path = request_.path;

    // https://thispointer.com/c-how-to-extract-file-extension-from-a-path-string-using-boost-c17-filesystem-library/
    boost::filesystem::path path_object(path);
    std::string extension = "";
    if (path_object.has_extension())
    {
        extension = path_object.extension().string();
    }
    // path does not have an extension
    
    std::string response = "";
    std::ifstream static_file(path.c_str());
    if (static_file.good()) {
        // file found
        std::string status_line = "HTTP/1.1 200 OK\r\n";
	std::string headers = "";
	if (extension.compare(".txt") == 0)
	    headers += "Content-Type: text/plain\r\n";
	else if (extension.compare(".html") == 0)
	    headers += "Content-Type: text/html\r\n";
	else if (extension.compare(".jpeg") == 0)
	    headers += "Content-Type: image/jpeg\r\n";
	else if (extension.compare(".png") == 0)
	    headers += "Content-Type: image/png\r\n";
	else if (extension.compare(".zip") == 0)
	    headers += "Content-Type: application/zip\r\n";
	else
            headers += "Content-Type: text/plain\r\n";
        headers += "Content-Length: " + std::to_string(bytes_transferred) + "\r\n";
        headers += "\r\n";
        response = status_line + headers;
        
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
