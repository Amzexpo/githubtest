#pragma once

#include <memory>
#include <mutex>
#include <iostream>

//实现一个单例模板类，使用模板确保可以进行复用
template <typename T>
class Singleton {
//使用protected，确保子类可以继承，且外界不能访问
protected:
    Singleton() = default; //显示声明默认构造函数，防止被自定义构造函数影响
    Singleton(const Singleton<T>&) = delete; //删除拷贝构造函数，防止单例类被拷贝
    Singleton& operator=(const Singleton<T>& st) = delete; //删除赋值构造函数，理由同上

    static std::shared_ptr<T> _instance; // 实现一个静态模板智能指针变量，可以延长闭包的生命周期

public:
    static std::shared_ptr<T> GetInstance() {
        //即使多个线程也只会调用一次函数callonce， 内部绑定一个回调函数，对静态局部变量进行一次初始化赋值
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){
            _instance = std::shared_ptr<T>(new T);
        });
        
        return _instance;
    }

    void PrintAddress() {
        std::cout << _instance.get() << std::endl;
    }

    //析构函数，输出提示信息，智能指针由系统管理自动释放内存
    ~Singleton() {
        std::cout << "this is singleton detruct" << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;//对静态局部变量进行初始化，只有在函数执行时会调用
