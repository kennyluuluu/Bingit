#include <boost/asio.hpp>
#include "config_params.h"
#include "handler_manager.h"
#include <sqlite3.h>

using boost::asio::ip::tcp;

class session;
struct config_params;

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
    bool init_sqlite3();
    void start_accept();
    void set_config_params(const char *file_name);
    void handle_accept(session* new_session, 
                        const boost::system::error_code& error);
    boost::asio::io_service& io_service_;
    tcp::acceptor *acceptor_;
    config_params params_;
    handler_manager manager_;
    sqlite3 *db;
};