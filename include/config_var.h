#ifndef CONFIG_VAR_H_
#define CONFIG_VAR_H_

#include <vector>
#include <string>

struct config_var
{
    config_var()
    {
        port = -1;
        echo_roots = std::vector<std::string>(1,"");
        static_roots = std::vector<std::string>(1,"");
    }
    int port;
    std::vector<std::string> echo_roots;
    std::vector<std::string> static_roots;
};

#endif
