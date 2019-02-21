#include "proxy_handler.h"
#include "curl/curl.h"

proxy_handler::proxy_handler(const NginxConfig &config)
{
    location = config.get_key_value("location");
    host = config.get_key_value("host");
}

handler *proxy_handler::create(const NginxConfig &config, const std::string &root_path)
{
    return new proxy_handler(config);
}


size_t curlCallback(void *contents, size_t size, size_t nmemb, std::string *serverResponse)
{
    size_t newLength = size*nmemb;
    try
    {
        serverResponse->append((char*)contents, newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

std::string proxy_handler::getRequestURI(const request& fullRequest)
{
    return fullRequest.path.substr(location.size());
}

std::unique_ptr<reply> proxy_handler::HandleRequest(const request &request)
{
    auto curl = curl_easy_init();
    std::string serverResponse;
    curl_easy_setopt(curl, CURLOPT_URL, (host + getRequestURI(request)).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &serverResponse);
    curl_easy_perform(curl);

    std::unordered_map<std::string, std::string> headers;
    headers["Content-Length"] = std::to_string(serverResponse.size());
    headers["Content-Type"] = "text/html";
    reply result = reply(request.http_version, 200, "text/html", serverResponse, headers);
    return std::make_unique<reply>(result);
}