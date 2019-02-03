#include <boost/bind.hpp>
#include <iostream>
#include "request_handler.h"

static_request_handler::static_request_handler(boost::asio::ip::tcp::socket* socket, request req)
    : socket_(socket), request_(req)
{
    
}

void send_response() {
    // TODO: Implement static request handling
}