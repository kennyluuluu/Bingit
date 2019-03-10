#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>
#include <string>
#include "health_handler.h"

health_handler::health_handler()
{
}

handler *health_handler::create(const NginxConfig &config, const std::string &root_path)
{
    return new health_handler();
}

std::unique_ptr<reply> health_handler::HandleRequest(const request &request)
{
    short code = 200;
    std::string mime_type = "text/plain";
    std::unordered_map<std::string, std::string> headers;
    std::string content = "OK";

    headers["Content-Type"] = mime_type;
    headers["Content-Length"] = std::to_string(content.size());

    reply result = reply(request.http_version, code, mime_type, content, headers);
    return std::make_unique<reply>(result);
}