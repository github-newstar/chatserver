#include "LogicSystem.hpp"
#include "MysqlMgr.hpp"
#include <json/json.h>
#include "CSession.hpp"
#include "const.h"
#include "StatusGrpcClient.hpp"

LogicSystem::LogicSystem() : b_stop_(false){
    RegisterCallback();
    workThread_ = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem(){
    b_stop_ = true;
    consume_.notify_all();
    workThread_.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg){
    std::unique_lock<std::mutex> lock(mtx_);
    msgQue_.push(msg);
    if(msgQue_.size() == 1){
        lock.unlock();
        consume_.notify_one();
    }
}

void LogicSystem::DealMsg(){
    for(;;){
        std::unique_lock<std::mutex> lock(mtx_);
        consume_.wait(lock, [this]{
            return !msgQue_.empty() || b_stop_;
        });
        if(b_stop_){
            while(!msgQue_.empty()){
                auto msgNode = msgQue_.front();
                msgQue_.pop();
                lock.unlock();
                std::cout << "recv_msg id is" << msgNode->recvNode_->msgId_<< std::endl;
                auto iter = fun_callbacks_.find(msgNode->recvNode_->msgId_);
                if(iter == fun_callbacks_.end()){
                    std::cout << "hasn't resigter this msgId " << msgNode->recvNode_->msgId_ << std::endl;
                    continue;
                }
                iter->second(msgNode->session_, msgNode->recvNode_->msgId_, msgNode->recvNode_->data_);
            }
            break;
        }
        
        //正常处理消息
        auto msgNode = msgQue_.front();
        msgQue_.pop();
        lock.unlock();
        std::cout << "recv_msg id is" << msgNode->recvNode_->msgId_
                  << std::endl;
        auto iter = fun_callbacks_.find(msgNode->recvNode_->msgId_);
        if (iter == fun_callbacks_.end()) {
            std::cout << "hasn't resigter this msgId "
                      << msgNode->recvNode_->msgId_ << std::endl;
            continue;
        }
        iter->second(msgNode->session_, msgNode->recvNode_->msgId_,
                     msgNode->recvNode_->data_);
    }
}
void LogicSystem::RegisterCallback(){
    fun_callbacks_[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}
void LogicSystem::LoginHandler(std::shared_ptr<CSession> session, const short &msg_id, const std::string &msg_data){
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    std::cout << "uid is " << uid << " token is " << root["token"].asString() << std::endl;
    
    //检查从状态服务器获取的token是否正确
    auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
    Json::Value rtValue;
    Defer defer([this, &rtValue, session]{
        std::string return_str = rtValue.toStyledString();
        session->Send(return_str, MSG_CHAT_LOGIN_RSP);
    });
    
    rtValue["error"] = rsp.error();
    if(rsp.error() != ErrorCodes::Success){
        return;
    }
    
    //内存中查询用户信息
    auto find_iter = users_.find(uid);
    std::shared_ptr<UserInfo> user_info = nullptr;
    if(find_iter == users_.end()){
        //从数据库中查询用户信息
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if(user_info == nullptr){
            rtValue["error"] = ErrorCodes::UidInvalid;
            return;
    }
    users_[uid] = user_info;
    }
    else{
        user_info = find_iter->second;
    }
    
    rtValue["uid"] = uid;
    rtValue["name"] = user_info->name;
    rtValue["token"] = rsp.token();
    std::cout << "rtvalue is " << rtValue.toStyledString() << std::endl;

}
