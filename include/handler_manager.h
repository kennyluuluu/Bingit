#pragma once

#include "handler.h"
#include <memory>

class handler_manager 
{
  public:
    handler_manager();
    unique_ptr<handler> createByName(const string& name,
		                     const NginxConfig& config,
				     const string& root_path);
};

