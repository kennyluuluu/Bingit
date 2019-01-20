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
#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include "session.h"
#include "server.h"
#include "config_parser.h"

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <config>\n";
            return 1;
        }

        NginxConfigParser config_parser;
        NginxConfig config;
        bool success = config_parser.Parse(argv[1], &config);

        if (!success)
        {
            std::cerr << "Invalid config provided: exiting with value of 1";
            return 1;
        }

        std::string str_conf = config.ToString();
        std::size_t port_pos = str_conf.find("listen ");

        if (port_pos == std::string::npos)
        {
            std::cerr << "No port number provided: exiting with value of 1";
            return 1;
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
        while (it != port_num.end() && std::isdigit(*it))
        {
            ++it;
        }

        if (port_num.empty() || it != port_num.end())
        {
            std::cerr << "Provided port number is empty or not a number: exiting with value of 1";
            return 1;
        }

        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        server s(io_service, atoi(port_num.c_str()));

        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
