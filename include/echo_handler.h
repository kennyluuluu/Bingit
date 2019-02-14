#include <boost/asio.hpp>
#include "request.h"
#include "handler.h"

using boost::asio::ip::tcp;

class echo_handler : public handler
{
  public:
    echo_handler(boost::asio::ip::tcp::socket* socket, request req);
    virtual ~echo_handler();
    std::string get_response(size_t bytes_transferred, char* data_);

};