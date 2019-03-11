#include <boost/asio.hpp>
#include <string>
#include "handler.h"
#include <sqlite3.h>
#include "curl/curl.h"

using boost::asio::ip::tcp;

class meme_handler : public handler
{
  public:
    static int meme_count;
    meme_handler(const NginxConfig &config, const std::string& server_root);
    static handler* create(const NginxConfig& config,
                           const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
    void set_db_ptr(sqlite3 *db);
  private:
    void prepare_new_request(std::string &path, 
                    short &code, 
                    std::string &mime_type, 
                    std::string &content);
    void prepare_edit_request(std::string &path,
                              short &code,
                              std::string &mime_type,
                              std::string &content,
                              const std::string &meme_id);
    void prepare_create_request(const std::string body,
                                short &code,
                                std::string &mime_type,
                                std::string &content);
    void prepare_view_request(const std::string path,
                                short &code,
                                std::string &mime_type,
                                std::string &content);
    void prepare_list_request(const std::string path,
                              short &code,
                              std::string &mime_type,
                              std::string &content);
    void prepare_delete_request(short &code,
                              std::string &mime_type,
                              std::string &content,
                              const std::string &meme_id);
    std::string handler_root;
    std::string location;
    std::string view_path;
    std::string unique_id;
    sqlite3 *db_;
};