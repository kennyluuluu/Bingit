#include <memory>
#include <string>
#include <proxy_handler.h>
#include "echo_handler.h"
#include "static_handler.h"
#include "bad_request_handler.h"
#include "status_handler.h"
#include "meme_handler.h"
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
		handler* new_handler_ptr = status_handler::create(config, root_path);
		// give access to url and code counter
		((status_handler*)new_handler_ptr)->setUrlMap(&url_counter);
		((status_handler*)new_handler_ptr)->setCodeMap(&code_counter);
		((status_handler*)new_handler_ptr)->setPathsMap(handler_paths);
		return std::unique_ptr<handler>(new_handler_ptr);
	}
	else if (name.compare("proxy") == 0)
	{
		return std::unique_ptr<handler>(proxy_handler::create(config, root_path));
	}
	else if (name.compare("meme") == 0)
	{
		return std::unique_ptr<handler>(meme_handler::create(config, root_path));
	}
  	else
  	{
    	return std::unique_ptr<handler>(bad_request_handler::create(config, root_path));
  	}
}

void handler_manager::setHandlerPathsPtr(std::map<std::string, 
                            std::pair<std::string, 
                            NginxConfig>>* given_handler_paths)
{
	handler_paths = given_handler_paths;
}