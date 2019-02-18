#include <boost/asio.hpp>
#include <unordered_map>
#include <string>
#include "handler.h"

using boost::asio::ip::tcp;

class status_handler : public handler
{
  public:
    status_handler(const NginxConfig& config);
    static handler* create(const NginxConfig& config,
                            const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
    // session should call setUrlMap() and setCodeMap()
    void setUrlMap(std::unordered_map<std::string, int>* url_counter_ptr);
    void setCodeMap(std::unordered_map<short, int>* code_counter_ptr);
  private:
    std::unordered_map<std::string, int>* url_counter_ptr_;
    std::unordered_map<short, int>* code_counter_ptr_;
};