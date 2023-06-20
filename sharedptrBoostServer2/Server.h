#ifndef _Server_H_
#define _Server_H_

#include "Session.h"

#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <string.h>
#include <memory>
#include <utility>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
// #include <json/json.h>




class Session;

class Server {
public:
    // Server() {}
    Server(boost::asio::io_context &ioc, short port);
    void clear_Session(std::string uuid);

private:
    void start_accept();
    void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code &error);
    boost::asio::io_context &_ioc;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<Session>> Smap;
};

#endif