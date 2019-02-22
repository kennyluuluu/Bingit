#include "handler.h"
#include "curl/curl.h"

class proxy_handler : public handler
{
public:
    proxy_handler(const NginxConfig& config);
    static handler* create(const NginxConfig& config,
                           const std::string& root_path);
    std::unique_ptr<reply> HandleRequest(const request& request);
private:
    long getServerResponse(CURL* curl, std::string& serverResponse, const char* url);
    std::string getRequestURI(const request& fullRequest);
    std::string location;
    std::string host;

};