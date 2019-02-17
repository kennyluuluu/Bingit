#include <memory>
#include <string>
#include "echo_handler.h"
#include "static_handler.h"
#include "404_handler.h"

unique_ptr<handler> handler_manager::createByName(const string& name,
		                                  const NginxConfig& config,
						  const string& root_path)
{
    if(name.compare("echo") == 0)
    {
        return echo_handler::create(config, root_path);
    }
    else if(name.compare("static") == 0)
    {
        return static_handler::create(config, root_path);
    }
    else
    {
        return 404_handler::create(config, root_path);
    }
}
