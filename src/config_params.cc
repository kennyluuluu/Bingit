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
    std::string root_key = "root";
    server_root = config.get_key_value(root_key);
    
    //if there's no server root, or the server root is the current directory
    //use "./"
    if(server_root.compare("") == 0 || server_root.compare(".") == 0)
    {
        server_root = "./";
    }
    // port = atoi(config.get_key_value(key).c_str());
}