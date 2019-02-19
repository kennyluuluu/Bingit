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
#include <vector>
#include <unordered_map>
#include "session.h"
#include "server.h"
#include "config_params.h"
#include "config_parser.h"

namespace logging = boost::log;
// namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

server::server(boost::asio::io_service &io_service, const char *file_name)
    : io_service_(io_service)
{
    init_logging();

    set_config_params(file_name);
    if (params_.port == -1)
    {
        acceptor_ = nullptr;
    }
    else
    {
        using boost::asio::ip::tcp;
        acceptor_ = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), params_.port));
    }
}

void server::init_logging()
{
    // exposes severity level attribute
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    // create logging file
    logging::add_file_log(
        keywords::file_name = "server_log_%N.log",
        keywords::rotation_size = 10000000,                                           // 10 MB
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // Midnight
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%",
        keywords::open_mode = std::ios_base::app);

    // enable console logging simulataneously
    logging::add_console_log(std::cout, boost::log::keywords::format = ">>[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%");

    // enables time stamps correctly
    boost::log::add_common_attributes();

    BOOST_LOG_TRIVIAL(trace) << "Log file created";
}

bool server::init()
{
    if (acceptor_ != nullptr)
    {
        BOOST_LOG_TRIVIAL(info) << "Initialized server on port " << params_.port << "...";
        start_accept();
        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to init server, acceptor is a nullptr";
        return false;
    }
}

bool server::kill()
{
    if (acceptor_ != nullptr && &io_service_ != nullptr)
    {
        BOOST_LOG_TRIVIAL(info) << "Killed server...";
        io_service_.stop();
        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to kill server, io_service is null";
        return false;
    }
}

void server::set_config_params(const char *file)
{
    NginxConfigParser config_parser;
    NginxConfig config;

    bool success = config_parser.Parse(file, &config);

    if (!success)
    {
        BOOST_LOG_TRIVIAL(error) << "Invalid config provided, failed to parse";
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return;
    }

    params_ = config_params(config);

    if(params_.port == -1)
    {
        BOOST_LOG_TRIVIAL(error) << "No valid port number found in config.";
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Config file parsed for params:";
    BOOST_LOG_TRIVIAL(info) << "==========================";
    BOOST_LOG_TRIVIAL(info) << "Port: " << params_.port;
    BOOST_LOG_TRIVIAL(info) << "Handler paths: ";
    for (std::pair<std::string, std::pair<std::string, NginxConfig>> ele : params_.handler_paths)
    {
        BOOST_LOG_TRIVIAL(info) << "--------------------";
        BOOST_LOG_TRIVIAL(info) << "Path: " << ele.first;
        BOOST_LOG_TRIVIAL(info) << "Type: " << ele.second.first;
    }
    BOOST_LOG_TRIVIAL(info) << "==========================";
}

void server::start_accept()
{
    session *new_session = new session(io_service_, params_, &manager_);
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

tcp::acceptor *server::get_acceptor()
{
    return this->acceptor_;
}

boost::asio::io_service &server::get_io_service()
{
    return this->io_service_;
}

server::~server()
{
    if (acceptor_ != nullptr)
    {
        delete acceptor_;
    }
}
