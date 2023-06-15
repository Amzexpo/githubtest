#include "Session.h"

Session::Session(boost::asio::io_context& ioc, Server *server) : _socket(ioc), _server(server){
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
}

void Session::Start() {
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, std::shared_ptr<Session> newsession) {
    if(!error) {
        std::cout << "server receive data is " << _data << std::endl;
        Send(_data, bytes_transferred);
        memset(_data, 0, max_length);
        // boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
        //     std::bind(&Session::handle_write, this, std::placeholders::_1, newsession));
        _socket.async_read_some(boost::asio::buffer(_data, max_length), std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
    }else {
        std::cout << "read error: " << error.message() <<std::endl;
        // delete this;
        _server->clear_Session(_uuid);
    }
}

void Session::handle_write(const boost::system::error_code &error, std::shared_ptr<Session> newsession) {
    if(!error) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        // std::cout << "server send is over" << std::endl;
        if(!_send_que.empty()) {
            auto& msgNode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgNode -> _data, msgNode -> _max_len), 
                std::bind(&Session::handle_write, this, std::placeholders::_1, newsession));
        } //对发送队列的数据进行持续发送
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length), 
            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
    }else {
        std::cout << "write error" << std::endl;
        // delete this;
        _server->clear_Session(_uuid);
    }
} 

void Session::Send(char *msg, int max_length) {
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    if(_send_que.size() > 0) {
        pending = true;
    }
    _send_que.push(std::make_shared<MsgNode>(msg, max_length));

    if(pending) {
        return;
    }

    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length), 
        std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));

}