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
        // std::vector<std::string> default_echo_root_vec(1, default_echo_root);
        // std::vector<std::string> static_root_vec(1, default_static_root);
        // static_root_vec.push_back("static2");
        // config_.echo_roots = default_echo_root_vec;
        // config_.static_roots = static_root_vec;
    }

    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
    std::string default_echo_root = "/echo";
    std::string default_static_root = "/static";
    config_params config_;
};

// // Test to make sure basic RequestLine passes
// TEST_F(SessionTest, StandardFileTest)
// {
//     const char* StandardRequest = "GET /static/test.txt HTTP/1.1\r\n";
//     bool success = parse_request_line(StandardRequest, strlen(StandardRequest), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Test to make sure basic RequestLine passes
// TEST_F(SessionTest, StandardEchoTest)
// {
//     const char* StandardRequest = "GET /echo HTTP/1.1\r\n";
//     bool success = parse_request_line(StandardRequest, strlen(StandardRequest), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Test to make sure wrong static root still go through (fail later in the static_response_handler)
// TEST_F(SessionTest, WrongStaticRoot)
// {
//     const char* StandardRequest = "GET /not_static/test.txt HTTP/1.1\r\n";
//     bool success = parse_request_line(StandardRequest, strlen(StandardRequest), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Test to make sure wrong echo root still go through
// TEST_F(SessionTest, WrongEchoRoot)
// {
//     const char* StandardRequest = "GET /not_echo/test.txt HTTP/1.1\r\n";
//     bool success = parse_request_line(StandardRequest, strlen(StandardRequest), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Test to make sure the second root works also
// TEST_F(SessionTest, ValidSecondRoot)
// {
//     const char* StandardRequest = "GET /static2/test.txt HTTP/1.1\r\n";
//     bool success = parse_request_line(StandardRequest, strlen(StandardRequest), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Test should fail without a method
// TEST_F(SessionTest, MissingMethodTest)
// {
//     const char* MissingMethod = " / HTTP/1.1\r\n";
//     bool success = parse_request_line(MissingMethod, strlen(MissingMethod), config_).req_type;
//     EXPECT_FALSE(success);
// }

// // Test should fail if method is unknown. Methods are case-sensitive.
// TEST_F(SessionTest, UnknownMethodTest)
// {
//     const char* UnknownMethod = "get / HTTP/1.1\r\n";
//     bool success = parse_request_line(UnknownMethod, strlen(UnknownMethod), config_).req_type;
//     EXPECT_FALSE(success);
// }

// // Test should fail if URI is missing
// TEST_F(SessionTest, MissingURITest)
// {
//     const char* MissingURI = "GET  HTTP/1.1\r\n";
//     bool success = parse_request_line(MissingURI, strlen(MissingURI), config_).req_type;
//     EXPECT_FALSE(success);
// }

// // Test should fail if more than one space separates elements
// TEST_F(SessionTest, SpacingTest)
// {
//     const char* SpaceAfterVersion = "GET / HTTP/1.1 \r\n";
//     const char* TooManySpaces = "GET  / HTTP/1.1\r\n";
//     bool success = parse_request_line(SpaceAfterVersion, strlen(SpaceAfterVersion), config_).req_type;
//     bool success2 = parse_request_line(TooManySpaces, strlen(TooManySpaces), config_).req_type;
//     EXPECT_FALSE(success);
//     EXPECT_FALSE(success2);
// }

// // Request-lines should end with a carraige return before the new line
// TEST_F(SessionTest, CarriageReturnTest)
// {
//     const char* MissingCR = "GET / HTTP/1.1\n";
//     bool success = parse_request_line(MissingCR, strlen(MissingCR), config_).req_type;
//     EXPECT_FALSE(success);
// }

// // Test for version format
// TEST_F(SessionTest, VersionTest)
// {
//     const char* IncorrectVersion = "GET / 1.1\r\n";
//     bool success = parse_request_line(IncorrectVersion, strlen(IncorrectVersion), config_).req_type;
//     EXPECT_FALSE(success);
// }

// // Message body for GET requests should be ignored.
// TEST_F(SessionTest, MessageBodyTest)
// {
//     const char* MessageBody = "GET / HTTP/1.1\r\nSomeMessage";
//     bool success = parse_request_line(MessageBody, strlen(MessageBody), config_).req_type;
//     EXPECT_TRUE(success);
// }

// // Simple sanity test to check if the socket is valid
// TEST_F(SessionTest, SimpleWorkingTest)
// {
//     session my_session(*io_service_ptr, config_);
//     EXPECT_NE(&my_session.socket(), nullptr);
// }
