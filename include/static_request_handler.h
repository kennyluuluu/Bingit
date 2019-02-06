#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "request.h"
#include "request_handler.h"
using boost::asio::ip::tcp;

class static_request_handler : public request_handler
{
  public:
    static_request_handler(boost::asio::ip::tcp::socket* socket, request req, std::vector<std::string> roots);
    virtual ~static_request_handler();
    std::string get_response(size_t bytes_transferred, char* data_);
    std::vector<std::string> roots_;

};