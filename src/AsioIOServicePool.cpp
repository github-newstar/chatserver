#include"AsioIOServicePool.hpp"

AsioIOServicePool::AsioIOServicePool(
    std::size_t size)
    : IOServices_(size), works_(size), nextIOServiceIndex_(0) {
        for(std::size_t i = 0; i < size; ++i) {
            works_[i] = std::unique_ptr<Work>(new Work(IOServices_[i]));
        }
        for(std::size_t i = 0; i < size; ++i) {
            threads_.emplace_back([this, i](){
                IOServices_[i].run();
            });
        }
    }
AsioIOServicePool::~AsioIOServicePool(){
    std::cout << "AsioIOServicePool::~AsioIOServicePool()" << std::endl;
}

//使用round-robin的方式返回一个io_service
boost::asio::io_context& AsioIOServicePool::GetIOService(){
    auto &ioc = IOServices_[nextIOServiceIndex_++];
    nextIOServiceIndex_ = nextIOServiceIndex_ % IOServices_.size();
    return ioc;
}
void AsioIOServicePool::Stop(){
    for(auto &work : works_){
        work->get_io_context().stop();
        work.reset();
    }
    for(auto &thread : threads_){
        thread.join();
    }
}