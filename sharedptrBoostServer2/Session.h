#ifndef _Session_H_
#define _Session_H_

#include "Server.h"
#include "MsgNode.h"
#include "const.h"

#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <string.h>
#include <memory>
#include <utility>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <queue>




using boost::asio::ip::tcp;

class Server;
class MsgNode;

class Session : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& ioc, Server *server);

    tcp::socket& Socket() {
        return _socket;
    }

    void Start();
    void Close();

    std::string& Uuid() {
        return _uuid;
    }

private:
    void handle_read(const boost::system::error_code &error,
        std::size_t bytes_transferred, std::shared_ptr<Session> ptr);
    void handle_write(const boost::system::error_code &error, std::shared_ptr<Session> ptr);
    void Send(char *msg, int max_length);

    std::string _uuid;
    tcp::socket _socket;
    Server* _server;

    bool _b_close = true;
    
    //接收或发送的数据
    // enum {max_length = 1024};
    char _data[MAX_LENGTH];
    //发送队列
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
    //收到的消息结构
    std::shared_ptr<MsgNode> _recv_msg_node;
    bool _b_head_parse;
    //收到的头部结构
    std::shared_ptr<MsgNode> _recv_head_node;
};

#endif 