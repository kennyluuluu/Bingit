#include <boost/bind.hpp>
#include <iostream>
#include "static_request_handler.h"

static_request_handler::static_request_handler(boost::asio::ip::tcp::socket* socket, request req)
    : request_handler(socket, req)
{
    
}

static_request_handler::~static_request_handler()
{

}

void static_request_handler::get_response() {
    // TODO: Implement static request handling
}