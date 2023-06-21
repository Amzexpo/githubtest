#pragma once

#include <queue>
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "Singleton.h"
#include "Session.h"
#include "const.h"


class Session;
class LogicNode;

//通过using给函数起别名，也可以使用 function模板构造一个方法，再起别名
// typedef  std::function<void(std::shared_ptr<Session>, short, std::string)> FunCallBack;
using FunCallBack = std::function<void(std::shared_ptr<Session>, short, std::string)>;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);

private:
    LogicSystem();
    void DealMsg();//用于工作线程调度逻辑队列里的信息
    void RegisterCallBacks();//给msg_id绑定回调函数，初始化工作
    void HelloWordCallBack(std::shared_ptr<Session>, short msg_id, std::string msg_data);
    std::thread _worker_thread;//工作线程
    std::queue<std::shared_ptr<LogicNode>> _msg_que;//逻辑消息队列存放着逻辑消息
    std::mutex _mutex;//局部的互斥锁，用于逻辑系统不需要工作时进行阻塞,还可以用于对队列进行互斥访问
    std::condition_variable _consume;//配合锁一起使用，在需要工作时能唤醒逻辑系统
    bool _b_stop;//用于启停逻辑系统
    std::map<short, FunCallBack> _fun_callbacks;//该map存放着不同消息id对应的不同回调函数 
};