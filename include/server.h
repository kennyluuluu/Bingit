#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session;

class server
{
  public:
    server(boost::asio::io_service& io_service, const char *file_name);
    ~server();
    bool init();
    tcp::acceptor* get_acceptor();
    boost::asio::io_service& get_io_service();

  private:
    void start_accept();
    short parse_port_number(const char *file_name);
    void handle_accept(session* new_session, 
                        const boost::system::error_code& error);
    boost::asio::io_service& io_service_;
    tcp::acceptor *acceptor_;

};