#ifndef REQUEST_H_
#define REQUEST_H_

#include <boost/asio.hpp>
#include <string>

struct request 
{
    std::string method;
    std::string path;
    std::string http_version;
    enum REQUEST_TYPE { INVALID, FILE, REPEAT};
    REQUEST_TYPE req_type;
};

#endif