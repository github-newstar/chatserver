#include<vector>
#include<boost/asio.hpp>
#include"Singleton.hpp"
#include"nocopyable.hpp"
#include<thread>

class AsioIOServicePool : public Singleton<AsioIOServicePool>, public nocopyable {
    friend class Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::shared_ptr<Work>;
    ~AsioIOServicePool();

    //使用round-robin的方式返回一个io_service
    boost::asio::io_context& GetIOService();
    void Stop();
private:
    AsioIOServicePool(std::size_t size=std::thread::hardware_concurrency());
    std::vector<IOService> IOServices_;
    std::vector<WorkPtr> works_;
    std::vector<std::thread> threads_;
    std::atomic_int nextIOServiceIndex_;
};