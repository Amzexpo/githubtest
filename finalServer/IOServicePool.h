#pragma once

#include <iostream>
#include <vector>

#include "const.h"
#include "Session.h"
#include "Singleton.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool> {
friend Singleton<AsioIOServicePool>;

public:
    //启用类型别名方便处理
    using IOService = boost::asio::io_context;//多个线程将调用多个io_contex, 每个线程拥有自己独立的消息队列
    using Work = boost::asio::io_context::work;//防止在初始化的时候，因为没有注册函数导致io_context提前结束
    using WorkPtr = std::unique_ptr<Work>;//存放着所有的work

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete; //删除其的拷贝构造函数
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete; //删除其的赋值构造函数

    boost::asio::io_context& GetIOService();
    void stop();

private:
    //默认构造函数，存放着
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

    std::vector<IOService> _ioServices; //一个vector数组，存放着所有的ioservice
    std::vector<WorkPtr> _works; //存放在works指针的数组
    std::vector<std::thread> _threads; //存放着所有线程
    std::size_t   _nextIOService; //轮询检索，为每一个线程分配一个ioservice

};