#include "Server.h"

//构造函数
Server::Server(boost::asio::io_context &ioc, short port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)){
    std::cout << "server is run on " << port << "port" << std::endl;
    start_accept();
}

//开始监听
void Server::start_accept() {
    // Session *new_session = new Session(_ioc);
    std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc, this);

    _acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));//引用技术加一
}

//清除所有的连接
void Server::clear_Session(std::string uuid) {
    Smap.erase(uuid);
}

//处理监听，即监听的回调函数
void Server::handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code &error) {
    if(!error) {
        new_session -> Start();
        Smap.insert(std::make_pair(new_session -> Uuid(), new_session));
    }else {
        //delete new_session;
        std::cout << "session accept error" << error.message() << std::endl;
    }

    start_accept(); //重新监听
}

void Session::Close() {
    _socket.close();
    _b_close = true;
}