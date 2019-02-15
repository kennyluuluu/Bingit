#include <boost/bind.hpp>
#include <iostream>
#include "404_handler.h"
#include <string>

404_handler ::404_handler(const NginxConfig &config)
{
  //TODO:really don't see any work to be done?
}

static handler * 404_handler ::create(const NginxConfig &config, const std::string &root_path)
{
  return new 404_handler(config);
}

unique_ptr<reply> 404_handler ::HandleRequest(const request &request)
{
  short code = 404;
  std::string mime_type = "text/plain";
  unordered_map<std::string, std::string> headers;
  std::string content = "404 Error: Page not found"

      headers["Content-Type"] = mime_type;
  headers["Content-Length"] = std::to_string(content.size());

  return make_unique<reply>(http_version, code, mime_type, content, headers);
}