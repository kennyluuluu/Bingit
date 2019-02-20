#include "gtest/gtest.h"
#include "status_handler.h"
#include "handler.h"
#include "config_parser.h"

class StatusHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        handler = new status_handler(my_config);
    }

    NginxConfig my_config;
    status_handler* handler;

};

TEST_F(StatusHandlerTest, CreateTest)
{
    handler = (status_handler*)status_handler::create(my_config, "");
    EXPECT_NE(handler, nullptr);
}