#ifndef CONFIG_PARAMS_H_
#define CONFIG_PARAMS_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include "config_parser.h"

class config_params
{
  public:
    config_params();
    config_params(NginxConfig& conf);

    short int port;
    std::unordered_map<std::string, std::pair<std::string, NginxConfig>> handler_paths;
};

#endif