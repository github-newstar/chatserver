#include "CServer.hpp"
#include "AsioIOServicePool.hpp"
#include <iostream>
#include <memory>
#include <mutex>
#include "CSession.hpp"

CServer::CServer(net::io_context &ioc, short port) 
    : ioc_(ioc)
    , port_(port)
    , acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "CServer contstructor" << std::endl;
        StartAccept();
}

CServer::~CServer() {
    std::cout << "CServer destructor" << std::endl;
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code &error) {
    if(!error){
        new_session->Start();
        std::lock_guard<std::mutex> lock(mtx_);
        sessions_[new_session->GetUuid()] = new_session;
    }else{
        std::cerr << "accept error: " << error.message() << std::endl;
    }
    StartAccept();
}

void CServer::StartAccept(){
    auto &ioc = AsioIOServicePool::GetInstance()->GetIOService();
    shared_ptr<CSession> newSession = make_shared<CSession>(ioc, this);
    acceptor_.async_accept(newSession->GetSocket(), 
        std::bind(&CServer::HandleAccept, this, newSession, std::placeholders::_1));
}
void CServer::ClearSession(std::string session_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    sessions_.erase(session_id);
}