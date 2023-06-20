#include "Session.h"


//初始化构造session
Session::Session(boost::asio::io_context& ioc, Server *server) : _socket(ioc), _server(server), _b_head_parse(false){
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
    _recv_msg_node = nullptr;
}

//启动异步读
void Session::Start() {
    memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

//读回调函数
void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, std::shared_ptr<Session> newsession) {
    if(!error) {
        //处理粘包问题
        int copy_len = 0;//已经复制的长度
        while(bytes_transferred > 0) {
            if(!_b_head_parse) {
                //如果收到的数据不足头部大小
                if(bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH) {
                    //此时要从原有数据的末尾开始复制，复制的数据为seesion数组里的数据加上已复制了的长度，copylen一直为0，保证代码格式
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;

                    std::memset(_data, 0, MAX_LENGTH);//清空数据，便于下次接收
                    //继续执行异步读操作
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
                    return;
                }
                //收到的数据比头部多，要通过头部剩余未复制的长度来进行处理
                //头部剩余未复制的长度
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                //将readsome的数据里属于头部的复制给头部
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                
                copy_len += head_remain;//表示得到的数据有一部分已经复制给了头部
                bytes_transferred -= head_remain;

                //得到头部数据
                short data_len = 0;
                memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);

                //将网络字节序转化为本地字节序
                data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);

                std::cout << "data_len is " << data_len << std::endl; 
                
                //如果头部长度非法
                if(data_len > MAX_LENGTH) {
                    std::cout << "invalid data length is " << data_len << std::endl;
                    _server->clear_Session(_uuid);
                    return;
                }

                //创建一个接收结点
                _recv_msg_node = std::make_shared<MsgNode>(data_len);

                //消息的长度小于头部规定的长度，说明数据未收全,则将部分数据先放到接收结点里
                if(bytes_transferred < data_len) {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    std::memset(_data, 0, MAX_LENGTH);//清空以便下次接收

                    //头节点处理完毕
                    _b_head_parse = true;

                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
                    return;
                }
                //如果能执行到这里，说明收到的数据是比头部规定的长度大，但可能出现粘包情况
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, data_len);

                _recv_msg_node->_cur_len += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                std::cout << "receiver data is " << _recv_msg_node->_data << std::endl;
                
                //此处可调用Send发送测试
                Send(_recv_msg_node->_data, _recv_msg_node->_total_len);

                //继续轮询剩余未处理数据,这也是设置轮询的用处之一
                _b_head_parse = false;//此时相当于切包到一个新的结点
                _recv_head_node->Clear();
                if(bytes_transferred <= 0) {
                    std::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
                        return;
                }
                continue;//此处则重新开始处理新的结点
            }

            //已经处理完头部，处理上次未接受完的消息数据
            //接受的数据仍然不足要求的长度
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if(bytes_transferred < remain_msg) {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                std::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
                return;    
            }

            //执行到这一步，则说明接受的数据已经等于或者超过了要求的长度，超过了则要考虑粘包
            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            std::cout << "receieve data is " << _recv_msg_node->_data << std::endl;

            //此处调用send测试
            Send(_recv_msg_node->_data, _recv_msg_node->_total_len);

            //继续轮询剩余未处理的数据
            _b_head_parse = false;
            _recv_head_node->Clear();

            //事实上只会等于0
            if(bytes_transferred <= 0) {
                memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));

                return;
            }
            continue;//这一步表示bytes_transferred大于0，表示发生了粘包现象，需要进行切包处理
        }
    }else {
        std::cout << "read error: " << error.message() <<std::endl;
        // delete this;
        _server->clear_Session(_uuid);
    }
}

//写回调函数
void Session::handle_write(const boost::system::error_code &error, std::shared_ptr<Session> newsession) {
    if(!error) {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        // std::cout << "server send is over" << std::endl;
        if(!_send_que.empty()) {
            auto& msgNode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgNode -> _data, msgNode -> _total_len), 
                std::bind(&Session::handle_write, this, std::placeholders::_1, newsession));
        } //对发送队列的数据进行持续发送
        // memset(_data, 0, MAX_LENGTH);
        // _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), 
        //     std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, newsession));
    }else {
        std::cout << "write error" << std::endl;
        // delete this;
        _server->clear_Session(_uuid);
    }
} 

//异步发送函数
void Session::Send(char *msg, int max_length) {
    // bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    int send_que_size = _send_que.size();

	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

    _send_que.push(std::make_shared<MsgNode>(msg, max_length));

   	if (send_que_size>0) {
		return;
	}

    auto& msgnode = _send_que.front();

    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length), 
        std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));

}