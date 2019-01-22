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

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred)
{
    if (!error)
    {
	// PARSE Request-Line
	// Request-Line = Method SP Request-URI SP HTTP-Version CRLF
	int i = 0; // iterator for data_
	std::string Method = "";
	std::string Request_URI = "";
	std::string HTTP_Version = "";
	
	while(i < bytes_transferred)	    	
	{
	    const char c = data_[i];
	    i++;
	    if (c == ' ')
	    {
	        // allowed methods
		if(Method.compare("OPTIONS") != 0 &&
		   Method.compare("GET") != 0 &&
		   Method.compare("HEAD") != 0 &&
		   Method.compare("POST") != 0 &&
		   Method.compare("PUT") != 0 &&
		   Method.compare("DELETE") != 0 &&
		   Method.compare("TRACE") != 0 &&
		   Method.compare("CONNECT") != 0 &&
		   Method.compare("GET") != 0)
		    return;	
	        break; // END of Method
	    }
	    Method += c;
	}

	// missing URI, version, and CRLF
	if (i >= bytes_transferred)
	    return;
	
	while (i < bytes_transferred)
	{
	    const char c = data_[i];
	    i++;
	    if (c == ' ')
	    {
	        // invalid syntax
		if (Request_URI.size() == 0)
		{
		    return;
		}
		break; // end of Request_URI
	    }
	    Request_URI += c;
	}
	
	while (i < bytes_transferred)
	{
	    const char c = data_[i];
	    i++;
	    // valid syntax for request-line
        
	    if (c == '\r' && data_[i] == '\n')
	    {
            // check formatting for HTTP
            if (HTTP_Version.compare(0, 5, "HTTP/") != 0) return;
            for (std::string::size_type i = 5; i < HTTP_Version.size(); i++)
            {
                if (!isdigit(HTTP_Version[i]) && HTTP_Version[i] != '.')
                    return;
            }
            
            // send response
            // generate status code and content type headers
            std::string status_line = HTTP_Version + " 200 OK\r\n";
            std::string content_type = "Content-Type: text/plain\r\n";
            std::string status_and_content = status_line + content_type;

            // attach request as body of response
            char response[status_and_content.size() + bytes_transferred];
            strncpy(response, status_and_content.c_str(), status_and_content.size());
            strncpy(response + status_and_content.size(), data_, bytes_transferred);

            // ECHO BACK RESPONSE (includes REQUEST)
            boost::asio::async_write(socket_,
                                    boost::asio::buffer(response, bytes_transferred + status_and_content.size()),
                                    boost::bind(&session::handle_write, this,
                                                boost::asio::placeholders::error));
		break;
	    }
	    HTTP_Version += c;
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
