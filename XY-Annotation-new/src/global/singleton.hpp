#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_

#include <mutex>
//在模板类 Singleton 中，可以定义单例模式的实现细节
template <typename T>
class Singleton {
public:
    //通过删除拷贝构造函数、赋值运算符、移动构造函数、移动赋值运算符等函数，确保了单例对象的唯一性
    Singleton(const Singleton &) = delete;				//删除 拷贝构造函数
    Singleton& operator=(const Singleton &) = delete;	//删除 赋值运算符
    Singleton(Singleton &&) = delete;					//删除 移动构造函数
    Singleton& operator=(Singleton &&) = delete;		//删除 移动赋值运算符

    //通过静态函数 instance() 返回单例对象的指针，并利用静态成员变量和静态成员函数确保单例对象的唯一性和线程安全性
    static T* instance()
    {
        static T object;
        return &object;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};

#endif
