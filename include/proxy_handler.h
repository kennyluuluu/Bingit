#include "handler.h"

class proxy_handler : public handler
{
public:
    proxy_handler(const NginxConfig& config);
    static handler* create(const NginxConfig& config,
                           const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
private:
    std::string getRequestURI(const request& fullRequest);
    std::string location;
    std::string host;

};