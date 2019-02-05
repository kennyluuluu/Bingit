#ifndef REQUEST_HANDLER_
#define REQUEST_HANDLER_

#include <boost/asio.hpp>
#include "request.h"
using boost::asio::ip::tcp;

class request_handler 
{
  public:
    request_handler(boost::asio::ip::tcp::socket* socket, request req);
    virtual ~request_handler();
    virtual std::string get_response(size_t bytes_transferred, char* data_) = 0;
    tcp::socket* get_socket();
    request get_request();

  private:
    request request_;
    tcp::socket* socket_;

};

#endif
