#include <boost/asio.hpp>
#include "request.h"
using boost::asio::ip::tcp;


class request_handler 
{
  public:
    request_handler(boost::asio::ip::tcp::socket* socket, request req);

  private:
    request request_;
    tcp::socket* socket_;

};
