#include <iostream>
#include <boost/asio.hpp>

#include "Server.h"
#include "Session.h"
#include "IOServicePool.h"



int main() {
    try {
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context io_context;
        // Server server(ioc, 6000);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, pool](auto, auto) {
            io_context.stop();
            pool->stop();
            });
            
        Server s(io_context, 6000);
        io_context.run();

    }catch(std::exception &e) {
        std::cout << "server start error " << e.what() << std::endl;
    }


    return 0;
}