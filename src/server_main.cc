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
#include <stdlib.h>
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include "session.h"
#include "server.h"
#include "config_parser.h"
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <atomic>

//boost::asio::io_service io_serv;
//handling ^C found here
//https://stackoverflow.com/questions/4250013/is-destructor-called-if-sigint-or-sigstp-issued
//and C++ documentation in sigaction
void got_signal(int)
{
    BOOST_LOG_TRIVIAL(info) << "Received ^C, exiting normally";
    //io_serv.stop();
    exit(0);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <config>" << std::endl;
            return 1;
        }
        boost::asio::io_service io_serv;

        //using namespace std; // For atoi.
        server s(io_serv, argv[1]);
        if(s.init())
        {
            // FROM: https://www.boost.org/doc/libs/1_52_0/doc/html/boost_asio/example/http/server3/server.cpp
            // std::vector<boost::shared_ptr<boost::thread>> threads;
            const size_t THREAD_POOL_SIZE = 20;
            // for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
            // {
            //     boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(
            //                                      &boost::asio::io_service::run, &s.get_io_service())));
            //     threads.push_back(thread);
            // }

            // for (std::size_t i = 0; i < threads.size(); ++i)
            //     threads[i]->join();

            boost::thread_group tg;
            for (std::size_t i = 0; i < THREAD_POOL_SIZE; ++i)
            {
                tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_serv));
            }
            io_serv.run();
            tg.join_all();
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    catch (std::exception &e)
    {
        BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
        // std::cerr << "Exception: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}
