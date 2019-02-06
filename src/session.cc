#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include <boost/log/trivial.hpp>

#include "config_var.h"
#include "session.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"

session::session(boost::asio::io_service &io_service, config_var& conf)
    : socket_(io_service), conf_(conf)
{
    //just in case check
    //the conf should always be valid when passed to session
    if(conf_.echo_roots.size() == 0)
    {
        std::string default_echo_root = "/echo";
        std::vector<std::string> default_echo_root_vec(1, default_echo_root);
        conf_.echo_roots = default_echo_root_vec;
    }
    else if(conf_.static_roots.size() == 0)
    {
        std::string default_static_root = "/static";
        std::vector<std::string> default_static_root_vec(1, default_static_root);
        conf_.static_roots = default_static_root_vec;
    }
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
    remote_ip = get_remote_ip();   
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

request parse_request_line(const char *request_line, size_t request_size, config_var& conf_)
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
                //the path / will always be mapped to the first static_root/index.html
                if(path.compare("/") == 0)
                {
                    path = conf_.static_roots[0] + "/index.html";
                    req_type = request::FILE;
                }
                else 
                {
                    for(int i = 0; i < conf_.echo_roots.size(); i++)
                    {
                        if(path.find(conf_.echo_roots[i]) == 0)
                        {
                            req_type = request::REPEAT;
                            break;
                        }
                    }

                    //if it wasn't an echo command
                    if(req_type == request::INVALID)
                    {
                        //all other paths are valid, but those not starting with static_roots
                        //will always return 404 in static_request_handler

                        if(path.find("/") == 0) //if the path starts with /, then its a valid path
                        {
                            req_type = request::FILE;
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
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
        req = parse_request_line(data_, bytes_transferred, conf_);

        if (req.req_type == request::FILE || req.req_type == request::REPEAT)
        {
            BOOST_LOG_TRIVIAL(info) << "Correctly formatted request received from: " << remote_ip;
            BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;
            if (req.req_type == request::REPEAT)
            {
                // construct echo request handler to handle echo request
                echo_request_handler a(&socket_, req);
                std::string response = a.get_response(bytes_transferred, data_);

                // convert c++ response string into buffer
                const char* response_buf = response.c_str();
                size_t response_len = response.size();

                // ECHO BACK RESPONSE (includes REQUEST)
                boost::asio::async_write(socket_,
                                boost::asio::buffer(response_buf, response_len),
                                boost::bind(&session::handle_write, this,
                                            boost::asio::placeholders::error));
            }
            else {
                static_request_handler a(&socket_, req, conf_.static_roots);
                std::string response = a.get_response(bytes_transferred, data_);

                // convert c++ response string into buffer
                const char* response_buf = response.c_str();
                size_t response_len = response.size();

                // ECHO BACK RESPONSE (includes REQUEST)
                boost::asio::async_write(socket_,
                                boost::asio::buffer(response_buf, response_len),
                                boost::bind(&session::handle_write, this,
                                            boost::asio::placeholders::error));

            }
        }
        else
        {
            // TODO: handle bad request
            BOOST_LOG_TRIVIAL(info) << "Incorrect request received from " << remote_ip;
            BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;
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

std::string session::get_remote_ip()
{
    // https://stackoverflow.com/questions/601763/how-to-get-ip-address-of-boostasioiptcpsocket
    boost::asio::ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
    boost::asio::ip::address remote_ad = remote_ep.address();
    return remote_ad.to_string();
}