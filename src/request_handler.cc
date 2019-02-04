//#include <iostream>
#include <boost/bind.hpp>
#include <iostream>
#include "request_handler.h"

request_handler::request_handler(boost::asio::ip::tcp::socket* socket, request req)
    : socket_(socket), request_(req)
{
    
}

request_handler::~request_handler()
{

}

tcp::socket* request_handler::get_socket()
{
    return this->socket_;
}

request request_handler::get_request()
{
    return this->request_;
}