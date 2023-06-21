#include "LogicSystem.h"

LogicSystem::LogicSystem() : _b_stop(false) {
    RegisterCallBacks();
    _worker_thread = std::thread (&LogicSystem::DealMsg, this);
}

//绑定回调函数，给对应的msg_id,绑定对应的回调函数，初始化工作
void LogicSystem::RegisterCallBacks() {
    _fun_callbacks[MSG_HELLO_WORD] = std::bind(&LogicSystem::HelloWordCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::HelloWordCallBack(std::shared_ptr<Session> session, short msg_id, std::string msg_data) {
    //调用json反序列化字符串
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
    root["data"] = "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}

void LogicSystem::DealMsg() {
    while(1) {
        std::unique_lock<std::mutex> unique_lk(_mutex);
        //判断队列为空则用条件变量阻塞等待，并释放锁
        //阻塞等待可以确保不会抢占时间片
        while(_msg_que.empty() && !_b_stop) {
            _consume.wait(unique_lk);
        }

        //判断是否为关闭状态，执行完所有逻辑后退出循环，如果外部执行了停止指令，会将当前逻辑执行完再停止
        if(_b_stop) {
            while (!_msg_que.empty()) {
                auto msg_node = _msg_que.front();
                std::cout << "recv_msg id is " << msg_node -> _recvnode -> _msg_id << std::endl;
                //通过msg_id 找到对应的回调函数的迭代器
                auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
                //如果迭代器指向最后说明没有找到
                if(call_back_iter == _fun_callbacks.end()) {
                    _msg_que.pop();//则将当前队列的首元素推出队列
                    continue; //轮询执行下一信息处理
                }
                //调用回调函数,即msg_id对应的逻辑处理
                call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id, std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
                _msg_que.pop();
            }
            break;//为关闭状态之间跳出循环
        }
        
        //如果b_stop没有停止，且执行到这一步，说明队列有数据
        auto msg_node = _msg_que.front();
        std::cout << "recv_msg id is " << msg_node->_recvnode->_msg_id << std::endl;

        auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
        if(call_back_iter == _fun_callbacks.end()) {
            _msg_que.pop();
            continue;
        }

        call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id, std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));

        _msg_que.pop();
    }
}

LogicSystem::~LogicSystem() {
    //将逻辑系统的启停标识置为false，随后进行销毁
    _b_stop = true;
    //此时要通过条件变量唤醒阻塞线程，防止逻辑系统还有数据没处理
    _consume.notify_one();
    //调用阻塞回收，确保在主线程回收之前，工作线程会先回收
    _worker_thread.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg) {
    //要往逻辑消息队列里放数据前要先锁住队列，防止线程安全问题
    std::unique_lock<std::mutex> unique_lk(_mutex);
    _msg_que.push(msg);
    //如果队列的大小是从0到1则要发送通知信号唤醒线程
    if(_msg_que.size() == 1) {
        _consume.notify_one();
    }

} 