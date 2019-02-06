#include <boost/asio.hpp>
#include "config_var.h"

using boost::asio::ip::tcp;

class session;
struct config_var;

class server
{
  public:
    server(boost::asio::io_service& io_service, const char *file_name);
    ~server();
    bool init();
    bool kill();
    tcp::acceptor* get_acceptor();
    boost::asio::io_service& get_io_service();

  private:
    void init_logging();
    void start_accept();
    config_var get_config_vars(const char *file_name);
    void handle_accept(session* new_session, 
                        const boost::system::error_code& error);
    boost::asio::io_service& io_service_;
    tcp::acceptor *acceptor_;
    config_var conf_;

};