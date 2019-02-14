#include <boost/bind.hpp>
#include <iostream>
#include "static_handler.h"
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

static_handler::static_handler(boost::asio::ip::tcp::socket *socket, request req, std::vector<std::string> roots)
    : handler(socket, req), roots_(roots)
{
}

static_handler::~static_handler()
{
}

std::string static_handler::get_response(size_t bytes_transferred, char *data_)
{
    std::string path = request_.path;
    std::string response = "";

    bool found = false;
    for(int i =0; i < roots_.size(); i++)
    {
        //need to add "/" here, bc roots like /static and /static2 would collide
        //bc /static is a substring of /static2, so all /static#'s would succeed
        if(path.find(roots_[i] + "/") == 0)
        {
            found = true;
            break;
        }
    }

    if(!found)
    {
        BOOST_LOG_TRIVIAL(info) << "Path does not use any of the static roots, returning 401";
        
        std::string status_line = "HTTP/1.1 401 Unauthorized\r\n";
        std::string headers = "Content-Type: text/plain\r\n";
        //27 is length of '404 Error: Page not found \r\n'
        headers += "Content-Length: 27\r\n";
        headers += "\r\n";
        headers += "401 Error: Unauthorized path provided\r\n";
        response = status_line + headers;
        
        return response;
    }

    //make the path point to the current directory
    path = "." + path;

    // https://thispointer.com/c-how-to-extract-file-extension-from-a-path-string-using-boost-c17-filesystem-library/
    boost::filesystem::path path_object(path);
    std::string extension = "";
    if (path_object.has_extension())
    {
        extension = path_object.extension().string();
    }

    std::ifstream static_file(path.c_str());

    if (static_file.good())
    {
        // file found
        std::string status_line = "HTTP/1.1 200 OK\r\n";
        std::string headers = "";
        
        std::string content_type = "";
        if (extension.compare(".txt") == 0)         { content_type = "text/plain"; }
        else if (extension.compare(".html") == 0)   { content_type = "text/html"; }
        else if (extension.compare(".jpeg") == 0 || 
                 extension.compare(".jpg"))         { content_type = "image/jpeg"; }
        else if (extension.compare(".png") == 0)    { content_type = "image/png"; }
        else if (extension.compare(".zip") == 0)    { content_type = "application/zip"; }
        else                                        { content_type += "text/plain"; }
        headers += "Content-Type: " + content_type + "\r\n"; 

        // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << static_file.rdbuf();

        //add content length of file
        headers += "Content-Length: " + std::to_string(buffer.str().size()) + "\r\n";
        headers += "\r\n";
        response = status_line + headers;

        response += buffer.str();
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Path does not exist, responding with 404";

        std::string status_line = "HTTP/1.1 404 Not Found\r\n";
        std::string headers = "Content-Type: text/plain\r\n";
        //27 is length of '404 Error: Page not found \r\n'
        headers += "Content-Length: 27\r\n";
        headers += "\r\n";
        headers += "404 Error: Page not found\r\n";
        response = status_line + headers;
    }
    static_file.close();
    return response;
}
