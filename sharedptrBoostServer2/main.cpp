#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include "Session.h"



int main() {
    try {
        boost::asio::io_context ioc;
        Server server(ioc, 6000);
        ioc.run();
    }catch(std::exception &e) {
        std::cout << "server start error " << e.what() << std::endl;
    }


    return 0;
}