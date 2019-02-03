#include "request_handler.h"

class static_request_handler : public request_handler
{
  public:
    static_request_handler(boost::asio::ip::tcp::socket* socket, request req);

}