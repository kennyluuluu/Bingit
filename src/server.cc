//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//#include <cstdlib>
//#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include <stdlib.h>
#include <string>
#include "session.h"
#include "server.h"
#include "config_parser.h"

namespace logging = boost::log;
// namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

server::server(boost::asio::io_service &io_service, const char *file_name)
    : io_service_(io_service)
{
    init_logging();

    short port_num = parse_port_number(file_name);
    if(port_num == -1){
        acceptor_ = nullptr;
    }
    else
    {
        using boost::asio::ip::tcp;
        acceptor_ = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), port_num));
    }
}

void server::init_logging()
{
    // exposes severity level attribute
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
    
    // create logging file
    logging::add_file_log(
        keywords::file_name = "server_log_%N.log",
        keywords::rotation_size = 10000000, // 10 MB 
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // Midnight
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%",
        keywords::open_mode = std::ios_base::app
    );

    // enable console logging simulataneously 
    logging::add_console_log(std::cout, boost::log::keywords::format = ">>[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%");

    // enables time stamps correctly
    boost::log::add_common_attributes();

    BOOST_LOG_TRIVIAL(trace) << "Log file created";

}

bool server::init()
{
    if(acceptor_ != nullptr)
    {
        BOOST_LOG_TRIVIAL(info) << "Initialized server...";
        start_accept();
        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to init server, acceptor is a nullptr";
        return false;
    }
}

short int server::parse_port_number(const char *file)
{
    NginxConfigParser config_parser;
    NginxConfig config;
    bool success = config_parser.Parse(file, &config);

    if (!success)
    {
        BOOST_LOG_TRIVIAL(error) << "Invalid config provided";
        return -1;
    }

    std::string str_conf = config.ToString();
    std::size_t port_pos = str_conf.find("listen ");

    if (port_pos == std::string::npos)
    {
        BOOST_LOG_TRIVIAL(error) << "No 'listen' parameter present in config";
        return -1;
    }

    //adding 7 to ignore 'listen '
    //adding it here in case it is an npos
    port_pos += 7;

    //find position of semi-colon after 'listen '
    std::size_t end_of_port_num = str_conf.find(";", port_pos);
    std::string port_num = str_conf.substr(port_pos, end_of_port_num - port_pos);

    //checking if provided port is a number
    //obtained and modified from:
    //https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
    std::string::const_iterator it = port_num.begin();

    while (it != port_num.end() && std::isdigit(*it)) ++it;

    if (port_num.empty() || it != port_num.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Provided port number is empty or not a number";
        return -1;
    }

    short a = atoi(port_num.c_str());

    //shouldn't happen bc the string parser above doesn't accept '-'
    if (a <= 0)
    {
        BOOST_LOG_TRIVIAL(error) << "Provided port number is <- 0";
        return -1;
    }

    BOOST_LOG_TRIVIAL(trace) << "Config file parsed correctly";
    BOOST_LOG_TRIVIAL(info) << "Server will be initialized on port " << a << "...";
    BOOST_LOG_TRIVIAL(info) << "Starting server on port " << a;
    return a;
}

void server::start_accept()
{
    session *new_session = new session(io_service_);
    acceptor_->async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void server::handle_accept(session *new_session,
                           const boost::system::error_code &error)
{
    if (!error)
    {
        new_session->start();
    }
    else
    {
        delete new_session;
    }

    start_accept();
}

tcp::acceptor* server::get_acceptor()
{
    return this->acceptor_;
}

boost::asio::io_service& server::get_io_service()
{
    return this->io_service_;
}

server::~server()
{
    if(acceptor_ != nullptr)
    {
        delete acceptor_;
    }
}