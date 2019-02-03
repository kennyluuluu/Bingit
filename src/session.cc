#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include "session.h"
#include "request_handler.h"

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

bool validate_http_version(std::string HTTP_version)
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
    {
        return false;
    }

    return true;
}

request parse_request_line(const char *request_line, size_t request_size)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    int iter = 0; // iterator for request_line
    std::string method = "";
    std::string path = "";
    std::string HTTP_version = "";
    request::REQUEST_TYPE req_type = request::INVALID;
    bool has_method = false;

    while (iter < request_size)
    {
        const char c = request_line[iter];
        iter++;
        if (c == ' ')
        {
            if (method.compare("GET") == 0)
            {
                has_method = true;
                break;
            }
            else
            {
                req_type = request::INVALID;
                break;
            }
        }
        method += c;
    }

    while (iter < request_size)
    {
        const char c = request_line[iter];
        iter++;
        if (c == ' ')
        {
            if (path.size() > 0 && has_method == true)
            {
                if (path.compare(0, 5, "echo") == 0)
                {
                    req_type = request::REPEAT;
                }
                // TODO: add /static to all paths at some point in time
                else if (path.compare(0, 8, "/") == 0)
                {
                    req_type = request::FILE;
                }
                else
                {
                    req_type = request::INVALID;
                }
                break;
            }
            else
            {
                req_type = request::INVALID;
                break;
            }
        }
        path += c;
    }

    while (1)
    {
        if (iter >= request_size) // \r\n were never found
        {
            req_type = request::INVALID;
            break;
        }
        const char c = request_line[iter];
        iter++;
        if (c == '\r' && request_line[iter] == '\n')
        {
            break;
        }
        HTTP_version += c;
    }

    //if the request is not already invalid, check if the HTTP version is valid
    if (req_type != request::INVALID && !validate_http_version(HTTP_version))
    {
        req_type = request::INVALID;
    }
    request req = {method, path, HTTP_version, req_type};
    return req;
}

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error)
    {
        request req;
        req = parse_request_line(data_, bytes_transferred);

        if (req.req_type == request::FILE || req.req_type == request::REPEAT)
        {
            request_handler a(&socket_, req);

            std::cout << "correct request" << std::endl;
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
            std::cout << "Invalid request received:" << std::endl;
            std::cout << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version << std::endl;
            std::string status_line = "HTTP/1.1 400 Bad Request\r\n\r\n";
            char response[status_line.size()];
            strncpy(response, status_line.c_str(), status_line.size());

            boost::asio::async_write(socket_,
                                    boost::asio::buffer(response, status_line.size()),
                                    boost::bind(&session::handle_write, this,
                                                boost::asio::placeholders::error));
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
