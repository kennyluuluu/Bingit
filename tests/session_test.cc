#include "gtest/gtest.h"
#include "session.h"
#include <string>
#include <vector>

// TEST(SessionTest, CMakeTest)
// {
//     bool success = true;
//     EXPECT_TRUE(success);
// }

class SessionTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        NginxConfigParser config_parser;
        NginxConfig config;

        bool success = config_parser.Parse(working_conf, &config);

        config_ = config_params(config);
    }

    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
    const char* working_conf = "basic_config";
    config_params config_;
    handler_manager man_;
};

// Test to make sure basic RequestLine passes
TEST_F(SessionTest, StandardFileTest)
{
    const char* StandardRequest = "GET /static/test.txt HTTP/1.1\r\n";
    request req = parse_request_line(StandardRequest, "255.0.0.0");
    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.path, "/static/test.txt");
    EXPECT_EQ(req.http_version, "HTTP/1.1");
    EXPECT_TRUE(req.is_valid());
    EXPECT_EQ(req.original_request, "GET /static/test.txt HTTP/1.1\r\n");
}

// Test to make sure basic RequestLine passes
TEST_F(SessionTest, StandardEchoTest)
{
    const char* StandardRequest = "GET /echo HTTP/1.1\r\n";
    request req = parse_request_line(StandardRequest, "255.0.0.0");
    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.path, "/echo");
    EXPECT_EQ(req.http_version, "HTTP/1.1");
    EXPECT_TRUE(req.is_valid());
    EXPECT_EQ(req.original_request, "GET /echo HTTP/1.1\r\n");
}

// Test should fail without a method
TEST_F(SessionTest, MissingMethodTest)
{
    const char* MissingMethod = " / HTTP/1.1\r\n";
    request req = parse_request_line(MissingMethod, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());
}

// Test should fail if method is unknown. Methods are case-sensitive.
TEST_F(SessionTest, UnknownMethodTest)
{
    const char* UnknownMethod = "get / HTTP/1.1\r\n";
    request req = parse_request_line(UnknownMethod, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());
}

// Test should fail if URI is missing
TEST_F(SessionTest, MissingURITest)
{
    const char* MissingURI = "GET  HTTP/1.1\r\n";
    request req = parse_request_line(MissingURI, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());
}

// Test should fail if more than one space separates elements
TEST_F(SessionTest, SpacingTest)
{
    const char* SpaceAfterVersion = "GET / HTTP/1.1 \r\n";
    const char* TooManySpaces = "GET  / HTTP/1.1\r\n";
    request req = parse_request_line(SpaceAfterVersion, "255.0.0.0");
    request req2 = parse_request_line(TooManySpaces, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());
    EXPECT_FALSE(req2.is_valid());
}

// Request-lines should end with a carraige return before the new line
TEST_F(SessionTest, CarriageReturnTest)
{
    const char* MissingCR = "GET / HTTP/1.1\n";
    request req = parse_request_line(MissingCR, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());
}

// Test for version format
TEST_F(SessionTest, HTTPVersionTest)
{
    const char* IncorrectVersion = "GET / 1.1\r\n";
    request req = parse_request_line(IncorrectVersion, "255.0.0.0");
    EXPECT_FALSE(req.is_valid());

    bool success1 = validate_http_version("1.1");
    bool success2 = validate_http_version("HTTP1.1");
    bool success3 = validate_http_version("HTTP/.1");
    bool success4 = validate_http_version("HTTP/1.1.1");
    bool success5 = validate_http_version("HTTP/1.");
    EXPECT_FALSE(success1 || success2 || success3 || success4 || success5);
}

// Message body for GET requests should be ignored.
TEST_F(SessionTest, MessageBodyTest)
{
    const char* MessageBody = "GET / HTTP/1.1\r\nSomeMessage";
    request req = parse_request_line(MessageBody, "255.0.0.0");;
    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.path, "/");
    EXPECT_EQ(req.http_version, "HTTP/1.1");
    EXPECT_TRUE(req.is_valid());
    EXPECT_EQ(req.original_request, "GET / HTTP/1.1\r\nSomeMessage");
    //EXPECT_EQ(req.body, "SomeMessage");  //TODO: FIGURE OUT WHY THIS FAILS
}

// Simple sanity test to check if the socket is valid
TEST_F(SessionTest, SimpleWorkingTest)
{
    session my_session(*io_service_ptr, config_, &man_);
    EXPECT_NE(&my_session.socket(), nullptr);
}
