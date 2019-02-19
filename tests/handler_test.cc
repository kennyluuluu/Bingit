#include "gtest/gtest.h"
#include "handler.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class RequestHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        sock = new tcp::socket(io_service);
        // req_handler = new handler(*sock, req);
    }
    void TearDown() override
    {
        delete sock;
        // delete req_handler;
    }

    // handler* req_handler;
    boost::asio::io_service io_service;
    //request req = {"GET", "echo", "HTTP/1.1", request::REPEAT};
    tcp::socket* sock = new tcp::socket(io_service);
};

TEST_F(RequestHandlerTest, BasicTest)
{
    EXPECT_TRUE(true);
}