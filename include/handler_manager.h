#pragma once

#include "handler.h"
#include <memory>

class handler_manager 
{
  public:
    handler_manager();
    std::unique_ptr<handler> createByName(const std::string& name,
                                          const NginxConfig& config,
                                          const std::string& root_path);
};

