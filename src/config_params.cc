#include "config_params.h"
#include <string>

config_params::config_params()
{
    port = -1;
}


config_params::config_params(NginxConfig& config)
{
    config.get_handler_paths(&handler_paths);
    std::string key = "port";
    port = config.get_port();
    // port = atoi(config.get_key_value(key).c_str());
}