#include <boost/bind.hpp>
#include <iostream>
#include "bad_request_handler.h"
#include <string>

bad_request_handler ::bad_request_handler(const NginxConfig &config)
{
  //TODO:really don't see any work to be done?
}

handler* bad_request_handler::create(const NginxConfig &config, const std::string &root_path)
{
  return new bad_request_handler(config);
}

std::unique_ptr<reply> bad_request_handler::HandleRequest(const request &request)
{
  short code = 404;
  std::string mime_type = "text/plain";
  std::unordered_map<std::string, std::string> headers;
  std::string content = "404 Error: File Not Found";

  headers["Content-Type"] = mime_type;
  headers["Content-Length"] = std::to_string(content.size());

  return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}