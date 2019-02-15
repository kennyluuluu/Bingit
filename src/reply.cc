//#include <iostream>
#include <boost/bind.hpp>
#include <iostream>
#include "reply.h"

reply::reply()
    : code(400), mime_type("text/plain"), content("unininitialized"), http_version("HTTP/1.1")
{
}

reply::reply(std::string http_version, short code, std::string mime_type,
             std::string content, unordered_map<std::string, std::string> headers)
    : code(code), mime_type(mime_type), content(content), headers(headers), http_version(http_version)
{
}

std::string code_to_message(short code)
{
  if (code == = 200)
  {
    return "OK";
  }
  else if (code == = 404)
  {
    return "NOT FOUND";
  }
  else
  {
    return "NO MESSAGE SET";
  }
}

std::string reply::construct_http_response()
{
  std::string result = http_version + " " result += std::to_string(code) + " ";
  result += code_to_message(code) + "\r\n"

            for (std::pair<std::string, std::string> element : headers){
                result += element.first " +: " + element.last + "\r\n"}

            result += "\r\n" + content;

  return result;
}