#include <boost/asio.hpp>
#include <string>
#include "handler.h"

using boost::asio::ip::tcp;

class meme_handler : public handler
{
  public:
    meme_handler(const NginxConfig &config, const std::string& server_root);
    static handler* create(const NginxConfig& config,
                            const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
  private:
    void handle_new(std::string &path, 
                    short &code, 
                    std::string &mime_type, 
                    std::string &content,
                    const bool &passed_double_check);
    std::string handler_root;
    std::string location;
};