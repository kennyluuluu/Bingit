#pragma once

#include "handler.h"
#include <memory>
#include <unordered_map>


class handler_manager 
{
  public:
    handler_manager();
    std::unique_ptr<handler> createByName(const std::string& name,
                                          const NginxConfig& config,
                                          const std::string& root_path);
    void setHandlerPathsPtr(std::unordered_map<std::string, 
                            std::pair<std::string, 
                            NginxConfig>>* given_handler_paths);
    std::unordered_map<std::string, int> url_counter;
    std::unordered_map<short, int> code_counter;
    std::unordered_map<std::string, std::pair<std::string, NginxConfig>>* handler_paths;

};

