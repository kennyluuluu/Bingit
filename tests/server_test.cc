#include "gtest/gtest.h"
#include "server.h"
#include "config_parser.h"

class ServerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    const char *good_port_config = "basic_config";
    const char *bad_port_config = "empty_config";
    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
};

// Simple sanity check, io_service is not null and working
// TEST_F(ServerTest, )
// {
//     server my_server(*io_service_ptr, "empty_config");
//     EXPECT_(io_service_ptr, nullptr);
// }

TEST_F(ServerTest, ServicePtrEqualsOriginal)
{
    server good_server(io_service, good_port_config);
    EXPECT_EQ(&good_server.get_io_service(), io_service_ptr);
}

TEST_F(ServerTest, AcceptorNotNullptr)
{
    server good_server(*io_service_ptr, good_port_config);
    EXPECT_NE(good_server.get_acceptor(), nullptr);
}

TEST_F(ServerTest, BadPortAcceptorIsNullptr)
{
    server bad_server(*io_service_ptr, bad_port_config);
    EXPECT_EQ(bad_server.get_acceptor(), nullptr);
}

TEST_F(ServerTest, ServerInitSuccess)
{
    server good_server(*io_service_ptr, good_port_config);
    bool success = good_server.init();
    EXPECT_EQ(success, true);
}

TEST_F(ServerTest, ServerInitFailure)
{
    server bad_server(*io_service_ptr, bad_port_config);
    bool success = bad_server.init();
    EXPECT_EQ(success, false);
}