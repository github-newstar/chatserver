#pragma once
#include <iostream>
#include <memory>
#include <mutex>

template <typename T> class Singleton {
  public:
  protected:
    Singleton() = default;
    Singleton(const Singleton & ) = delete;
    Singleton& operator=(const Singleton & ) = delete;

    static std::shared_ptr<T> instance_;

  public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag flag;
        std::call_once(flag, [&]() { instance_ = std::shared_ptr<T>(new T); });
        return instance_;
    }
    void PrintAddress() { std::cout << instance_.get() << std::endl; }
    ~Singleton() { std::cout << "signleton destructor" << std::endl; }
};

template <typename T> std::shared_ptr<T> Singleton<T>::instance_ = nullptr;