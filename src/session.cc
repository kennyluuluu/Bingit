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

request parse_request_line(const char *request_line, std::string ip)
{
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
    std::string method = "";
    std::string path = "";
    std::string HTTP_version = "";
    std::string body = "";
    std::string original_request(request_line);
    std::unordered_map<std::string, std::string> headers;

    // empty request used to signal an invalid request
    request invalid_request(method, path, "HTTP/1.1", headers, body, original_request, false);

    while (strlen(request_line) > 0)
    {
        if (*request_line == ' ')
        {
            request_line++;
            if (method.compare("GET") != 0) // only GET requests are supported
            {
                BOOST_LOG_TRIVIAL(info) << "HTTP Request with unsupported method: \"" << method << "\" received from  " << ip;
                return invalid_request;
            }
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
    
    if(path.size() == 0)
    {
        BOOST_LOG_TRIVIAL(info) << "HTTP Request with empty path received from " << ip;
        return invalid_request;
    }

    if(path.size() == 0)
    {
        return invalid_request;
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

    if (!has_carriage_line_feed)
    {
        BOOST_LOG_TRIVIAL(info) << "HTTP Request without CRLF after HTTP version received from " << ip;
	return invalid_request;
    }

    //the request line must end in a \r\n and the http_version must be valid
    if (validate_http_version(HTTP_version) != true)
    {
        BOOST_LOG_TRIVIAL(info) << "HTTP Request with invalid HTTP version: \"" << HTTP_version << "\" received from " << ip;
        return invalid_request;
    }

    while(strlen(request_line) > 1)
    {
        std::string line = "";
        while (strlen(request_line) > 1)
        {
            if (*request_line == '\r' && *(request_line + 1) == '\n')
            {
                request_line += 2;
                break;
            }
            line += *request_line;
            request_line++;
        }
        // check for end of header
        if (line.size() == 0)
        {
            body = request_line;
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

    request req(method, path, HTTP_version, headers, body, original_request, true);
    return req;
}

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error)
    {
        std::string name = "";
        NginxConfig config;
        request req = parse_request_line(data_, remote_ip);
        BOOST_LOG_TRIVIAL(info) << "REQUEST RECEIVED: Method: " << req.method << " Path: " << req.path << " HTTP Version: " << req.http_version << " Is_Valid: " << req.is_valid();

        //only check if the request matches a handler if the
        //request is valid in the first place
        if(req.is_valid())
        {
            for( const auto &ele : params_.handler_paths)
            {
                //if the path of the request contains the location
                //of some handler defined in the config

                //behavior
                //e.g. req.path = "/static2/index.html"
                //would collide with some handler with location "/static2",
		//but not some handler with location "/static"
                if(req.path.compare(ele.first) == 0 ||
                   req.path.find(ele.first + "/") == 0)
                {
                    config = ele.second.second;
                    name = ele.second.first;
                    BOOST_LOG_TRIVIAL(info) << "Found appropriate handler: " << ele.second.first;
                    break;
                }
            }
        }

        if(name.compare("") == 0)
        {
            BOOST_LOG_TRIVIAL(info) << "No valid handler found for path, using bad request handler instead";
        }
    
        std::unique_ptr<handler> handler_ = manager_->createByName(name,
                                                                    config,
                                                                    params_.server_root);
        std::unique_ptr<reply> response = handler_->HandleRequest(req);

        // update url counter
        std::unordered_map<std::string, int>::const_iterator url_iter = manager_->url_counter.find(req.path);
        if (url_iter == manager_->url_counter.end()) 
        {
            manager_->url_counter[req.path] = 0;
        }
        manager_->url_counter[req.path] += 1;

        // update code counter
        std::unordered_map<short, int>::const_iterator code_iter = manager_->code_counter.find(response->code);
        if (code_iter == manager_->code_counter.end())
        {
            manager_->code_counter[response->code] = 0;
        }
        manager_->code_counter[response->code] += 1;

        std::string http_response = response.get()->construct_http_response();

        // convert c++ response string into buffer
        const char *http_response_buf = http_response.c_str();
        size_t response_len = http_response.size();
        BOOST_LOG_TRIVIAL(info) << "Sending " << response.get()->code << " response";

        // writes response
        boost::asio::async_write(socket_,
                                    boost::asio::buffer(http_response_buf, response_len),
                                    boost::bind(&session::handle_write, this,
                                            boost::asio::placeholders::error));
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
