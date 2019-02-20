#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

#include "config_params.h"
#include "session.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "reply.h"
#include "request.h"

session::session(boost::asio::io_service &io_service, config_params &params, handler_manager* manager)
    : socket_(io_service), params_(params), manager_(manager)
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

 request parse_request_line(const char *request_line, size_t request_size, config_params &params_)
 {
     // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     std::string method = "";
     std::string path = "";
     std::string HTTP_version = "";
     std::string body = "";
     std::string original_request(request_line);
     std::unordered_map<std::string, std::string> headers;
     bool has_method = false;

     // empty request used to signal an invalid request
     request invalid_request(method, path, HTTP_version, headers, body, original_request, false);

    while (strlen(request_line) > 0)
    {
        if (*request_line == ' ')
        {
            request_line++;
            if (method.compare("GET") != 0) // only GET requests are supported
                return invalid_request;
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
            request_line += 2; // to get past the newline
            break;
        }
        HTTP_version += *request_line;
        request_line++;
    }

    //the reqest line must end in a \r\n and the http_version must be valid
    if (validate_http_version(HTTP_version) != true)
        return invalid_request;

    while(strlen(request_line) > 1)
    {
        std::string line = "";
        while (strlen(request_line) > 1)
        {
            if (*request_line == '\r' && *(request_line + 1) == '\n')
            {
                has_carriage_line_feed = true;
                request_line += 2;
                break;
            }
            line += *request_line;
            request_line++;
        }
        // check for end of header
        if (line.size() == 0)
        {
            break;
        }
        std::size_t pos = line.find(":");
        if(pos != std::string::npos)
        {
            std::string value(line.substr(pos +1));
            boost::trim(value);
            headers[line.substr(0, pos)] = value; 
        }
    }

    if (!has_carriage_line_feed)
        return invalid_request;
    




    request req(method, path, HTTP_version, headers, body, original_request, true);
    return req;
}

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error && false)
    {
	request request_;
	std::string name = "";
	NginxConfig config;
	request request_path = parse_request_line(data_, bytes_transferred, params_);
	std::unique_ptr<handler> handler_ = manager_->createByName(name,
                                                                config,
                                                                request_path);
    std::unique_ptr<reply> response = handler_->HandleRequest(request_);


    // update url counter
    std::unordered_map<std::string, int>::const_iterator url_iter = manager_->url_counter.find(request_path);
    if (url_iter == manager_->url_counter.end()) 
    {
        manager_->url_counter[request_path] = 0;
    }
    manager_->url_counter[request_path] += 1;

    // update code counter
    std::unordered_map<short, int>::const_iterator code_iter = manager_->code_counter.find(response->code);
    if (code_iter == manager_->code_counter.end())
    {
        manager_->code_counter[response->code] = 0;
    }
    manager_->code_counter[response->code] += 1;

	//parse_request_line(data_, bytes_transferred, params_);

        // if (req.req_type == request::REPEAT || req.req_type == request::FILE)
        // {
        //     std::string response = "";
        //     if (req.req_type == request::REPEAT)
        //     {
        //         BOOST_LOG_TRIVIAL(info) << "Valid echo request received from " << remote_ip;
        //         BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;
        //         // construct echo request handler to handle echo request
        //         echo_handler a(&socket_, req);
        //         response = a.get_response(bytes_transferred, data_);
        //     }
        //     else if (req.req_type == request::FILE)
        //     {
        //         BOOST_LOG_TRIVIAL(info) << "Valid file request received from " << remote_ip;
        //         BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;

        //         static_handler a(&socket_, req, params_.static_roots);
        //         response = a.get_response(bytes_transferred, data_);
        //     }

        //     // convert c++ response string into buffer
        //     const char *response_buf = response.c_str();
        //     size_t response_len = response.size();

        //     // writes response
        //     boost::asio::async_write(socket_,
        //                              boost::asio::buffer(response_buf, response_len),
        //                              boost::bind(&session::handle_write, this,
        //                                          boost::asio::placeholders::error));
        // }

        // else
        // {
        //     BOOST_LOG_TRIVIAL(info) << "Invalid request received from " << remote_ip;
        //     BOOST_LOG_TRIVIAL(info) << "Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version;

        //     std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        //     const char *response_buf = response.c_str();
        //     size_t response_len = response.size();

        //     boost::asio::async_write(socket_,
        //                              boost::asio::buffer(response_buf, response_len),
        //                              boost::bind(&session::handle_write, this,
        //                                          boost::asio::placeholders::error));
        // }
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
