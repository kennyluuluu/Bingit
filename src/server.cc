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
#include <string>
#include "session.h"
#include "server.h"
#include "config_parser.h"

short int server::parse_port_number(const char *file)
{
    NginxConfigParser config_parser;
    NginxConfig config;
    bool success = config_parser.Parse(file, &config);

    if (!success)
    {
        std::cerr << "Invalid config provided: exiting with value of 1";
        return -1;
    }

    std::string str_conf = config.ToString();
    std::size_t port_pos = str_conf.find("listen ");

    if (port_pos == std::string::npos)
    {
        std::cerr << "No port number provided: exiting with value of 1";
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
        std::cerr << "Provided port number is empty or not a number: exiting with value of 1";
        return 1;
    }

    short a = atoi(port_num.c_str());

    if (a <= 0) return -1;

    return a;

}

server::server(boost::asio::io_service &io_service, const char *file_name)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), server::parse_port_number(file_name)))
{
    start_accept();
}

void server::start_accept()
{
    session *new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
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