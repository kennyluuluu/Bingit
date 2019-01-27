#include "gtest/gtest.h"
#include "server.h"

// TEST(ServerTest, CMakeTest)
// {
//     bool success = true;
//     EXPECT_TRUE(success);
// }

class ServerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    short port_num = 8080;
    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
};

// Simple sanity check, io_service is not null and working
TEST_F(ServerTest, SimpleWorkingTest)
{
    server my_server(*io_service_ptr, port_num);
    EXPECT_NE(io_service_ptr, nullptr);
}