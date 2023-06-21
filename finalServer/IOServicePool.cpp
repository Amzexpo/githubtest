#include "IOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size) :  _ioServices(size), _works(size), _nextIOService(0){
    // _works是unique_ptr的vector类型，所以初始化时要么放在构造函数初始化列表里初始化，要么通过一个临时的std::unique_ptr右值初始化，采取第二种。
    for(std::size_t i = 0; i < size; i++) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
    }

    //遍历多个io_service，创建多个线程，每个线程内部启动ioservice
    for(std::size_t i = 0; i < _ioServices.size(); i++) {
        //创建一个线程并放入
        _threads.emplace_back([this, i]() { 
            _ioServices[i].run();
        });
    }
}

//根据内部索引返回一个ioservice
boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }

    return service;
}

//停止服务池，调用work.reset(),结束每个work，确保ioservice能够结束，最后阻塞等待每个线程回收
void AsioIOServicePool::stop() {
    for(auto& work : _works) {
        work.reset();
    }

    for(auto& t : _threads) {
        t.join();
    }
}

AsioIOServicePool::~AsioIOServicePool() {
	std::cout << "AsioIOServicePool destruct" << std::endl;
}