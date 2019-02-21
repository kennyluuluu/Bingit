#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <boost/log/trivial.hpp>
#include "status_handler.h"

status_handler::status_handler(const NginxConfig &config)
{
    // TODO:?
}

handler *status_handler::create(const NginxConfig &config, const std::string &root_path)
{
    // TODO:?
    return new status_handler(config);
}

std::unique_ptr<reply> status_handler::HandleRequest(const request &request)
{
    short code = 200;
    std::string mime_type = "text/plain";
    std::unordered_map<std::string, std::string> headers;
    std::string content = "Total Requests: " + std::to_string(url_counter_ptr_->size()) + "\n\n";



    for(std::pair<std::string, int> element : *url_counter_ptr_)
    {
        content += element.first + ":" + std::to_string(element.second) + "\n";
    }
    content += "\n";
    for(std::pair<short, int> element : *code_counter_ptr_)
    {
        content += std::to_string(element.first) + ":" + std::to_string(element.second) + "\n";
    }
    content += "\n";
    for(std::pair<std::string, std::pair<std::string, NginxConfig>> element : *paths_map_ptr_)
    {
        content += (element.second).first + ":" + (element.first) + "\n";
    }



    headers["Content-Type"] = mime_type;
    headers["Content-Length"] = std::to_string(content.size());
    // TODO
    return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}

void status_handler::setUrlMap(std::unordered_map<std::string, int>* url_counter_ptr)
{
    url_counter_ptr_ = url_counter_ptr;
}

void status_handler::setCodeMap(std::unordered_map<short, int>* code_counter_ptr)
{
    code_counter_ptr_ = code_counter_ptr;
}

void status_handler::setPathsMap(std::unordered_map<std::string, std::pair<std::string, NginxConfig>>* paths_map_ptr)
{
    paths_map_ptr_ = paths_map_ptr;
}
