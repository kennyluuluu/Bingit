#pragma once

#include "handler.h"
#include <memory>
#include <map>
#include <sqlite3.h>

class handler_manager 
{
  public:
    handler_manager();
    std::unique_ptr<handler> createByName(const std::string& name,
                                          const NginxConfig& config,
                                          const std::string& root_path);
    void setHandlerPathsPtr(std::map<std::string, 
                            std::pair<std::string, 
                            NginxConfig>>* given_handler_paths);
    std::map<std::string, int> url_counter;
    std::map<short, int> code_counter;
    std::map<std::string, std::pair<std::string, NginxConfig>>* handler_paths;
    sqlite3 *db_;
};

