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
#include "session.h"
#include "server.h"
#include "config_var.h"
#include "config_parser.h"

namespace logging = boost::log;
// namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

server::server(boost::asio::io_service &io_service, const char *file_name)
    : io_service_(io_service)
{
    init_logging();

    conf_ = get_config_vars(file_name);
    if(conf_.port == -1){
        acceptor_ = nullptr;
    }
    else
    {
        using boost::asio::ip::tcp;
        acceptor_ = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), conf_.port));
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
        BOOST_LOG_TRIVIAL(info) << "Initialized server on port " << conf_.port << "...";
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
    if(acceptor_ != nullptr && &io_service_ != nullptr)
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

short int parse_for_port_number(std::string& str_conf)
{
    std::size_t port_pos = str_conf.find("listen ");

    if (port_pos == std::string::npos)
    {
        BOOST_LOG_TRIVIAL(error) << "No 'listen' parameter present in config";
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return -1;
    }

    //length of 'listen '
    const int num_chars_in_listen_ = 7;
    port_pos += num_chars_in_listen_;

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
        BOOST_LOG_TRIVIAL(error) << "Provided port number is empty or not a number: " << port_num;
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return -1;
    }

    short int port = atoi(port_num.c_str());

    //shouldn't happen bc the string parser above doesn't accept '-'
    if (port <= 0)
    {
        BOOST_LOG_TRIVIAL(error) << "Provided port number is less than or equal to 0: " << port;
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return -1;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Found port " << port << " in config...";
    }

    return port;
}

void parse_for_echo_roots(config_var&  conf, std::string& str_conf)
{

    std::string default_echo_root = "/echo";
    std::vector<std::string> default_echo_root_vec(1, default_echo_root);

    std::size_t echo_root_pos = str_conf.find("echo_root ");

    if (echo_root_pos == std::string::npos)
    {
        BOOST_LOG_TRIVIAL(info) << "No 'echo_root' config found, using default " << default_echo_root;
        conf.echo_roots = default_echo_root_vec;
        return;
    }

    //length of 'echo_root '
    const int num_chars_in_echo_root_ = 10;
    echo_root_pos += num_chars_in_echo_root_;

    //find position of semi-colon after 'echo_root '
    std::size_t end_of_echo_root = str_conf.find(";", echo_root_pos);
    std::string echo_root_csv = str_conf.substr(echo_root_pos, end_of_echo_root - echo_root_pos);

    std::vector<std::string> roots;

    if(echo_root_csv.size() == 0)
    {
        BOOST_LOG_TRIVIAL(info) << "No 'echo_root' variable provided, using default " << default_echo_root;
        conf.echo_roots = default_echo_root_vec;
        return;
    }
    //if the variable is not a csv, use it as the sole root
    else if(echo_root_csv.find(',') == std::string::npos)
    {
        roots = std::vector<std::string>(1, echo_root_csv);
    }
    else
    {
        //https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
        std::stringstream ss(echo_root_csv);

        while( ss.good() )
        {
            std::string substr;
            getline( ss, substr, ',' );
            roots.push_back( substr );
        }

        if(roots.size() == 0)
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid echo root variable provided: '" << echo_root_csv << "'";
            BOOST_LOG_TRIVIAL(info) <<  "Using default " << default_echo_root;
            conf.echo_roots = default_echo_root_vec;
            return;
        }
    }

    for(int i=0; i < roots.size(); i++)
    {
        BOOST_LOG_TRIVIAL(info) <<  "Parsed echo_root: " << roots[i];
        if(roots[i].size() == 0 || roots[i].compare(0,1, "/") != 0)
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid echo root provided: '" << roots[i] << "'";
            BOOST_LOG_TRIVIAL(info) <<  "Using default " << default_echo_root;
            conf.echo_roots = default_echo_root_vec;
            return;
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Using variable as echo_roots: " << echo_root_csv;

    conf.echo_roots = roots;
}


void parse_for_static_roots(config_var&  conf, std::string& str_conf)
{
    std::string default_static_root = "/static";
    std::vector<std::string> default_static_root_vec(1, default_static_root);

    std::size_t static_root_pos = str_conf.find("static_root ");

    if (static_root_pos == std::string::npos)
    {
        BOOST_LOG_TRIVIAL(info) << "No 'static_root' config found, using default " << default_static_root;
        conf.static_roots = default_static_root_vec;
        return;
    }

    //length of 'static_root '
    const int num_chars_in_static_root_ = 12;
    static_root_pos += num_chars_in_static_root_;

    //find position of semi-colon after 'static_root '
    std::size_t end_of_static_root = str_conf.find(";", static_root_pos);
    std::string static_root_csv = str_conf.substr(static_root_pos, end_of_static_root - static_root_pos);

    std::vector<std::string> roots;

    if(static_root_csv.size() == 0)
    {
        BOOST_LOG_TRIVIAL(info) << "No 'static_root' variable provided, using default " << default_static_root;
        conf.static_roots = default_static_root_vec;
        return;
    }
    //if the variable is not a csv, use it as the sole root
    else if(static_root_csv.find(',') == std::string::npos)
    {
        roots = std::vector<std::string>(1, static_root_csv);
    }
    else
    {
        //https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
        std::stringstream ss(static_root_csv);

        while( ss.good() )
        {
            std::string substr;
            getline( ss, substr, ',' );
            roots.push_back( substr );
        }

        if(roots.size() == 0)
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid static root variable provided: '" << static_root_csv << "'";
            BOOST_LOG_TRIVIAL(info) <<  "Using default " << default_static_root;
            conf.static_roots = default_static_root_vec;
            return;
        }
    }

    for(int i=0; i < roots.size(); i++)
    { 
        BOOST_LOG_TRIVIAL(info) <<  "Parsed static_root: " << roots[i];
        if(roots[i].size() == 0  || roots[i].compare(0,1, "/") != 0)
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid static root provided: '" << roots[i] << "'";
            BOOST_LOG_TRIVIAL(info) <<  "Using default " << default_static_root;
            conf.static_roots = default_static_root_vec;
            return;
        }
    }

    BOOST_LOG_TRIVIAL(info) << "Using variable as static_roots: " << static_root_csv;

    conf.static_roots = roots;
}

config_var server::get_config_vars(const char *file)
{
    NginxConfigParser config_parser;
    NginxConfig config;

    bool success = config_parser.Parse(file, &config);

    config_var result;
    result.port = -1;
    if (!success)
    {
        BOOST_LOG_TRIVIAL(error) << "Invalid config provided, failed to parse";
        BOOST_LOG_TRIVIAL(error) << "Config rejected";
        return result;
    }

    std::string str_conf = config.ToString();

    //search for port number
    short int port_num = parse_for_port_number(str_conf);
    if(port_num == -1)
    {
        return result;
    }
    else
    {
        result.port = port_num;
    }
    
    //search for echo_roots
    parse_for_echo_roots(result, str_conf);

    //search for static_roots
    parse_for_static_roots(result, str_conf);

    BOOST_LOG_TRIVIAL(trace) << "Config file parsed for variables";
    return result;
}

void server::start_accept()
{
    session *new_session = new session(io_service_, conf_);
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