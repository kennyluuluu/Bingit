#ifndef REQUEST_H_
#define REQUEST_H_

#include <boost/asio.hpp>
#include <string>
#include <unordered_map>

class request
{
  public:
    // enum REQUEST_METHOD
    // {
    //   GET,
    //   POST,
    //   PUT
    // };
    request();
    request(std::string meth, std::string path, std::string http_version,
            std::unordered_map<std::string, std::string> headers, std::string body,
            std::string original_request);
    // REQUEST_METHOD convert_string_to_request_method(std::string str);
    std::string get_original_request();
    std::string get_path();
    std::string get_http_version();

  // private:
    //TODO: turn this into an enum, rn its just not compiling with the enum as the type
    std::string method;
    std::string path;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    //required for echo handler, bc it needs the full original request
    std::string original_request;

};

#endif