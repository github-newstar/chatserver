
#include "CServer.hpp"
#include <boost/asio/signal_set.hpp>
#include <condition_variable>
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.hpp"
#include "configMgr.hpp"

using namespace std;
bool bStop = false;
std::condition_variable cond_quit;
std::mutex mtx_quit;

int main() {
    try {
        auto &cfg = ConfigMgr::GetInstance();
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set sig(ioc, SIGINT, SIGTERM);

        sig.async_wait([&ioc, pool](auto, auto) {
            ioc.stop();
            pool->Stop();
        });
        
        auto  port = cfg["SelfServer"]["Port"];
        CServer cs(ioc, atoi(port.c_str()));
        ioc.run();
    } catch (std::exception &e) {
        std::cerr << "main exception: " << e.what() << std::endl;
    }
    return 0;
}