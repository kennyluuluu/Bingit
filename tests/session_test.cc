#include "gtest/gtest.h"
#include "session.h"
#include <string>

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
    }

    boost::asio::io_service io_service;
    boost::asio::io_service *io_service_ptr = &io_service;
};

// Test to make sure basic RequestLine passes
TEST(RequestLineParserTest, StandardTest)
{
    const char* StandardRequest = "GET / HTTP/1.1\r\n";
    bool success = parse_request_line(StandardRequest, strlen(StandardRequest)).req_type;
    EXPECT_TRUE(success);
}
// Test should fail without a method
TEST(RequestLineParserTest, MissingMethodTest)
{
    const char* MissingMethod = " / HTTP/1.1\r\n";
    bool success = parse_request_line(MissingMethod, strlen(MissingMethod)).req_type;
    EXPECT_FALSE(success);
}

// Test should fail if method is unknown. Methods are case-sensitive.
TEST(RequestLineParserTest, UnknownMethodTest)
{
    const char* UnknownMethod = "get / HTTP/1.1\r\n";
    bool success = parse_request_line(UnknownMethod, strlen(UnknownMethod)).req_type;
    EXPECT_FALSE(success);
}

// Test should fail if URI is missing
TEST(RequestLineParserTest, MissingURITest)
{
    const char* MissingURI = "GET  HTTP/1.1\r\n";
    bool success = parse_request_line(MissingURI, strlen(MissingURI)).req_type;
    EXPECT_FALSE(success);
}

// Test should fail if more than one space separates elements
TEST(RequestLineParserTest, SpacingTest)
{
    const char* SpaceAfterVersion = "GET / HTTP/1.1 \r\n";
    const char* TooManySpaces = "GET  / HTTP/1.1\r\n";
    bool success = parse_request_line(SpaceAfterVersion, strlen(SpaceAfterVersion)).req_type;
    bool success2 = parse_request_line(TooManySpaces, strlen(TooManySpaces)).req_type;
    EXPECT_FALSE(success);
    EXPECT_FALSE(success2);
}

// Request-lines should end with a carraige return before the new line
TEST(RequestLineParserTest, CarraigeReturnTest)
{
    const char* MissingCR = "GET / HTTP/1.1\n";
    bool success = parse_request_line(MissingCR, strlen(MissingCR)).req_type;
    EXPECT_FALSE(success);
}

// Test for version format
TEST(RequestLineParserTest, VersionTest)
{
    const char* IncorrectVersion = "GET / 1.1\r\n";
    bool success = parse_request_line(IncorrectVersion, strlen(IncorrectVersion)).req_type;
    EXPECT_FALSE(success);
}

// Message body for GET requests should be ignored.
TEST(RequestLineParserTest, MessageBodyTest)
{
    const char* MessageBody = "GET / HTTP/1.1\r\nSomeMessage";
    bool success = parse_request_line(MessageBody, strlen(MessageBody)).req_type;
    EXPECT_TRUE(success);
}

// Simple sanity test to check if the socket is valid
TEST_F(SessionTest, SimpleWorkingTest)
{
    session my_session(*io_service_ptr);
    EXPECT_NE(&my_session.socket(), nullptr);
}
