#include <memory>
#include <string>
#include "echo_handler.h"
#include "static_handler.h"
#include "bad_request_handler.h"
#include "handler_manager.h"

handler_manager::handler_manager()
{
}

std::unique_ptr<handler> handler_manager::createByName(const std::string &name,
                                                       const NginxConfig &config,
                                                       const std::string &root_path)
{
	if (name.compare("echo") == 0)
  	{
    	return std::unique_ptr<handler>(echo_handler::create(config, root_path));
  	}
  	else if (name.compare("static") == 0)
  	{
    	return std::unique_ptr<handler>(static_handler::create(config, root_path));
  	}
	else if (name.compare("status") == 0)
	{
		return std::unique_ptr<handler>(status_handler::create(config, root_path));
	}
  	else
  	{
    	return std::unique_ptr<handler>(bad_request_handler::create(config, root_path));
  	}
}
