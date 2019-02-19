#include <boost/asio.hpp>
#include <string>
#include "request.h"
#include "config_params.h"

using boost::asio::ip::tcp;

class session 
{
  public:
    session(boost::asio::io_service& io_service, config_params& params);
    tcp::socket& socket();
    std::string get_remote_ip();
    void start();

  private:
    void handle_read(const boost::system::error_code& error, 
                        size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::string remote_ip;
    config_params params_;
};

bool validate_http_version(std::string HTTP_version);
request parse_request_line(const char *request_line, size_t request_size, config_params& params);
