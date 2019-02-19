#ifndef REQUEST_HANDLER_H_
#define REQUEST_HANDLER_H_

#include <boost/asio.hpp>
#include "config_parser.h"
#include "request.h"
#include "reply.h"
#include <memory>
using boost::asio::ip::tcp;

class handler 
{
  public:
    virtual std::unique_ptr<reply> HandleRequest(const request& request) = 0;

};

#endif