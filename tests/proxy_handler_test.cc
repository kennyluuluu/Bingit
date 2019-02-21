#include "proxy_handler.h"
#include "request.h"
#include "config_parser.h"
#include "gtest/gtest.h"

TEST(ProxyHandlerTest, curlSuccess)
{
    NginxConfig my_config;
    request req;
    handler* handler = proxy_handler::create(my_config, "");
    handler -> HandleRequest(req);
}