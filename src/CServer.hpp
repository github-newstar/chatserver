#pragma once
#include <boost/asio.hpp>
#include<map>
#include <mutex>
#include <memory>
using namespace std;
using boost::asio::ip::tcp;
namespace net = boost::asio;

class CSession;
class CServer {
  public:
    CServer(net::io_context &ioc, short port);
    ~CServer();
    void ClearSession(std::string session_id);
  private:
    void HandleAccept(std::shared_ptr<CSession> session, const boost::system::error_code &error);
    void StartAccept();

    net::io_context& ioc_;
    short port_;
    tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<CSession>> sessions_;
    std::mutex mtx_;
};