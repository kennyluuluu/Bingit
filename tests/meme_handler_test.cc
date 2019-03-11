#include "gtest/gtest.h"
#include "meme_handler.h"
#include "handler.h"
#include "config_parser.h"
#include "request.h"
#include "reply.h"
#include "session.h"
#include <string>
#include <fstream>

class MemeHandlerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        NginxConfigParser p;
        p.Parse("meme_config", &my_config);
        params_ = config_params(my_config);
        std::string req_path = "/memes/new";
        handler_ = meme_handler::create(my_config, "./");
    }

    void TearDown() override
    {
        delete handler_;
    }

    NginxConfig my_config;
    config_params params_;
    handler* handler_;
    request req;
    request req2;
};

TEST_F(MemeHandlerTest, CreateTest)
{
    EXPECT_NE(handler_, nullptr);
}

TEST_F(MemeHandlerTest, NewMemeTest)
{
    req = parse_request_line("GET /meme/new HTTP/1.1\r\n\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("200") != std::string::npos); // TODO: Figure out why this is 404
}

TEST_F(MemeHandlerTest, ValidCreateMemeTest)
{
    req = parse_request_line("POST /meme/create HTTP/1.1\r\n\r\nimage=img&top=t&bottom=b", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("200") != std::string::npos);

    req2 = parse_request_line("GET /meme/view?id=1 HTTP/1.1\r\n", "IP");
    std::unique_ptr<reply> response2 = handler_->HandleRequest(req2);
    std::string response_string2 = response.get()->construct_http_response();
    EXPECT_TRUE(response_string2.find("200") != std::string::npos);
}

TEST_F(MemeHandlerTest, InvalidCreateMemeTest)
{
    req = parse_request_line("POST /meme/create HTTP/1.1\r\n\r\nimage=&top=&bottom=", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("400") != std::string::npos);
}

TEST_F(MemeHandlerTest, ViewMemeTest)
{
    req = parse_request_line("GET /meme/view?id=1 HTTP/1.1\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("404") != std::string::npos);
}

TEST_F(MemeHandlerTest, ListMemeTest)
{
    req = parse_request_line("GET /meme/list HTTP/1.1\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("200") != std::string::npos);
}

TEST_F(MemeHandlerTest, DeleteMemeTest)
{
    req = parse_request_line("GET /meme/view?id=1 HTTP/1.1\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    std::unique_ptr<reply> response = handler_->HandleRequest(req);
    std::string response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("404") != std::string::npos);

    req = parse_request_line("POST /meme/create HTTP/1.1\r\n\r\nimage=img&top=t&bottom=b", "IP");
    EXPECT_TRUE(req.is_valid());
    response = handler_->HandleRequest(req);
    response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("200") != std::string::npos);

    req2 = parse_request_line("GET /meme/view?id=1 HTTP/1.1\r\n", "IP");
    std::unique_ptr<reply> response2 = handler_->HandleRequest(req2);
    std::string response_string2 = response.get()->construct_http_response();
    EXPECT_TRUE(response_string2.find("200") != std::string::npos);

    req = parse_request_line("GET /meme/delete?id=1 HTTP/1.1\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    response = handler_->HandleRequest(req);
    response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("200") != std::string::npos);

    req = parse_request_line("GET /meme/view?id=1 HTTP/1.1\r\n", "IP");
    EXPECT_TRUE(req.is_valid());
    response = handler_->HandleRequest(req);
    response_string = response.get()->construct_http_response();
    EXPECT_TRUE(response_string.find("404") != std::string::npos);
}