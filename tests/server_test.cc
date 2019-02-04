#include "gtest/gtest.h"
#include "server.h"
#include "config_parser.h"

class ServerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        serv = new server(io_service, good_port_config);
    }

    void TearDown() override
    {
        delete serv;
        serv = nullptr;
    }

    const char *good_port_config = "basic_config";
    const char *bad_port_config = "non_number_port_config";
    const char *empty_config = "empty_config";
    const char *non_existent_config = "this_doesnt_exist";
    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
    server* serv;
};

//check if server is using the same io service that was passed in
TEST_F(ServerTest, ServicePtrEqualsOriginal)
{
    EXPECT_EQ(&(serv->get_io_service()), io_service_ptr);
}

TEST_F(ServerTest, AcceptorNotNullptr)
{
    EXPECT_NE(serv->get_acceptor(), nullptr);
}

TEST_F(ServerTest, BadPortAcceptorIsNullptr)
{
    server bad_server(*io_service_ptr, empty_config);
    EXPECT_EQ(bad_server.get_acceptor(), nullptr);
}

TEST_F(ServerTest, NonExistentConfigFails)
{
    server bad_server(*io_service_ptr, non_existent_config);
    EXPECT_EQ(bad_server.get_acceptor(), nullptr);
}


TEST_F(ServerTest, BadConfigCantInit)
{
    server bad_server(*io_service_ptr, empty_config);
    bool success = bad_server.init();
    EXPECT_FALSE(success);
}

TEST_F(ServerTest, BadConfigCantKill)
{
    server bad_server(*io_service_ptr, empty_config);
    bool success = bad_server.kill();
    EXPECT_FALSE(success);
}

TEST_F(ServerTest, NonNumberPortConfig)
{
    const char* bad_conf = bad_port_config;
    server bad_server(*io_service_ptr, bad_conf);
    EXPECT_EQ(bad_server.get_acceptor(), nullptr);
}

TEST_F(ServerTest, ServerInitAndKillSuccess)
{
    bool success = serv->init();
    bool success2 = serv->kill();
    EXPECT_TRUE(success);
    EXPECT_TRUE(success2);
}