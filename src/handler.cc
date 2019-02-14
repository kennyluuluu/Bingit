//#include <iostream>
#include <boost/bind.hpp>
#include <iostream>
#include "handler.h"

handler::handler(boost::asio::ip::tcp::socket* socket, request req)
    : socket_(socket), request_(req)
{
    
}

handler::~handler()
{

}

tcp::socket* handler::get_socket()
{
    return this->socket_;
}

request handler::get_request()
{
    return this->request_;
}