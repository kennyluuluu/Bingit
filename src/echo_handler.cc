#include <boost/bind.hpp>
#include <iostream>
#include "echo_handler.h"
#include <string>

echo_handler::echo_handler(const NginxConfig &config)
{
  //TODO:really don't see any work to be done?
}

static handler *echo_handler::create(const NginxConfig &config, const std::string &root_path)
{
  return new echo_handler(config);
}

unique_ptr<reply> echo_handler::HandleRequest(const request &request)
{
  short code = 200;
  std::string mime_type = "text/plain";
  unordered_map<std::string, std::string> headers;
  std::string content = request.get_original_request();

  headers["Content-Type"] = mime_type;
  headers["Content-Length"] = std::to_string(content.size());

  return make_unique<reply>(http_version, code, mime_type, content, headers);
}