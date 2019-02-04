#include <boost/asio.hpp>
#include "request.h"
#include "request_handler.h"
using boost::asio::ip::tcp;

class static_request_handler : public request_handler
{
  public:
    static_request_handler(boost::asio::ip::tcp::socket* socket, request req);
    virtual ~static_request_handler();
    void get_response();

};