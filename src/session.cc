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
	    if (c == '\r\n' && HTTP_Version.size() > 0)
	    {
		// ECHO BACK REQUEST    
		boost::asio::async_write(socket_,
                	                 boost::asio::buffer(data_, bytes_transferred),
                        	         boost::bind(&session::handle_write, this,
                                           boost::asio::placeholders::error));
		break;
	    }
	    HTTP_Version += c;
	}
	//fprintf(stderr, "HTTP_Version is %s", HTTP_Version.c_str()); DEBUG
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
