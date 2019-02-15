#ifndef REPLY_H_
#define REPLY_H_

#include <boost/asio.hpp>
#include <string>
#include <unordered_map>

class reply
{
  public:
    reply();
    reply(std::string http_version, short code, std::string mime_type,
          std::string content, unordered_map<std::string, std::string> headers);
    std::string construct_http_response();

  private:
    std::string http_version;
    short code;
    std::string mime_type;
    std::string content;
    unordered_map<std::string, std::string> headers;
};

#endif