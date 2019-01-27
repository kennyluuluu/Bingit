//#include <iostream>
#include <boost/bind.hpp>
#include "session.h"

session::session(boost::asio::io_service &io_service)
    : socket_(io_service)
{
}

tcp::socket &session::socket()
{
    return socket_;
}

void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&session::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

bool check_http_version(std::string HTTP_version)
{

    // version must be longer than 5 characters e.g. 'HTTP/x.x'
    if (HTTP_version.size() < 5 || !isdigit(HTTP_version[5]))
        return false;
    // version cannot end with .
    if (HTTP_version[HTTP_version.size() - 1] == '.')
        return false;
    // check formatting for HTTP
    if (HTTP_version.compare(0, 5, "HTTP/") != 0)
        return false;

    int count = 0;

    for (std::string::size_type i = 5; i < HTTP_version.size(); i++)
    {
        if (HTTP_version[i] == '.') // count the number of .
        {
            count++;
        }
        else if (!isdigit(HTTP_version[i])) //if a non-digit is found in the version number, fail
        {
            return false;
        }
    }

    if (count != 1)
        return false; // only 1 decimal allowed in version

    return true;
}

bool parse_request_line(char *request_line, size_t request_size)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    int iter = 0; // iterator for request_line
    std::string method = "";
    std::string URI = "";
    std::string HTTP_version = "";

    while (iter < request_size)
    {
        const char c = request_line[iter];
        iter++;
        if (c == ' ')
        {
            if (method.compare("GET") == 0)
            {
                break;
            }
            else
                return false;
        }
        method += c;
    }

    while (iter < request_size)
    {
        const char c = request_line[iter];
        iter++;
        if (c == ' ')
        {
            // parse URI format
            if (URI.size() > 0)
            {
                break;
            }
            else
                return false;
        }
        URI += c;
    }

    while (iter < request_size)
    {
        const char c = request_line[iter];
        iter++;
        if (c == '\r' && request_line[iter] == '\n')
        {
            break;
        }
        HTTP_version += c;
    }

    return check_http_version(HTTP_version);

    // no errors detected
}

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error)
    {
        if (parse_request_line(data_, bytes_transferred))
        {
            // send response
            // generate status code and content type headers
            std::string status_line = "HTTP/1.1 200 OK\r\n";
            std::string headers = "Content-Type: text/plain\r\n";
            headers += "Content-Length: " + std::to_string(bytes_transferred) + "\r\n";
            // TODO: add more headers

            //must have empty line after headers
            headers += "\r\n";

            std::string status_and_headers = status_line + headers;

            // attach request as body of response
            char response[status_and_headers.size() + bytes_transferred];
            strncpy(response, status_and_headers.c_str(), status_and_headers.size());
            strncpy(response + status_and_headers.size(), data_, bytes_transferred);

            // ECHO BACK RESPONSE (includes REQUEST)
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(response, bytes_transferred + status_and_headers.size()),
                                     boost::bind(&session::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
        else
        {
            // TODO: handle bad request
        }
    }
    else
    {
        delete this;
    }
}

void session::handle_write(const boost::system::error_code &error)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
