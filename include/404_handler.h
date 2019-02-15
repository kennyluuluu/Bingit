#include <boost/asio.hpp>
#include "handler.h"

using boost::asio::ip::tcp;

class 404_handler : public handler
{
  public:
    404_handler(const NginxConfig& config);
    static handler* create(const NginxConfig& config,
                            const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);

};