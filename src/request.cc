//#include <iostream>
#include <boost/bind.hpp>
#include <iostream>
#include "request.h"

request::request()
    : method("GET"), path("/"), http_version("HTTP/1.1"), body("")
      , original_request("GET / HTTP/1.1\r\n\r\n")
{
}

request::request(std::string meth, std::string path, std::string http_version,
                 std::unordered_map<std::string, std::string> headers, std::string body,
                 std::string original_request)
    : path(path), http_version(http_version), headers(headers),
      body(body), original_request(original_request), method(meth)
{
  // method = convert_string_to_request_method(meth);
}

// REQUEST_METHOD convert_string_to_request_method(std::string str)
// {
//   if(str.compare("GET") == 0)         { return GET }
//   else if(str.compare("POST") == 0)   { return POST }
//   else if(str.compare("PUT") == 0)    { return PUT }
//   else                                { return INVALID }
// }


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