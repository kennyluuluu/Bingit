#include <boost/asio.hpp>
#include <string>
#include "handler.h"
using boost::asio::ip::tcp;

class health_handler : public handler
{
  public:
    health_handler();
    static handler* create(const NginxConfig& config,
                            const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
  private:
};