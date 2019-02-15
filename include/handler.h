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
    handler();
    virtual static handler* create(const NginxConfig& config,
                            const std::string& root_path) = 0;
    std::unique_ptr<reply> HandleRequest(const request& request) = 0;

};

#endif