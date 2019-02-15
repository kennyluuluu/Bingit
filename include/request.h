#ifndef REQUEST_H_
#define REQUEST_H_

#include <boost/asio.hpp>
#include <string>
#include <unordered_map>

class request
{
  public:
    request();
    request(REQUEST_METHOD method, std::string path, std::string http_version,
            unordered_map<std::string, std::string> headers, std::string body,
            std::string original_request);
    std::string get_original_request();
    std::string get_path();
    std::string get_http_version();

  private:
    REQUEST_METHOD method;
    std::string path;
    std::string http_version;
    unordered_map<std::string, std::string> headers;
    std::string body;
    //required for echo handler, bc it needs the full original request
    std::string original_request;

    enum REQUEST_METHOD
    {
      GET = "GET",
      POST = "POST",
      PUT = "PUT"
    };
};

#endif