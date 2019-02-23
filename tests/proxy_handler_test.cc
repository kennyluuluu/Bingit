#include "gtest/gtest.h"
#include "proxy_handler.h"
#include "handler.h"
#include "config_parser.h"
#include "request.h"
class ProxyHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        handler = new proxy_handler(my_config);
    }

    NginxConfig my_config;
    proxy_handler* handler;

};

TEST_F(ProxyHandlerTest, CreateTest)
{
    handler = (proxy_handler*)proxy_handler::create(my_config, "");
    EXPECT_NE(handler, nullptr);
}

TEST_F(ProxyHandlerTest, curlSuccess)
{
    request req;
    std::unique_ptr<reply> response = handler->HandleRequest(req);
    std::string tmp = response.get()->construct_http_response();
    EXPECT_TRUE(tmp.find("200"));
}
