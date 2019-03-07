#include <boost/bind.hpp>
#include <iostream>
#include "echo_handler.h"
#include <string>
#include <boost/log/trivial.hpp>

echo_handler::echo_handler(const NginxConfig &config)
{
}

handler *echo_handler::create(const NginxConfig &config, const std::string &root_path)
{
  return new echo_handler(config);
}

std::unique_ptr<reply> echo_handler::HandleRequest(const request &request)
{
  short code = 200;
  std::string mime_type = "text/plain";
  std::unordered_map<std::string, std::string> headers;
  std::string content = request.original_request;

  BOOST_LOG_TRIVIAL(info) << "Echo content:\n" << content;


  headers["Content-Type"] = mime_type;
  headers["Content-Length"] = std::to_string(content.size());

  reply result = reply(request.http_version, code, mime_type, content, headers);

  return std::make_unique<reply>(result);
}