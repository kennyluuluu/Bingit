//#include <iostream>
#include <boost/bind.hpp>
#include <iostream>
#include "request.h"

request::request()
    : method(GET), path("/"), http_version("HTTP/1.1"), body("")
                                                            original_request("GET / HTTP/1.1\r\n\r\n")
{
}

request::request(REQUEST_METHOD method, std::string path, std::string http_version,
                 unordered_map<std::string, std::string> headers, std::string body,
                 std::string original_request)
    : method(method), path(path), http_version(http_version), headers(headers),
      body(body), original_request(original_request)
{
}

std::string request::get_original_request()
{
  return original_request;
}

std::string request::get_http_version()
{
  return http_version;
}

std::string request::get_path()
{
  return path;
}