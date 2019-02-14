#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include <boost/log/trivial.hpp>

#include "config_var.h"
#include "session.h"
#include "echo_handler.h"
#include "static_handler.h"

session::session(boost::asio::io_service &io_service, config_var &conf)
    : socket_(io_service), conf_(conf)
{
    //just in case check
    //the conf should always be valid when passed to session
    if (conf_.echo_roots.size() == 0)
    {
        std::string default_echo_root = "/echo";
        std::vector<std::string> default_echo_root_vec(1, default_echo_root);
        conf_.echo_roots = default_echo_root_vec;
    }
    else if (conf_.static_roots.size() == 0)
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
    if (HTTP_version.size() < 5 ||                      // version must be at least 8 characters e.g. 'HTTP/x.x'
        !isdigit(HTTP_version[5]) ||                    // edge case e.g. "HTTP/.1"
        HTTP_version[HTTP_version.size() - 1] == '.' || // edge case e.g. "HTTP/1."
        HTTP_version.compare(0, 5, "HTTP/") != 0)
        return false;

    int count = 0;

    for (std::string::size_type i = 5; i < HTTP_version.size(); i++)
    {
        if (HTTP_version[i] == '.') // count the number of .
            count++;
        else if (!isdigit(HTTP_version[i])) //if a non-digit is found in the version number, fail
            return false;
    }

    if (count != 1)
        return false;
    else
        return true;
}

request parse_request_line(const char *request_line, size_t request_size, config_var &conf_)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    std::string method = "";
    std::string path = "";
    std::string HTTP_version = "";
    request::REQUEST_TYPE req_type = request::INVALID;
    bool has_method = false;

    while (strlen(request_line) > 0)
    {
        if (*request_line == ' ')
        {
            request_line++;
            if (method.compare("GET") == 0)
                has_method = true;
            else
                req_type = request::INVALID;
            break;
        }
        method += *request_line;
        request_line++;
    }

    while (strlen(request_line) > 0)
    {
        if (*request_line == ' ')
        {
            request_line++;
            //the path / will always be mapped to the first static_root/index.html
            if (path.compare("/") == 0 && has_method)
            {
                path = conf_.static_roots[0] + "/index.html";
                req_type = request::FILE;
            }
            else
            {
                for (int i = 0; i < conf_.echo_roots.size(); i++)
                {
                    if (path.find(conf_.echo_roots[i]) == 0 && has_method)
                    {
                        req_type = request::REPEAT;
                        break;
                    }
                }
            }
            // if not an echo command, but is still a valid path
            if (req_type == request::INVALID && path.find("/") == 0 && has_method)
            {
                req_type = request::FILE;
            }
            //req_type will remain INVALID if none of these conditions are met
            break;
        }
        path += *request_line;
        request_line++;
    }

    bool has_carriage_line_feed = false;

    while (strlen(request_line) > 1)
    {
        if (*request_line == '\r' && *(request_line + 1) == '\n')
        {
            has_carriage_line_feed = true;
            break;
        }
        HTTP_version += *request_line;
        request_line++;
    }


    //the reqest line must end in a \r\n and the http_version must be valid
    if (!has_carriage_line_feed || validate_http_version(HTTP_version) != true)
        req_type = request::INVALID;

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

        if (req.req_type == request::REPEAT || req.req_type == request::FILE)
        {
            std::string response = "";
            if (req.req_type == request::REPEAT)
            {
                BOOST_LOG_TRIVIAL(info) << "Valid echo request received from " << remote_ip;
                BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;
                // construct echo request handler to handle echo request
                echo_handler a(&socket_, req);
                response = a.get_response(bytes_transferred, data_);
            }
            else if (req.req_type == request::FILE)
            {
                BOOST_LOG_TRIVIAL(info) << "Valid file request received from " << remote_ip;
                BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;

                static_handler a(&socket_, req, conf_.static_roots);
                response = a.get_response(bytes_transferred, data_);
            }

            // convert c++ response string into buffer
            const char *response_buf = response.c_str();
            size_t response_len = response.size();

            // writes response
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(response_buf, response_len),
                                     boost::bind(&session::handle_write, this,
                                                 boost::asio::placeholders::error));
        }

        else
        {
            BOOST_LOG_TRIVIAL(info) << "Invalid request received from " << remote_ip;
            BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;

            std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            const char *response_buf = response.c_str();
            size_t response_len = response.size();

            boost::asio::async_write(socket_,
                                     boost::asio::buffer(response_buf, response_len),
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
